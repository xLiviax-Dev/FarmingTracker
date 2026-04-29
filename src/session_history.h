#ifndef SESSION_HISTORY_H
#define SESSION_HISTORY_H

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace SessionHistory
{
struct DropEntry
{
    int itemId;
    std::string itemName;
    std::string rarity;
    int count;
    long long totalValue;
};

struct SessionData
{
    std::string startTime;
    std::string endTime;
    int durationSeconds;
    long long totalProfit;
    long long profitPerHour;
    int totalDrops;
    std::vector<DropEntry> topDrops;
    std::map<std::string, int> rarityCounts;
    std::string mapName;
};

enum class SummaryPeriod
{
    Today,
    ThisWeek,
    ThisMonth
};

struct SummaryData
{
    SummaryPeriod period;
    long long totalProfit;
    long long profitPerHour;
    int totalDrops;
    int sessionCount;
    int totalDurationSeconds;
    std::vector<DropEntry> topDrops;
    std::map<std::string, int> rarityCounts;
    long long previousPeriodProfit; // For comparison
};

void Init(const char* addonDir);
void Shutdown();
void SaveSession(const SessionData& session);
std::vector<SessionData> LoadSessions();
void ClearHistory();
int GetSessionCount();
bool IsEnabled();
void SetEnabled(bool enabled);
int GetMaxSessions();
void SetMaxSessions(int maxSessions);
bool GetSaveAllItems();
void SetSaveAllItems(bool saveAllItems);
bool GetOverwrite();
void SetOverwrite(bool overwrite);
SummaryData GetSummary(SummaryPeriod period);
std::string ExportToJson();
bool ImportFromJson(const std::string& jsonData);

} // namespace SessionHistory

#endif // SESSION_HISTORY_H
