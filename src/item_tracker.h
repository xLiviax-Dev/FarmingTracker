#pragma once
#include <map>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <functional>

#include "../include/nlohmann/json.hpp"

enum class StatType { Item, Currency };

// Item Type enum
enum class ItemType {
    Unknown, Armor, Weapon, Trinket, Gizmo, CraftingMaterial, Consumable,
    GatheringTool, Bag, Container, MiniPet, GizmoContainer, Backpack,
    UpgradeComponent, Tool, Trophy, Unlock
};

struct ApiDetails
{
    std::string name;
    std::string description;
    std::string iconUrl;
    int         vendorValue  = 0;
    int         tpBuyPrice   = 0;
    int         tpSellPrice  = 0;
    bool        noSell       = false;
    bool        accountBound = false;
    std::string rarity;
    bool        loaded       = false;
    ItemType    itemType     = ItemType::Unknown;
    bool        knownByApi   = false;
};

struct Stat
{
    int         apiId  = 0;
    StatType    type   = StatType::Item;
    long long   count  = 0;
    ApiDetails  details;
    bool        isFavorite = false;
    bool        isIgnored  = false;

    bool IsItem()     const { return type == StatType::Item; }
    bool IsCurrency() const { return type == StatType::Currency; }
    bool IsCoin()     const { return type == StatType::Currency && apiId == 1; }
    bool HasCustomProfit() const;
    long long GetCustomProfit() const;
    long long GetMaxProfit() const; // Custom or calculated
};

namespace ItemTracker
{
    // === Core Functions ===
    void Update(const std::map<int, int>& items, const std::map<int, int>& currencies);
    void AddDrop(const std::map<int, long long>& items, const std::map<int, long long>& currencies);
    void Reset();
    void SaveCurrentSession();
    void ClearPersistedData(const char* addonDir);

    std::map<int, Stat> GetItemsCopy();
    std::map<int, Stat> GetCurrenciesCopy();

    std::chrono::seconds GetSessionDuration();

    // Totals: TP after 15% fee; vendor only if vendor value and not NoSell
    long long CalcTotalTpSellProfit();
    long long CalcTotalTpInstantProfit(); // Best Bid (Instant Sell)
    long long CalcTotalVendorProfit();
    long long CalcTotalCustomProfit(); // New: Custom profit calculation

    long long TpSellProceedsPerUnitCopper(const ApiDetails& d);
    long long TpBuyProceedsPerUnitCopper(const ApiDetails& d);
    bool      CanSellOnTp(const ApiDetails& d);
    bool      CanSellToVendor(const ApiDetails& d);

    int RarityRank(const std::string& rarity);

    std::vector<int> CollectPendingItemIds();
    bool             NeedCurrencyTable();

    void ApplyItemsFromApi(const nlohmann::json& itemsArray, const nlohmann::json& pricesArray);
    void ApplyCurrencyTable(const nlohmann::json& currenciesArray);

    struct CoinSplit { int gold; int silver; int copper; bool negative; };
    CoinSplit SplitCoin(long long copperValue);

    // === Advanced Features ===
    
    // Favorites System
    void SetFavorite(int apiId, bool favorite);
    bool IsFavorite(int apiId);
    std::map<int, Stat> GetFavoriteItems();
    std::map<int, Stat> GetFavoriteCurrencies();

    // Ignored Items (delegates to IgnoredItemsManager)
    bool IsItemIgnored(int apiId);
    bool IsCurrencyIgnored(int apiId);

    // Advanced Filtering
    std::map<int, Stat> GetFilteredItems();
    std::map<int, Stat> GetFilteredCurrencies();
    bool PassesFilter(const Stat& stat);

    // Search
    std::map<int, Stat> GetSearchedItems(const std::string& searchTerm);
    std::map<int, Stat> GetSearchedCurrencies(const std::string& searchTerm);

    // Multi-Sort
    enum SortMode {
        PriceDesc, PriceAsc, CountDesc, CountAsc, NameAZ, NameZA,
        ProfitDesc, ProfitAsc, RarityDesc, RarityAsc, TypeAZ, TypeZA
    };
    std::vector<std::pair<int, Stat>> GetSortedItems(SortMode mode, bool secondary = false);
    std::vector<std::pair<int, Stat>> GetSortedCurrencies(SortMode mode, bool secondary = false);

    // Custom Profit Integration
    long long GetStatProfit(const Stat& stat);
    long long GetStatProfitPerHour(const Stat& stat, std::chrono::seconds sessionDuration);
    long long GetTotalProfitPerHour(std::chrono::seconds sessionDuration);
    long long GetTpSellProfitPerHour(std::chrono::seconds sessionDuration);

    // Opportunity Cost (Trading Details)
    long long GetOpportunityCostProfit();
    long long GetOpportunityCostProfitPerHour(std::chrono::seconds sessionDuration);
    // GetOpportunityCostTradingProfit removed - not implemented
    
    // Moving Average for Profit per Hour (like drf.rs)
    void UpdateProfitHistory();
    long long GetMovingAverageProfitPerHour();

    // Sparkline Data
    std::vector<std::pair<std::chrono::steady_clock::time_point, long long>> GetProfitHistory();

    // Best Drop
    std::pair<int, Stat> GetBestDrop();

    // Export Functions
    std::string ExportToJson();
    std::string ExportToCsv();

    // Persistence Functions
    void SaveData(const char* addonDir);
    void LoadData(const char* addonDir);
}
