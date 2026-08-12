#include "ui_common.h"
#include "shared.h"
#include "settings.h"
#include <sstream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <set>
#include <mutex>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

static long long AbsLL(long long x)
{
    return x < 0 ? -x : x;
}

// ---------------------------------------------------------------------------
// Icon Disk-Cache
// ---------------------------------------------------------------------------
static std::string s_IconCacheDir; // e.g. ".../addons/FarmingTracker/icon_cache\\"
static std::set<int> s_PendingDownloads;
static std::mutex s_PendingMutex;
static std::set<int> s_VerifiedDiskCache; // Items we know are on disk to avoid GetFileAttributesA calls every frame
static HINTERNET s_HttpSession = NULL;

void UICommon::InitIconCache(const char* addonDir)
{
    if (!addonDir || !addonDir[0]) return;
    s_IconCacheDir = std::string(addonDir) + "\\icon_cache";
    // Create directory if it doesn't exist
    CreateDirectoryA(s_IconCacheDir.c_str(), NULL);

    // Clear stale verified cache in case folder was deleted between sessions
    {
        std::lock_guard<std::mutex> lock(s_PendingMutex);
        s_VerifiedDiskCache.clear();
    }

    // Pre-populate verified disk cache with existing icons
    std::string pattern = s_IconCacheDir + "\\*.png";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        std::lock_guard<std::mutex> lock(s_PendingMutex);
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // Extract itemId from filename: FTi_<itemId>.png
                if (strncmp(fd.cFileName, "FTi_", 4) == 0)
                {
                    int itemId = atoi(fd.cFileName + 4);
                    if (itemId > 0)
                        s_VerifiedDiskCache.insert(itemId);
                }
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }

    if (!s_HttpSession)
    {
        s_HttpSession = WinHttpOpen(L"FarmingTracker-IconCache/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

        if (s_HttpSession)
        {
            DWORD connectMs = 10000, receiveMs = 15000;
            WinHttpSetOption(s_HttpSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &connectMs, sizeof(connectMs));
            WinHttpSetOption(s_HttpSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &receiveMs, sizeof(receiveMs));
        }
    }

    // Pre-fetch icons for frequently used items
    PreFetchFrequentIcons();
}

// Returns the full path for a cached icon file: icon_cache\\FTi_<itemId>.png
static std::string GetCachePath(int itemId)
{
    char filename[64];
    snprintf(filename, sizeof(filename), "\\FTi_%d.png", itemId);
    return s_IconCacheDir + filename;
}

// Returns true if a cached file exists for this itemId
static bool IsCached(int itemId)
{
    if (s_IconCacheDir.empty()) return false;
    std::string path = GetCachePath(itemId);
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

// Pre-fetch icons for frequently used items (coins, common materials, etc.)
void UICommon::PreFetchFrequentIcons()
{
    // Common GW2 items that are frequently used
    static const int frequentItemIds[] = {
        1,    // Coin
        2,    // Karma
        3,    // Laurel
        4,    // Gem
        5,    // Spirit Shard
        6,    // Test Token
        9,    // Badge of Honor
        11,   // Geode
        15,   // Bandit Crest
        16,   // Airship Part
        17,   // Lumps of Mithrillium
        18,   // Thermionic Catlyst
        19,   // Glob of Elder Spirit Residue
        20,   // Pile of Lucent Residue
        21,   // Empyreal Fragment
        22,   // Dragonite Ore
        23,   // Empyreal Star
        24,   // Obsidian Shard
        25,   // Mystic Coin
        26,   // Crystal
        27,   // Fractal Token
        28,   // Magnetite Shard
        29,   // Pristine Fractal Relic
        30,   // Fractal Relic
        31,   // Ancient Fractal Relic
        32,   // Stabilizing Matrix
        33,   // Fractal Encryption
        34,   // Pristine Fractal Encryption
        35,   // Fractal Encryption
        36,   // Gaeting Crystal
        19684, // Volatile Magic
        19709, // Ley-Line Crystal
        19976, // Unbound Magic
        24277, // Bag of Coins
        24300, // Bag of Gems
        46178, // Trade Contract
        68425, // Spirit Shards
        70831, // Eternal Ice
        71662, // Mistborn Mote
        72687, // Winterberry
        72688, // Ancient Bark
        72689, // Orrian Pearls
        72690, // Amber Chunks
        72691, // Freshwater Pearls
        72692, // Giant Eyes
        72693, // Karka Shells
        72694, // Onyx Lobsters
        72695, // Vials of Blood
        72696, // Vials of Powerful Blood
        72697, // Venom Sacs
        72698, // Powerful Venom Sacs
        72699, // Ancient Bones
        72700, // Ancient Fangs
        72701, // Elaborate Totems
        72702, // Heavy Skulls
        72703, // Large Claws
        72704, // Large Scales
        72705, // Vicious Fangs
        72706, // Vicious Claws
        72707, // Armored Scales
        72708, // Piles of Crystalline Dust
        72709, // Piles of Imbued Dust
        72710, // Piles of Radiant Dust
        72711, // Piles of Luminous Dust
        72712, // Piles of Glittering Dust
        72713, // Piles of Shimmering Dust
        72714, // Piles of Sparkling Dust
        72715, // Piles of Dust
        72716, // Bone Shards
        72717, // Bloodstone Dust
        72718, // Demon's Breath
        72719, // Dragon Scales
        72720, // Destroyer Lodestones
        72721, // Dread Shards
        72722, // Eternal Ice Shards
        72723, // Glacial Lodestones
        72724, // Imbued Shards
        72725, // Large Bones
        72726, // Large Fangs
        72727, // Large Scales
        72728, // Large Skulls
        72729, // Large Totems
        72730, // Molten Lodestones
        72731, // Obsidian Shards
        72732, // Onyx Shards
        72733, // Piles of Ash
        72734, // Piles of Bone
        72735, // Piles of Blood
        72736, // Piles of Crystal
        72737, // Piles of Dust
        72738, // Piles of Fang
        72739, // Piles of Scales
        72740, // Piles of Shards
        72741, // Piles of Skull
        72742, // Piles of Totem
        72743, // Piles of Venom
        72744, // Pristine Bloodstone Dust
        72745, // Pristine Demon's Breath
        72746, // Pristine Dragon Scales
        72747, // Pristine Dread Shards
        72748, // Pristine Eternal Ice Shards
        72749, // Pristine Glacial Lodestones
        72750, // Pristine Imbued Shards
        72751, // Pristine Molten Lodestones
        72752, // Pristine Onyx Shards
        72753, // Pristine Relic Fragments
        72754, // Pristine Vile Blood
        72755, // Relic Fragments
        72756, // Vile Blood
        83143, // Mistborn Motes
        83554, // Volatile Magic
        93968, // Ley-Line Crystals
        94020, // Unbound Magic
    };

    for (int itemId : frequentItemIds)
    {
        // NOTE: We intentionally do NOT add the itemId to s_PendingDownloads here,
        // because PreFetch has no icon URL and cannot start the download thread.
        // Marking it as "pending" without actually starting the thread would
        // permanently block the real EnsureItemIconTexture() call later (it
        // returns early when s_PendingDownloads.count(itemId) is true),
        // leaving the icon blank forever without any error message.
        // Instead we only populate the verified disk cache if the file is
        // already cached, which saves a future GetFileAttributesA call.
        if (IsCached(itemId))
        {
            std::lock_guard<std::mutex> lock(s_PendingMutex);
            s_VerifiedDiskCache.insert(itemId);
        }
    }
}

// Download raw PNG bytes from a GW2 render URL and write to disk
static bool DownloadIconToDisk(const std::string& url, const std::string& destPath)
{
    // Parse URL into host and path
    size_t p = url.find("://");
    if (p == std::string::npos) return false;
    p += 3;
    size_t sl = url.find('/', p);
    if (sl == std::string::npos) return false;
    std::string host = url.substr(p, sl - p);
    std::string path = url.substr(sl);

    auto Utf8ToWide = [](const std::string& s) -> std::wstring
    {
        if (s.empty()) return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring w(n, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
        return w;
    };

    std::wstring wHost = Utf8ToWide(host);
    std::wstring wPath = Utf8ToWide(path);

    if (!s_HttpSession) return false;

    HINTERNET hConnect = WinHttpConnect(s_HttpSession, wHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) return false;

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); return false; }

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!ok || !WinHttpReceiveResponse(hRequest, nullptr))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect);
        return false;
    }

    DWORD status = 0;
    DWORD sz = sizeof(status);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
    if (status != 200)
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect);
        return false;
    }

    std::vector<char> buf;
    DWORD dwSize = 0;
    do
    {
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;
        size_t offset = buf.size();
        buf.resize(offset + dwSize);
        DWORD downloaded = 0;
        if (!WinHttpReadData(hRequest, buf.data() + offset, dwSize, &downloaded)) break;
        buf.resize(offset + downloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);

    if (buf.empty()) return false;

    // Write to disk (temp name first, then rename for atomicity)
    std::string tmpPath = destPath + ".tmp";
    FILE* f = nullptr;
    if (fopen_s(&f, tmpPath.c_str(), "wb") != 0 || !f) return false;
    fwrite(buf.data(), 1, buf.size(), f);
    fclose(f);

    // Rename tmp -> final
    DeleteFileA(destPath.c_str());
    if (!MoveFileA(tmpPath.c_str(), destPath.c_str()))
    {
        DeleteFileA(tmpPath.c_str());
        return false;
    }
    return true;
}

void UICommon::EnforceIconCacheLimit()
{
    if (s_IconCacheDir.empty()) return;
    // Cache APIDefs at function entry — this may be called from a detached thread
    // where APIDefs could be cleared by AddonUnload at any moment.
    auto* apiDefs = APIDefs;
    int maxIcons;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        maxIcons = g_Settings.iconCacheMaxIcons;
    }
    if (maxIcons <= 0) return;

    // Enumerate all .png files in cache dir with their last-write time
    std::string pattern = s_IconCacheDir + "\\*.png";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    using FileEntry = std::pair<FILETIME, std::string>; // (lastWrite, fullPath)
    std::vector<FileEntry> files;
    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        std::string fullPath = s_IconCacheDir + "\\" + fd.cFileName;
        files.push_back({ fd.ftLastWriteTime, fullPath });
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);

    int count = static_cast<int>(files.size());
    if (count <= maxIcons) return;

    // Sort oldest-first (smallest FILETIME first)
    std::sort(files.begin(), files.end(), [](const FileEntry& a, const FileEntry& b)
    {
        if (a.first.dwHighDateTime != b.first.dwHighDateTime)
            return a.first.dwHighDateTime < b.first.dwHighDateTime;
        return a.first.dwLowDateTime < b.first.dwLowDateTime;
    });

    // Delete oldest files until we're at the limit
    int toDelete = count - maxIcons;
    for (int i = 0; i < toDelete; i++)
    {
        DeleteFileA(files[i].second.c_str());
        if (apiDefs)
            apiDefs->Log(LOGL_INFO, "FarmingTracker", ("Icon cache: evicted " + files[i].second).c_str());
    }
}

namespace UICommon
{
    // Shared UI state variables
    char s_SearchBuf[256] = {};
    char s_ItemsSearchBuf[256] = {};
    char s_CurrenciesSearchBuf[256] = {};
    bool s_ShowMainWindow = true;
    char s_AccountNameBuf[128] = "";
    char s_AccountDrfBuf[512] = "";
    char s_AccountGw2Buf[512] = "";
    char s_NewProfileNameBuf[128] = "";

    // Thread safety
    std::mutex s_AccountNameMutex;

    void EnsureItemIconTexture(int itemId, const std::string& url)
    {
        if (!APIDefs || url.empty() || !g_Settings.showItemIcons) return;

        char texId[80];
        if (snprintf(texId, sizeof texId, "FTi_%d", itemId) < 0) return;

        // Already loaded into GPU — nothing to do
        if (APIDefs->Textures_Get(texId)) return;

        if (g_Settings.enableIconCache && !s_IconCacheDir.empty())
        {
            // Check if we already verified this item is on disk during this session
            bool isOnDisk = false;
            {
                std::lock_guard<std::mutex> lock(s_PendingMutex);
                if (s_VerifiedDiskCache.count(itemId)) {
                    isOnDisk = true;
                }
            }

            // If not verified yet, check disk (only once per session)
            if (!isOnDisk && IsCached(itemId)) {
                isOnDisk = true;
                std::string cachePathForTouch = GetCachePath(itemId);
                // Touch the file ONCE PER SESSION (not per frame) so EnforceIconCacheLimit
                // still knows approximate recency, without issuing Win32 file syscalls 60x/s.
                HANDLE hFile = CreateFileA(cachePathForTouch.c_str(), FILE_WRITE_ATTRIBUTES,
                    FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    FILETIME ft;
                    GetSystemTimeAsFileTime(&ft);
                    SetFileTime(hFile, nullptr, nullptr, &ft);
                    CloseHandle(hFile);
                }
                std::lock_guard<std::mutex> lock(s_PendingMutex);
                s_VerifiedDiskCache.insert(itemId);
            }

            std::string cachePath = GetCachePath(itemId);

            if (isOnDisk)
            {
                // (Touch removed from the hot render-path: now happens once-per-session above,
                //  or implicitly when a file is freshly downloaded.)
                // Load from disk — Nexus handles the GPU upload
                if (!APIDefs->Textures_GetOrCreateFromFile(texId, cachePath.c_str()))
                {
                    // File missing (e.g. user deleted cache folder) — evict from verified cache
                    // and recreate the folder so future downloads work again
                    {
                        std::lock_guard<std::mutex> lock(s_PendingMutex);
                        s_VerifiedDiskCache.erase(itemId);
                    }
                    CreateDirectoryA(s_IconCacheDir.c_str(), NULL);
                    APIDefs->Log(LOGL_WARNING, "FarmingTracker",
                        ("Failed to load icon from disk: " + cachePath + ", re-downloading").c_str());
                    size_t p = url.find("://");
                    if (p != std::string::npos)
                    {
                        p += 3;
                        size_t sl = url.find('/', p);
                        if (sl != std::string::npos)
                        {
                            std::string host = url.substr(p, sl - p);
                            std::string path = url.substr(sl);
                            APIDefs->Textures_LoadFromURL(texId, host.c_str(), path.c_str(), nullptr);
                        }
                    }
                }
            }
            else
            {
                // Not cached yet: download in background thread, save to disk, then load
                {
                    std::lock_guard<std::mutex> lock(s_PendingMutex);
                    if (s_PendingDownloads.count(itemId)) return; // Already downloading
                    s_PendingDownloads.insert(itemId);
                }

                struct DownloadCtx
                {
                    int         itemId;
                    std::string url;
                    std::string cachePath;
                    char        texId[80];
                };
                auto* ctx = new DownloadCtx{ itemId, url, cachePath, {} };
                snprintf(ctx->texId, sizeof(ctx->texId), "%s", texId);

                // Fire-and-forget thread: download PNG, write to disk, then ask Nexus to load it
                std::thread([ctx]()
                {
                    // Cache APIDefs pointer at thread start to avoid use-after-free on AddonUnload
                    auto* apiDefs = APIDefs;
                    bool ok = DownloadIconToDisk(ctx->url, ctx->cachePath);
                    if (ok)
                    {
                        if (apiDefs)
                        {
                            apiDefs->Textures_GetOrCreateFromFile(ctx->texId, ctx->cachePath.c_str());
                            
                            // Add to verified cache so we don't check disk again
                            std::lock_guard<std::mutex> lock(s_PendingMutex);
                            s_VerifiedDiskCache.insert(ctx->itemId);
                            
                            UICommon::EnforceIconCacheLimit();
                        }
                    }
                    else if (!ok)
                    {
                        // Download failed — fall back to Nexus URL loader so the icon still shows
                        if (apiDefs)
                        {
                            size_t p = ctx->url.find("://");
                            if (p != std::string::npos)
                            {
                                p += 3;
                                size_t sl = ctx->url.find('/', p);
                                if (sl != std::string::npos)
                                {
                                    std::string host = ctx->url.substr(p, sl - p);
                                    std::string path = ctx->url.substr(sl);
                                    apiDefs->Textures_LoadFromURL(ctx->texId, host.c_str(), path.c_str(), nullptr);
                                }
                            }
                        }
                    }

                    // Remove from pending list
                    {
                        std::lock_guard<std::mutex> lock(s_PendingMutex);
                        s_PendingDownloads.erase(ctx->itemId);
                    }
                    delete ctx;
                }).detach();
            }
        }
        else
        {
            // --- Cache disabled: original behaviour, load directly from URL ---
            size_t p = url.find("://");
            if (p == std::string::npos) return;
            p += 3;
            size_t sl = url.find('/', p);
            if (sl == std::string::npos) return;
            std::string host = url.substr(p, sl - p);
            std::string path = url.substr(sl);
            APIDefs->Textures_LoadFromURL(texId, host.c_str(), path.c_str(),
                [](const char* aIdentifier, Texture_t* aTexture)
                {
                    if (APIDefs && aTexture)
                        APIDefs->Log(LOGL_INFO, "FarmingTracker",
                            ("Icon loaded: " + std::string(aIdentifier)).c_str());
                });
        }
    }

    void DrawItemIconCell(int itemId, const std::string& url, float sz, const std::string& rarity, bool forceShow)
    {
        if ((!g_Settings.showItemIcons && !forceShow) || url.empty())
        {
            ImGui::Dummy(ImVec2(sz, sz));
            return;
        }

        // Ensure texture is loaded
        EnsureItemIconTexture(itemId, url);
        char texId[80];
        if (snprintf(texId, sizeof texId, "FTi_%d", itemId) < 0) {
            ImGui::Dummy(ImVec2(sz, sz));
            return;
        }
        Texture_t* tex = APIDefs ? APIDefs->Textures_Get(texId) : nullptr;

        if (tex && tex->Resource)
        {
            // Icon loaded successfully
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImGui::Image((ImTextureID)tex->Resource, ImVec2(sz, sz));

            if (g_Settings.showRarityBorder && !rarity.empty() && !g_Settings.disableComplexVisualsOnLowPerf)
            {
                // Rarity color for border
                ImVec4 borderColor = ImVec4(1.f, 1.f, 1.f, 1.f);
                if (rarity == "Junk") borderColor = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                else if (rarity == "Basic") borderColor = ImVec4(1.f, 1.f, 1.f, 1.f);
                else if (rarity == "Fine") borderColor = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                else if (rarity == "Masterwork") borderColor = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                else if (rarity == "Rare") borderColor = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                else if (rarity == "Exotic") borderColor = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                else if (rarity == "Ascended") borderColor = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                else if (rarity == "Legendary") borderColor = ImVec4(1.0f, 0.5f, 0.8f, 1.f);

                // Draw border using window draw list
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddRect(cursorPos, ImVec2(cursorPos.x + sz, cursorPos.y + sz),
                    ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, g_Settings.rarityBorderSize);
            }
        }
        else
        {
            // Icon not loaded yet - show placeholder
            ImGui::Dummy(ImVec2(sz, sz));
        }
    }

    const char* StatusText(DrfStatus s)
    {
        switch (s)
        {
            case DrfStatus::Disconnected: return Localization::GetText("status_disconnected");
            case DrfStatus::Connecting:   return Localization::GetText("status_connecting");
            case DrfStatus::Connected:    return Localization::GetText("status_connected");
            case DrfStatus::AuthFailed:   return Localization::GetText("status_auth_failed");
            case DrfStatus::Reconnecting: return Localization::GetText("status_reconnecting");
            case DrfStatus::Error:        return Localization::GetText("status_error");
            default:                      return Localization::GetText("status_unknown");
        }
    }

    ImVec4 StatusColor(DrfStatus s)
    {
        switch (s)
        {
            case DrfStatus::Connected:    return ImVec4(0.2f, 0.9f, 0.2f, 1.f);
            case DrfStatus::AuthFailed:
            case DrfStatus::Error:        return ImVec4(0.9f, 0.2f, 0.2f, 1.f);
            case DrfStatus::Reconnecting:
            case DrfStatus::Connecting:   return ImVec4(0.9f, 0.8f, 0.1f, 1.f);
            default:                      return ImVec4(0.6f, 0.6f, 0.6f, 1.f);
        }
    }

    ImVec4 ValueColor(long long value)
    {
        if (value > 0) return ImVec4(1.f, 0.84f, 0.f, 1.f);
        if (value < 0) return ImVec4(0.9f, 0.2f, 0.2f, 1.f);
        return ImVec4(0.7f, 0.7f, 0.7f, 1.f);
    }

    const char* FormatCoin(long long copper)
    {
        static thread_local char buf[64];
        bool neg     = copper < 0;
        long long ac = copper < 0 ? -copper : copper;

        int g = (int)(ac / 10000);
        int s = (int)((ac % 10000) / 100);
        int c = (int)(ac % 100);

        size_t written = 0;
        if (neg)
        {
            buf[written++] = '-';
            buf[written] = '\0';
        }

        // Match original ostringstream format exactly:
        //   if (g > 0)       oss << g << "g ";
        //   if (s > 0 || g > 0) oss << s << "s ";
        //   oss << c << "c";
        if (g > 0)
            written += (size_t)snprintf(buf + written, sizeof(buf) - written, "%dg ", g);
        if (s > 0 || g > 0)
            written += (size_t)snprintf(buf + written, sizeof(buf) - written, "%ds ", s);
        snprintf(buf + written, sizeof(buf) - written, "%dc", c);
        return buf;
    }

    const char* FormatCompact(long long value)
    {
        static thread_local char buf[32];
        long long av = value < 0 ? -value : value;
        const char* sign = value < 0 ? "-" : "";
        if (av >= 1000000LL)
        {
            double d = (double)av / 1000000.0;
            if (d == (long long)d)
                snprintf(buf, sizeof(buf), "%s%lldM", sign, (long long)d);
            else
                snprintf(buf, sizeof(buf), "%s%.1fM", sign, d);
        }
        else if (av >= 1000LL)
        {
            double d = (double)av / 1000.0;
            if (d == (long long)d)
                snprintf(buf, sizeof(buf), "%s%lldK", sign, (long long)d);
            else
                snprintf(buf, sizeof(buf), "%s%.1fK", sign, d);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%s%lld", sign, av);
        }
        return buf;
    }

    void DrawCoinDisplay(long long copper)
    {
        bool neg     = copper < 0;
        long long ac = copper < 0 ? -copper : copper;
        int g        = (int)(ac / 10000);
        int s        = (int)((ac % 10000) / 100);
        int c        = (int)(ac % 100);
        
        if (neg) ImGui::Text("-");
        else if (copper > 0) ImGui::Text("+");
        ImGui::SameLine(0, 0);
        
        if (g > 0)
        {
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%d", g);
            ImGui::SameLine(0, 2);
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "g");
            ImGui::SameLine(0, 4);
        }
        
        if (s > 0 || g > 0)
        {
            ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.f, 1.f), "%d", s);
            ImGui::SameLine(0, 2);
            ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.f, 1.f), "s");
            ImGui::SameLine(0, 4);
        }
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%d", c);
        ImGui::SameLine(0, 2);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "c");
    }

    const char* FormatDuration(long long seconds)
    {
        static thread_local char buf[64];
        long long h = seconds / 3600;
        long long m = (seconds % 3600) / 60;
        long long s = seconds % 60;

        size_t written = 0;
        // Match original ostringstream format exactly:
        //   if (h > 0) oss << h << "h ";
        //   if (m > 0 || h > 0) oss << m << "m ";
        //   oss << s << "s";
        if (h > 0)
            written += (size_t)snprintf(buf + written, sizeof(buf) - written, "%lldh ", h);
        if (m > 0 || h > 0)
            written += (size_t)snprintf(buf + written, sizeof(buf) - written, "%lldm ", m);
        snprintf(buf + written, sizeof(buf) - written, "%llds", s);
        return buf;
    }

    void TextWithTooltip(const char* text, float maxWidth, const ImVec4& color)
    {
        ImVec2 textSize = ImGui::CalcTextSize(text);
        if (textSize.x > maxWidth)
        {
            ImGui::TextColored(color, "%s", text);
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(color, "%s", text);
                ImGui::EndTooltip();
            }
        }
        else
        {
            ImGui::TextColored(color, "%s", text);
        }
    }

    bool PassesRarityFilter(const Stat& st)
    {
        if (g_Settings.itemRarityFilterMin <= 0) return true;
        if (!st.details.loaded) return true;
        return ItemTracker::RarityRank(st.details.rarity) >= g_Settings.itemRarityFilterMin;
    }

    void SortVisible(std::vector<std::pair<int, Stat>>& v)
    {
        int mode = g_Settings.itemSortMode;
        std::sort(v.begin(), v.end(), [mode](const auto& a, const auto& b) {
            const Stat& sa = a.second;
            const Stat& sb = b.second;
            switch (mode)
            {
            default:
            case 0: return AbsLL(sa.count) > AbsLL(sb.count);
            case 1: return AbsLL(sa.count) < AbsLL(sb.count);
            case 2: return a.first < b.first;
            case 3: return a.first > b.first;
            case 4:
            {
                std::string na = sa.details.loaded ? sa.details.name : "";
                std::string nb = sb.details.loaded ? sb.details.name : "";
                if (na.empty() && nb.empty()) return a.first < b.first;
                if (na.empty()) return false;
                if (nb.empty()) return true;
                if (na != nb) return na < nb;
                return a.first < b.first;
            }
            }
        });
    }

    float CalcTableRowHeight(float contentHeight)
    {
        const float padY = ImGui::GetStyle().CellPadding.y * 2.0f;
        const float frameH = ImGui::GetFrameHeight();
        const float h = (contentHeight > frameH ? contentHeight : frameH);
        return h + padY;
    }

    void AlignTableCell(float rowHeight, float itemHeight)
    {
        const float padY = ImGui::GetStyle().CellPadding.y * 2.0f;
        const float innerH = rowHeight - padY;
        const float offset = (innerH - itemHeight) * 0.5f;
        if (offset > 0.0f)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
    }

    void AlignTableCellText(float rowHeight)
    {
        // Horizontales Padding: Schrift nicht direkt am Zellrand (User-Wunsch: +10px nach rechts)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
        AlignTableCell(rowHeight, ImGui::GetTextLineHeight());
    }

    void AlignTableCellFrame(float rowHeight)
    {
        AlignTableCell(rowHeight, ImGui::GetFrameHeight());
    }

    void AlignTableCellIcon(float rowHeight, float iconSize)
    {
        AlignTableCell(rowHeight, iconSize);
    }

    bool ShouldShowTooltip()
    {
        bool enabled;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            enabled = g_Settings.enableTooltips;
        }
        return enabled;
    }

    void SetTooltipIfEnabled(const char* tooltip)
    {
        if (ShouldShowTooltip() && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip);
    }

    bool RedGradientButton(const char* label, const char* id, bool hovered_override)
    {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 textSize = ImGui::CalcTextSize(label);
        float padX = 8.0f;
        float padY = 4.0f;
        ImVec2 btnSize = ImVec2(textSize.x + padX * 2.0f, textSize.y + padY * 2.0f);

        ImGui::SetCursorScreenPos(cursor);
        ImGui::InvisibleButton(id, btnSize);
        bool hovered = hovered_override ? true : ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 btnMin = cursor;
        ImVec2 btnMax = ImVec2(cursor.x + btnSize.x, cursor.y + btnSize.y);

        // Red gradient colors (matching HTML plugin button design)
        ImU32 colRedTop    = IM_COL32(192, 48,  48,  255); // #c03030
        ImU32 colRedBot    = IM_COL32(80,  10,  10,  255); // #500a0a
        ImU32 colRedTopHov = IM_COL32(220, 70,  70,  255); // brighter on hover
        ImU32 colRedBotHov = IM_COL32(110, 20,  20,  255); // brighter on hover
        ImU32 colRedBorder = IM_COL32(122, 26,  26,  255); // #7a1a1a
        ImU32 colRedBar    = IM_COL32(192, 48,  48,  255); // #c03030

        // Draw gradient background
        dl->AddRectFilledMultiColor(btnMin, btnMax,
            hovered ? colRedTopHov : colRedTop, hovered ? colRedTopHov : colRedTop,
            hovered ? colRedBotHov : colRedBot, hovered ? colRedBotHov : colRedBot);

        // Draw border
        dl->AddRect(btnMin, btnMax, colRedBorder, 4.0f, 0, 0.5f);

        // Draw left accent bar
        dl->AddRectFilled(ImVec2(btnMin.x, btnMin.y), ImVec2(btnMin.x + 3.0f, btnMax.y), colRedBar, 2.0f);

        // Draw text
        ImVec2 textPos = ImVec2(btnMin.x + padX + 3.0f, btnMin.y + padY);
        dl->AddText(textPos, IM_COL32(255, 220, 220, 255), label);

        return clicked;
    }

    bool GreenGradientButton(const char* label, const char* id, bool hovered_override)
    {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 textSize = ImGui::CalcTextSize(label);
        float padX = 8.0f;
        float padY = 4.0f;
        ImVec2 btnSize = ImVec2(textSize.x + padX * 2.0f, textSize.y + padY * 2.0f);

        ImGui::SetCursorScreenPos(cursor);
        ImGui::InvisibleButton(id, btnSize);
        bool hovered = hovered_override ? true : ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 btnMin = cursor;
        ImVec2 btnMax = ImVec2(cursor.x + btnSize.x, cursor.y + btnSize.y);

        // Green gradient colors (matching HTML plugin button design)
        ImU32 colGreenTop    = IM_COL32(42,  154, 42,  255); // #2a9a2a
        ImU32 colGreenBot    = IM_COL32(10,  58,  10,  255); // #0a3a0a
        ImU32 colGreenTopHov = IM_COL32(60,  180, 60,  255); // brighter on hover
        ImU32 colGreenBotHov = IM_COL32(20,  80,  20,  255); // brighter on hover
        ImU32 colGreenBorder = IM_COL32(26,  107, 26,  255); // #1a6b1a
        ImU32 colGreenBar    = IM_COL32(42,  154, 42,  255); // #2a9a2a

        // Draw gradient background
        dl->AddRectFilledMultiColor(btnMin, btnMax,
            hovered ? colGreenTopHov : colGreenTop, hovered ? colGreenTopHov : colGreenTop,
            hovered ? colGreenBotHov : colGreenBot, hovered ? colGreenBotHov : colGreenBot);

        // Draw border
        dl->AddRect(btnMin, btnMax, colGreenBorder, 4.0f, 0, 0.5f);

        // Draw left accent bar
        dl->AddRectFilled(ImVec2(btnMin.x, btnMin.y), ImVec2(btnMin.x + 3.0f, btnMax.y), colGreenBar, 2.0f);

        // Draw text
        ImVec2 textPos = ImVec2(btnMin.x + padX + 3.0f, btnMin.y + padY);
        dl->AddText(textPos, IM_COL32(221, 221, 221, 255), label); // #dfd

        return clicked;
    }

    bool OrangeGradientButton(const char* label, const char* id, bool hovered_override)
    {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 textSize = ImGui::CalcTextSize(label);
        float padX = 8.0f;
        float padY = 4.0f;
        ImVec2 btnSize = ImVec2(textSize.x + padX * 2.0f, textSize.y + padY * 2.0f);

        ImGui::SetCursorScreenPos(cursor);
        ImGui::InvisibleButton(id, btnSize);
        bool hovered = hovered_override ? true : ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 btnMin = cursor;
        ImVec2 btnMax = ImVec2(cursor.x + btnSize.x, cursor.y + btnSize.y);

        // Orange gradient colors (matching HTML plugin button design)
        ImU32 colOrangeTop    = IM_COL32(192, 112, 64,  255); // #C07040
        ImU32 colOrangeMid    = IM_COL32(107, 51,  24,  255); // #6B3318
        ImU32 colOrangeBot    = IM_COL32(46,  21,  8,   255); // #2E1508
        ImU32 colOrangeTopHov = IM_COL32(220, 140, 90,  255); // brighter on hover
        ImU32 colOrangeMidHov = IM_COL32(130, 70,  40,  255); // brighter on hover
        ImU32 colOrangeBotHov = IM_COL32(60,  30,  15,  255); // brighter on hover
        ImU32 colOrangeBorder = IM_COL32(139, 68,  34,  255); // #8B4422
        ImU32 colOrangeBar    = IM_COL32(94,  51,  27,  255); // #5e331b

        // Draw gradient background
        dl->AddRectFilledMultiColor(btnMin, btnMax,
            hovered ? colOrangeTopHov : colOrangeTop, hovered ? colOrangeTopHov : colOrangeTop,
            hovered ? colOrangeBotHov : colOrangeBot, hovered ? colOrangeBotHov : colOrangeBot);

        // Draw border
        dl->AddRect(btnMin, btnMax, colOrangeBorder, 4.0f, 0, 0.5f);

        // Draw left accent bar
        dl->AddRectFilled(ImVec2(btnMin.x, btnMin.y), ImVec2(btnMin.x + 3.0f, btnMax.y), colOrangeBar, 2.0f);

        // Draw text
        ImVec2 textPos = ImVec2(btnMin.x + padX + 3.0f, btnMin.y + padY);
        dl->AddText(textPos, IM_COL32(255, 255, 255, 255), label); // #ffffff

        return clicked;
    }

    void DrawTextWithOutline(ImDrawList* dl, ImFont* font, float font_size, ImVec2 pos, ImU32 text_color,
                              ImU32 outline_color1, float outline_width1,
                              ImU32 outline_color2, float outline_width2,
                              const char* text)
    {
        // Helper function to generate optimized outline offsets
        auto GetOptimizedOffsets = [](float width) -> std::vector<ImVec2>
        {
            std::vector<ImVec2> offsets;
            if (width <= 0.0f) return offsets;

            // Use only cardinal directions (no diagonals) for efficiency, covers most of the outline
            for (float w = 1.0f; w <= width; w += 1.0f)
            {
                offsets.emplace_back(-w, 0.0f);
                offsets.emplace_back(w, 0.0f);
                offsets.emplace_back(0.0f, -w);
                offsets.emplace_back(0.0f, w);
            }
            return offsets;
        };

        // Draw first outline (if enabled)
        if (outline_width1 > 0.0f)
        {
            auto offsets = GetOptimizedOffsets(outline_width1);
            for (const auto& off : offsets)
                dl->AddText(font, font_size, ImVec2(pos.x + off.x, pos.y + off.y), outline_color1, text);
        }

        // Draw second outline (if enabled)
        if (outline_width2 > 0.0f)
        {
            auto offsets = GetOptimizedOffsets(outline_width2);
            for (const auto& off : offsets)
                dl->AddText(font, font_size, ImVec2(pos.x + off.x, pos.y + off.y), outline_color2, text);
        }

        // Draw actual text last on top
        dl->AddText(font, font_size, pos, text_color, text);
    }

    void TextColoredWithOutline(const ImVec4& text_color, const ImVec4& outline_color, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        ImVec2 cursor = ImGui::GetCursorPos();

        // Optimized 4-way outline instead of 8-way
        ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
        ImGui::TextColored(outline_color, "%s", buf);
        ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
        ImGui::TextColored(outline_color, "%s", buf);
        ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
        ImGui::TextColored(outline_color, "%s", buf);
        ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
        ImGui::TextColored(outline_color, "%s", buf);

        // Actual text
        ImGui::SetCursorPos(cursor);
        ImGui::TextColored(text_color, "%s", buf);
    }

    void TextColoredWithSimpleOutline(const ImVec4& text_color, const ImVec4& outline_color, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        ImVec2 cursor = ImGui::GetCursorPos();

        // Simple 2x outline: 1x outline + 1x text (maximum performance)
        ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
        ImGui::TextColored(outline_color, "%s", buf);

        ImGui::SetCursorPos(cursor);
        ImGui::TextColored(text_color, "%s", buf);
    }
}
