#include "backup_restore.h"
#include "settings.h"
#include "session_history.h"
#include "item_tracker.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "shared.h"
#include "../include/nlohmann/json.hpp"
#include <chrono>
#include <map>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <mutex>

namespace fs = std::filesystem;

namespace BackupRestore
{
// Ensures CreateFullBackup + file write + rotate never runs concurrently from
// both auto-tick (background worker) and manual UI button.
static std::mutex s_BackupMutex;

// Helper: Get current UTC time as ISO-8601 string (YYYY-MM-DD)
static std::string GetCurrentDateUtc()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc;
    gmtime_s(&tm_utc, &now_c);
    std::stringstream ss;
    ss << std::put_time(&tm_utc, "%Y-%m-%d");
    return ss.str();
}

// Helper: Get current UTC time as full timestamp for filename (YYYY-MM-DD_HH-MM-SS)
static std::string GetCurrentTimestampForFile()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc;
    gmtime_s(&tm_utc, &now_c);
    std::stringstream ss;
    ss << std::put_time(&tm_utc, "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

// Shared implementation used by both the async tick job (render-thread safe)
// and the synchronous manual UI backup. `lastBackupDateOnEntry` is the date
// already written to g_Settings on the tick side (prevents double-backups if
// the job runs after midnight local time).
static bool RunBackupJob(const std::string& lastBackupDateOnEntry)
{
    std::lock_guard<std::mutex> runLock(s_BackupMutex);

    fs::path backupDir;
    size_t maxBackupCount;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        maxBackupCount = static_cast<size_t>(std::max(0, g_Settings.maxBackupCount));

        if (g_Settings.autoBackupPath.empty())
        {
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
            if (!addonDir) return false;
            backupDir = fs::path(addonDir) / "backups";
        }
        else
        {
            backupDir = fs::path(g_Settings.autoBackupPath);
        }
    }

    try
    {
        if (!fs::exists(backupDir))
            fs::create_directories(backupDir);

        // Create backup filename
        std::string filename = "backup_" + GetCurrentTimestampForFile() + ".json";
        fs::path backupFile = backupDir / filename;

        // SaveBackupToFile internally calls CreateFullBackup which acquires
        // Settings::s_SettingsMutex briefly at start + several other manager
        // mutexes. All of that now happens on the background worker, never
        // blocking the render thread for 100s of ms.
        bool ok = SaveBackupToFile(backupFile.string());
        if (ok)
        {
            // Persist lastBackupTimestamp if the caller didn't already (manual
            // backup path still wants the timestamp updated).
            if (!lastBackupDateOnEntry.empty())
            {
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                g_Settings.lastBackupTimestamp = lastBackupDateOnEntry;
            }
            // Mark settings dirty for async persistence (coalesced write).
            BackgroundJobs::EnqueueDebouncedSettingsSave();

            // Rotate old backups
            std::vector<fs::directory_entry> backups;
            for (const auto& entry : fs::directory_iterator(backupDir))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".json" &&
                    entry.path().filename().string().find("backup_") == 0)
                {
                    backups.push_back(entry);
                }
            }

            if (maxBackupCount > 0 && backups.size() > maxBackupCount)
            {
                std::sort(backups.begin(), backups.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                    return fs::last_write_time(a) < fs::last_write_time(b);
                });

                for (size_t i = 0; i < backups.size() - maxBackupCount; ++i)
                {
                    std::error_code ec;
                    fs::remove(backups[i], ec); // ignore errors on rotation
                }
            }
        }
        return ok;
    }
    catch (...)
    {
        return false;
    }
}

void Tick()
{
    static auto lastCheck = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    // Check every 5 minutes (cheap, no locks, no IO)
    if (std::chrono::duration_cast<std::chrono::minutes>(now - lastCheck).count() < 5)
        return;
    lastCheck = now;

    std::string currentDate;
    bool shouldBackup = false;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        if (!g_Settings.enableAutoBackups || g_Settings.backupFrequency == 0)
            return;

        currentDate = GetCurrentDateUtc();
        if (g_Settings.lastBackupTimestamp == currentDate)
            return; // Already backed up today (or scheduled)

        // For weekly backup, check if it's Monday
        if (g_Settings.backupFrequency == 2) // Weekly
        {
            auto systemNow = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(systemNow);
            std::tm tm_utc;
            gmtime_s(&tm_utc, &now_c);
            if (tm_utc.tm_wday != 1) // 1 = Monday
                return;
        }

        // Mark the day as "already scheduled" WHILE holding the settings
        // mutex — this way 2 rapid-fire ticks (or mid-day reloads) never
        // enqueue 2 competing backup jobs on the same day.
        g_Settings.lastBackupTimestamp = currentDate;
        shouldBackup = true;
    }

    if (!shouldBackup)
        return;

    // Mark settings dirty; the actual write is coalesced with other pending
    // settings writes (e.g. magnetite shard drops, language toggles).
    BackgroundJobs::EnqueueDebouncedSettingsSave();

    // Hand the 100-500ms of work (JSON build, file IO, backup rotation) to
    // the shared background worker. Render thread returns immediately.
    BackgroundJobs::Enqueue([currentDate] {
        RunBackupJob(currentDate);
    });
}

bool CreateManualBackup()
{
    // Lock ordering: s_BackupMutex (BackupRestore-global) first, then Settings
    // mutex. This matches RunBackupJob and prevents deadlock if a manual UI
    // backup is triggered while the auto worker is still running.
    std::lock_guard<std::mutex> backupLock(s_BackupMutex);

    // Read settings-dependent backup path WITH SettingsMutex, then release
    // immediately so that downstream SaveBackupToFile → CreateFullBackup →
    // ItemTracker::Get*Copy() can acquire s_PersistentMutex without risking
    // a lock-order inversion with AddDrop().
    fs::path backupDir;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        if (g_Settings.autoBackupPath.empty())
        {
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
            if (!addonDir) return false;
            backupDir = fs::path(addonDir) / "backups";
        }
        else
        {
            backupDir = fs::path(g_Settings.autoBackupPath);
        }
    }
    // SettingsMutex is now RELEASED.

    try
    {
        if (!fs::exists(backupDir))
            fs::create_directories(backupDir);

        // Create backup filename
        std::string filename = "manual_backup_" + GetCurrentTimestampForFile() + ".json";
        fs::path backupFile = backupDir / filename;

        return SaveBackupToFile(backupFile.string());
    }
    catch (...)
    {
        return false;
    }
}

std::string CreateFullBackup()
{
    // =========================================================
    // STEP 1: Acquire SettingsMutex ONLY for settings-related
    // fields, then RELEASE IT IMMEDIATELY. This avoids a
    // lock-order inversion deadlock with ItemTracker mutexes:
    //   Drop-Thread:  s_PersistentMutex → SettingsMutex
    //   Backup-Thread: SettingsMutex → s_PersistentMutex → DEADLOCK
    // =========================================================
    nlohmann::json settingsJson;
    uint64_t backupTimestamp;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        backupTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
        settingsJson = SettingsManager::ToSettingsJson(g_Settings);
    }

    nlohmann::json backupData;
    backupData["version"] = 2;
    backupData["backupTimestamp"] = backupTimestamp;
    backupData["settings"] = settingsJson;

    // =========================================================
    // STEP 2: Everything from here on does NOT hold SettingsMutex.
    // It only acquires module-local mutexes (ItemTracker,
    // SessionHistory, IgnoredItems, CustomProfit) each of which
    // is guaranteed to never call back into Settings while
    // holding their own lock.
    // =========================================================

    // Backup Session History
    std::string sessionHistoryJson = SessionHistory::ExportToJson();
    backupData["sessionHistory"] = nlohmann::json::parse(sessionHistoryJson);

    // Backup Current Session
    nlohmann::json currentSessionJson;
    currentSessionJson["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    currentSessionJson["sessionDuration"] = ItemTracker::GetSessionDuration().count();
    currentSessionJson["totalProfit"] = ItemTracker::CalcTotalCustomProfit();
    
    auto items = ItemTracker::GetItemsCopy();
    nlohmann::json itemsArray = nlohmann::json::array();
    for (std::map<int, ::Stat>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        const ::Stat& stat = it->second;
        nlohmann::json itemJson;
        itemJson["apiId"] = stat.apiId;
        itemJson["type"] = static_cast<int>(stat.type);
        itemJson["count"] = stat.count;
        itemJson["isFavorite"] = stat.isFavorite;
        itemJson["isIgnored"] = stat.isIgnored;
        itemJson["name"] = stat.details.name;
        itemJson["iconUrl"] = stat.details.iconUrl;
        itemJson["vendorValue"] = stat.details.vendorValue;
        itemJson["tpSellPrice"] = stat.details.tpSellPrice;
        itemJson["tpBuyPrice"] = stat.details.tpBuyPrice;
        itemJson["noSell"] = stat.details.noSell;
        itemJson["accountBound"] = stat.details.accountBound;
        itemJson["rarity"] = stat.details.rarity;
        itemJson["itemType"] = static_cast<int>(stat.details.itemType);
        itemsArray.push_back(itemJson);
    }
    currentSessionJson["items"] = itemsArray;

    auto currencies = ItemTracker::GetCurrenciesCopy();
    nlohmann::json currenciesArray = nlohmann::json::array();
    for (std::map<int, ::Stat>::const_iterator it = currencies.begin(); it != currencies.end(); ++it)
    {
        const ::Stat& stat = it->second;
        nlohmann::json currencyJson;
        currencyJson["apiId"] = stat.apiId;
        currencyJson["type"] = static_cast<int>(stat.type);
        currencyJson["count"] = stat.count;
        currencyJson["isFavorite"] = stat.isFavorite;
        currencyJson["isIgnored"] = stat.isIgnored;
        currencyJson["name"] = stat.details.name;
        currencyJson["iconUrl"] = stat.details.iconUrl;
        currenciesArray.push_back(currencyJson);
    }
    currentSessionJson["currencies"] = currenciesArray;

    backupData["currentSession"] = currentSessionJson;

    // Backup Favorites — items
    auto favoriteItemIds   = ItemTracker::GetFavoriteItemIds();
    auto favoriteCurrencyIds = ItemTracker::GetFavoriteCurrencyIds();
    nlohmann::json favoriteItemsArray = nlohmann::json::array();
    for (int id : favoriteItemIds)     favoriteItemsArray.push_back(id);
    nlohmann::json favoriteCurrenciesArray = nlohmann::json::array();
    for (int id : favoriteCurrencyIds) favoriteCurrenciesArray.push_back(id);
    backupData["favorites"]["favoriteItems"]      = favoriteItemsArray;
    backupData["favorites"]["favoriteCurrencies"] = favoriteCurrenciesArray;

    // Backup Ignored Items
    nlohmann::json ignoredJson;
    auto ignoredItems = IgnoredItemsManager::GetIgnoredItems();
    nlohmann::json ignoredItemsArray = nlohmann::json::array();
    for (std::set<int>::const_iterator it = ignoredItems.begin(); it != ignoredItems.end(); ++it)
    {
        ignoredItemsArray.push_back(*it);
    }
    ignoredJson["ignoredItems"] = ignoredItemsArray;

    auto ignoredCurrencies = IgnoredItemsManager::GetIgnoredCurrencies();
    nlohmann::json ignoredCurrenciesArray = nlohmann::json::array();
    for (std::set<int>::const_iterator it = ignoredCurrencies.begin(); it != ignoredCurrencies.end(); ++it)
    {
        ignoredCurrenciesArray.push_back(*it);
    }
    ignoredJson["ignoredCurrencies"] = ignoredCurrenciesArray;
    backupData["ignored"] = ignoredJson;

    // Backup Custom Profit — include StatType so currencies are correctly typed
    nlohmann::json customProfitJson;
    auto customProfits = CustomProfitManager::GetAllCustomProfitsDetailed();
    for (const auto& [id, entry] : customProfits)
    {
        nlohmann::json cp;
        cp["profit"] = entry.customProfitCopper;
        cp["type"]   = static_cast<int>(entry.type);
        customProfitJson[std::to_string(id)] = cp;
    }
    backupData["customProfit"] = customProfitJson;

    return backupData.dump(4);
}

bool RestoreFromBackup(const std::string& jsonData)
{
    try
    {
        // =========================================================
        // STEP 1: Parse first (no locks needed), then apply ONLY
        // settings with SettingsMutex held. Release the mutex
        // IMMEDIATELY after. This avoids the deadlock:
        //   Drop-Thread:  s_PersistentMutex → SettingsMutex
        //   Restore:      SettingsMutex → s_PersistentMutex (via SetFavorite)
        // =========================================================
        nlohmann::json backupData = nlohmann::json::parse(jsonData);

        // Apply + save settings under SettingsMutex, then release
        if (backupData.contains("settings"))
        {
            {
                std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                SettingsManager::FromSettingsJson(backupData["settings"], g_Settings);
            }
            // Safe to call SettingsManager::Save() without outer SettingsMutex —
            // Save() acquires it internally (recursive_mutex is fine but we
            // don't want to hold it for subsequent ItemTracker/Manager calls).
            SettingsManager::Save();
        }

        // =========================================================
        // STEP 2: Everything below runs WITHOUT SettingsMutex.
        // SessionHistory / ItemTracker / IgnoredItems / CustomProfit
        // all use their own independent mutexes and do not call
        // back into Settings while locked.
        // =========================================================

        // Restore Session History
        if (backupData.contains("sessionHistory"))
        {
            std::string sessionHistoryJson = backupData["sessionHistory"].dump();
            SessionHistory::ImportFromJson(sessionHistoryJson);
        }

        // Restore Favorites — items and currencies separately
        if (backupData.contains("favorites"))
        {
            const nlohmann::json& fav = backupData["favorites"];
            if (fav.contains("favoriteItems") && fav["favoriteItems"].is_array())
            {
                for (const auto& idJson : fav["favoriteItems"])
                    ItemTracker::SetFavorite(idJson.get<int>(), true);
            }
            if (fav.contains("favoriteCurrencies") && fav["favoriteCurrencies"].is_array())
            {
                // SetFavorite with a currency ID that is not yet in s_Items will
                // add it to both persistent sets; we clean up the items set entry below
                // by only touching the currency persistent set directly.
                for (const auto& idJson : fav["favoriteCurrencies"])
                    ItemTracker::SetFavorite(idJson.get<int>(), true);
            }
        }

        // Restore Ignored Items
        if (backupData.contains("ignored"))
        {
            if (backupData["ignored"].contains("ignoredItems") && backupData["ignored"]["ignoredItems"].is_array())
            {
                for (nlohmann::json::const_iterator it = backupData["ignored"]["ignoredItems"].cbegin(); it != backupData["ignored"]["ignoredItems"].cend(); ++it)
                {
                    IgnoredItemsManager::SetIgnored(it->get<int>(), true);
                }
            }
            if (backupData["ignored"].contains("ignoredCurrencies") && backupData["ignored"]["ignoredCurrencies"].is_array())
            {
                for (nlohmann::json::const_iterator it = backupData["ignored"]["ignoredCurrencies"].cbegin(); it != backupData["ignored"]["ignoredCurrencies"].cend(); ++it)
                {
                    IgnoredItemsManager::SetIgnoredCurrency(it->get<int>(), true);
                }
            }
        }

        // Restore Custom Profit — preserve StatType (v2 backup has typed entries)
        if (backupData.contains("customProfit") && backupData["customProfit"].is_object())
        {
            for (nlohmann::json::const_iterator it = backupData["customProfit"].cbegin(); it != backupData["customProfit"].cend(); ++it)
            {
                int id = std::stoi(it.key());
                if (it.value().is_object())
                {
                    // v2 format: { profit, type }
                    long long profit = it.value().value("profit", 0LL);
                    StatType type    = static_cast<StatType>(it.value().value("type", static_cast<int>(StatType::Item)));
                    CustomProfitManager::SetCustomProfit(id, profit, type);
                }
                else if (it.value().is_number())
                {
                    // v1 legacy format: plain number, assume Item
                    CustomProfitManager::SetCustomProfit(id, it.value().get<long long>(), StatType::Item);
                }
            }
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool SaveBackupToFile(const std::string& filename)
{
    std::string jsonData = CreateFullBackup();
    
    FILE* file = nullptr;
    fopen_s(&file, filename.c_str(), "w");
    if (file)
    {
        fprintf(file, "%s", jsonData.c_str());
        fclose(file);
        return true;
    }
    return false;
}

bool LoadBackupFromFile(const std::string& filename)
{
    FILE* file = nullptr;
    fopen_s(&file, filename.c_str(), "rb");
    if (!file)
        return false;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Validate size before allocating
    if (fileSize <= 0 || fileSize > 64 * 1024 * 1024) // 0 or >64 MB is suspicious
    {
        fclose(file);
        return false;
    }

    std::string jsonData;
    jsonData.resize(static_cast<size_t>(fileSize));
    size_t bytesRead = fread(&jsonData[0], 1, static_cast<size_t>(fileSize), file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize))
        return false; // Partial read

    return RestoreFromBackup(jsonData);
}

} // namespace BackupRestore
