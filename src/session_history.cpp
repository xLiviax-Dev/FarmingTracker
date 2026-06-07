#include "session_history.h"
#include "settings.h"
#include "shared.h"
#include "../include/nlohmann/json.hpp"
#include <fstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace SessionHistory
{
static std::string s_addonDir;
static bool s_enabled = false;
static int s_maxSessions = 20;
static bool s_saveAllItems = true;
static bool s_overwrite = true;

// Thread safety
static std::mutex s_Mutex;

// In-memory cache to avoid repeated disk reads
static std::vector<SessionData> s_Cache;
static bool s_CacheValid = false;

void Init(const char* addonDir)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    if (addonDir)
        s_addonDir = addonDir;
    s_CacheValid = false;
}

void Shutdown()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_Cache.clear();
    s_CacheValid = false;
}

void SaveSession(const SessionData& session)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    if (!s_enabled || s_addonDir.empty())
        return;

    // Use cache if valid, else load from disk
    if (!s_CacheValid)
    {
        // Load without holding lock recursively — build cache inline
        std::string filePath = s_addonDir + "/session_history.json";
        std::ifstream inFile(filePath);
        s_Cache.clear();
        if (inFile.is_open())
        {
            try
            {
                nlohmann::json j;
                inFile >> j;
                if (j.is_array())
                {
                    for (const auto& sessionJson : j)
                    {
                        SessionData s;
                        s.startTime        = sessionJson.value("startTime", "");
                        s.endTime          = sessionJson.value("endTime", "");
                        s.durationSeconds  = sessionJson.value("durationSeconds", 0);
                        s.totalProfit      = sessionJson.value("totalProfit", 0LL);
                        s.profitPerHour    = sessionJson.value("profitPerHour", 0LL);
                        s.totalDrops       = sessionJson.value("totalDrops", 0);
                        s.averageMagicFind = sessionJson.value("averageMagicFind", -1);
                        s.mapName          = sessionJson.value("mapName", "");
                        if (sessionJson.contains("topDrops") && sessionJson["topDrops"].is_array())
                            for (const auto& d : sessionJson["topDrops"]) { DropEntry drop; drop.itemId=d.value("itemId",0); drop.itemName=d.value("itemName",""); drop.iconUrl=d.value("iconUrl",""); drop.isCurrency=d.value("isCurrency",false); drop.rarity=d.value("rarity",""); drop.count=d.value("count",0); drop.totalValue=d.value("totalValue",0LL); drop.magicFind=d.value("magicFind",-1); drop.timestamp=d.value("timestamp",""); s.topDrops.push_back(drop); }
                        if (sessionJson.contains("allDrops") && sessionJson["allDrops"].is_array())
                            for (const auto& d : sessionJson["allDrops"]) { DropEntry drop; drop.itemId=d.value("itemId",0); drop.itemName=d.value("itemName",""); drop.iconUrl=d.value("iconUrl",""); drop.isCurrency=d.value("isCurrency",false); drop.rarity=d.value("rarity",""); drop.count=d.value("count",0); drop.totalValue=d.value("totalValue",0LL); drop.magicFind=d.value("magicFind",-1); drop.timestamp=d.value("timestamp",""); s.allDrops.push_back(drop); }
                        if (sessionJson.contains("rarityCounts") && sessionJson["rarityCounts"].is_object())
                            for (auto& [r,c] : sessionJson["rarityCounts"].items()) s.rarityCounts[r] = c.get<int>();
                        s_Cache.push_back(s);
                    }
                }
            }
            catch (...) {}
        }
        s_CacheValid = true;
    }

    // Replace existing session with same start time
    bool found = false;
    for (auto& s : s_Cache)
    {
        if (s.startTime == session.startTime)
        {
            s = session;
            found = true;
            break;
        }
    }

    if (!found)
    {
        // If overwrite disabled and limit reached, do not save
        if (!s_overwrite && static_cast<int>(s_Cache.size()) >= s_maxSessions)
            return;

        s_Cache.push_back(session);
    }

    // Trim oldest entries when limit exceeded (works for both overwrite=true and false)
    if (s_maxSessions > 0 && static_cast<int>(s_Cache.size()) > s_maxSessions)
    {
        int excess = static_cast<int>(s_Cache.size()) - s_maxSessions;
        s_Cache.erase(s_Cache.begin(), s_Cache.begin() + excess);
    }

    // Persist cache to disk
    std::string filePath = s_addonDir + "/session_history.json";
    nlohmann::json j = nlohmann::json::array();
    for (const auto& s : s_Cache)
    {
        nlohmann::json sessionJson;
        sessionJson["startTime"]        = s.startTime;
        sessionJson["endTime"]          = s.endTime;
        sessionJson["durationSeconds"]  = s.durationSeconds;
        sessionJson["totalProfit"]      = s.totalProfit;
        sessionJson["profitPerHour"]    = s.profitPerHour;
        sessionJson["totalDrops"]       = s.totalDrops;
        sessionJson["averageMagicFind"] = s.averageMagicFind;
        sessionJson["mapName"]          = s.mapName;
        sessionJson["note"]             = s.note;
        sessionJson["topDrops"] = nlohmann::json::array();
        for (const auto& drop : s.topDrops)
        {
            nlohmann::json dropJson;
            dropJson["itemId"]     = drop.itemId;
            dropJson["itemName"]   = drop.itemName;
            dropJson["iconUrl"]    = drop.iconUrl;
            dropJson["isCurrency"] = drop.isCurrency;
            dropJson["rarity"]     = drop.rarity;
            dropJson["count"]      = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            dropJson["magicFind"]  = drop.magicFind;
            dropJson["timestamp"]  = drop.timestamp;
            sessionJson["topDrops"].push_back(dropJson);
        }
        sessionJson["allDrops"] = nlohmann::json::array();
        for (const auto& drop : s.allDrops)
        {
            nlohmann::json dropJson;
            dropJson["itemId"]     = drop.itemId;
            dropJson["itemName"]   = drop.itemName;
            dropJson["iconUrl"]    = drop.iconUrl;
            dropJson["isCurrency"] = drop.isCurrency;
            dropJson["rarity"]     = drop.rarity;
            dropJson["count"]      = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            dropJson["magicFind"]  = drop.magicFind;
            dropJson["timestamp"]  = drop.timestamp;
            sessionJson["allDrops"].push_back(dropJson);
        }
        sessionJson["rarityCounts"] = nlohmann::json::object();
        for (const auto& [rarity, count] : s.rarityCounts)
            sessionJson["rarityCounts"][rarity] = count;
        j.push_back(sessionJson);
    }
    std::ofstream file(filePath);
    if (file.is_open())
    {
        file << j.dump(4);
        file.close();
    }
}

std::vector<SessionData> LoadSessions()
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    // Return cached data if valid
    if (s_CacheValid)
        return s_Cache;

    s_Cache.clear();

    if (s_addonDir.empty())
    {
        s_CacheValid = true;
        return s_Cache;
    }

    std::string filePath = s_addonDir + "/session_history.json";
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        s_CacheValid = true;
        return s_Cache;
    }

    try
    {
        nlohmann::json j;
        file >> j;

        if (!j.is_array())
        {
            s_CacheValid = true;
            return s_Cache;
        }

        for (const auto& sessionJson : j)
        {
            SessionData s;
            s.startTime        = sessionJson.value("startTime", "");
            s.endTime          = sessionJson.value("endTime", "");
            s.durationSeconds  = sessionJson.value("durationSeconds", 0);
            s.totalProfit      = sessionJson.value("totalProfit", 0LL);
            s.profitPerHour    = sessionJson.value("profitPerHour", 0LL);
            s.totalDrops       = sessionJson.value("totalDrops", 0);
            s.averageMagicFind = sessionJson.value("averageMagicFind", -1);
            s.mapName          = sessionJson.value("mapName", "");
            s.note             = sessionJson.value("note", "");

            if (sessionJson.contains("topDrops") && sessionJson["topDrops"].is_array())
            {
                for (const auto& dropJson : sessionJson["topDrops"])
                {
                    DropEntry drop;
                    drop.itemId     = dropJson.value("itemId", 0);
                    drop.itemName   = dropJson.value("itemName", "");
                    drop.iconUrl    = dropJson.value("iconUrl", "");
                    drop.isCurrency = dropJson.value("isCurrency", false);
                    drop.rarity     = dropJson.value("rarity", "");
                    drop.count      = dropJson.value("count", 0);
                    drop.totalValue = dropJson.value("totalValue", 0LL);
                    drop.magicFind  = dropJson.value("magicFind", -1);
                    drop.timestamp  = dropJson.value("timestamp", "");
                    s.topDrops.push_back(drop);
                }
            }

            if (sessionJson.contains("allDrops") && sessionJson["allDrops"].is_array())
            {
                for (const auto& dropJson : sessionJson["allDrops"])
                {
                    DropEntry drop;
                    drop.itemId     = dropJson.value("itemId", 0);
                    drop.itemName   = dropJson.value("itemName", "");
                    drop.iconUrl    = dropJson.value("iconUrl", "");
                    drop.isCurrency = dropJson.value("isCurrency", false);
                    drop.rarity     = dropJson.value("rarity", "");
                    drop.count      = dropJson.value("count", 0);
                    drop.totalValue = dropJson.value("totalValue", 0LL);
                    drop.magicFind  = dropJson.value("magicFind", -1);
                    drop.timestamp  = dropJson.value("timestamp", "");
                    s.allDrops.push_back(drop);
                }
            }

            if (sessionJson.contains("rarityCounts") && sessionJson["rarityCounts"].is_object())
            {
                for (auto& [rarity, count] : sessionJson["rarityCounts"].items())
                    s.rarityCounts[rarity] = count.get<int>();
            }

            s_Cache.push_back(s);
        }
    }
    catch (const std::exception& e)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker",
                (std::string("SessionHistory: Failed to load session_history.json: ") + e.what()).c_str());
        s_Cache.clear();
    }

    s_CacheValid = true;
    return s_Cache;
}

void ClearHistory()
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    s_Cache.clear();
    s_CacheValid = true; // Cache is now valid (and empty)

    if (s_addonDir.empty())
        return;

    std::string filePath = s_addonDir + "/session_history.json";
    std::remove(filePath.c_str());
}

int GetSessionCount()
{
    // Use LoadSessions() which uses the cache — no extra disk read
    return static_cast<int>(LoadSessions().size());
}

bool IsEnabled()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_enabled;
}

void SetEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_enabled = enabled;
}

int GetMaxSessions()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_maxSessions;
}

void SetMaxSessions(int maxSessions)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_maxSessions = std::clamp(maxSessions, 1, 50);
}

bool GetSaveAllItems()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_saveAllItems;
}

void SetSaveAllItems(bool saveAllItems)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_saveAllItems = saveAllItems;
}

bool GetOverwrite()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_overwrite;
}

void SetOverwrite(bool overwrite)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_overwrite = overwrite;
}

SummaryData GetSummary(SummaryPeriod period)
{
    SummaryData summary;
    summary.period = period;
    summary.totalProfit = 0;
    summary.profitPerHour = 0;
    summary.totalDrops = 0;
    summary.sessionCount = 0;
    summary.totalDurationSeconds = 0;
    summary.previousPeriodProfit = 0;

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    struct tm nowTm;
    localtime_s(&nowTm, &nowTimeT);

    // Calculate time thresholds
    struct tm startOfDay = nowTm;
    startOfDay.tm_hour = 0;
    startOfDay.tm_min = 0;
    startOfDay.tm_sec = 0;
    time_t dayStart = std::mktime(&startOfDay);

    struct tm startOfWeek = startOfDay;
    int weekday = startOfDay.tm_wday;
    if (weekday == 0) weekday = 7; // Sunday = 7, Monday = 1
    startOfWeek.tm_mday -= (weekday - 1);
    time_t weekStart = std::mktime(&startOfWeek);

    struct tm startOfMonth = startOfDay;
    startOfMonth.tm_mday = 1;
    time_t monthStart = std::mktime(&startOfMonth);

    // Calculate previous period start
    time_t periodStart;
    time_t previousPeriodStart;
    time_t previousPeriodEnd;

    switch (period)
    {
    case SummaryPeriod::Today:
        periodStart = dayStart;
        previousPeriodStart = dayStart - 86400; // Yesterday
        previousPeriodEnd = dayStart;
        break;
    case SummaryPeriod::ThisWeek:
        periodStart = weekStart;
        previousPeriodStart = weekStart - (7 * 86400); // Previous week
        previousPeriodEnd = weekStart;
        break;
    case SummaryPeriod::ThisMonth:
        periodStart = monthStart;
        previousPeriodStart = monthStart - (30 * 86400); // Previous month (approx)
        previousPeriodEnd = monthStart;
        break;
    }

    // Load sessions
    auto sessions = LoadSessions();

    // Aggregate data for current period
    for (const auto& session : sessions)
    {
        // Parse session start time
        struct tm sessionTm = {};
        std::istringstream ss(session.startTime);
        ss >> std::get_time(&sessionTm, "%Y-%m-%d %H:%M:%S");
        time_t sessionTime = std::mktime(&sessionTm);

        if (sessionTime >= periodStart)
        {
            summary.totalProfit += session.totalProfit;
            summary.totalDrops += session.totalDrops;
            summary.totalDurationSeconds += session.durationSeconds;
            summary.sessionCount++;

            // Aggregate top drops
            for (const auto& drop : session.topDrops)
            {
                bool found = false;
                for (auto& summaryDrop : summary.topDrops)
                {
                    if (summaryDrop.itemId == drop.itemId)
                    {
                        summaryDrop.count += drop.count;
                        summaryDrop.totalValue += drop.totalValue;
                        if (summaryDrop.iconUrl.empty() && !drop.iconUrl.empty())
                            summaryDrop.iconUrl = drop.iconUrl;
                        if (!summaryDrop.isCurrency && drop.isCurrency)
                            summaryDrop.isCurrency = true;
                        if ((summaryDrop.itemName.empty() || summaryDrop.itemName == "Unknown") && !drop.itemName.empty() && drop.itemName != "Unknown")
                            summaryDrop.itemName = drop.itemName;
                        if ((summaryDrop.rarity.empty() || summaryDrop.rarity == "Unknown") && !drop.rarity.empty() && drop.rarity != "Unknown")
                            summaryDrop.rarity = drop.rarity;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    summary.topDrops.push_back(drop);
                }
            }

            // Aggregate rarity counts
            for (const auto& [rarity, count] : session.rarityCounts)
            {
                summary.rarityCounts[rarity] += count;
            }
        }
        else if (sessionTime >= previousPeriodStart && sessionTime < previousPeriodEnd)
        {
            // Calculate previous period profit for comparison
            summary.previousPeriodProfit += session.totalProfit;
        }
    }

    // Calculate profit per hour
    if (summary.totalDurationSeconds > 0)
    {
        summary.profitPerHour = (summary.totalProfit * 3600) / summary.totalDurationSeconds;
    }

    // Sort top drops by value and keep top 10
    std::sort(summary.topDrops.begin(), summary.topDrops.end(),
        [](const DropEntry& a, const DropEntry& b) { return a.totalValue > b.totalValue; });
    if (summary.topDrops.size() > 10)
    {
        summary.topDrops.resize(10);
    }

    return summary;
}

std::string SessionHistory::ExportToJson()
{
    nlohmann::json exportData;
    exportData["version"] = 1;
    exportData["exportTimestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

    auto sessions = LoadSessions();
    exportData["sessionCount"] = static_cast<int>(sessions.size());

    nlohmann::json sessionsArray = nlohmann::json::array();
    for (const auto& session : sessions)
    {
        nlohmann::json sessionJson;
        sessionJson["startTime"] = session.startTime;
        sessionJson["endTime"] = session.endTime;
        sessionJson["durationSeconds"] = session.durationSeconds;
        sessionJson["totalProfit"] = session.totalProfit;
        sessionJson["profitPerHour"] = session.profitPerHour;
        sessionJson["totalDrops"] = session.totalDrops;
        sessionJson["mapName"] = session.mapName;

        // Save top drops
        sessionJson["topDrops"] = nlohmann::json::array();
        for (const auto& drop : session.topDrops)
        {
            nlohmann::json dropJson;
            dropJson["itemId"] = drop.itemId;
            dropJson["itemName"] = drop.itemName;
            dropJson["iconUrl"] = drop.iconUrl;
            dropJson["isCurrency"] = drop.isCurrency;
            dropJson["rarity"] = drop.rarity;
            dropJson["count"] = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            dropJson["timestamp"] = drop.timestamp;
            sessionJson["topDrops"].push_back(dropJson);
        }

        // Save all drops
        sessionJson["allDrops"] = nlohmann::json::array();
        for (const auto& drop : session.allDrops)
        {
            nlohmann::json dropJson;
            dropJson["itemId"] = drop.itemId;
            dropJson["itemName"] = drop.itemName;
            dropJson["iconUrl"] = drop.iconUrl;
            dropJson["isCurrency"] = drop.isCurrency;
            dropJson["rarity"] = drop.rarity;
            dropJson["count"] = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            dropJson["timestamp"] = drop.timestamp;
            sessionJson["allDrops"].push_back(dropJson);
        }

        // Save rarity counts
        sessionJson["rarityCounts"] = nlohmann::json::object();
        for (std::map<std::string, int>::const_iterator it = session.rarityCounts.begin(); it != session.rarityCounts.end(); ++it)
        {
            sessionJson["rarityCounts"][it->first] = it->second;
        }

        sessionsArray.push_back(sessionJson);
    }
    exportData["sessions"] = sessionsArray;

    return exportData.dump(4);
}

bool SessionHistory::ImportFromJson(const std::string& jsonData)
{
    try
    {
        nlohmann::json importData = nlohmann::json::parse(jsonData);

        if (!importData.contains("sessions") || !importData["sessions"].is_array())
            return false;

        // Clear existing history (also resets cache)
        ClearHistory();

        for (const auto& sessionJson : importData["sessions"])
        {
            SessionData session;
            session.startTime       = sessionJson.value("startTime", "");
            session.endTime         = sessionJson.value("endTime", "");
            session.durationSeconds = sessionJson.value("durationSeconds", 0);
            session.totalProfit     = sessionJson.value("totalProfit", 0LL);
            session.profitPerHour   = sessionJson.value("profitPerHour", 0LL);
            session.totalDrops      = sessionJson.value("totalDrops", 0);
            session.mapName         = sessionJson.value("mapName", "");

            if (sessionJson.contains("topDrops") && sessionJson["topDrops"].is_array())
            {
                for (const auto& dropJson : sessionJson["topDrops"])
                {
                    DropEntry drop;
                    drop.itemId     = dropJson.value("itemId", 0);
                    drop.itemName   = dropJson.value("itemName", "");
                    drop.iconUrl    = dropJson.value("iconUrl", "");
                    drop.isCurrency = dropJson.value("isCurrency", false);
                    drop.rarity     = dropJson.value("rarity", "");
                    drop.count      = dropJson.value("count", 0);
                    drop.totalValue = dropJson.value("totalValue", 0LL);
                    drop.timestamp  = dropJson.value("timestamp", "");
                    session.topDrops.push_back(drop);
                }
            }

            if (sessionJson.contains("allDrops") && sessionJson["allDrops"].is_array())
            {
                for (const auto& dropJson : sessionJson["allDrops"])
                {
                    DropEntry drop;
                    drop.itemId     = dropJson.value("itemId", 0);
                    drop.itemName   = dropJson.value("itemName", "");
                    drop.iconUrl    = dropJson.value("iconUrl", "");
                    drop.isCurrency = dropJson.value("isCurrency", false);
                    drop.rarity     = dropJson.value("rarity", "");
                    drop.count      = dropJson.value("count", 0);
                    drop.totalValue = dropJson.value("totalValue", 0LL);
                    drop.timestamp  = dropJson.value("timestamp", "");
                    session.allDrops.push_back(drop);
                }
            }

            if (sessionJson.contains("rarityCounts") && sessionJson["rarityCounts"].is_object())
            {
                for (nlohmann::json::const_iterator it = sessionJson["rarityCounts"].cbegin(); it != sessionJson["rarityCounts"].cend(); ++it)
                    session.rarityCounts[it.key()] = it.value().get<int>();
            }

            // SaveSession locks s_Mutex — but since ClearHistory() released it above,
            // and SaveSession() re-acquires it, this is safe (non-recursive).
            SaveSession(session);
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

void SetSessionNote(const std::string& startTime, const std::string& note)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (auto& s : s_Cache)
    {
        if (s.startTime == startTime)
        {
            s.note = note;
            break;
        }
    }
    // Persist: rebuild JSON on disk
    if (s_addonDir.empty()) return;
    std::string filePath = s_addonDir + "/session_history.json";
    nlohmann::json j = nlohmann::json::array();
    for (const auto& s : s_Cache)
    {
        nlohmann::json sj;
        sj["startTime"]        = s.startTime;
        sj["endTime"]          = s.endTime;
        sj["durationSeconds"]  = s.durationSeconds;
        sj["totalProfit"]      = s.totalProfit;
        sj["profitPerHour"]    = s.profitPerHour;
        sj["totalDrops"]       = s.totalDrops;
        sj["averageMagicFind"] = s.averageMagicFind;
        sj["mapName"]          = s.mapName;
        sj["note"]             = s.note;
        sj["topDrops"]  = nlohmann::json::array();
        for (const auto& d : s.topDrops) { nlohmann::json dj; dj["itemId"]=d.itemId; dj["itemName"]=d.itemName; dj["iconUrl"]=d.iconUrl; dj["isCurrency"]=d.isCurrency; dj["rarity"]=d.rarity; dj["count"]=d.count; dj["totalValue"]=d.totalValue; dj["magicFind"]=d.magicFind; dj["timestamp"]=d.timestamp; sj["topDrops"].push_back(dj); }
        sj["allDrops"]  = nlohmann::json::array();
        for (const auto& d : s.allDrops) { nlohmann::json dj; dj["itemId"]=d.itemId; dj["itemName"]=d.itemName; dj["iconUrl"]=d.iconUrl; dj["isCurrency"]=d.isCurrency; dj["rarity"]=d.rarity; dj["count"]=d.count; dj["totalValue"]=d.totalValue; dj["magicFind"]=d.magicFind; dj["timestamp"]=d.timestamp; sj["allDrops"].push_back(dj); }
        sj["rarityCounts"] = nlohmann::json::object();
        for (const auto& [r,c] : s.rarityCounts) sj["rarityCounts"][r] = c;
        j.push_back(sj);
    }
    std::ofstream file(filePath);
    if (file.is_open()) { file << j.dump(4); }
}

std::string ExportToCsv()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::vector<SessionData> sessions = LoadSessions();
    
    std::stringstream csv;
    csv << "Start Time,End Time,Duration (s),Total Profit,Profit/Hour,Total Drops,Average Magic Find,Map Name,Note\n";
    
    for (const auto& s : sessions)
    {
        csv << s.startTime << ","
            << s.endTime << ","
            << s.durationSeconds << ","
            << s.totalProfit << ","
            << s.profitPerHour << ","
            << s.totalDrops << ","
            << s.averageMagicFind << ","
            << s.mapName << ","
            << s.note << "\n";
    }
    
    return csv.str();
}

bool ImportFromCsv(const std::string& csvData)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::stringstream ss(csvData);
    std::string line;
    
    // Skip header
    std::getline(ss, line);
    
    std::vector<SessionData> newSessions;
    
    while (std::getline(ss, line))
    {
        if (line.empty()) continue;
        
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> cells;
        
        while (std::getline(lineStream, cell, ','))
        {
            cells.push_back(cell);
        }
        
        if (cells.size() >= 9)
        {
            try
            {
                SessionData s;
                s.startTime = cells[0];
                s.endTime = cells[1];
                s.durationSeconds = std::stoi(cells[2]);
                s.totalProfit = std::stoll(cells[3]);
                s.profitPerHour = std::stoll(cells[4]);
                s.totalDrops = std::stoi(cells[5]);
                s.averageMagicFind = std::stoi(cells[6]);
                s.mapName = cells[7];
                s.note = cells[8];
                
                newSessions.push_back(s);
            }
            catch (...)
            {
                // Skip invalid lines
            }
        }
    }
    
    if (!newSessions.empty())
    {
        s_Cache = newSessions;
        s_CacheValid = true;
        
        // Persist to disk
        if (s_addonDir.empty()) return true;
        std::string filePath = s_addonDir + "/session_history.json";
        nlohmann::json j = nlohmann::json::array();
        for (const auto& s : s_Cache)
        {
            nlohmann::json sj;
            sj["startTime"]        = s.startTime;
            sj["endTime"]          = s.endTime;
            sj["durationSeconds"]  = s.durationSeconds;
            sj["totalProfit"]      = s.totalProfit;
            sj["profitPerHour"]    = s.profitPerHour;
            sj["totalDrops"]       = s.totalDrops;
            sj["averageMagicFind"] = s.averageMagicFind;
            sj["mapName"]          = s.mapName;
            sj["note"]             = s.note;
            sj["topDrops"]  = nlohmann::json::array();
            for (const auto& d : s.topDrops) { nlohmann::json dj; dj["itemId"]=d.itemId; dj["itemName"]=d.itemName; dj["iconUrl"]=d.iconUrl; dj["isCurrency"]=d.isCurrency; dj["rarity"]=d.rarity; dj["count"]=d.count; dj["totalValue"]=d.totalValue; dj["magicFind"]=d.magicFind; dj["timestamp"]=d.timestamp; sj["topDrops"].push_back(dj); }
            sj["allDrops"]  = nlohmann::json::array();
            for (const auto& d : s.allDrops) { nlohmann::json dj; dj["itemId"]=d.itemId; dj["itemName"]=d.itemName; dj["iconUrl"]=d.iconUrl; dj["isCurrency"]=d.isCurrency; dj["rarity"]=d.rarity; dj["count"]=d.count; dj["totalValue"]=d.totalValue; dj["magicFind"]=d.magicFind; dj["timestamp"]=d.timestamp; sj["allDrops"].push_back(dj); }
            sj["rarityCounts"] = nlohmann::json::object();
            for (const auto& [r,c] : s.rarityCounts) sj["rarityCounts"][r] = c;
            j.push_back(sj);
        }
        std::ofstream file(filePath);
        if (file.is_open()) { file << j.dump(4); }
        
        return true;
    }
    
    return false;
}

} // namespace SessionHistory
