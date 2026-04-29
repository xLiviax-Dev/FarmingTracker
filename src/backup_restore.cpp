#include "backup_restore.h"
#include "settings.h"
#include "session_history.h"
#include "item_tracker.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "../include/nlohmann/json.hpp"
#include <chrono>
#include <map>

namespace BackupRestore
{
std::string CreateFullBackup()
{
    nlohmann::json backupData;
    backupData["version"] = 1;
    backupData["backupTimestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

    // Backup Settings
    nlohmann::json settingsJson;
    settingsJson["showMainWindow"] = g_Settings.showMainWindow;
    settingsJson["showMiniWindow"] = g_Settings.showMiniWindow;
    settingsJson["mainWindowClickThrough"] = g_Settings.mainWindowClickThrough;
    settingsJson["miniWindowClickThrough"] = g_Settings.miniWindowClickThrough;
    settingsJson["mainWindowOpacity"] = g_Settings.mainWindowOpacity;
    settingsJson["miniWindowOpacity"] = g_Settings.miniWindowOpacity;
    settingsJson["accentColorR"] = g_Settings.accentColorR;
    settingsJson["accentColorG"] = g_Settings.accentColorG;
    settingsJson["accentColorB"] = g_Settings.accentColorB;
    settingsJson["enableSessionHistory"] = g_Settings.enableSessionHistory;
    settingsJson["saveAllItemsInHistory"] = g_Settings.saveAllItemsInHistory;
    settingsJson["overwriteSessionHistory"] = g_Settings.overwriteSessionHistory;
    settingsJson["enableBestDropHighlight"] = g_Settings.enableBestDropHighlight;
    settingsJson["enableBestDropInMiniWindow"] = g_Settings.enableBestDropInMiniWindow;
    settingsJson["enableSummariesInProfitTab"] = g_Settings.enableSummariesInProfitTab;
    // Add more settings as needed
    backupData["settings"] = settingsJson;

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

    // Backup Favorites
    nlohmann::json favoritesJson;
    auto favoriteItems = ItemTracker::GetFavoriteItems();
    nlohmann::json favoriteItemsArray = nlohmann::json::array();
    for (std::map<int, ::Stat>::const_iterator it = favoriteItems.begin(); it != favoriteItems.end(); ++it)
    {
        favoriteItemsArray.push_back(it->first);
    }
    favoritesJson["favoriteItems"] = favoriteItemsArray;

    auto favoriteCurrencies = ItemTracker::GetFavoriteCurrencies();
    nlohmann::json favoriteCurrenciesArray = nlohmann::json::array();
    for (std::map<int, ::Stat>::const_iterator it = favoriteCurrencies.begin(); it != favoriteCurrencies.end(); ++it)
    {
        favoriteCurrenciesArray.push_back(it->first);
    }
    favoritesJson["favoriteCurrencies"] = favoriteCurrenciesArray;
    backupData["favorites"] = favoritesJson;

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

    // Backup Custom Profit
    nlohmann::json customProfitJson;
    auto customProfits = CustomProfitManager::GetAllCustomProfits();
    for (std::map<int, long long>::const_iterator it = customProfits.begin(); it != customProfits.end(); ++it)
    {
        customProfitJson[std::to_string(it->first)] = it->second;
    }
    backupData["customProfit"] = customProfitJson;

    return backupData.dump(4);
}

bool RestoreFromBackup(const std::string& jsonData)
{
    try
    {
        nlohmann::json backupData = nlohmann::json::parse(jsonData);

        // Restore Settings
        if (backupData.contains("settings"))
        {
            nlohmann::json settingsJson = backupData["settings"];
            g_Settings.showMainWindow = settingsJson.value("showMainWindow", true);
            g_Settings.showMiniWindow = settingsJson.value("showMiniWindow", true);
            g_Settings.mainWindowClickThrough = settingsJson.value("mainWindowClickThrough", false);
            g_Settings.miniWindowClickThrough = settingsJson.value("miniWindowClickThrough", false);
            g_Settings.mainWindowOpacity = settingsJson.value("mainWindowOpacity", 1.0f);
            g_Settings.miniWindowOpacity = settingsJson.value("miniWindowOpacity", 1.0f);
            g_Settings.accentColorR = settingsJson.value("accentColorR", 0.5f);
            g_Settings.accentColorG = settingsJson.value("accentColorG", 0.5f);
            g_Settings.accentColorB = settingsJson.value("accentColorB", 0.5f);
            g_Settings.enableSessionHistory = settingsJson.value("enableSessionHistory", true);
            g_Settings.saveAllItemsInHistory = settingsJson.value("saveAllItemsInHistory", false);
            g_Settings.overwriteSessionHistory = settingsJson.value("overwriteSessionHistory", true);
            g_Settings.enableBestDropHighlight = settingsJson.value("enableBestDropHighlight", true);
            g_Settings.enableBestDropInMiniWindow = settingsJson.value("enableBestDropInMiniWindow", false);
            g_Settings.enableSummariesInProfitTab = settingsJson.value("enableSummariesInProfitTab", true);
            SettingsManager::Save();
        }

        // Restore Session History
        if (backupData.contains("sessionHistory"))
        {
            std::string sessionHistoryJson = backupData["sessionHistory"].dump();
            SessionHistory::ImportFromJson(sessionHistoryJson);
        }

        // Restore Current Session (optional - may not want to restore current session)
        // This is commented out as restoring current session may not be desired
        /*
        if (backupData.contains("currentSession"))
        {
            // Implementation would go here if needed
        }
        */

        // Restore Favorites
        if (backupData.contains("favorites"))
        {
            if (backupData["favorites"].contains("favoriteItems") && backupData["favorites"]["favoriteItems"].is_array())
            {
                for (nlohmann::json::const_iterator it = backupData["favorites"]["favoriteItems"].cbegin(); it != backupData["favorites"]["favoriteItems"].cend(); ++it)
                {
                    ItemTracker::SetFavorite(it->get<int>(), true);
                }
            }
            // Note: SetFavoriteCurrency doesn't exist, skipping currency favorites restore
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

        // Restore Custom Profit
        if (backupData.contains("customProfit") && backupData["customProfit"].is_object())
        {
            for (nlohmann::json::const_iterator it = backupData["customProfit"].cbegin(); it != backupData["customProfit"].cend(); ++it)
            {
                int id = std::stoi(it.key());
                long long profit = it.value().get<long long>();
                CustomProfitManager::SetCustomProfit(id, profit);
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
    fopen_s(&file, filename.c_str(), "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        std::string jsonData;
        jsonData.resize(fileSize);
        fread(&jsonData[0], 1, fileSize, file);
        fclose(file);

        return RestoreFromBackup(jsonData);
    }
    return false;
}

} // namespace BackupRestore
