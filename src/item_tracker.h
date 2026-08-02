#pragma once
#include <map>
#include <set>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <functional>
#include <cstdint>

#include "../include/nlohmann/json.hpp"
#include "session_history.h"

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
    int         level        = 0;
    bool        loaded       = false;
    ItemType    itemType     = ItemType::Unknown;
    std::string upgradeComponentType; // e.g., "Infusion", "Rune", "Sigil", etc.
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
    bool        notificationPending = false;
    int         lastMagicFind = -1;

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
    void SetMagicFind(int magicFind);
    int  GetMagicFind();
    void SetCurrentMapId(int mapId); // called from OnMumbleIdentityUpdated
    void Reset();
    void SafeReset(); // Reset session data while preserving favorites, ignored items and custom profits
    void SaveCurrentSession();       // Async (BackgroundJobs) if available, sync fallback
    void SaveCurrentSessionSync();   // Explicit blocking (used at DLL unload for data integrity)
    void ClearPersistedData(const char* addonDir);

    std::map<int, Stat> GetItemsCopy();
    std::map<int, Stat> GetCurrenciesCopy();
    
    Stat GetItemStat(int itemId);
    Stat GetCurrencyStat(int currencyId);

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

    void ApplyItemsFromApi(const std::vector<int>& requestedIds, const nlohmann::json& itemsArray, const nlohmann::json& pricesArray);
    void ApplyCurrencyTable(const nlohmann::json& currenciesArray);
    void ClearItemDetails(); // Clear all item details to force reload on language change
    void ForceReloadAll(); // New: Mark all items as not loaded to force re-fetch

    struct CoinSplit { int gold; int silver; int copper; bool negative; };
    CoinSplit SplitCoin(long long copperValue);

    // === Advanced Features ===
    
    // Favorites System
    void SetFavorite(int apiId, bool favorite);
    std::vector<std::pair<int, Stat>> GetFavoriteItems();
    std::vector<std::pair<int, Stat>> GetFavoriteCurrencies();

    // === NEW: "View" Getters (0-copy hot path) ===
    // Return a const reference to a thread-local snapshot of cached results.
    // The reference is valid until the *next call to any *View() function on
    // the same thread* (or the thread exits). ImGui main render thread = single
    // consumer, so this is perfectly safe. No heap copies on cache hit.
    // Internally uses GetItemsStateVersion() atomic to detect changes: if nothing
    // changed since last call, returns the same reference in 1 atomic read.
    const std::vector<std::pair<int, Stat>>& GetFilteredItemsView();
    const std::vector<std::pair<int, Stat>>& GetFilteredCurrenciesView();
    const std::vector<std::pair<int, Stat>>& GetFavoriteItemsView();
    const std::vector<std::pair<int, Stat>>& GetFavoriteCurrenciesView();

    // Timeline / Drops history
    std::vector<SessionHistory::DropEntry> GetSessionDropsCopy();
    // Cheap change-detector for UI-side caching: bumps whenever s_SessionDrops is
    // mutated (new drop, reset, item/currency removal, load). Lock-free atomic read.
    uint64_t GetSessionDropsVersion();

    // Cheap change-detector for GetFilteredItems()/GetFilteredCurrencies()/GetFavoriteItems()/
    // GetFavoriteCurrencies() caching: bumps whenever item/currency count, favorite status,
    // or ignored/session-ignored/skip-once status changes. Lock-free atomic read.
    void BumpItemsStateVersion();
    void ForceCacheInvalidate();
    uint64_t GetItemsStateVersion();
    
    // Bump session drops version to invalidate Timeline cache when ignore status changes
    void BumpSessionDropsVersion();

    bool IsFavorite(int apiId);
    std::set<int> GetFavoriteItemIds();
    std::set<int> GetFavoriteCurrencyIds();

    // Categories
    std::string GetCurrencyCategory(int currencyId);

    // Ignored Items (delegates to IgnoredItemsManager)
    bool IsItemIgnored(int apiId);
    bool IsCurrencyIgnored(int apiId);
    
    // Update ignored flag in s_Items/s_Currencies
    void UpdateItemIgnoredFlag(int apiId, bool ignored);
    void UpdateCurrencyIgnoredFlag(int apiId, bool ignored);
    
    // Reset item/currency count to 0
    void ResetItemCount(int apiId);
    void ResetCurrencyCount(int apiId);

    // Remove item/currency completely
    void RemoveItem(int apiId);
    void RemoveCurrency(int apiId);

    // Advanced Filtering
    std::vector<std::pair<int, Stat>> GetFilteredItems();
    std::vector<std::pair<int, Stat>> GetFilteredCurrencies();
    bool PassesFilter(const Stat& stat);

    // Search
    std::vector<std::pair<int, Stat>> GetSearchedItems(const std::string& searchTerm);
    std::vector<std::pair<int, Stat>> GetSearchedCurrencies(const std::string& searchTerm);

    // Multi-Sort
    enum SortMode {
        PriceDesc, PriceAsc, CountDesc, CountAsc, NameAZ, NameZA,
        ProfitDesc, ProfitAsc, RarityDesc, RarityAsc, TypeAZ, TypeZA
    };
    
    // Filter options for sorted views
    struct SortFilterOptions {
        bool excludeIgnored = true;
        bool excludeZeroCount = true;
        bool excludeCurrencies = true;
        int rarityFilterMin = 0; // 0 = no filter, 1-8 = min rarity rank
        std::string searchTerm; // Search term for name filtering
        bool excludeFavorites = false; // If true, exclude favorite items
        bool excludeNonFavorites = false; // If true, exclude non-favorite items
    };
    
    std::vector<std::pair<int, Stat>> GetSortedItems(SortMode mode, bool secondary = false);
    std::vector<std::pair<int, Stat>> GetSortedCurrencies(SortMode mode, bool secondary = false);
    // View versions: cache by (itemsVersion, SortMode, FilterOptions) in thread_local storage.
    const std::vector<std::pair<int, Stat>>& GetSortedItemsView(SortMode mode, const SortFilterOptions& filter = {});
    const std::vector<std::pair<int, Stat>>& GetSortedCurrenciesView(SortMode mode, const SortFilterOptions& filter = {});

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
    std::vector<std::pair<std::chrono::system_clock::time_point, long long>> GetProfitHistory();

    // Best Drop
    std::pair<int, Stat> GetBestDrop();
    std::pair<int, Stat> GetBestDropTotalValue(); // New: Best drop by total session value

    // Export Functions
    std::string ExportToJson();
    std::string ExportToCsv();
    std::string ExportFavoritesToJson();

    // Persistence Functions
    void InitSaveWorker();
    void ShutdownSaveWorker();
    void SaveData(const char* addonDir);
    void SaveDataImmediate(const char* addonDir);
    void LoadData(const char* addonDir);
    void ImportFavoritesFromJson(const nlohmann::json& j);
    void ImportFromJson(const nlohmann::json& j);
}
