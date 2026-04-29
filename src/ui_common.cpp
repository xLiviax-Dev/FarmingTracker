#include "ui_common.h"
#include "shared.h"
#include <sstream>
#include <algorithm>
#include <cstring>

static long long AbsLL(long long x)
{
    return x < 0 ? -x : x;
}

namespace UICommon
{
    // Shared UI state variables
    char s_SearchBuf[256] = {};
    bool s_ShowMainWindow = true;
    char s_AccountNameBuf[128] = "";
    char s_AccountDrfBuf[512] = "";
    char s_AccountGw2Buf[512] = "";
    char s_NewProfileNameBuf[128] = "";
void EnsureItemIconTexture(int itemId, const std::string& url)
{
    if (!APIDefs || url.empty() || !g_Settings.showItemIcons) return;

    char texId[80];
    if (snprintf(texId, sizeof texId, "FTi_%d", itemId) < 0) return;
    if (APIDefs->Textures_Get(texId)) return;

    size_t p = url.find("://");
    if (p == std::string::npos) return;
    p += 3;
    size_t sl = url.find('/', p);
    if (sl == std::string::npos) return;
    std::string host = url.substr(0, sl);
    std::string path = url.substr(sl);
    
    // Use async API with callback
    APIDefs->Textures_LoadFromURL(texId, host.c_str(), path.c_str(), [](const char* aIdentifier, Texture_t* aTexture) {
        // Callback - texture loaded (or failed)
        // Check APIDefs to avoid use-after-free during unload
        if (APIDefs && aTexture)
        {
            APIDefs->Log(LOGL_INFO, "FarmingTracker", ("Icon loaded: " + std::string(aIdentifier)).c_str());
        }
    });
}

void DrawItemIconCell(int itemId, const std::string& url, float sz, const std::string& rarity)
{
    if (!g_Settings.showItemIcons || url.empty())
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

        if (g_Settings.showRarityBorder && !rarity.empty())
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

std::string FormatCoin(long long copper)
{
    bool neg     = copper < 0;
    long long ac = copper < 0 ? -copper : copper;
    int g        = (int)(ac / 10000);
    int s        = (int)((ac % 10000) / 100);
    int c        = (int)(ac % 100);
    std::ostringstream oss;
    if (neg) oss << "-";
    if (g > 0)       oss << g << "g ";
    if (s > 0 || g > 0) oss << s << "s ";
    oss << c << "c";
    return oss.str();
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

std::string FormatDuration(long long seconds)
{
    long long h = seconds / 3600;
    long long m = (seconds % 3600) / 60;
    long long s = seconds % 60;
    std::ostringstream oss;
    if (h > 0) oss << h << "h ";
    if (m > 0 || h > 0) oss << m << "m ";
    oss << s << "s";
    return oss.str();
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
}
