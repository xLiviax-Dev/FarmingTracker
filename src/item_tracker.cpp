#include "item_tracker.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "search_manager.h"
#include "settings.h"
#include "../include/nlohmann/json.hpp"

#include <mutex>
#include <chrono>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <sstream>
#include <deque>

using json = nlohmann::json;

static std::mutex s_Mutex;
static std::map<int, Stat> s_Items;
static std::map<int, Stat> s_Currencies;

static std::chrono::steady_clock::time_point s_SessionStart =
    std::chrono::steady_clock::now();
static std::mutex s_SessionStartMutex;

// Salvage Kit data structure
struct SalvageKitInfo
{
    int goldPrice;      // Price in copper
    int karmaPrice;     // Price in karma
    int uses;           // Number of uses
    bool infinite;      // If true, uses cost per use (not calculated)
    int costPerUse;     // For infinite kits only (in copper)
};

// Salvage Kits data (ID -> Info)
static std::map<int, SalvageKitInfo> s_SalvageKits = {
    // Infinite Salvage Kits
    {44602, {0, 0, 0, true, 3}},    // Copper-Fed Salvage-o-Matic: 3 copper per use
    {67027, {0, 0, 0, true, 60}},   // Silver-Fed Salvage-o-Matic: 60 copper per use
    {89409, {0, 0, 0, true, 30}},   // Runecrafter's Salvage-o-Matic: 30 copper per use

    // Basic Salvage Kits
    {23038, {32, 28, 15, false, 0}},   // Crude Salvage Kit: 32c, 28 Karma, 15 uses
    {23040, {88, 77, 25, false, 0}},   // Basic Salvage Kit: 88c, 77 Karma, 25 uses
    {23041, {288, 252, 25, false, 0}}, // Fine Salvage Kit: 2s 88c, 252 Karma, 25 uses
    {23042, {800, 2800, 25, false, 0}}, // Journeyman Salvage Kit: 8s, 2800 Karma, 25 uses
    {23043, {1536, 5600, 25, false, 0}}, // Master Salvage Kit: 15s 36c, 5600 Karma, 25 uses
    {20185, {2624, 8652, 250, false, 0}} // Mystic Salvage Kit: 26s 24c, 8652 Karma, 250 uses
};

// Moving Average for Profit per Hour (like drf.rs)
struct ProfitHistoryEntry
{
    std::chrono::steady_clock::time_point timestamp;
    long long profitPerHour;
};

static std::deque<ProfitHistoryEntry> s_ProfitHistory;
static std::chrono::steady_clock::time_point s_LastHistoryUpdate =
    std::chrono::steady_clock::now();
static std::mutex s_ProfitHistoryMutex; // Protect profit history access
static const int HISTORY_UPDATE_INTERVAL_SECONDS = 10; // Update every 10 seconds
static const int MAX_HISTORY_ENTRIES = 10; // Keep last 10 entries

static void UpdateOrInsert(std::map<int, Stat>& map,
                           int apiId, long long delta, StatType type)
{
    auto it = map.find(apiId);
    if (it != map.end())
        it->second.count += delta;
    else
    {
        Stat s;
        s.apiId = apiId;
        s.type  = type;
        s.count = delta;
        map[apiId] = s;
    }
}

void ItemTracker::AddDrop(const std::map<int, long long>& items,
                          const std::map<int, long long>& currencies)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    for (auto& [id, delta] : items)
        UpdateOrInsert(s_Items, id, delta, StatType::Item);

    for (auto& [id, delta] : currencies)
        UpdateOrInsert(s_Currencies, id, delta, StatType::Currency);
}

void ItemTracker::Reset()
{
    // Lock profit history FIRST to avoid deadlock with UpdateProfitHistory
    std::lock_guard<std::mutex> profitLock(s_ProfitHistoryMutex);
    s_ProfitHistory.clear();
    s_LastHistoryUpdate = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> sessionLock(s_SessionStartMutex);
    s_SessionStart = std::chrono::steady_clock::now();

    // Lock items/currencies LAST (after profit history)
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_Items.clear();
    s_Currencies.clear();
}

std::map<int, Stat> ItemTracker::GetItemsCopy()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_Items;
}

std::map<int, Stat> ItemTracker::GetCurrenciesCopy()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_Currencies;
}

std::chrono::seconds ItemTracker::GetSessionDuration()
{
    std::lock_guard<std::mutex> lock(s_SessionStartMutex);
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - s_SessionStart);
}

// === Advanced Features Implementation ===

// Stat method implementations
bool Stat::HasCustomProfit() const
{
    return CustomProfitManager::HasCustomProfit(apiId);
}

long long Stat::GetCustomProfit() const
{
    return CustomProfitManager::GetCustomProfit(apiId);
}

long long Stat::GetMaxProfit() const
{
    if (HasCustomProfit())
        return GetCustomProfit();
    
    if (IsCoin())
        return count;
    
    if (!details.loaded)
        return 0;
    
    long long tpProfit = ItemTracker::TpSellProceedsPerUnitCopper(details);
    long long vendorProfit = details.noSell ? 0 : details.vendorValue;
    return std::max(tpProfit, vendorProfit);
}

// Favorites System
void ItemTracker::SetFavorite(int apiId, bool favorite)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    // Update in items
    auto itemIt = s_Items.find(apiId);
    if (itemIt != s_Items.end())
        itemIt->second.isFavorite = favorite;
    
    // Update in currencies
    auto currencyIt = s_Currencies.find(apiId);
    if (currencyIt != s_Currencies.end())
        currencyIt->second.isFavorite = favorite;
}

bool ItemTracker::IsFavorite(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    auto itemIt = s_Items.find(apiId);
    if (itemIt != s_Items.end())
        return itemIt->second.isFavorite;
    
    auto currencyIt = s_Currencies.find(apiId);
    if (currencyIt != s_Currencies.end())
        return currencyIt->second.isFavorite;
    
    return false;
}

std::map<int, Stat> ItemTracker::GetFavoriteItems()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::map<int, Stat> favorites;
    
    for (const auto& [id, stat] : s_Items)
        if (stat.isFavorite)
            favorites[id] = stat;
    
    return favorites;
}

std::map<int, Stat> ItemTracker::GetFavoriteCurrencies()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::map<int, Stat> favorites;
    
    for (const auto& [id, stat] : s_Currencies)
        if (stat.isFavorite)
            favorites[id] = stat;
    
    return favorites;
}

// Ignored Items (delegates to IgnoredItemsManager)
bool ItemTracker::IsItemIgnored(int apiId)
{
    return IgnoredItemsManager::IsItemIgnored(apiId);
}

bool ItemTracker::IsCurrencyIgnored(int apiId)
{
    return IgnoredItemsManager::IsCurrencyIgnored(apiId);
}

// Advanced Filtering
bool ItemTracker::PassesFilter(const Stat& stat)
{
    // Count filter - but always allow coins like DRF does
    if (stat.IsCoin()) return true; // Always include coins in profit calculations
    
    // Ignored items filter
    if (stat.IsItem() && IsItemIgnored(stat.apiId) && !g_Settings.filterIgnored) return false;
    if (stat.IsCurrency() && IsCurrencyIgnored(stat.apiId) && !g_Settings.filterIgnored) return false;
    if (stat.IsItem() && !IsItemIgnored(stat.apiId) && !g_Settings.filterNotIgnored) return false;
    if (stat.IsCurrency() && !IsCurrencyIgnored(stat.apiId) && !g_Settings.filterNotIgnored) return false;

    // Favorite filter
    if (stat.IsItem() && stat.isFavorite && !g_Settings.filterFavorite) return false;
    if (stat.IsItem() && !stat.isFavorite && !g_Settings.filterNotFavorite) return false;
    if (stat.IsCurrency() && stat.isFavorite && !g_Settings.filterFavorite) return false;
    if (stat.IsCurrency() && !stat.isFavorite && !g_Settings.filterNotFavorite) return false;

    // API knowledge filter
    if (!stat.details.knownByApi && !g_Settings.filterUnknownByApi) return false;
    if (stat.details.knownByApi && !g_Settings.filterKnownByApi) return false;
    
    // Sell method filters
    if (stat.IsItem() && stat.details.loaded)
    {
        bool canSellToVendor = CanSellToVendor(stat.details);
        bool canSellOnTp = CanSellOnTp(stat.details);
        bool hasCustomProfit = stat.HasCustomProfit();

        if (canSellToVendor && !g_Settings.filterSellableToVendor) return false;
        if (canSellOnTp && !g_Settings.filterSellableOnTp) return false;
        if (hasCustomProfit && !g_Settings.filterCustomProfit) return false;

        // Account-bound filter
        if (stat.details.accountBound && !g_Settings.filterAccountBound) return false;
        if (!stat.details.accountBound && !g_Settings.filterNotAccountBound) return false;

        // NoSell filter
        if (stat.details.noSell && !g_Settings.filterNoSell) return false;
        if (!stat.details.noSell && !g_Settings.filterNotNoSell) return false;
    }
    
    // Item type filter
    if (stat.IsItem() && stat.details.loaded)
    {
        switch (stat.details.itemType)
        {
            case ItemType::Armor: if (!g_Settings.filterTypeArmor) return false; break;
            case ItemType::Weapon: if (!g_Settings.filterTypeWeapon) return false; break;
            case ItemType::Trinket: if (!g_Settings.filterTypeTrinket) return false; break;
            case ItemType::Gizmo: if (!g_Settings.filterTypeGizmo) return false; break;
            case ItemType::CraftingMaterial: if (!g_Settings.filterTypeCraftingMaterial) return false; break;
            case ItemType::Consumable: if (!g_Settings.filterTypeConsumable) return false; break;
            case ItemType::GatheringTool: if (!g_Settings.filterTypeGatheringTool) return false; break;
            case ItemType::Bag: if (!g_Settings.filterTypeBag) return false; break;
            case ItemType::Container: if (!g_Settings.filterTypeContainer) return false; break;
            case ItemType::MiniPet: if (!g_Settings.filterTypeMiniPet) return false; break;
            case ItemType::GizmoContainer: if (!g_Settings.filterTypeGizmoContainer) return false; break;
            case ItemType::Backpack: if (!g_Settings.filterTypeBackpack) return false; break;
            case ItemType::UpgradeComponent: if (!g_Settings.filterTypeUpgradeComponent) return false; break;
            case ItemType::Tool: if (!g_Settings.filterTypeTool) return false; break;
            case ItemType::Trophy: if (!g_Settings.filterTypeTrophy) return false; break;
            case ItemType::Unlock: if (!g_Settings.filterTypeUnlock) return false; break;
            default: break;
        }
    }
    
    // Currency filter
    if (stat.IsCurrency())
    {
        switch (stat.apiId)
        {
            case 2: if (!g_Settings.filterKarma) return false; break;
            case 3: if (!g_Settings.filterLaurel) return false; break;
            case 4: if (!g_Settings.filterGem) return false; break;
            case 7: if (!g_Settings.filterFractalRelic) return false; break;
            case 15: if (!g_Settings.filterBadgeOfHonor) return false; break;
            case 16: if (!g_Settings.filterGuildCommendation) return false; break;
            case 18: if (!g_Settings.filterTransmutationCharge) return false; break;
            default: break;
        }
    }

    // Price range filter
    if (stat.IsItem() && stat.details.loaded)
    {
        long long pricePerUnit = GetStatProfit(stat) / (stat.count != 0 ? stat.count : 1);
        if (g_Settings.filterMinPrice > 0 && pricePerUnit < g_Settings.filterMinPrice) return false;
        if (g_Settings.filterMaxPrice > 0 && pricePerUnit > g_Settings.filterMaxPrice) return false;
    }

    // Quantity range filter
    if (g_Settings.filterMinQuantity > 0 && std::abs(stat.count) < g_Settings.filterMinQuantity) return false;
    if (g_Settings.filterMaxQuantity > 0 && std::abs(stat.count) > g_Settings.filterMaxQuantity) return false;

    return true;
}

std::map<int, Stat> ItemTracker::GetFilteredItems()
{
    std::map<int, Stat> filtered;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        for (const auto& [id, stat] : s_Items)
            if (PassesFilter(stat))
                filtered[id] = stat;
    }
    return filtered;
}

std::map<int, Stat> ItemTracker::GetFilteredCurrencies()
{
    auto currencies = GetCurrenciesCopy();
    std::map<int, Stat> filtered;
    for (auto& [id, st] : currencies)
    {
        // Filter out infinite salvage kits (they appear as ID 1 in DRF, but we track them separately)
        // IDs: 44602 (Copper-Fed), 67027 (Silver-Fed), 89409 (Runecrafter's)
        if (id == 44602 || id == 67027 || id == 89409)
            continue;

        if (PassesFilter(st))
            filtered[id] = st;
    }
    return filtered;
}

// Search functionality
std::map<int, Stat> ItemTracker::GetSearchedItems(const std::string& searchTerm)
{
    auto items = GetFilteredItems();
    if (searchTerm.empty())
        return items;
    
    std::map<int, Stat> searched;
    for (const auto& [id, stat] : items)
        if (SearchManager::MatchesSearch(stat.details.name, searchTerm))
            searched[id] = stat;
    
    return searched;
}

std::map<int, Stat> ItemTracker::GetSearchedCurrencies(const std::string& searchTerm)
{
    auto currencies = GetFilteredCurrencies();
    if (searchTerm.empty())
        return currencies;
    
    std::map<int, Stat> searched;
    for (const auto& [id, stat] : currencies)
        if (SearchManager::MatchesSearchCurrency(stat.details.name, searchTerm))
            searched[id] = stat;
    
    return searched;
}

// Multi-Sort implementation
std::vector<std::pair<int, Stat>> ItemTracker::GetSortedItems(SortMode mode, bool)
{
    auto items = GetFilteredItems();
    std::vector<std::pair<int, Stat>> sorted(items.begin(), items.end());
    
    std::sort(sorted.begin(), sorted.end(), [mode](const auto& a, const auto& b) {
        const auto& statA = a.second;
        const auto& statB = b.second;
        
        switch (mode)
        {
            case SortMode::CountDesc:
                return std::abs(statA.count) > std::abs(statB.count);
            case SortMode::CountAsc:
                return std::abs(statA.count) < std::abs(statB.count);
            case SortMode::ApiIdAsc:
                return statA.apiId < statB.apiId;
            case SortMode::ApiIdDesc:
                return statA.apiId > statB.apiId;
            case SortMode::NameAZ:
                return statA.details.name < statB.details.name;
            case SortMode::NameZA:
                return statA.details.name > statB.details.name;
            case SortMode::ProfitDesc:
                return statA.GetMaxProfit() > statB.GetMaxProfit();
            case SortMode::ProfitAsc:
                return statA.GetMaxProfit() < statB.GetMaxProfit();
            case SortMode::RarityDesc:
                return RarityRank(statA.details.rarity) > RarityRank(statB.details.rarity);
            case SortMode::RarityAsc:
                return RarityRank(statA.details.rarity) < RarityRank(statB.details.rarity);
            case SortMode::TypeAZ:
                return static_cast<int>(statA.details.itemType) < static_cast<int>(statB.details.itemType);
            case SortMode::TypeZA:
                return static_cast<int>(statA.details.itemType) > static_cast<int>(statB.details.itemType);
            default:
                return false;
        }
    });
    
    return sorted;
}

std::vector<std::pair<int, Stat>> ItemTracker::GetSortedCurrencies(SortMode mode, bool)
{
    auto currencies = GetFilteredCurrencies();
    std::vector<std::pair<int, Stat>> sorted(currencies.begin(), currencies.end());
    
    std::sort(sorted.begin(), sorted.end(), [mode](const auto& a, const auto& b) {
        const auto& statA = a.second;
        const auto& statB = b.second;
        
        switch (mode)
        {
            case SortMode::CountDesc:
                return std::abs(statA.count) > std::abs(statB.count);
            case SortMode::CountAsc:
                return std::abs(statA.count) < std::abs(statB.count);
            case SortMode::ApiIdAsc:
                return statA.apiId < statB.apiId;
            case SortMode::ApiIdDesc:
                return statA.apiId > statB.apiId;
            case SortMode::NameAZ:
                return statA.details.name < statB.details.name;
            case SortMode::NameZA:
                return statA.details.name > statB.details.name;
            case SortMode::ProfitDesc:
                return statA.GetMaxProfit() > statB.GetMaxProfit();
            case SortMode::ProfitAsc:
                return statA.GetMaxProfit() < statB.GetMaxProfit();
            default:
                return false;
        }
    });
    
    return sorted;
}

// Custom Profit Integration
long long ItemTracker::GetStatProfit(const Stat& stat)
{
    // Use custom profit if set
    if (stat.HasCustomProfit())
        return CustomProfitManager::GetCustomProfit(stat.apiId) * stat.count;

    // Use MAX profit from Vendor or TP Sell (TP Buy is buy price, not profit)
    if (stat.IsCoin())
        return stat.count; // Coins are counted directly

    // Salvage kits are not calculated as cost, just show negative count
    if (s_SalvageKits.find(stat.apiId) != s_SalvageKits.end())
        return 0; // Salvage kits only show usage count, no profit calculation

    // Calculate all possible sell values per unit
    long long vendorPrice = stat.details.vendorValue;
    long long tpSellPrice = static_cast<long long>(stat.details.tpSellPrice * 85.0 / 100.0); // 15% fee on sell

    // Account-bound items can only be sold to vendor (if vendorValue > 0 and not noSell)
    // They cannot be sold on TP
    long long maxPrice = 0;
    if (vendorPrice > 0 && !stat.details.noSell) maxPrice = vendorPrice;

    // For non-account-bound items, also consider TP sell price
    if (!stat.details.accountBound)
    {
        if (tpSellPrice > maxPrice) maxPrice = tpSellPrice;
    }

    // If no price is available, item is not tradeable
    if (maxPrice == 0)
        return 0;

    return maxPrice * stat.count;
}

long long ItemTracker::GetStatProfitPerHour(const Stat& stat, std::chrono::seconds sessionDuration)
{
    long long totalProfit = GetStatProfit(stat);
    
    if (totalProfit == 0)
        return 0;
    
    double sessionSeconds = static_cast<double>(sessionDuration.count());
    if (sessionSeconds < 1.0) // session just started - avoid inflated values
        return 0;
    
    double hours = sessionSeconds / 3600.0;
    return static_cast<long long>(totalProfit / hours);
}

long long ItemTracker::GetTotalProfitPerHour(std::chrono::seconds sessionDuration)
{
    // Update profit history first
    UpdateProfitHistory();

    // Use moving average immediately if we have history
    if (!s_ProfitHistory.empty())
        return GetMovingAverageProfitPerHour();

    // Fallback: simple calculation if no history yet
    long long totalProfit = CalcTotalCustomProfit();

    double sessionSeconds = static_cast<double>(sessionDuration.count());
    if (sessionSeconds < 1.0) // session just started - avoid inflated values
        return 0;

    double hours = sessionSeconds / 3600.0;
    if (hours < 0.001) // avoid division by very small numbers
        return 0;

    long long currentProfitPerHour = static_cast<long long>(totalProfit / hours);

    return currentProfitPerHour;
}

long long ItemTracker::GetTpSellProfitPerHour(std::chrono::seconds sessionDuration)
{
    long long tpSellProfit = CalcTotalTpSellProfit();

    double sessionSeconds = static_cast<double>(sessionDuration.count());
    if (sessionSeconds < 1.0)
        return 0;

    double hours = sessionSeconds / 3600.0;
    if (hours < 0.001)
        return 0;

    long long tpSellPerHour = static_cast<long long>(tpSellProfit / hours);

    return tpSellPerHour;
}

long long ItemTracker::GetOpportunityCostProfit()
{
    // Opportunity Cost = TP Sell Price - Actual Profit
    // Calculate TP Sell Profit (max possible)
    long long tpSellProfit = CalcTotalTpSellProfit();
    long long actualProfit = CalcTotalCustomProfit();

    // Opportunity Cost is the difference
    return tpSellProfit - actualProfit;
}

long long ItemTracker::GetOpportunityCostProfitPerHour(std::chrono::seconds sessionDuration)
{
    long long opportunityCost = GetOpportunityCostProfit();

    double sessionSeconds = static_cast<double>(sessionDuration.count());
    if (sessionSeconds < 1.0)
        return 0;

    double hours = sessionSeconds / 3600.0;
    if (hours < 0.001)
        return 0;

    long long opportunityCostPerHour = static_cast<long long>(opportunityCost / hours);

    return opportunityCostPerHour;
}

std::string ItemTracker::ExportToJson()
{
    nlohmann::json exportData;
    exportData["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    exportData["sessionDuration"] = GetSessionDuration().count();
    exportData["totalProfit"] = CalcTotalCustomProfit();
    exportData["totalTpSellProfit"] = CalcTotalTpSellProfit();
    exportData["totalVendorProfit"] = CalcTotalVendorProfit();

    nlohmann::json itemsArray = nlohmann::json::array();
    auto items = GetItemsCopy();
    for (auto& [id, stat] : items)
    {
        nlohmann::json item;
        item["apiId"] = id;
        item["count"] = stat.count;
        item["name"] = stat.details.loaded ? stat.details.name : "Unknown";
        item["rarity"] = stat.details.loaded ? stat.details.rarity : "Unknown";
        item["type"] = stat.details.loaded ? static_cast<int>(stat.details.itemType) : 0;
        item["profit"] = GetStatProfit(stat);
        item["isFavorite"] = stat.isFavorite;
        item["isIgnored"] = stat.isIgnored;
        item["vendorValue"] = stat.details.loaded ? stat.details.vendorValue : 0;
        item["tpSellPrice"] = stat.details.loaded ? stat.details.tpSellPrice : 0;
        item["tpBuyPrice"] = stat.details.loaded ? stat.details.tpBuyPrice : 0;
        itemsArray.push_back(item);
    }
    exportData["items"] = itemsArray;

    nlohmann::json currenciesArray = nlohmann::json::array();
    auto currencies = GetCurrenciesCopy();
    for (auto& [id, stat] : currencies)
    {
        nlohmann::json currency;
        currency["apiId"] = id;
        currency["count"] = stat.count;
        currency["name"] = stat.details.loaded ? stat.details.name : "Unknown";
        currency["isFavorite"] = stat.isFavorite;
        currency["isIgnored"] = stat.isIgnored;
        currenciesArray.push_back(currency);
    }
    exportData["currencies"] = currenciesArray;

    return exportData.dump(4);
}

std::string ItemTracker::ExportToCsv()
{
    std::stringstream csv;
    csv << "Type,API ID,Name,Count,Profit,Rarity,Is Favorite,Is Ignored,Vendor Value,TP Sell Price,TP Buy Price\n";

    auto items = GetItemsCopy();
    for (auto& [id, stat] : items)
    {
        std::string name = stat.details.loaded ? stat.details.name : "Unknown";
        std::string rarity = stat.details.loaded ? stat.details.rarity : "Unknown";
        long long profit = GetStatProfit(stat);
        int vendorValue = stat.details.loaded ? stat.details.vendorValue : 0;
        int tpSellPrice = stat.details.loaded ? stat.details.tpSellPrice : 0;
        int tpBuyPrice = stat.details.loaded ? stat.details.tpBuyPrice : 0;

        csv << "Item," << id << ",\"" << name << "\"," << stat.count << "," << profit << ",\""
            << rarity << "\"," << (stat.isFavorite ? "Yes" : "No") << ","
            << (stat.isIgnored ? "Yes" : "No") << "," << vendorValue << ","
            << tpSellPrice << "," << tpBuyPrice << "\n";
    }

    auto currencies = GetCurrenciesCopy();
    for (auto& [id, stat] : currencies)
    {
        std::string name = stat.details.loaded ? stat.details.name : "Unknown";
        csv << "Currency," << id << ",\"" << name << "\"," << stat.count << ",0,N/A,"
            << (stat.isFavorite ? "Yes" : "No") << ","
            << (stat.isIgnored ? "Yes" : "No") << ",0,0,0\n";
    }

    return csv.str();
}

long long ItemTracker::CalcTotalCustomProfit()
{
    // Copy data first to avoid holding mutex while calling PassesFilter
    std::map<int, Stat> itemsCopy, currenciesCopy;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        itemsCopy = s_Items;
        currenciesCopy = s_Currencies;
    }
    
    long long total = 0;
    
    for (const auto& [id, stat] : itemsCopy)
        if (PassesFilter(stat))
            total += GetStatProfit(stat);
    
    for (const auto& [id, stat] : currenciesCopy)
        if (PassesFilter(stat))
            total += GetStatProfit(stat);
    
    return total;
}

void ItemTracker::UpdateProfitHistory()
{
    auto now = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(s_ProfitHistoryMutex);
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - s_LastHistoryUpdate);
    
    if (duration.count() < HISTORY_UPDATE_INTERVAL_SECONDS)
        return; // Not time to update yet
    
    s_LastHistoryUpdate = now;
    
    // Copy data first to avoid deadlock with CalcTotalCustomProfit
    auto sessionDuration = GetSessionDuration();
    
    if (sessionDuration.count() < 1)
        return;
    
    // Calculate profit outside of lock to avoid deadlock
    long long totalProfit = CalcTotalCustomProfit();
    
    double hours = static_cast<double>(sessionDuration.count()) / 3600.0;
    long long currentProfitPerHour = static_cast<long long>(totalProfit / hours);
    
    // Add new entry
    ProfitHistoryEntry entry;
    entry.timestamp = now;
    entry.profitPerHour = currentProfitPerHour;
    
    s_ProfitHistory.push_back(entry);

    // Keep only last MAX_HISTORY_ENTRIES entries
    while (s_ProfitHistory.size() > MAX_HISTORY_ENTRIES)
        s_ProfitHistory.pop_front();
}

long long ItemTracker::GetMovingAverageProfitPerHour()
{
    std::lock_guard<std::mutex> lock(s_ProfitHistoryMutex);
    if (s_ProfitHistory.empty())
        return 0;
    
    long long sum = 0;
    for (const auto& entry : s_ProfitHistory)
        sum += entry.profitPerHour;
    
    return sum / static_cast<long long>(s_ProfitHistory.size());
}

long long ItemTracker::TpSellProceedsPerUnitCopper(const ApiDetails& d)
{
    if (d.tpSellPrice <= 0) return 0;
    // Apply 15% TP fee (85/100)
    return static_cast<long long>(d.tpSellPrice * 85.0 / 100.0);
}

long long ItemTracker::TpBuyProceedsPerUnitCopper(const ApiDetails& d)
{
    if (d.tpBuyPrice <= 0) return 0;
    // TP Instant Sell also has 15% fee (5% listing + 10% exchange)
    return static_cast<long long>(d.tpBuyPrice * 85.0 / 100.0);
}

bool ItemTracker::CanSellOnTp(const ApiDetails& d)
{
    // Account-bound items cannot be sold on TP
    if (d.accountBound)
        return false;

    return TpSellProceedsPerUnitCopper(d) > 0 || TpBuyProceedsPerUnitCopper(d) > 0;
}

bool ItemTracker::CanSellToVendor(const ApiDetails& d)
{
    return d.vendorValue > 0 && !d.noSell;
}

long long ItemTracker::CalcTotalTpSellProfit()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    long long total = 0;
    for (auto& [id, stat] : s_Items)
    {
        if (!stat.details.loaded) continue;
        long long per = TpSellProceedsPerUnitCopper(stat.details);
        if (per > 0) total += stat.count * per;
    }
    return total;
}

long long ItemTracker::CalcTotalTpInstantProfit()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    long long total = 0;
    for (auto& [id, stat] : s_Items)
    {
        if (!stat.details.loaded) continue;
        long long per = TpBuyProceedsPerUnitCopper(stat.details);
        if (per > 0) total += stat.count * per;
    }
    return total;
}

long long ItemTracker::CalcTotalVendorProfit()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    long long total = 0;
    for (auto& [id, stat] : s_Items)
    {
        if (!stat.details.loaded) continue;
        if (CanSellToVendor(stat.details))
            total += stat.count * stat.details.vendorValue;
    }
    return total;
}

int ItemTracker::RarityRank(const std::string& rarity)
{
    static const char* order[] = {
        "Junk", "Basic", "Fine", "Masterwork", "Rare", "Exotic", "Ascended", "Legendary"
    };
    for (int i = 0; i < 8; ++i)
    {
        if (rarity == order[i]) return i;
    }
    return 0;
}

std::vector<int> ItemTracker::CollectPendingItemIds()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::set<int> ids;
    for (auto& [id, st] : s_Items)
    {
        if (st.count == 0) continue;
        // Only load data for items that are not loaded yet
        if (!st.details.loaded)
            ids.insert(id);
    }
    // Also load data for salvage kits that are tracked as currencies
    for (auto& [id, st] : s_Currencies)
    {
        if (st.count == 0) continue;
        if (s_SalvageKits.find(id) != s_SalvageKits.end() && !st.details.loaded)
            ids.insert(id);
    }
    return std::vector<int>(ids.begin(), ids.end());
}

bool ItemTracker::NeedCurrencyTable()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (auto& [id, st] : s_Currencies)
    {
        if (st.count == 0 || id == 1) continue;
        if (!st.details.loaded) return true;
    }
    return false;
}

static bool JsonHasNoSell(const nlohmann::json& item)
{
    if (!item.contains("flags") || !item["flags"].is_array()) return false;
    for (auto& f : item["flags"])
    {
        if (f.is_string())
        {
            std::string flag = f.get<std::string>();
            // AccountBound and SoulbindOnAcquire block TP sale only, not vendor sale
            if (flag == "NoSell")
                return true;
        }
    }
    return false;
}

static bool JsonHasAccountBound(const nlohmann::json& item)
{
    if (!item.contains("flags") || !item["flags"].is_array()) return false;
    for (auto& f : item["flags"])
    {
        if (f.is_string())
        {
            std::string flag = f.get<std::string>();
            if (flag == "AccountBound" || flag == "SoulbindOnAcquire")
                return true;
        }
    }
    return false;
}

static std::string BuildIconUrl(const std::string& iconField)
{
    if (iconField.empty()) return {};
    if (iconField.find("http://") == 0 || iconField.find("https://") == 0)
        return iconField;
    if (iconField[0] == '/')
        return "https://render.guildwars2.com" + iconField;
    return "https://render.guildwars2.com/" + iconField;
}

void ItemTracker::ApplyItemsFromApi(const json& itemsArray, const json& pricesArray)
{
    if (!itemsArray.is_array() || !pricesArray.is_array()) return;

    std::lock_guard<std::mutex> lock(s_Mutex);

    for (auto& item : itemsArray)
    {
        if (!item.contains("id")) continue;
        int id = item["id"].get<int>();

        // Check if this is an item or a salvage kit tracked as currency
        auto it = s_Items.find(id);
        Stat* st = nullptr;
        if (it != s_Items.end())
        {
            st = &it->second;
        }
        else if (s_SalvageKits.find(id) != s_SalvageKits.end())
        {
            // Salvage kit tracked as currency
            auto curIt = s_Currencies.find(id);
            if (curIt != s_Currencies.end())
                st = &curIt->second;
        }

        if (!st) continue;

        st->details.name        = item.value("name", "");
        st->details.vendorValue = item.value("vendor_value", 0);
        st->details.rarity      = item.value("rarity", std::string());
        st->details.noSell      = JsonHasNoSell(item);
        st->details.accountBound = JsonHasAccountBound(item);
        if (item.contains("icon") && item["icon"].is_string())
            st->details.iconUrl = BuildIconUrl(item["icon"].get<std::string>());
        st->details.loaded      = true;
        st->details.knownByApi  = true;

        // Set item type from API data
        if (item.contains("type") && item["type"].is_string())
        {
            std::string t = item["type"].get<std::string>();
            if (t == "Armor") st->details.itemType = ItemType::Armor;
            else if (t == "Weapon") st->details.itemType = ItemType::Weapon;
            else if (t == "Trinket") st->details.itemType = ItemType::Trinket;
            else if (t == "Gizmo") st->details.itemType = ItemType::Gizmo;
            else if (t == "CraftingMaterial") st->details.itemType = ItemType::CraftingMaterial;
            else if (t == "Consumable") st->details.itemType = ItemType::Consumable;
            else if (t == "GatheringTool") st->details.itemType = ItemType::GatheringTool;
            else if (t == "Bag") st->details.itemType = ItemType::Bag;
            else if (t == "Container") st->details.itemType = ItemType::Container;
            else if (t == "MiniPet") st->details.itemType = ItemType::MiniPet;
            else if (t == "GizmoContainer") st->details.itemType = ItemType::GizmoContainer;
            else if (t == "Backpack") st->details.itemType = ItemType::Backpack;
            else if (t == "UpgradeComponent") st->details.itemType = ItemType::UpgradeComponent;
            else if (t == "Tool") st->details.itemType = ItemType::Tool;
            else if (t == "Trophy") st->details.itemType = ItemType::Trophy;
            else if (t == "Unlock") st->details.itemType = ItemType::Unlock;
        }
    }

    for (auto& pr : pricesArray)
    {
        if (!pr.contains("id")) continue;
        int id = pr["id"].get<int>();

        // Check if this is an item or a salvage kit tracked as currency
        auto it = s_Items.find(id);
        Stat* st = nullptr;
        if (it != s_Items.end())
        {
            st = &it->second;
        }
        else if (s_SalvageKits.find(id) != s_SalvageKits.end())
        {
            // Salvage kit tracked as currency
            auto curIt = s_Currencies.find(id);
            if (curIt != s_Currencies.end())
                st = &curIt->second;
        }

        if (!st) continue;

        if (pr.contains("sells") && pr["sells"].contains("unit_price"))
            st->details.tpSellPrice = pr["sells"]["unit_price"].get<int>();
        if (pr.contains("buys") && pr["buys"].contains("unit_price"))
            st->details.tpBuyPrice = pr["buys"]["unit_price"].get<int>();
    }

    // Mark items that were not returned by the API as still loaded (don't reset them)
    // This preserves details for items that are no longer in inventory (salvaged/destroyed)
}

void ItemTracker::ApplyCurrencyTable(const json& currenciesArray)
{
    if (!currenciesArray.is_array()) return;

    std::lock_guard<std::mutex> lock(s_Mutex);

    for (auto& c : currenciesArray)
    {
        if (!c.contains("id")) continue;
        int id = c["id"].get<int>();
        auto it = s_Currencies.find(id);
        if (it == s_Currencies.end()) continue;

        it->second.details.name = c.value("name", "");
        if (c.contains("icon") && c["icon"].is_string())
            it->second.details.iconUrl = BuildIconUrl(c["icon"].get<std::string>());
        it->second.details.loaded = true;
    }
}

ItemTracker::CoinSplit ItemTracker::SplitCoin(long long copperValue)
{
    CoinSplit result{};
    result.negative = copperValue < 0;
    long long abs_val = std::abs(copperValue);
    result.gold   = static_cast<int>(abs_val / 10000);
    result.silver = static_cast<int>((abs_val % 10000) / 100);
    result.copper = static_cast<int>(abs_val % 100);
    return result;
}
