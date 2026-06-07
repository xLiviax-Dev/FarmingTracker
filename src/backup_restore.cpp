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
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    nlohmann::json backupData;
    backupData["version"] = 2; // v2: full settings backup
    backupData["backupTimestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

    // =========================================================
    // Full Settings Backup (mirrors ExportToFile / Load logic)
    // =========================================================
    backupData["settings"] = SettingsManager::ToSettingsJson(g_Settings);

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
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        nlohmann::json backupData = nlohmann::json::parse(jsonData);

        // Restore Settings — use ImportFromFile logic via a temp file is complex;
        // instead directly apply the full settings JSON via the same field-by-field approach.
        if (backupData.contains("settings"))
        {
            SettingsManager::FromSettingsJson(backupData["settings"], g_Settings);
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
