#ifndef SESSION_HISTORY_H
#define SESSION_HISTORY_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

namespace SessionHistory
{
struct DropEntry
{
    int itemId;
    std::string itemName;
    std::string iconUrl;
    bool isCurrency = false;
    std::string rarity;
    int count;
    long long totalValue;
    int magicFind = -1;
    std::string timestamp; // ISO-8601 format: "YYYY-MM-DD HH:MM:SS"
    std::string characterName; // Character name at the time of the drop
};

struct SessionData
{
    std::string startTime;
    std::string endTime;
    int durationSeconds;
    long long totalProfit;
    long long profitPerHour;
    int totalDrops;
    int averageMagicFind = -1;
    std::vector<DropEntry> topDrops;
    std::vector<DropEntry> allDrops; // All drops with timestamps (when saveAllItems is enabled)
    std::map<std::string, int> rarityCounts;
    std::string mapName;
    std::string note; // User-editable session note/tag
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
std::string ExportToCsv();
bool ImportFromCsv(const std::string& csvData);
void SetSessionNote(const std::string& startTime, const std::string& note);

} // namespace SessionHistory

#endif // SESSION_HISTORY_H
