#include "session_history.h"
#include "settings.h"
#include "../include/nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace SessionHistory
{
static std::string s_addonDir;
static bool s_enabled = false;
static int s_maxSessions = 20;
static bool s_saveAllItems = false;
static bool s_overwrite = true;

void Init(const char* addonDir)
{
    if (addonDir)
        s_addonDir = addonDir;
}

void Shutdown()
{
}

void SaveSession(const SessionData& session)
{
    if (!s_enabled || s_addonDir.empty())
        return;

    // Load existing sessions
    auto sessions = LoadSessions();

    // Check if limit is reached and overwrite is disabled
    if (!s_overwrite && static_cast<int>(sessions.size()) >= s_maxSessions)
    {
        // Don't save if limit reached and overwrite is disabled
        return;
    }

    // Add new session
    sessions.push_back(session);

    // Remove oldest sessions if limit exceeded (only if overwrite is enabled)
    if (s_overwrite)
    {
        while (static_cast<int>(sessions.size()) > s_maxSessions)
        {
            sessions.erase(sessions.begin());
        }
    }

    // Save to file
    std::string filePath = s_addonDir + "/session_history.json";
    nlohmann::json j = nlohmann::json::array();

    for (const auto& s : sessions)
    {
        nlohmann::json sessionJson;
        sessionJson["startTime"] = s.startTime;
        sessionJson["endTime"] = s.endTime;
        sessionJson["durationSeconds"] = s.durationSeconds;
        sessionJson["totalProfit"] = s.totalProfit;
        sessionJson["profitPerHour"] = s.profitPerHour;
        sessionJson["totalDrops"] = s.totalDrops;
        sessionJson["mapName"] = s.mapName;

        // Save top drops
        sessionJson["topDrops"] = nlohmann::json::array();
        for (const auto& drop : s.topDrops)
        {
            nlohmann::json dropJson;
            dropJson["itemId"] = drop.itemId;
            dropJson["itemName"] = drop.itemName;
            dropJson["rarity"] = drop.rarity;
            dropJson["count"] = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            sessionJson["topDrops"].push_back(dropJson);
        }

        // Save rarity counts
        sessionJson["rarityCounts"] = nlohmann::json::object();
        for (const auto& [rarity, count] : s.rarityCounts)
        {
            sessionJson["rarityCounts"][rarity] = count;
        }

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
    std::vector<SessionData> sessions;

    if (s_addonDir.empty())
        return sessions;

    std::string filePath = s_addonDir + "/session_history.json";
    std::ifstream file(filePath);

    if (!file.is_open())
        return sessions;

    try
    {
        nlohmann::json j;
        file >> j;

        if (!j.is_array())
            return sessions;

        for (const auto& sessionJson : j)
        {
            SessionData s;
            s.startTime = sessionJson.value("startTime", "");
            s.endTime = sessionJson.value("endTime", "");
            s.durationSeconds = sessionJson.value("durationSeconds", 0);
            s.totalProfit = sessionJson.value("totalProfit", 0LL);
            s.profitPerHour = sessionJson.value("profitPerHour", 0LL);
            s.totalDrops = sessionJson.value("totalDrops", 0);
            s.mapName = sessionJson.value("mapName", "");

            // Load top drops
            if (sessionJson.contains("topDrops") && sessionJson["topDrops"].is_array())
            {
                for (const auto& dropJson : sessionJson["topDrops"])
                {
                    DropEntry drop;
                    drop.itemId = dropJson.value("itemId", 0);
                    drop.itemName = dropJson.value("itemName", "");
                    drop.rarity = dropJson.value("rarity", "");
                    drop.count = dropJson.value("count", 0);
                    drop.totalValue = dropJson.value("totalValue", 0LL);
                    s.topDrops.push_back(drop);
                }
            }

            // Load rarity counts
            if (sessionJson.contains("rarityCounts") && sessionJson["rarityCounts"].is_object())
            {
                for (auto& [rarity, count] : sessionJson["rarityCounts"].items())
                {
                    s.rarityCounts[rarity] = count.get<int>();
                }
            }

            sessions.push_back(s);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading session history: " << e.what() << std::endl;
    }

    return sessions;
}

void ClearHistory()
{
    if (s_addonDir.empty())
        return;

    std::string filePath = s_addonDir + "/session_history.json";
    std::remove(filePath.c_str());
}

int GetSessionCount()
{
    return static_cast<int>(LoadSessions().size());
}

bool IsEnabled()
{
    return s_enabled;
}

void SetEnabled(bool enabled)
{
    s_enabled = enabled;
}

int GetMaxSessions()
{
    return s_maxSessions;
}

void SetMaxSessions(int maxSessions)
{
    s_maxSessions = std::clamp(maxSessions, 1, 50);
}

bool GetSaveAllItems()
{
    return s_saveAllItems;
}

void SetSaveAllItems(bool saveAllItems)
{
    s_saveAllItems = saveAllItems;
}

bool GetOverwrite()
{
    return s_overwrite;
}

void SetOverwrite(bool overwrite)
{
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
            dropJson["rarity"] = drop.rarity;
            dropJson["count"] = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            sessionJson["topDrops"].push_back(dropJson);
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
        {
            return false;
        }

        // Clear existing history first
        ClearHistory();

        // Import sessions
        for (const auto& sessionJson : importData["sessions"])
        {
            SessionData session;
            session.startTime = sessionJson.value("startTime", "");
            session.endTime = sessionJson.value("endTime", "");
            session.durationSeconds = sessionJson.value("durationSeconds", 0);
            session.totalProfit = sessionJson.value("totalProfit", 0LL);
            session.profitPerHour = sessionJson.value("profitPerHour", 0LL);
            session.totalDrops = sessionJson.value("totalDrops", 0);
            session.mapName = sessionJson.value("mapName", "");

            // Load top drops
            if (sessionJson.contains("topDrops") && sessionJson["topDrops"].is_array())
            {
                for (const auto& dropJson : sessionJson["topDrops"])
                {
                    DropEntry drop;
                    drop.itemId = dropJson.value("itemId", 0);
                    drop.itemName = dropJson.value("itemName", "");
                    drop.rarity = dropJson.value("rarity", "");
                    drop.count = dropJson.value("count", 0);
                    drop.totalValue = dropJson.value("totalValue", 0LL);
                    session.topDrops.push_back(drop);
                }
            }

            // Load rarity counts
            if (sessionJson.contains("rarityCounts") && sessionJson["rarityCounts"].is_object())
            {
                for (nlohmann::json::const_iterator it = sessionJson["rarityCounts"].cbegin(); it != sessionJson["rarityCounts"].cend(); ++it)
                {
                    session.rarityCounts[it.key()] = it.value().get<int>();
                }
            }

            SaveSession(session);
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace SessionHistory
