#include "loot_logger.h"
#include "settings.h"
#include "gw2_api.h"
#include "item_tracker.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

namespace
{
    // -----------------------------------------------------------------------
    // Internal state
    // -----------------------------------------------------------------------

    std::mutex              s_Mutex;
    std::mutex              s_QueueMutex;
    std::condition_variable s_QueueCv;
    std::thread             s_WriteThread;
    std::atomic<bool>       s_Shutdown{ false };

    // Write queue — main/DRF thread pushes, write thread pops
    struct QueueEntry
    {
        std::string csvLine;    // ready-to-write CSV row (empty if JSON only)
        std::string jsonObject; // ready-to-write JSON object (empty if CSV only)
    };
    std::deque<QueueEntry> s_Queue;

    // Current session state
    std::string              s_LogFolder;
    std::string              s_CurrentSessionBase; // path without extension
    int                      s_SessionIndex = 0;
    std::vector<LootLogger::DropEntry> s_CurrentSessionEntries;

    // Map name cache: map_id -> name
    std::unordered_map<int, std::string> s_MapCache;
    std::mutex                           s_MapMutex;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    std::string UtcNowIso()
    {
        auto now  = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }

    // Returns "YYYY-MM-DD" for today in UTC
    std::string UtcTodayDate()
    {
        auto now  = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d");
        return ss.str();
    }

    // Returns "HH-MM-SS" for now in UTC
    std::string UtcNowTime()
    {
        auto now  = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%H-%M-%S");
        return ss.str();
    }

    // Escapes a string for CSV (wraps in quotes if it contains comma/quote/newline)
    std::string CsvEscape(const std::string& s)
    {
        if (s.find_first_of(",\"\n\r") == std::string::npos)
            return s;
        std::string out = "\"";
        for (char c : s)
        {
            if (c == '"') out += "\"\"";
            else          out += c;
        }
        out += '"';
        return out;
    }

    // Escapes a string for JSON
    std::string JsonEscape(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 4);
        for (unsigned char c : s)
        {
            if      (c == '"')  out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else if (c < 0x20)  { /* skip control chars */ }
            else                out += c;
        }
        return out;
    }

    // Builds a CSV header line
    std::string CsvHeader(bool includeMap, bool includeMagicFind)
    {
        std::string h = "timestamp,item_id,item_name,quantity,item_type,rarity,sell_price_tp,vendor_price";
        if (includeMagicFind) h += ",magic_find";
        if (includeMap)   h += ",map_id,map_name";
        return h + "\n";
    }

    // Builds a CSV row from a DropEntry
    std::string ToCsvRow(const LootLogger::DropEntry& e,
                          bool includeMap, bool includeMagicFind)
    {
        std::ostringstream ss;
        ss << CsvEscape(e.timestampUtc) << ","
           << e.itemId << ","
           << CsvEscape(e.itemName) << ","
           << e.quantity << ","
           << CsvEscape(e.itemType) << ","
           << CsvEscape(e.rarity) << ","
           << e.sellPriceTp << ","
           << e.vendorPrice;

        if (includeMagicFind)
            ss << "," << e.magicFind;

        if (includeMap)
            ss << "," << e.mapId << "," << CsvEscape(e.mapName);

        ss << "\n";
        return ss.str();
    }

    // Builds a JSON object from a DropEntry (single object, no array wrapper)
    std::string ToJsonObject(const LootLogger::DropEntry& e,
                              bool includeMap, bool includeMagicFind)
    {
        std::ostringstream ss;
        ss << "{\n"
           << "  \"timestamp\": \""  << JsonEscape(e.timestampUtc) << "\",\n"
           << "  \"item_id\": "      << e.itemId   << ",\n"
           << "  \"item_name\": \""  << JsonEscape(e.itemName) << "\",\n"
           << "  \"quantity\": "     << e.quantity << ",\n"
           << "  \"item_type\": \""  << JsonEscape(e.itemType) << "\",\n"
           << "  \"rarity\": \""     << JsonEscape(e.rarity) << "\",\n"
           << "  \"sell_price_tp\": "<< e.sellPriceTp << ",\n"
           << "  \"vendor_price\": "<< e.vendorPrice;

        if (includeMagicFind)
            ss << ",\n  \"magic_find\": " << e.magicFind;

        if (includeMap)
        {
            ss << ",\n  \"map_id\": "    << e.mapId
               << ",\n  \"map_name\": \"" << JsonEscape(e.mapName) << "\"";
        }

        ss << "\n}";
        return ss.str();
    }

    // Appends text to a file, creating it (and parent dirs) if necessary
    void AppendToFile(const std::string& path, const std::string& text)
    {
        try
        {
            fs::create_directories(fs::path(path).parent_path());
            std::ofstream f(path, std::ios::app);
            if (f.is_open()) f << text;
        }
        catch (...) {}
    }

    // Deletes log sub-directories older than maxDays
    void RotateOldLogs(const std::string& folder, int maxDays)
    {
        if (maxDays <= 0 || folder.empty()) return;
        try
        {
            auto cutoff = std::chrono::system_clock::now()
                        - std::chrono::hours(24 * maxDays);

            for (const auto& entry : fs::directory_iterator(folder))
            {
                if (!entry.is_directory()) continue;
                // Folder names are "YYYY-MM-DD" — parse them
                std::string name = entry.path().filename().string();
                if (name.size() != 10) continue;

                std::tm tm{};
                std::istringstream ss(name);
                ss >> std::get_time(&tm, "%Y-%m-%d");
                if (ss.fail()) continue;

#ifdef _WIN32
                auto folderTime = std::chrono::system_clock::from_time_t(_mkgmtime(&tm));
#else
                auto folderTime = std::chrono::system_clock::from_time_t(timegm(&tm));
#endif
                if (folderTime < cutoff)
                {
                    fs::remove_all(entry.path());
                    Gw2Api::Log("LootLogger: removed old log folder: " + name, "info");
                }
            }
        }
        catch (...) {}
    }

    // The write-thread loop: drains the queue to disk
    void WriteThreadLoop()
    {
        while (true)
        {
            std::deque<QueueEntry> batch;
            {
                std::unique_lock<std::mutex> lock(s_QueueMutex);
                s_QueueCv.wait(lock, [] {
                    return s_Shutdown.load() || !s_Queue.empty();
                });
                std::swap(batch, s_Queue);
            }

            // Read current settings (format, map/buff flags) once per batch
            int         format;
            bool        includeMap;
            std::string basePath;
            {
                std::lock_guard<std::mutex> lock(s_Mutex);
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                format       = g_Settings.lootLogFormat;
                includeMap   = g_Settings.lootLogIncludeMap;
                basePath     = s_CurrentSessionBase;
            }

            if (basePath.empty())
            {
                if (s_Shutdown.load() && batch.empty()) break;
                continue;
            }

            for (const auto& entry : batch)
            {
                if ((format == 0 || format == 2) && !entry.csvLine.empty())
                    AppendToFile(basePath + ".csv", entry.csvLine);
                if ((format == 1 || format == 2) && !entry.jsonObject.empty())
                    AppendToFile(basePath + ".json", entry.jsonObject + ",\n");
            }

            if (s_Shutdown.load() && batch.empty()) break;
        }
    }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
namespace LootLogger
{

void Init(const std::string& addonDir)
{
    // Resolve log folder
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        if (g_Settings.lootLogFolder.empty() && !addonDir.empty())
            s_LogFolder = addonDir + "\\loot-logs";
        else
            s_LogFolder = g_Settings.lootLogFolder;
    }

    // Rotate old logs
    int maxDays;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        maxDays = g_Settings.lootLogMaxDays;
    }
    RotateOldLogs(s_LogFolder, maxDays);

    // Start write thread
    s_Shutdown.store(false);
    s_WriteThread = std::thread(WriteThreadLoop);

    // Open first session
    StartNewSession(addonDir);

    Gw2Api::Log("LootLogger: initialised — folder: " + s_LogFolder, "info");
}

void Shutdown()
{
    // Signal write thread and wait for queue to drain
    {
        std::lock_guard<std::mutex> lock(s_QueueMutex);
        s_Shutdown.store(true);
    }
    s_QueueCv.notify_all();

    if (s_WriteThread.joinable())
        s_WriteThread.join();

    Gw2Api::Log("LootLogger: shutdown complete", "info");
}

void StartNewSession(const std::string& addonDir)
{
    bool enabled;
    int  format;
    bool includeMap, includeMagicFind;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        enabled      = g_Settings.enableLootLog;
        format       = g_Settings.lootLogFormat;
        includeMap   = g_Settings.lootLogIncludeMap;
        includeMagicFind = g_Settings.lootLogIncludeMagicFind;

        if (!g_Settings.lootLogFolder.empty())
            s_LogFolder = g_Settings.lootLogFolder;
        else if (s_LogFolder.empty() && !addonDir.empty())
            s_LogFolder = addonDir + "\\loot-logs";
    }

    if (!enabled || s_LogFolder.empty()) return;

    std::lock_guard<std::mutex> lock(s_Mutex);

    // Find next available session index for today
    std::string today   = UtcTodayDate();
    std::string timeStr = UtcNowTime();
    std::string dayDir  = s_LogFolder + "\\" + today;

    s_SessionIndex = 1;
    while (true)
    {
        char idx[8];
        snprintf(idx, sizeof(idx), "%03d", s_SessionIndex);
        std::string candidate = dayDir + "\\session_" + idx + "_" + timeStr;
        if (!fs::exists(candidate + ".csv") && !fs::exists(candidate + ".json"))
        {
            s_CurrentSessionBase = candidate;
            break;
        }
        ++s_SessionIndex;
    }

    // Write CSV header
    if (format == 0 || format == 2)
        AppendToFile(s_CurrentSessionBase + ".csv",
                     CsvHeader(includeMap, includeMagicFind));

    // Write JSON array opening bracket
    if (format == 1 || format == 2)
        AppendToFile(s_CurrentSessionBase + ".json", "[\n");

    s_CurrentSessionEntries.clear();

    Gw2Api::Log(
        "LootLogger: new session started — "
        + fs::path(s_CurrentSessionBase).filename().string(),
        "info"
    );
}

void LogDrop(const DropEntry& entry)
{
    bool enabled, logItems, logCurrencies;
    int  format;
    bool includeMap, includeMagicFind;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        enabled        = g_Settings.enableLootLog;
        logItems       = g_Settings.lootLogItems;
        logCurrencies  = g_Settings.lootLogCurrencies;
        format         = g_Settings.lootLogFormat;
        includeMap     = g_Settings.lootLogIncludeMap;
        includeMagicFind = g_Settings.lootLogIncludeMagicFind;
    }

    Gw2Api::Log("LootLogger: LogDrop called — enabled: " + std::to_string(enabled) + ", logItems: " + std::to_string(logItems) + ", logCurrencies: " + std::to_string(logCurrencies), "debug");

    if (!enabled) return;
    if (entry.itemType == "Currency" && !logCurrencies) return;
    if (entry.itemType != "Currency" && !logItems)      return;
    if (entry.quantity <= 0)                            return;

    Gw2Api::Log("LootLogger: LogDrop passed checks — " + entry.itemName + " (qty: " + std::to_string(entry.quantity) + ")", "debug");

    // Build queue entry
    QueueEntry qe;
    if (format == 0 || format == 2)
        qe.csvLine    = ToCsvRow(entry, includeMap, includeMagicFind);
    if (format == 1 || format == 2)
        qe.jsonObject = ToJsonObject(entry, includeMap, includeMagicFind);

    // Add to write queue (non-blocking)
    {
        std::lock_guard<std::mutex> lock(s_QueueMutex);
        s_Queue.push_back(std::move(qe));
    }
    s_QueueCv.notify_one();

    // Keep an in-memory copy for the UI tab
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_CurrentSessionEntries.push_back(entry);
        // Cap in-memory list at 500 entries to avoid unbounded growth
        if (s_CurrentSessionEntries.size() > 500)
            s_CurrentSessionEntries.erase(s_CurrentSessionEntries.begin());
    }
}

void LogDrop(
    int          itemId,
    const std::string& itemName,
    long long    quantity,
    bool         isCurrency,
    const std::string& itemType,
    const std::string& rarity,
    long long    sellPriceTp,
    int          mapId,
    const std::string& mapName)
{
    DropEntry e;
    e.timestampUtc = UtcNowIso();
    e.mapId        = mapId;
    e.mapName      = mapName;
    e.itemId       = itemId;
    e.itemName     = itemName;
    e.quantity     = quantity;
    e.itemType     = isCurrency ? "Currency" : itemType;
    e.rarity       = rarity;
    e.sellPriceTp  = sellPriceTp;
    e.magicFind    = ItemTracker::GetMagicFind();

    LogDrop(e);
}

std::string ResolveMapName(int mapId, const std::string& apiToken)
{
    if (mapId <= 0) return "";

    {
        std::lock_guard<std::mutex> lock(s_MapMutex);
        auto it = s_MapCache.find(mapId);
        // Only return cache hit if it's a real name (not a fallback)
        if (it != s_MapCache.end() && it->second.rfind("map_", 0) != 0)
            return it->second;
    }

    // Not cached (or only has fallback) — fetch from API
    // /v2/maps is a public endpoint — no API token required
    std::string name;
    {
        nlohmann::json mapJson;
        std::string    err;
        // Pass empty token so GetJson does NOT append access_token= to the URL
        std::string url = "/v2/maps/" + std::to_string(mapId) + "?lang=" + Gw2Api::GetLanguageCode();
        Gw2Api::Log("LootLogger: resolving map " + std::to_string(mapId) + " via API: " + url, "debug");
        if (Gw2Api::GetJson(url, "", mapJson, err))
        {
            if (mapJson.contains("name"))
                name = mapJson["name"].get<std::string>();
        }
        if (name.empty())
            Gw2Api::Log("LootLogger: could not resolve map " + std::to_string(mapId) + ": " + err, "warning");
        else
            Gw2Api::Log("LootLogger: resolved map " + std::to_string(mapId) + " to: " + name, "debug");
    }

    if (!name.empty())
    {
        // Only cache real names — never cache the fallback string
        std::lock_guard<std::mutex> lock(s_MapMutex);
        s_MapCache[mapId] = name;
        return name;
    }

    // Return temporary fallback without caching it so next drop retries
    return "map_" + std::to_string(mapId);
}

std::vector<DropEntry> GetCurrentSessionEntries()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    Gw2Api::Log("LootLogger: GetCurrentSessionEntries called — " + std::to_string(s_CurrentSessionEntries.size()) + " entries", "debug");
    return s_CurrentSessionEntries;
}

std::string GetCurrentSessionPath()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    int format;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        format = g_Settings.lootLogFormat;
    }
    if (s_CurrentSessionBase.empty()) return "";
    return s_CurrentSessionBase + (format == 1 ? ".json" : ".csv");
}

std::string GetLogFolder()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_LogFolder;
}

void RemoveEntriesForItem(int itemId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CurrentSessionEntries.erase(
        std::remove_if(s_CurrentSessionEntries.begin(), s_CurrentSessionEntries.end(),
            [itemId](const DropEntry& entry) { return entry.itemId == itemId; }),
        s_CurrentSessionEntries.end()
    );
}

std::mutex& GetSessionEntriesMutex()
{
    return s_Mutex;
}

std::vector<DropEntry>& GetSessionEntriesRef()
{
    // Caller MUST hold s_Mutex
    return s_CurrentSessionEntries;
}

} // namespace LootLogger
