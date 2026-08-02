#include "item_tracker.h"
#include "loot_logger.h"
#include "magnetite_tracker.h"
#include "gaeting_tracker.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "pinned_items.h"
#include "skip_once_manager.h"
#include "session_ignore_manager.h"
#include "search_manager.h"
#include "settings.h"
#include "session_history.h"
#include "gw2_api.h"
#include "ui_notifications.h"
#include "localization.h"
#include "ui_common.h"
#include "shared.h"
#include "auto_reset.h"
#include "../include/nlohmann/json.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <climits>
#include <atomic>
#include <memory>

using json = nlohmann::json;

static std::mutex s_Mutex;
static std::map<int, Stat> s_Items;
static std::map<int, Stat> s_Currencies;
static std::atomic<int> s_MagicFind{ -1 };
static std::atomic<int> s_LastKnownMapId{ 0 };  // updated via SetCurrentMapId()

// Persistent user data — survives session resets
static std::mutex s_PersistentMutex;
static std::set<int> s_PersistentFavoriteItems;
static std::set<int> s_PersistentFavoriteCurrencies;

static std::chrono::system_clock::time_point s_SessionStart =
    std::chrono::system_clock::now();
static std::mutex s_SessionStartMutex;

// Track all drops with timestamps for session history
static std::vector<SessionHistory::DropEntry> s_SessionDrops;
static std::mutex s_SessionDropsMutex;
// Bumped on every mutation of s_SessionDrops (AddDrop/Reset/RemoveItem/RemoveCurrency/LoadData)
// so UI-side caches (Timeline tab) can cheaply detect changes without diffing content.
static std::atomic<uint64_t> s_SessionDropsVersion{ 0 };

// Bumped whenever item/currency count, favorite status, or ignored/session-ignored/
// skip-once status changes. Lets UI-side caches for GetFilteredItems()/GetFilteredCurrencies()/
// GetFavoriteItems()/GetFavoriteCurrencies() detect changes cheaply without diffing content.
static std::atomic<uint64_t> s_ItemsStateVersion{ 0 };

// ===========================================================================
// Async Save Worker (Option A)
// ===========================================================================
// SaveData() called from render thread no longer does JSON serialization
// or file IO. Instead it takes a cheap snapshot of shared state and pushes
// it to a single-slot "drop all but last" queue. A dedicated worker thread
// picks it up, builds the JSON string, and writes farming_data.json to
// disk in the background. This eliminates the 3-5s micro-stutter caused by
// synchronous WriteFile + Defender/cache flush in the render hot path.

struct PersistedStatSaveView
{
    long long   count;
    bool        isFavorite;
    int         lastMagicFind;
};

struct SaveSnapshot
{
    // ItemTracker core (captured under pLock/dropsLock/lock)
    int64_t                                            timestamp;
    std::time_t                                        sessionStart;
    int                                                magicFind;
    std::map<int, PersistedStatSaveView>               items;
    std::map<int, PersistedStatSaveView>               currencies;
    std::vector<SessionHistory::DropEntry>             sessionDrops;
    std::set<int>                                      persistentFavoriteItems;
    std::set<int>                                      persistentFavoriteCurrencies;

    // External managers (captured after releasing core locks, each has own mutex)
    std::set<int>                                      ignoredItems;
    std::set<int>                                      ignoredCurrencies;
    std::map<int, CustomProfitEntry>                   customProfits;
    nlohmann::json                                     pinnedItemsJson;

    std::string                                        dataPath;
};

static std::atomic<bool>        s_SaveWorkerShutdown{ true };
static std::thread              s_SaveWorkerThread;
static std::mutex               s_SaveQueueMutex;
static std::condition_variable  s_SaveQueueCv;
static std::unique_ptr<SaveSnapshot> s_PendingSave;  // single slot — "drop all but last"

static void WriteSnapshotToDisk(const SaveSnapshot& snap);

static void SaveWorkerLoop()
{
    for (;;)
    {
        std::unique_ptr<SaveSnapshot> snap;
        {
            std::unique_lock<std::mutex> lk(s_SaveQueueMutex);
            s_SaveQueueCv.wait(lk, [] {
                return s_PendingSave != nullptr || s_SaveWorkerShutdown.load(std::memory_order_acquire);
            });
            if (s_SaveWorkerShutdown.load(std::memory_order_acquire) && s_PendingSave == nullptr)
                return;
            snap = std::move(s_PendingSave);
        }

        if (snap)
            WriteSnapshotToDisk(*snap);
    }
}

static void WriteSnapshotToDisk(const SaveSnapshot& snap)
{
    try
    {
        nlohmann::json data;
        data["timestamp"] = snap.timestamp;
        data["sessionStart"] = snap.sessionStart;
        data["magicFind"] = snap.magicFind;

        nlohmann::json itemsArray = nlohmann::json::array();
        for (const auto& [id, sv] : snap.items)
        {
            nlohmann::json item;
            item["apiId"] = id;
            item["count"] = sv.count;
            item["isFavorite"] = sv.isFavorite;
            item["lastMagicFind"] = sv.lastMagicFind;
            itemsArray.push_back(item);
        }
        data["items"] = itemsArray;

        nlohmann::json currenciesArray = nlohmann::json::array();
        for (const auto& [id, sv] : snap.currencies)
        {
            nlohmann::json cur;
            cur["apiId"] = id;
            cur["count"] = sv.count;
            cur["isFavorite"] = sv.isFavorite;
            cur["lastMagicFind"] = sv.lastMagicFind;
            currenciesArray.push_back(cur);
        }
        data["currencies"] = currenciesArray;

        nlohmann::json dropsArray = nlohmann::json::array();
        for (const auto& drop : snap.sessionDrops)
        {
            nlohmann::json dropJson;
            dropJson["itemId"] = drop.itemId;
            dropJson["itemName"] = drop.itemName;
            dropJson["iconUrl"] = drop.iconUrl;
            dropJson["isCurrency"] = drop.isCurrency;
            dropJson["rarity"] = drop.rarity;
            dropJson["count"] = drop.count;
            dropJson["totalValue"] = drop.totalValue;
            dropJson["magicFind"] = drop.magicFind;
            dropJson["timestamp"] = drop.timestamp;
            dropJson["characterName"] = drop.characterName;
            dropsArray.push_back(dropJson);
        }
        data["sessionDrops"] = dropsArray;

        nlohmann::json ignoredItemsArray = nlohmann::json::array();
        for (int id : snap.ignoredItems)
            ignoredItemsArray.push_back(id);
        data["ignoredItems"] = ignoredItemsArray;

        nlohmann::json ignoredCurrenciesArray = nlohmann::json::array();
        for (int id : snap.ignoredCurrencies)
            ignoredCurrenciesArray.push_back(id);
        data["ignoredCurrencies"] = ignoredCurrenciesArray;

        nlohmann::json favoriteItemsArray = nlohmann::json::array();
        for (int id : snap.persistentFavoriteItems)
            favoriteItemsArray.push_back(id);
        data["favoriteItems"] = favoriteItemsArray;

        nlohmann::json favoriteCurrenciesArray = nlohmann::json::array();
        for (int id : snap.persistentFavoriteCurrencies)
            favoriteCurrenciesArray.push_back(id);
        data["favoriteCurrencies"] = favoriteCurrenciesArray;

        nlohmann::json customProfitsJson = nlohmann::json::object();
        for (const auto& [id, entry] : snap.customProfits)
        {
            nlohmann::json cp;
            cp["profit"] = entry.customProfitCopper;
            cp["type"] = static_cast<int>(entry.type);
            customProfitsJson[std::to_string(id)] = cp;
        }
        data["customProfits"] = customProfitsJson;

        data["pinnedItems"] = snap.pinnedItemsJson;

        std::ofstream file(snap.dataPath);
        if (file.is_open())
        {
            file << data.dump(4);
            file.close();
            if (APIDefs) APIDefs->Log(LOGL_INFO, "FarmingTracker", "Farming data saved successfully.");
        }
        else
        {
            if (APIDefs) APIDefs->Log(LOGL_CRITICAL, "FarmingTracker", ("Failed to open " + snap.dataPath + " for writing!").c_str());
        }
    }
    catch (const std::exception& e)
    {
        if (APIDefs) APIDefs->Log(LOGL_CRITICAL, "FarmingTracker", ("Exception while saving farming data: " + std::string(e.what())).c_str());
    }
}

void ItemTracker::InitSaveWorker()
{
    if (s_SaveWorkerThread.joinable())
        return;
    s_SaveWorkerShutdown.store(false, std::memory_order_release);
    s_SaveWorkerThread = std::thread(SaveWorkerLoop);
}

void ItemTracker::ShutdownSaveWorker()
{
    {
        std::lock_guard<std::mutex> lk(s_SaveQueueMutex);
        s_SaveWorkerShutdown.store(true, std::memory_order_release);
    }
    s_SaveQueueCv.notify_one();
    if (s_SaveWorkerThread.joinable())
        s_SaveWorkerThread.join();

    if (s_PendingSave)
    {
        WriteSnapshotToDisk(*s_PendingSave);
        s_PendingSave.reset();
    }
}

void ItemTracker::BumpItemsStateVersion()
{
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
}

void ItemTracker::ForceCacheInvalidate()
{
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
}

void ItemTracker::BumpSessionDropsVersion()
{
    s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
}

uint64_t ItemTracker::GetItemsStateVersion()
{
    return s_ItemsStateVersion.load(std::memory_order_relaxed);
}

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
    std::chrono::system_clock::time_point timestamp;
    long long profitPerHour;
};

static std::deque<ProfitHistoryEntry> s_ProfitHistory;
static std::chrono::system_clock::time_point s_LastHistoryUpdate =
    std::chrono::system_clock::now();
static std::mutex s_ProfitHistoryMutex; // Protect profit history access
static const int HISTORY_UPDATE_INTERVAL_SECONDS = 10; // Update every 10 seconds
static const int MAX_HISTORY_ENTRIES = 10; // Keep last 10 entries

static bool s_ProfitGoalReached = false;

// Settings snapshot used by PassesFilter — collected once per filter call, no lock held during filtering.
struct FilterSettings
{
    bool filterIgnored, filterNotIgnored;
    bool filterFavorite, filterNotFavorite;
    bool filterUnknownByApi, filterKnownByApi;
    bool filterSellableToVendor, filterSellableOnTp, filterCustomProfit;
    bool filterAccountBound, filterNotAccountBound;
    bool filterNoSell, filterNotNoSell;
    bool filterTypeArmor, filterTypeWeapon, filterTypeTrinket, filterTypeGizmo;
    bool filterTypeCraftingMaterial, filterTypeConsumable, filterTypeGatheringTool;
    bool filterTypeBag, filterTypeContainer, filterTypeMiniPet, filterTypeGizmoContainer;
    bool filterTypeBackpack, filterTypeUpgradeComponent, filterTypeTool, filterTypeTrophy, filterTypeUnlock;
    bool filterKarma, filterLaurel, filterGem, filterFractalRelic;
    bool filterBadgeOfHonor, filterGuildCommendation, filterTransmutationCharge, filterSpiritShards;
    bool filterUnboundMagic, filterVolatileMagic, filterAirshipParts, filterGeode;
    bool filterLeyLineCrystals, filterTradeContracts, filterElegyMosaic, filterUncommonCoins;
    bool filterAstralAcclaim, filterPristineFractalRelics, filterUnstableFractalEssence;
    bool filterMagnetiteShards, filterGaetingCrystals, filterProphetShards, filterGreenProphetShards;
    bool filterWvWSkirmishTickets, filterProofsOfHeroics, filterPvpLeagueTickets, filterAscendedShardsOfGlory;
    bool filterResearchNotes, filterTyrianDefenseSeal;
    bool filterTestimonyOfDesertHeroics, filterTestimonyOfJadeHeroics, filterTestimonyOfCastoranHeroics;
    bool filterLegendaryInsight, filterTalesOfDungeonDelving, filterImperialFavor, filterCanachCoins;
    bool filterAncientCoin, filterUnusualCoin, filterJadeSliver, filterStaticCharge;
    bool filterPinchOfStardust, filterCalcifiedGasp, filterUrsusOblige;
    bool filterGaetingCrystalJanthir, filterAntiquatedDucat, filterAetherRichSap;
    int  filterMinPriceGold, filterMinPriceSilver, filterMinPriceCopper;
    int  filterMaxPriceGold, filterMaxPriceSilver, filterMaxPriceCopper;
    int  filterMinQuantity, filterMaxQuantity;
    int  itemRarityFilterMin;
    int  maxHistoryItems;

    bool operator==(const FilterSettings& other) const
    {
        return std::memcmp(this, &other, sizeof(FilterSettings)) == 0;
    }

    static FilterSettings FromGlobal()
    {
        FilterSettings s;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            s.filterIgnored               = g_Settings.filterIgnored;
            s.filterNotIgnored            = g_Settings.filterNotIgnored;
            s.filterFavorite              = g_Settings.filterFavorite;
            s.filterNotFavorite           = g_Settings.filterNotFavorite;
            s.filterUnknownByApi          = g_Settings.filterUnknownByApi;
            s.filterKnownByApi            = g_Settings.filterKnownByApi;
            s.filterSellableToVendor      = g_Settings.filterSellableToVendor;
            s.filterSellableOnTp          = g_Settings.filterSellableOnTp;
            s.filterCustomProfit          = g_Settings.filterCustomProfit;
            s.filterAccountBound          = g_Settings.filterAccountBound;
            s.filterNotAccountBound       = g_Settings.filterNotAccountBound;
            s.filterNoSell                = g_Settings.filterNoSell;
            s.filterNotNoSell             = g_Settings.filterNotNoSell;
            s.filterTypeArmor             = g_Settings.filterTypeArmor;
            s.filterTypeWeapon            = g_Settings.filterTypeWeapon;
            s.filterTypeTrinket           = g_Settings.filterTypeTrinket;
            s.filterTypeGizmo             = g_Settings.filterTypeGizmo;
            s.filterTypeCraftingMaterial  = g_Settings.filterTypeCraftingMaterial;
            s.filterTypeConsumable        = g_Settings.filterTypeConsumable;
            s.filterTypeGatheringTool     = g_Settings.filterTypeGatheringTool;
            s.filterTypeBag               = g_Settings.filterTypeBag;
            s.filterTypeContainer         = g_Settings.filterTypeContainer;
            s.filterTypeMiniPet           = g_Settings.filterTypeMiniPet;
            s.filterTypeGizmoContainer    = g_Settings.filterTypeGizmoContainer;
            s.filterTypeBackpack          = g_Settings.filterTypeBackpack;
            s.filterTypeUpgradeComponent  = g_Settings.filterTypeUpgradeComponent;
            s.filterTypeTool              = g_Settings.filterTypeTool;
            s.filterTypeTrophy            = g_Settings.filterTypeTrophy;
            s.filterTypeUnlock            = g_Settings.filterTypeUnlock;
            s.filterKarma                 = g_Settings.filterKarma;
            s.filterLaurel                = g_Settings.filterLaurel;
            s.filterGem                   = g_Settings.filterGem;
            s.filterFractalRelic          = g_Settings.filterFractalRelic;
            s.filterBadgeOfHonor          = g_Settings.filterBadgeOfHonor;
            s.filterGuildCommendation     = g_Settings.filterGuildCommendation;
            s.filterTransmutationCharge   = g_Settings.filterTransmutationCharge;
            s.filterSpiritShards          = g_Settings.filterSpiritShards;
            s.filterUnboundMagic          = g_Settings.filterUnboundMagic;
            s.filterVolatileMagic         = g_Settings.filterVolatileMagic;
            s.filterAirshipParts          = g_Settings.filterAirshipParts;
            s.filterGeode                 = g_Settings.filterGeode;
            s.filterLeyLineCrystals       = g_Settings.filterLeyLineCrystals;
            s.filterTradeContracts        = g_Settings.filterTradeContracts;
            s.filterElegyMosaic           = g_Settings.filterElegyMosaic;
            s.filterUncommonCoins         = g_Settings.filterUncommonCoins;
            s.filterAstralAcclaim         = g_Settings.filterAstralAcclaim;
            s.filterPristineFractalRelics = g_Settings.filterPristineFractalRelics;
            s.filterUnstableFractalEssence= g_Settings.filterUnstableFractalEssence;
            s.filterMagnetiteShards       = g_Settings.filterMagnetiteShards;
            s.filterGaetingCrystals       = g_Settings.filterGaetingCrystals;
            s.filterProphetShards         = g_Settings.filterProphetShards;
            s.filterGreenProphetShards    = g_Settings.filterGreenProphetShards;
            s.filterWvWSkirmishTickets    = g_Settings.filterWvWSkirmishTickets;
            s.filterProofsOfHeroics       = g_Settings.filterProofsOfHeroics;
            s.filterPvpLeagueTickets      = g_Settings.filterPvpLeagueTickets;
            s.filterAscendedShardsOfGlory = g_Settings.filterAscendedShardsOfGlory;
            s.filterResearchNotes         = g_Settings.filterResearchNotes;
            s.filterTyrianDefenseSeal     = g_Settings.filterTyrianDefenseSeal;
            s.filterTestimonyOfDesertHeroics    = g_Settings.filterTestimonyOfDesertHeroics;
            s.filterTestimonyOfJadeHeroics      = g_Settings.filterTestimonyOfJadeHeroics;
            s.filterTestimonyOfCastoranHeroics  = g_Settings.filterTestimonyOfCastoranHeroics;
            s.filterLegendaryInsight      = g_Settings.filterLegendaryInsight;
            s.filterTalesOfDungeonDelving = g_Settings.filterTalesOfDungeonDelving;
            s.filterImperialFavor         = g_Settings.filterImperialFavor;
            s.filterCanachCoins           = g_Settings.filterCanachCoins;
            s.filterAncientCoin           = g_Settings.filterAncientCoin;
            s.filterUnusualCoin           = g_Settings.filterUnusualCoin;
            s.filterJadeSliver            = g_Settings.filterJadeSliver;
            s.filterStaticCharge          = g_Settings.filterStaticCharge;
            s.filterPinchOfStardust       = g_Settings.filterPinchOfStardust;
            s.filterCalcifiedGasp         = g_Settings.filterCalcifiedGasp;
            s.filterUrsusOblige           = g_Settings.filterUrsusOblige;
            s.filterGaetingCrystalJanthir = g_Settings.filterGaetingCrystalJanthir;
            s.filterAntiquatedDucat       = g_Settings.filterAntiquatedDucat;
            s.filterAetherRichSap         = g_Settings.filterAetherRichSap;
            s.filterMinPriceGold          = g_Settings.filterMinPriceGold;
            s.filterMinPriceSilver        = g_Settings.filterMinPriceSilver;
            s.filterMinPriceCopper        = g_Settings.filterMinPriceCopper;
            s.filterMaxPriceGold          = g_Settings.filterMaxPriceGold;
            s.filterMaxPriceSilver        = g_Settings.filterMaxPriceSilver;
            s.filterMaxPriceCopper        = g_Settings.filterMaxPriceCopper;
            s.filterMinQuantity           = g_Settings.filterMinQuantity;
            s.filterMaxQuantity           = g_Settings.filterMaxQuantity;
            s.itemRarityFilterMin         = g_Settings.itemRarityFilterMin;
            s.maxHistoryItems             = g_Settings.maxHistoryItems;
        }
        return s;
    }
};

// Cache structures for filtered/favorite items and currencies
struct FilteredItemsCache
{
    uint64_t itemsVersion = UINT64_MAX;
    bool hasSettings = false;
    FilterSettings lastFilterSettings;
    std::vector<std::pair<int, Stat>> result;
};
static FilteredItemsCache s_FilteredItemsCache;
static std::mutex s_FilteredItemsCacheMutex;

static FilteredItemsCache s_FilteredCurrenciesCache;
static std::mutex s_FilteredCurrenciesCacheMutex;

struct FavoriteItemsCache
{
    uint64_t itemsVersion = UINT64_MAX;
    std::string lastSearchTerm;
    std::vector<std::pair<int, Stat>> result;
};
static FavoriteItemsCache s_FavoriteItemsCache;
static std::mutex s_FavoriteItemsCacheMutex;

static FavoriteItemsCache s_FavoriteCurrenciesCache;
static std::mutex s_FavoriteCurrenciesCacheMutex;

static bool PassesFilterImpl(const Stat& stat, const FilterSettings& f);
static void CheckAndTriggerNotification(int apiId, Stat& st);

static void ProcessPendingNotifications()
{
    // Collect notifications to process outside of locks
    std::vector<std::pair<int, Stat>> itemNotifications;
    std::vector<std::pair<int, Stat>> currencyNotifications;
    
    // Collect pending notifications for items
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        for (auto& [id, st] : s_Items)
        {
            if (st.notificationPending)
            {
                itemNotifications.push_back({id, st});
                st.notificationPending = false;
            }
        }
    }
    
    // Collect pending notifications for currencies
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        for (auto& [id, st] : s_Currencies)
        {
            if (st.notificationPending)
            {
                currencyNotifications.push_back({id, st});
                st.notificationPending = false;
            }
        }
    }
    
    // Process notifications outside of locks
    for (auto& [id, st] : itemNotifications)
    {
        CheckAndTriggerNotification(id, st);
    }
    
    for (auto& [id, st] : currencyNotifications)
    {
        CheckAndTriggerNotification(id, st);
    }
}

static void CheckAndTriggerNotification(int apiId, Stat& st)
{
    // Bug #5 fix: never notify for zero-count items (persistent-only, not actually dropped)
    if (st.count == 0) { st.notificationPending = false; return; }
    if (!st.details.loaded) { return; }
    if (!st.notificationPending) return;

    // Exclude items from notification blacklist
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        if (std::find(g_Settings.notificationBlacklist.begin(), g_Settings.notificationBlacklist.end(), apiId) != g_Settings.notificationBlacklist.end())
        {
            st.notificationPending = false;
            return;
        }
    }

    // Snapshot settings
    bool enableNotifications;
    bool notificationPrecursorAlert;
    bool notificationInfusionAlert;
    bool notificationEnableMinValue;
    float notificationMinValueGold;
    bool notificationEnableMinRarity;
    int  notificationMinRarity;
    bool notificationCombineValueAndRarity;
    bool notificationIncludeNonProfit;
    bool notificationIncludeAgonyInfusions;

    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        enableNotifications          = g_Settings.enableNotifications;
        notificationPrecursorAlert   = g_Settings.notificationPrecursorAlert;
        notificationInfusionAlert    = g_Settings.notificationInfusionAlert;
        notificationEnableMinValue   = g_Settings.notificationEnableMinValue;
        notificationMinValueGold    = g_Settings.notificationMinValueGold;
        notificationEnableMinRarity  = g_Settings.notificationEnableMinRarity;
        notificationMinRarity        = g_Settings.notificationMinRarity;
        notificationCombineValueAndRarity = g_Settings.notificationCombineValueAndRarity;
        notificationIncludeNonProfit = g_Settings.notificationIncludeNonProfit;
        notificationIncludeAgonyInfusions = g_Settings.notificationIncludeAgonyInfusions;
    }

    if (!enableNotifications) { st.notificationPending = false; return; }

    bool shouldNotify = false;
    std::string specialText;

    // 1. Pre-Cursor check
    if (notificationPrecursorAlert)
    {
        std::string lowerName = st.details.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::string lowerDesc = st.details.description;
        std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);

        // Precursor search terms in different languages
        bool containsPrecursor = (lowerName.find("precursor") != std::string::npos || lowerDesc.find("precursor") != std::string::npos ||
                                 lowerName.find("präkursor") != std::string::npos || lowerDesc.find("präkursor") != std::string::npos ||
                                 lowerName.find("précurseur") != std::string::npos || lowerDesc.find("précurseur") != std::string::npos ||
                                 lowerName.find("прекурсор") != std::string::npos || lowerDesc.find("прекурсор") != std::string::npos ||
                                 lowerName.find("前置") != std::string::npos || lowerDesc.find("前置") != std::string::npos);

        if ((containsPrecursor ||
              (st.details.rarity == "Exotic" && st.details.itemType == ItemType::Weapon &&
               st.details.level == 80 && st.details.vendorValue == 0 && !st.details.accountBound)) &&
             apiId != 76179) // Amalgamated Gemstone
         {
            shouldNotify = true;
            specialText  = Localization::GetText("precursor_drop_label");
        }
    }

    // 2. Infusion check
    if (!shouldNotify && notificationInfusionAlert)
    {
        bool isInfusion = false;
        if (st.details.itemType == ItemType::UpgradeComponent)
        {
            std::string lut = st.details.upgradeComponentType;
            std::transform(lut.begin(), lut.end(), lut.begin(), ::tolower);
            isInfusion = (lut == "infusion");
        }
        if (isInfusion && !notificationIncludeAgonyInfusions)
        {
            std::string lowerName = st.details.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            if (lowerName.find("agony") != std::string::npos ||
                lowerName.find("qual")  != std::string::npos)
                isInfusion = false;
        }
        if (isInfusion)
        {
            shouldNotify = true;
            specialText  = Localization::GetText("infusion_drop_label");
        }
    }

    // 3. Value / rarity check
    if (!shouldNotify)
    {
        long long value    = ItemTracker::GetStatProfit(st) / (st.count != 0 ? std::abs(st.count) : 1);
        int       rarityRk = ItemTracker::RarityRank(st.details.rarity);

        bool valueMet  = notificationEnableMinValue  && value    >= static_cast<long long>(notificationMinValueGold * 10000);
        bool rarityMet = notificationEnableMinRarity && rarityRk >= notificationMinRarity;

        if (notificationCombineValueAndRarity)
            shouldNotify = (notificationEnableMinValue && notificationEnableMinRarity)
                               ? (valueMet && rarityMet)
                               : (valueMet || rarityMet);
        else
            shouldNotify = (valueMet || rarityMet);

        if (shouldNotify && specialText.empty() && value <= 0 && !notificationIncludeNonProfit)
            shouldNotify = false;
    }

    if (shouldNotify)
        UINotifications::AddNotification(apiId, st, specialText);
    st.notificationPending = false;
}

static void UpdateOrInsert(std::map<int, Stat>& map,
                           int apiId, long long delta, StatType type,
                           bool isIgnored = false, bool isFavorite = false,
                           bool skipDelta = false)
{
    auto it = map.find(apiId);
    if (it != map.end())
    {
        it->second.isIgnored = isIgnored; // Update ignored status
        it->second.isFavorite = isFavorite; // Update favorite status
        
        if (!isIgnored && !skipDelta) // Only modify count if not ignored and not skipping delta
        {
            it->second.count += delta;
            if (delta > 0)
            {
                it->second.notificationPending = true;
            }
        }
    }
    else
    {
        if (!isIgnored) // Only add to map if not ignored
        {
            Stat s;
            s.apiId = apiId;
            s.type  = type;
            s.count = skipDelta ? 0 : delta;
            if (delta > 0 && !skipDelta) s.notificationPending = true;

            // Re-apply persistent flags (passed as parameters to avoid deadlock)
            s.isFavorite = isFavorite;
            s.isIgnored = isIgnored;

            map[apiId] = s;
        }
        else // If ignored, still ensure we have a Stat entry with isIgnored set (so UI shows it as ignored)
        {
            Stat s;
            s.apiId = apiId;
            s.type  = type;
            s.count = 0; // Don't track count for ignored items
            s.isFavorite = isFavorite;
            s.isIgnored = isIgnored;
            map[apiId] = s;
        }
    }
}

// Converts ItemType enum to a log-friendly string
static std::string ItemTypeToString(ItemType t)
{
    switch (t)
    {
        case ItemType::Armor:            return "Armor";
        case ItemType::Weapon:           return "Weapon";
        case ItemType::Trinket:          return "Trinket";
        case ItemType::Gizmo:            return "Gizmo";
        case ItemType::CraftingMaterial: return "CraftingMaterial";
        case ItemType::Consumable:       return "Consumable";
        case ItemType::Container:        return "Container";
        case ItemType::Bag:              return "Bag";
        case ItemType::Backpack:         return "Backpack";
        case ItemType::UpgradeComponent: return "UpgradeComponent";
        case ItemType::Tool:             return "Tool";
        case ItemType::Trophy:           return "Trophy";
        case ItemType::Unlock:           return "Unlock";
        case ItemType::MiniPet:          return "MiniPet";
        default:                         return "Unknown";
    }
}

void ItemTracker::AddDrop(const std::map<int, long long>& items,
                          const std::map<int, long long>& currencies)
{
    struct LogEntry {
        int id;
        std::string name;
        long long delta;
        bool isCurrency;
        std::string type;
        std::string rarity;
        long long price;
        long long vendorPrice;
    };
    std::vector<LogEntry> logEntries;

    // Snapshot rarity-ignore settings BEFORE acquiring the big 3-mutex block
    // to avoid a lock-order inversion with Settings::s_SettingsMutex.
    // Thread A (Drop): holds s_PersistentMutex → s_SessionDropsMutex → s_Mutex → wants SettingsMutex
    // Thread B (Backup): holds SettingsMutex → wants s_PersistentMutex/s_Mutex → DEADLOCK!
    struct RarityIgnoreSnapshot {
        bool Junk, Basic, Fine, Masterwork, Rare, Exotic, Ascended, Legendary;
    };
    RarityIgnoreSnapshot rarityIg = {false, false, false, false, false, false, false, false};
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        rarityIg.Junk       = g_Settings.ignoredRarityToggleJunk;
        rarityIg.Basic      = g_Settings.ignoredRarityToggleBasic;
        rarityIg.Fine       = g_Settings.ignoredRarityToggleFine;
        rarityIg.Masterwork = g_Settings.ignoredRarityToggleMasterwork;
        rarityIg.Rare       = g_Settings.ignoredRarityToggleRare;
        rarityIg.Exotic     = g_Settings.ignoredRarityToggleExotic;
        rarityIg.Ascended   = g_Settings.ignoredRarityToggleAscended;
        rarityIg.Legendary  = g_Settings.ignoredRarityToggleLegendary;
    }

    // Global Lock Order: 1. s_PersistentMutex, 4. s_SessionDropsMutex, 5. s_Mutex
    // All locks released before processing notifications and logging to avoid deadlock and blocking.
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);

        // Get current timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &now_time);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);

        // 1. Process Items
        for (auto& [id, delta] : items)
        {
            // Check if ignored first!
            bool isFavorite = s_PersistentFavoriteItems.count(id) > 0;
            bool isIgnored = IgnoredItemsManager::IsItemIgnored(id) || SessionIgnoreManager::IsItemIgnoredForSession(id);
            bool skipOnce = SkipOnceManager::IsItemSkippedOnce(id);
            bool isIgnoredForDrop = isIgnored;
            
            if (skipOnce) {
                isIgnoredForDrop = true;
                SkipOnceManager::UnskipOnceItem(id);
            }
            
            auto existIt = s_Items.find(id);
            if (!isIgnored && existIt != s_Items.end() && existIt->second.details.loaded)
            {
                const std::string& rarity = existIt->second.details.rarity;
                bool shouldIgnore = false;
                // Use local snapshot — NO SettingsMutex acquisition inside the 3-mutex block!
                if      (rarity == "Junk")       shouldIgnore = rarityIg.Junk;
                else if (rarity == "Basic")      shouldIgnore = rarityIg.Basic;
                else if (rarity == "Fine")       shouldIgnore = rarityIg.Fine;
                else if (rarity == "Masterwork") shouldIgnore = rarityIg.Masterwork;
                else if (rarity == "Rare")       shouldIgnore = rarityIg.Rare;
                else if (rarity == "Exotic")     shouldIgnore = rarityIg.Exotic;
                else if (rarity == "Ascended")   shouldIgnore = rarityIg.Ascended;
                else if (rarity == "Legendary")  shouldIgnore = rarityIg.Legendary;

                if (shouldIgnore) {
                    IgnoredItemsManager::IgnoreItem(id);
                    isIgnored = true;
                    isIgnoredForDrop = true;
                }
            }
            
            UpdateOrInsert(s_Items, id, delta, StatType::Item, isIgnored, isFavorite, skipOnce);
            s_Items[id].lastMagicFind = s_MagicFind.load();

            // Only add to session history and log if not ignored
            if (!isIgnoredForDrop) {
                // Session history entry
                SessionHistory::DropEntry drop;
                drop.itemId = id;
                drop.itemName = ""; 
                drop.isCurrency = false;
                drop.rarity = "";
                drop.count = static_cast<int>(delta);
                drop.totalValue = 0; 
                drop.magicFind = s_MagicFind.load();
                drop.timestamp = timestamp;
                {
                    std::lock_guard<std::mutex> accLock(UICommon::s_AccountNameMutex);
                    drop.characterName = UICommon::s_AccountNameBuf;
                }
                s_SessionDrops.push_back(drop);
                s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
                s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);

                // Prepare logging info while under lock
                if (delta > 0) {
                    const auto& st = s_Items[id];
                    LogEntry le;
                    le.id = id;
                    le.delta = delta;
                    le.isCurrency = false;
                    // Use unit price (per single item), NOT GetStatProfit which returns count * price
                    if (st.details.loaded) {
                        long long vendorPrice = CanSellToVendor(st.details) ? (long long)st.details.vendorValue : 0;
                        long long tpSellPrice = CanSellOnTp(st.details) ? TpSellProceedsPerUnitCopper(st.details) : 0;
                        le.price  = std::max(vendorPrice, tpSellPrice);
                        le.vendorPrice = vendorPrice;
                        le.name   = st.details.name;
                        le.rarity = st.details.rarity;
                        le.type   = ItemTypeToString(st.details.itemType);
                    } else {
                        le.price  = -1; // unknown — ui_loot_log skips negative prices
                        le.vendorPrice = -1; // unknown
                        le.name   = "Item #" + std::to_string(id);
                        le.rarity = "Unknown";
                        le.type   = "Unknown";
                    }
                    logEntries.push_back(le);
                }
            }
        }

        // 2. Process Currencies
        for (auto& [id, delta] : currencies)
        {
            // Check if ignored first!
            bool isFavorite = s_PersistentFavoriteCurrencies.count(id) > 0;
            bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(id) || SessionIgnoreManager::IsCurrencyIgnoredForSession(id);
            bool skipOnce = SkipOnceManager::IsCurrencySkippedOnce(id);
            bool isIgnoredForDrop = isIgnored;
            
            if (skipOnce) {
                isIgnoredForDrop = true;
                SkipOnceManager::UnskipOnceCurrency(id);
            }
            
            UpdateOrInsert(s_Currencies, id, delta, StatType::Currency, isIgnored, isFavorite, skipOnce);
            s_Currencies[id].lastMagicFind = s_MagicFind.load();

            // Only add to session history and log if not ignored
            if (!isIgnoredForDrop) {
                // Session history entry
                SessionHistory::DropEntry drop;
                drop.itemId = id;
                drop.itemName = ""; 
                drop.isCurrency = true;
                drop.rarity = "";
                drop.count = static_cast<int>(delta);
                drop.totalValue = 0; 
                drop.magicFind = s_MagicFind.load();
                drop.timestamp = timestamp;
                {
                    std::lock_guard<std::mutex> accLock(UICommon::s_AccountNameMutex);
                    drop.characterName = UICommon::s_AccountNameBuf;
                }
                s_SessionDrops.push_back(drop);
                s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
                s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);

                // Prepare logging info while under lock
                if (delta > 0) {
                    const auto& st = s_Currencies[id];
                    LogEntry le;
                    le.id = id;
                    le.delta = delta;
                    le.isCurrency = true;
                    // For currencies: use custom profit per unit if set, otherwise 0 (no TP price)
                    if (st.HasCustomProfit()) {
                        le.price = CustomProfitManager::GetCustomProfit(id); // already per-unit
                    } else if (id == 1) {
                        le.price = 1; // coins: 1 copper each
                    } else {
                        le.price = 0; // no price for most currencies
                    }
                    le.vendorPrice = 0; // currencies have no vendor price
                    if (st.details.loaded) {
                        le.name = st.details.name;
                        le.type = "Currency";
                    } else {
                        le.name = "Currency #" + std::to_string(id);
                        le.type = "Currency";
                    }
                    le.rarity = "";
                    logEntries.push_back(le);
                }
            }
        }
    } // ← All locks released here

    // 3. Magnetite Tracker integration
    auto magIt = currencies.find(MagnetiteTracker::CURRENCY_ID);
    if (magIt != currencies.end() && magIt->second > 0)
        MagnetiteTracker::OnDrfShardsEarned(static_cast<int>(magIt->second));

    // 3b. Gaeting Crystal Tracker integration (currency 39)
    auto gaeIt = currencies.find(GaetingTracker::CURRENCY_ID);
    if (gaeIt != currencies.end() && gaeIt->second > 0)
        GaetingTracker::OnDrfCrystalsEarned(static_cast<int>(gaeIt->second));

    // 4. Loot Logger — process entries without blocking the main DRF thread
    std::string apiToken;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        apiToken = g_Settings.gw2ApiKey;
    }

    int mapId = s_LastKnownMapId.load();
    for (const auto& le : logEntries)
    {
        LootLogger::LogDrop(le.id, le.name, le.delta, le.isCurrency, le.type, le.rarity, le.price,
                            mapId, LootLogger::ResolveMapName(mapId, apiToken));
    }

    // 5. Process notifications AFTER releasing all locks
    ProcessPendingNotifications();

    // Request save with debounce (2 seconds delay)
    AutoReset::RequestSave();
}

void ItemTracker::SetMagicFind(int magicFind)
{
    s_MagicFind.store(magicFind);
}

void ItemTracker::SetCurrentMapId(int mapId)
{
    s_LastKnownMapId.store(mapId);
}

int ItemTracker::GetMagicFind()
{
    return s_MagicFind.load();
}

void ItemTracker::UpdateItemIgnoredFlag(int apiId, bool ignored)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_Items.find(apiId);
    if (it != s_Items.end())
    {
        it->second.isIgnored = ignored;
    }
}

void ItemTracker::UpdateCurrencyIgnoredFlag(int apiId, bool ignored)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_Currencies.find(apiId);
    if (it != s_Currencies.end())
    {
        it->second.isIgnored = ignored;
    }
}

// ---------------------------------------------------------------------------
// Session save helpers (HOCH-2 fix: keep render-thread footprint tiny)
//
// Everything between the `namespace ItemTracker { ... }` block below has
// direct access to the translation-unit statics (s_MagicFind, s_Mutex,
// s_Items, s_Currencies, s_SessionDrops*, s_LastKnownMapId, …) without
// needing explicit `ItemTracker::` qualifiers on every symbol.
// ---------------------------------------------------------------------------
namespace ItemTracker {

// Captures everything needed to later write a session snapshot to disk. All
// heavy mutex-guarded copies happen here, but NO HTTP calls and NO disk IO.
// Returns true if session history is enabled and `out` was populated.
static bool BuildSessionSnapshot(SessionHistory::SessionData& out,
                                 int& outMapId,
                                 std::string& outApiToken)
{
    bool enableSessionHistory;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        enableSessionHistory = g_Settings.enableSessionHistory;
    }

    if (!enableSessionHistory)
        return false;

    // Collect session data
    SessionHistory::SessionData sessionData;

    // Get session duration
    auto duration = GetSessionDuration();
    sessionData.durationSeconds = static_cast<int>(duration.count());
    sessionData.averageMagicFind = s_MagicFind.load();

    // Get session start and end time
    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    struct tm timeInfo;
    localtime_s(&timeInfo, &nowTimeT);
    char timeBuffer[64];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    sessionData.endTime = timeBuffer;

    // Calculate start time
    auto startTime = now - std::chrono::duration_cast<std::chrono::system_clock::duration>(duration);
    auto startTimeT = std::chrono::system_clock::to_time_t(startTime);
    localtime_s(&timeInfo, &startTimeT);
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    sessionData.startTime = timeBuffer;

    // Get profit data — CalcTotalCustomProfit acquires s_Mutex internally,
    // so call it OUTSIDE s_ProfitHistoryMutex to maintain lock order.
    sessionData.totalProfit = CalcTotalCustomProfit();
    if (duration.count() > 0)
        sessionData.profitPerHour = (sessionData.totalProfit * 3600) / duration.count();
    else
        sessionData.profitPerHour = 0;

    int mapId = s_LastKnownMapId.load();
    std::string apiToken;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        apiToken = g_Settings.gw2ApiKey;
    }

    // Get items and collect top drops and rarity counts
    {
        // Global Lock Order: 4. s_SessionDropsMutex, 5. s_Mutex
        std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);

        sessionData.totalDrops = static_cast<int>(s_Items.size());

        // Collect top drops (by value)
        std::vector<std::pair<long long, SessionHistory::DropEntry>> drops;
        drops.reserve(s_Items.size());
        for (const auto& [id, stat] : s_Items)
        {
            long long value = GetStatProfit(stat);
            SessionHistory::DropEntry drop;
            drop.itemId = id;
            drop.itemName = stat.details.loaded ? stat.details.name : "Unknown";
            drop.iconUrl = stat.details.loaded ? stat.details.iconUrl : "";
            drop.isCurrency = false;
            drop.rarity = stat.details.loaded ? stat.details.rarity : "Unknown";
            drop.count = static_cast<int>(stat.count);
            drop.totalValue = value;
            drops.push_back({value, drop});

            // Collect rarity counts
            std::string rarity = stat.details.loaded ? stat.details.rarity : "Unknown";
            sessionData.rarityCounts[rarity]++;
        }

        // Sort by value descending and take top 10
        std::sort(drops.begin(), drops.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });

        for (size_t i = 0; i < std::min<size_t>(drops.size(), 10); i++)
        {
            sessionData.topDrops.push_back(drops[i].second);
        }

        // Populate allDrops with item details (deep-copies every drop into sessionData)
        for (auto& drop : s_SessionDrops)
        {
            auto it = s_Items.find(drop.itemId);
            if (it != s_Items.end() && it->second.details.loaded)
            {
                drop.itemName = it->second.details.name;
                drop.iconUrl  = it->second.details.iconUrl;
                drop.rarity   = it->second.details.rarity;
                long long vendorPpu = CanSellToVendor(it->second.details) ? (long long)it->second.details.vendorValue : 0LL;
                long long tpPpu     = CanSellOnTp(it->second.details)     ? TpSellProceedsPerUnitCopper(it->second.details) : 0LL;
                long long unitProfit = it->second.HasCustomProfit()
                    ? CustomProfitManager::GetCustomProfit(drop.itemId)
                    : std::max(vendorPpu, tpPpu);
                drop.totalValue = unitProfit * drop.count;
            }
            else
            {
                auto currIt = s_Currencies.find(drop.itemId);
                if (currIt != s_Currencies.end() && currIt->second.details.loaded)
                {
                    drop.itemName = currIt->second.details.name;
                    drop.iconUrl  = currIt->second.details.iconUrl;
                    drop.rarity   = currIt->second.details.rarity;
                    long long customPpu = currIt->second.HasCustomProfit() ? CustomProfitManager::GetCustomProfit(drop.itemId) : 0LL;
                    drop.totalValue = (drop.itemId == 1) ? drop.count : customPpu * drop.count;
                }
            }
            sessionData.allDrops.push_back(drop);
        }
    }

    // mapName is resolved in the caller so the HTTP cache-miss path never runs
    // under item mutexes. We just record the data needed to resolve it later.
    out       = std::move(sessionData);
    outMapId  = mapId;
    outApiToken = std::move(apiToken);
    return true;
}

static void FinalizeSessionSave(SessionHistory::SessionData& sessionData,
                                int mapId,
                                const std::string& apiToken)
{
    // May block on HTTP if map name isn't cached (worker thread only!)
    sessionData.mapName = LootLogger::ResolveMapName(mapId, apiToken);
    if (sessionData.mapName == "Unknown" || sessionData.mapName.empty())
    {
        sessionData.mapName = "Map #" + std::to_string(mapId);
    }

    // Write JSON + ofstream to session_history.json (may block on file IO)
    SessionHistory::SaveSession(sessionData);
}

void SaveCurrentSessionSync()
{
    SessionHistory::SessionData sessionData;
    int mapId = 0;
    std::string apiToken;
    if (!BuildSessionSnapshot(sessionData, mapId, apiToken))
        return;
    FinalizeSessionSave(sessionData, mapId, apiToken);
}

void SaveCurrentSession()
{
    SessionHistory::SessionData sessionData;
    int mapId = 0;
    std::string apiToken;
    if (!BuildSessionSnapshot(sessionData, mapId, apiToken))
        return;

    // Hand the captured deep-copy to the shared background worker. The
    // lambda owns all the state so there's no lifetime risk. BackgroundJobs
    // will run it synchronously inline if the worker already shut down.
    BackgroundJobs::Enqueue(
        [sessionData = std::move(sessionData), mapId, apiToken = std::move(apiToken)]() mutable
        {
            FinalizeSessionSave(sessionData, mapId, apiToken);
        });
}

} // namespace ItemTracker

void ItemTracker::Reset()
{
    // Save session history before resetting
    ItemTracker::SaveCurrentSession();

    // Save immediately (reset is critical for data integrity)
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
    if (addonDir)
    {
        ItemTracker::SaveData(addonDir);
    }

    // Clear active notifications
    UINotifications::ClearAll();

    // Clear skip once and session ignore
    SkipOnceManager::ClearAll();
    SessionIgnoreManager::ClearAll();

    // Global Lock Order: 1. s_PersistentMutex, 2. s_ProfitHistoryMutex, 3. s_SessionStartMutex, 4. s_SessionDropsMutex, 5. s_Mutex
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> profitLock(s_ProfitHistoryMutex);
    std::lock_guard<std::mutex> sessionLock(s_SessionStartMutex);
    std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);

    s_ProfitHistory.clear();
    s_LastHistoryUpdate = std::chrono::system_clock::now();
    s_ProfitGoalReached = false;

    s_SessionStart = std::chrono::system_clock::now();

    s_SessionDrops.clear();
    s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);

    // Don't clear s_Items and s_Currencies - keep API data for Loot Log display
    // Only reset the count values
    for (auto& [id, stat] : s_Items)
    {
        stat.count = 0;
    }
    for (auto& [id, stat] : s_Currencies)
    {
        stat.count = 0;
    }
}

void ItemTracker::SafeReset()
{
    // Persistent stores (favorites, ignored, custom profit) survive reset automatically.
    // We only need to reset session data and then re-apply persistent flags
    // to any items that might still be in the map after reset (there won't be any,
    // but we call Reset() which clears s_Items/s_Currencies).
    // The persistent flags will be re-applied when new drops come in via AddDrop().
    Reset();
    // IgnoredItemsManager and CustomProfitManager are independent static stores
    // and are NOT touched by Reset(), so they survive automatically.
    // s_PersistentFavoriteItems/Currencies also survive Reset() since Reset()
    // only clears s_Items and s_Currencies.
}

void ItemTracker::ClearPersistedData(const char* addonDir)
{
    if (!addonDir)
        return;

    std::string dataPath = std::string(addonDir) + "\\farming_data.json";
    
    // Delete the persistence file
    std::remove(dataPath.c_str());
}

std::map<int, Stat> ItemTracker::GetItemsCopy()
{
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    std::map<int, Stat> copy = s_Items;
    for (auto& [id, stat] : copy)
    {
        stat.isIgnored = IgnoredItemsManager::IsItemIgnored(id) || SessionIgnoreManager::IsItemIgnoredForSession(id);
    }
    return copy;
}

std::map<int, Stat> ItemTracker::GetCurrenciesCopy()
{
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    std::map<int, Stat> copy = s_Currencies;
    for (auto& [id, stat] : copy)
    {
        stat.isIgnored = IgnoredItemsManager::IsCurrencyIgnored(id) || SessionIgnoreManager::IsCurrencyIgnoredForSession(id);
    }
    return copy;
}

Stat ItemTracker::GetItemStat(int itemId)
{
    // Snapshot ignored state BEFORE acquiring s_PersistentMutex/s_Mutex
    // to avoid circular lock: s_PersistentMutex -> s_Mutex -> IgnoredItems::s_Mutex
    // vs IgnoreItem: IgnoredItems::s_Mutex -> (no longer calls back)
    bool isIgnored = IgnoredItemsManager::IsItemIgnored(itemId) || SessionIgnoreManager::IsItemIgnoredForSession(itemId);

    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);

    auto it = s_Items.find(itemId);
    if (it != s_Items.end())
    {
        Stat s = it->second;
        s.isIgnored = isIgnored;
        return s;
    }

    Stat empty;
    empty.apiId = itemId;
    empty.type = StatType::Item;
    empty.isFavorite = s_PersistentFavoriteItems.count(itemId) > 0;
    empty.isIgnored = isIgnored;
    return empty;
}

Stat ItemTracker::GetCurrencyStat(int currencyId)
{
    bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(currencyId) || SessionIgnoreManager::IsCurrencyIgnoredForSession(currencyId);

    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);

    auto it = s_Currencies.find(currencyId);
    if (it != s_Currencies.end())
    {
        Stat s = it->second;
        s.isIgnored = isIgnored;
        return s;
    }

    Stat empty;
    empty.apiId = currencyId;
    empty.type = StatType::Currency;
    empty.isFavorite = s_PersistentFavoriteCurrencies.count(currencyId) > 0;
    empty.isIgnored = isIgnored;
    return empty;
}

std::chrono::seconds ItemTracker::GetSessionDuration()
{
    std::lock_guard<std::mutex> lock(s_SessionStartMutex);
    auto now = std::chrono::system_clock::now();
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
        return GetCustomProfit() * std::abs(count);
    
    if (IsCoin())
        return count;
    
    if (!details.loaded)
        return 0;
    
    long long tpProfit = ItemTracker::TpSellProceedsPerUnitCopper(details);
    long long vendorProfit = ItemTracker::CanSellToVendor(details) ? (long long)details.vendorValue : 0;
    return std::max(tpProfit, vendorProfit) * std::abs(count);
}

// Favorites System
void ItemTracker::SetFavorite(int apiId, bool favorite)
{
    // If adding to favorites, remove from ignored first (before acquiring locks to avoid deadlock)
    if (favorite)
    {
        IgnoredItemsManager::UnignoreItem(apiId);
        IgnoredItemsManager::UnignoreCurrency(apiId);
    }

    // Update persistent store first (survives reset)
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        
        // We don't know for sure if it's an item or currency here without checking maps,
        // but we can check where it exists or just update both (the load logic handles it).
        // However, it's better to be precise if possible.
        bool isItem = false;
        bool isCurrency = false;
        
        {
            std::lock_guard<std::mutex> lock(s_Mutex);
            if (s_Items.count(apiId) > 0) isItem = true;
            if (s_Currencies.count(apiId) > 0) isCurrency = true;
        }

        if (favorite)
        {
            if (isItem)
            {
                s_PersistentFavoriteItems.insert(apiId);
                s_PersistentFavoriteCurrencies.erase(apiId);
            }
            else if (isCurrency)
            {
                s_PersistentFavoriteCurrencies.insert(apiId);
                s_PersistentFavoriteItems.erase(apiId);
            }
            else
            {
                // If it's neither yet (e.g. adding from search), we'll add to both and the load logic will sort it out
                s_PersistentFavoriteItems.insert(apiId);
                s_PersistentFavoriteCurrencies.insert(apiId);
            }
        }
        else
        {
            s_PersistentFavoriteItems.erase(apiId);
            s_PersistentFavoriteCurrencies.erase(apiId);
        }
    }

    // Update in-memory state — persistence handled by the 5-second periodic save loop
    // (AutoReset::Tick fallback) already. No need to pay CreateSaveSnapshot()
    // cost on every single favorite click; worst case data is persisted
    // within 5 seconds.
    std::lock_guard<std::mutex> lock(s_Mutex);

    // Update in items if present
    auto itemIt = s_Items.find(apiId);
    if (itemIt != s_Items.end())
        itemIt->second.isFavorite = favorite;

    // Update in currencies if present
    auto currencyIt = s_Currencies.find(apiId);
    if (currencyIt != s_Currencies.end())
        currencyIt->second.isFavorite = favorite;

    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
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

std::set<int> ItemTracker::GetFavoriteItemIds()
{
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    return s_PersistentFavoriteItems;
}

std::set<int> ItemTracker::GetFavoriteCurrencyIds()
{
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    return s_PersistentFavoriteCurrencies;
}

void ItemTracker::ResetItemCount(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_Items.find(apiId);
    if (it != s_Items.end())
    {
        it->second.count = 0;
    }
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
}

void ItemTracker::RemoveItem(int apiId)
{
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_Items.erase(apiId);
    s_PersistentFavoriteItems.erase(apiId);
    s_PersistentFavoriteCurrencies.erase(apiId);
    s_SessionDrops.erase(
        std::remove_if(s_SessionDrops.begin(), s_SessionDrops.end(),
            [apiId](const SessionHistory::DropEntry& entry) { 
                return !entry.isCurrency && entry.itemId == apiId; 
            }),
        s_SessionDrops.end()
    );
    s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
}

void ItemTracker::ResetCurrencyCount(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_Currencies.find(apiId);
    if (it != s_Currencies.end())
    {
        it->second.count = 0;
    }
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
}

void ItemTracker::RemoveCurrency(int apiId)
{
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_Currencies.erase(apiId);
    s_PersistentFavoriteItems.erase(apiId);
    s_PersistentFavoriteCurrencies.erase(apiId);
    s_SessionDrops.erase(
        std::remove_if(s_SessionDrops.begin(), s_SessionDrops.end(),
            [apiId](const SessionHistory::DropEntry& entry) { 
                return entry.isCurrency && entry.itemId == apiId; 
            }),
        s_SessionDrops.end()
    );
    s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
}

std::vector<std::pair<int, Stat>> ItemTracker::GetFavoriteItems()
{
    uint64_t currentVersion = s_ItemsStateVersion.load(std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> cacheLock(s_FavoriteItemsCacheMutex);
        if (s_FavoriteItemsCache.itemsVersion == currentVersion)
        {
            return s_FavoriteItemsCache.result; // Cache hit
        }
    }

    std::map<int, Stat> favorites;
    std::set<int> persistentIds;
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        persistentIds = s_PersistentFavoriteItems;
    }

    {
        std::lock_guard<std::mutex> lock(s_Mutex);

        // 1. Get current session favorites
        for (const auto& [id, stat] : s_Items)
        {
            if (stat.isFavorite)
            {
                favorites[id] = stat;
            }
        }

        // 2. Add persistent favorites that are not in the current session yet
        for (int id : persistentIds)
        {
            if (favorites.find(id) == favorites.end())
            {
                // Skip if ID is already in currencies (don't show it as item)
                if (s_Currencies.count(id) > 0) continue;

                Stat s;
                s.apiId = id;
                s.type = StatType::Item;
                s.count = 0;
                s.isFavorite = true;

                favorites[id] = s;
            }
        }
    }

    auto result = std::vector<std::pair<int, Stat>>(favorites.begin(), favorites.end());

    // Update cache
    {
        std::lock_guard<std::mutex> cacheLock(s_FavoriteItemsCacheMutex);
        s_FavoriteItemsCache.itemsVersion = currentVersion;
        s_FavoriteItemsCache.lastSearchTerm.clear();
        s_FavoriteItemsCache.result = result;
    }

    return result;
}

std::vector<std::pair<int, Stat>> ItemTracker::GetFavoriteCurrencies()
{
    uint64_t currentVersion = s_ItemsStateVersion.load(std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> cacheLock(s_FavoriteCurrenciesCacheMutex);
        if (s_FavoriteCurrenciesCache.itemsVersion == currentVersion)
        {
            return s_FavoriteCurrenciesCache.result; // Cache hit
        }
    }

    std::map<int, Stat> favorites;
    std::set<int> persistentIds;
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        persistentIds = s_PersistentFavoriteCurrencies;
    }

    {
        std::lock_guard<std::mutex> lock(s_Mutex);

        // 1. Get current session favorites
        for (const auto& [id, stat] : s_Currencies)
        {
            if (stat.isFavorite)
            {
                favorites[id] = stat;
            }
        }

        // 2. Add persistent favorites that are not in the current session yet
        for (int id : persistentIds)
        {
            if (favorites.find(id) == favorites.end())
            {
                // Skip if ID is already in items (don't show it as currency)
                if (s_Items.count(id) > 0) continue;

                Stat s;
                s.apiId = id;
                s.type = StatType::Currency;
                s.count = 0;
                s.isFavorite = true;

                favorites[id] = s;
            }
        }
    }

    auto result = std::vector<std::pair<int, Stat>>(favorites.begin(), favorites.end());

    // Update cache
    {
        std::lock_guard<std::mutex> cacheLock(s_FavoriteCurrenciesCacheMutex);
        s_FavoriteCurrenciesCache.itemsVersion = currentVersion;
        s_FavoriteCurrenciesCache.lastSearchTerm.clear();
        s_FavoriteCurrenciesCache.result = result;
    }

    return result;
}

std::vector<SessionHistory::DropEntry> ItemTracker::GetSessionDropsCopy()
{
    // Global Lock Order: 4. s_SessionDropsMutex, 5. s_Mutex
    std::lock_guard<std::mutex> lock(s_SessionDropsMutex);
    std::lock_guard<std::mutex> statsLock(s_Mutex);
    
    std::vector<SessionHistory::DropEntry> drops = s_SessionDrops;
    for (auto& drop : drops)
    {
        if (drop.isCurrency)
        {
            auto it = s_Currencies.find(drop.itemId);
            if (it != s_Currencies.end() && it->second.details.loaded)
            {
                drop.itemName = it->second.details.name;
                drop.iconUrl  = it->second.details.iconUrl;
                drop.rarity   = it->second.details.rarity;
                // unit profit * this drop's count
                long long customPpu = it->second.HasCustomProfit() ? CustomProfitManager::GetCustomProfit(drop.itemId) : 0LL;
                drop.totalValue = (drop.itemId == 1) ? drop.count : customPpu * drop.count;
            }
        }
        else
        {
            auto it = s_Items.find(drop.itemId);
            if (it != s_Items.end() && it->second.details.loaded)
            {
                drop.itemName = it->second.details.name;
                drop.iconUrl  = it->second.details.iconUrl;
                drop.rarity   = it->second.details.rarity;
                // unit profit * this drop's count (NOT total session profit)
                long long vendorPpu = CanSellToVendor(it->second.details) ? (long long)it->second.details.vendorValue : 0LL;
                long long tpPpu     = CanSellOnTp(it->second.details)     ? TpSellProceedsPerUnitCopper(it->second.details) : 0LL;
                long long unitProfit = it->second.HasCustomProfit()
                    ? CustomProfitManager::GetCustomProfit(drop.itemId)
                    : std::max(vendorPpu, tpPpu);
                drop.totalValue = unitProfit * drop.count;
            }
        }
    }

    return drops;
}

uint64_t ItemTracker::GetSessionDropsVersion()
{
    return s_SessionDropsVersion.load(std::memory_order_relaxed);
}

std::string ItemTracker::GetCurrencyCategory(int currencyId)
{
    switch (currencyId)
    {
        case 1: // Gold
        case 2: // Karma
        case 3: // Laurels
        case 4: // Gems
        case 18: // Transmutation Charges
        case 23: // Spirit Shards
        case 61: // Research Notes
        case 72: // Astral Acclaim
            return Localization::GetText("currency_cat_common");

        case 7: // Fractal Relics
        case 24: // Pristine Fractal Relics
        case 50: // Unstable Fractal Essence
            return Localization::GetText("currency_cat_fractal");

        case 28: // Magnetite Shards
        case 35: // Gaeting Crystals
        case 53: // Prophet Shards
        case 57: // Green Prophet Shards
        case 70: // Legendary Insight
            return Localization::GetText("currency_cat_raid_strike");

        case 15: // Badges of Honor
        case 26: // WvW Skirmish Claim Tickets
        case 31: // Proofs of Heroics
        case 36: // Testimony of Desert Heroics
        case 65: // Testimony of Jade Heroics
        case 82: // Testimony of Castoran Heroics
            return Localization::GetText("currency_cat_wvw");

        case 30: // PvP League Tickets
        case 33: // Ascended Shards of Glory
            return Localization::GetText("currency_cat_pvp");

        case 19: // Airship Parts
        case 20: // Ley Line Crystals
        case 22: // Geodes
        case 32: // Unbound Magic
        case 38: // Trade Contracts
        case 39: // Elegy Mosaics
        case 45: // Volatile Magic
        case 52: // Tyrian Defense Seals
        case 62: // Unusual Coins
        case 64: // Jade Slivers
        case 66: // Ancient Coins
        case 67: // Canach Coins
        case 68: // Imperial Favor
        case 69: // Tales of Dungeon Delving
        case 73: // Pinch of Stardust
        case 75: // Calcified Gasps
        case 78: // Static Charges
            return Localization::GetText("currency_cat_map");

        case 76: // Ursus Oblige
        case 77: // Gaeting Crystal (Janthir)
        case 81: // Antiquated Ducat
        case 83: // Aether-Rich Sap
            return Localization::GetText("currency_cat_janthir");

        default:
            return Localization::GetText("currency_cat_other");
    }
}

// Ignored Items (delegates to IgnoredItemsManager + Skip Once + Session Ignore)
bool ItemTracker::IsItemIgnored(int apiId)
{
    return IgnoredItemsManager::IsItemIgnored(apiId)
        || SessionIgnoreManager::IsItemIgnoredForSession(apiId)
        || SkipOnceManager::IsItemSkippedOnce(apiId);
}

bool ItemTracker::IsCurrencyIgnored(int apiId)
{
    return IgnoredItemsManager::IsCurrencyIgnored(apiId)
        || SessionIgnoreManager::IsCurrencyIgnoredForSession(apiId)
        || SkipOnceManager::IsCurrencySkippedOnce(apiId);
}

// Advanced Filtering
static bool PassesFilterImpl(const Stat& stat, const FilterSettings& f)
{
    // Ignored / Favorite
    if (stat.isIgnored  && !f.filterIgnored)    return false;
    if (!stat.isIgnored && !f.filterNotIgnored) return false;
    if (stat.isFavorite  && !f.filterFavorite)    return false;
    if (!stat.isFavorite && !f.filterNotFavorite) return false;

    // Coins always pass filter (they're the base currency)
    if (stat.IsCoin()) return true;

    // API knowledge — only apply when details are loaded.
    // If details are not yet loaded (e.g. immediately after reset), knownByApi is false
    // by default and would incorrectly filter the item out before the API response arrives.
    if (stat.details.loaded)
    {
        if (!stat.details.knownByApi && !f.filterUnknownByApi) return false;
        if (stat.details.knownByApi  && !f.filterKnownByApi)   return false;
    }

    // Sell method / account-bound / no-sell (items only)
    if (stat.IsItem() && stat.details.loaded)
    {
        bool canSellToVendor = ItemTracker::CanSellToVendor(stat.details);
        bool canSellOnTp     = ItemTracker::CanSellOnTp(stat.details);
        bool hasCustomProfit = stat.HasCustomProfit();

        if (canSellToVendor  && !f.filterSellableToVendor) return false;
        if (canSellOnTp      && !f.filterSellableOnTp)     return false;
        if (hasCustomProfit  && !f.filterCustomProfit)      return false;
        if (stat.details.accountBound  && !f.filterAccountBound)    return false;
        if (!stat.details.accountBound && !f.filterNotAccountBound) return false;
        if (stat.details.noSell  && !f.filterNoSell)    return false;
        if (!stat.details.noSell && !f.filterNotNoSell) return false;
    }

    // Item type filter
    if (stat.IsItem() && stat.details.loaded)
    {
        switch (stat.details.itemType)
        {
            case ItemType::Armor:             if (!f.filterTypeArmor)            return false; break;
            case ItemType::Weapon:            if (!f.filterTypeWeapon)           return false; break;
            case ItemType::Trinket:           if (!f.filterTypeTrinket)          return false; break;
            case ItemType::Gizmo:             if (!f.filterTypeGizmo)            return false; break;
            case ItemType::CraftingMaterial:  if (!f.filterTypeCraftingMaterial) return false; break;
            case ItemType::Consumable:        if (!f.filterTypeConsumable)       return false; break;
            case ItemType::GatheringTool:     if (!f.filterTypeGatheringTool)    return false; break;
            case ItemType::Bag:               if (!f.filterTypeBag)              return false; break;
            case ItemType::Container:         if (!f.filterTypeContainer)        return false; break;
            case ItemType::MiniPet:           if (!f.filterTypeMiniPet)          return false; break;
            case ItemType::GizmoContainer:    if (!f.filterTypeGizmoContainer)   return false; break;
            case ItemType::Backpack:          if (!f.filterTypeBackpack)         return false; break;
            case ItemType::UpgradeComponent:  if (!f.filterTypeUpgradeComponent) return false; break;
            case ItemType::Tool:              if (!f.filterTypeTool)             return false; break;
            case ItemType::Trophy:            if (!f.filterTypeTrophy)           return false; break;
            case ItemType::Unlock:            if (!f.filterTypeUnlock)           return false; break;
            default: break;
        }
    }

    // Currency filter
    if (stat.IsCurrency())
    {
        // Ignore status respects filterIgnored flag (consistent with top-level ignored/favorite checks)
        if (stat.isIgnored && !f.filterIgnored) return false;
        
        switch (stat.apiId)
        {
            case 1:  break; // Gold – always show
            case 2:  if (!f.filterKarma)                      return false; break;
            case 3:  if (!f.filterLaurel)                     return false; break;
            case 4:  if (!f.filterGem)                        return false; break;
            case 7:  if (!f.filterFractalRelic)               return false; break;
            case 15: if (!f.filterBadgeOfHonor)               return false; break;
            case 16: if (!f.filterGuildCommendation)          return false; break;
            case 18: if (!f.filterTransmutationCharge)        return false; break;
            case 23: if (!f.filterSpiritShards)               return false; break;
            case 32: if (!f.filterUnboundMagic)               return false; break;
            case 45: if (!f.filterVolatileMagic)              return false; break;
            case 19: if (!f.filterAirshipParts)               return false; break;
            case 22: if (!f.filterGeode)                      return false; break;
            case 20: if (!f.filterLeyLineCrystals)            return false; break;
            case 38: if (!f.filterTradeContracts)             return false; break;
            case 39: if (!f.filterElegyMosaic)                return false; break;
            case 62: if (!f.filterUncommonCoins)              return false; break;
            case 72: if (!f.filterAstralAcclaim)              return false; break;
            case 24: if (!f.filterPristineFractalRelics)      return false; break;
            case 50: if (!f.filterUnstableFractalEssence)     return false; break;
            case 28: if (!f.filterMagnetiteShards)            return false; break;
            case 35: if (!f.filterGaetingCrystals)            return false; break;
            case 53: if (!f.filterProphetShards)              return false; break;
            case 57: if (!f.filterGreenProphetShards)         return false; break;
            case 26: if (!f.filterWvWSkirmishTickets)         return false; break;
            case 31: if (!f.filterProofsOfHeroics)            return false; break;
            case 30: if (!f.filterPvpLeagueTickets)           return false; break;
            case 33: if (!f.filterAscendedShardsOfGlory)     return false; break;
            case 61: if (!f.filterResearchNotes)              return false; break;
            case 52: if (!f.filterTyrianDefenseSeal)          return false; break;
            case 36: if (!f.filterTestimonyOfDesertHeroics)   return false; break;
            case 65: if (!f.filterTestimonyOfJadeHeroics)     return false; break;
            case 82: if (!f.filterTestimonyOfCastoranHeroics) return false; break;
            case 70: if (!f.filterLegendaryInsight)           return false; break;
            case 69: if (!f.filterTalesOfDungeonDelving)      return false; break;
            case 68: if (!f.filterImperialFavor)              return false; break;
            case 67: if (!f.filterCanachCoins)                return false; break;
            case 66: if (!f.filterAncientCoin)                return false; break;
            case 64: if (!f.filterJadeSliver)                 return false; break;
            case 78: if (!f.filterStaticCharge)               return false; break;
            case 73: if (!f.filterPinchOfStardust)            return false; break;
            case 75: if (!f.filterCalcifiedGasp)              return false; break;
            case 76: if (!f.filterUrsusOblige)                return false; break;
            case 77: if (!f.filterGaetingCrystalJanthir)      return false; break;
            case 81: if (!f.filterAntiquatedDucat)            return false; break;
            case 83: if (!f.filterAetherRichSap)              return false; break;
            default: break;
        }
    }

    // Price range filter
    if (stat.IsItem() && stat.details.loaded)
    {
        long long pricePerUnit = ItemTracker::GetStatProfit(stat) / (stat.count != 0 ? stat.count : 1);
        long long minPriceCopper = (long long)f.filterMinPriceGold * 10000 + f.filterMinPriceSilver * 100 + f.filterMinPriceCopper;
        long long maxPriceCopper = (long long)f.filterMaxPriceGold * 10000 + f.filterMaxPriceSilver * 100 + f.filterMaxPriceCopper;
        if (minPriceCopper > 0 && pricePerUnit < minPriceCopper) return false;
        if (maxPriceCopper > 0 && pricePerUnit > maxPriceCopper) return false;
    }

    // Quantity range filter
    if (f.filterMinQuantity > 0 && std::abs(stat.count) < f.filterMinQuantity) return false;
    if (f.filterMaxQuantity > 0 && std::abs(stat.count) > f.filterMaxQuantity) return false;

    // Rarity filter
    if (stat.IsItem() && stat.details.loaded)
        if (f.itemRarityFilterMin > 0 && ItemTracker::RarityRank(stat.details.rarity) < f.itemRarityFilterMin) return false;

    return true;
}

bool ItemTracker::PassesFilter(const Stat& stat)
{
    // Snapshot all settings once — no mutex needed (UI thread writes, workers read only).
    // On x86/x64 aligned bool/int reads are naturally atomic at the hardware level.
    FilterSettings f = FilterSettings::FromGlobal();
    return PassesFilterImpl(stat, f);
}

std::vector<std::pair<int, Stat>> ItemTracker::GetFilteredItems()
{
    // Get ignored snapshot BEFORE acquiring s_Mutex to avoid circular deadlock:
    // GetFilteredItems(s_Mutex) -> IsItemIgnored(IgnoredItems::s_Mutex) vs.
    // IgnoreItem(IgnoredItems::s_Mutex) -> SetFavorite(s_PersistentMutex -> s_Mutex)
    std::set<int> ignoredSnapshot = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> sessionIgnoredSnapshot = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> skipOnceSnapshot = SkipOnceManager::GetSkippedItems();

    // Get filter settings snapshot BEFORE acquiring s_Mutex to avoid circular deadlock:
    // GetFilteredItems(s_Mutex) -> PassesFilter -> FilterSettings::FromGlobal(s_SettingsMutex)
    // Note: We disable favorite filtering for GetFilteredItems to avoid conflicts with
    // the Drops tab's custom filter system. The Drops tab uses SortFilterOptions which
    // has its own excludeFavorites/excludeNonFavorites flags.
    // We also disable item-type filters to show all items in the Drops tab.
    FilterSettings filterSettings = FilterSettings::FromGlobal();
    filterSettings.filterFavorite = true; // Always allow favorites
    filterSettings.filterNotFavorite = true; // Always allow non-favorites
    filterSettings.filterIgnored = true; // Managed separately via excludeIgnored in SortFilterOptions
    filterSettings.filterNotIgnored = true; // Prevent global filter from hiding everything
    filterSettings.filterTypeArmor = true;
    filterSettings.filterTypeWeapon = true;
    filterSettings.filterTypeTrinket = true;
    filterSettings.filterTypeGizmo = true;
    filterSettings.filterTypeCraftingMaterial = true;
    filterSettings.filterTypeConsumable = true;
    filterSettings.filterTypeGatheringTool = true;
    filterSettings.filterTypeBag = true;
    filterSettings.filterTypeContainer = true;
    filterSettings.filterTypeMiniPet = true;
    filterSettings.filterTypeGizmoContainer = true;
    filterSettings.filterTypeBackpack = true;
    filterSettings.filterTypeUpgradeComponent = true;
    filterSettings.filterTypeTool = true;
    filterSettings.filterTypeTrophy = true;
    filterSettings.filterTypeUnlock = true;
    filterSettings.filterSellableToVendor = true;
    filterSettings.filterSellableOnTp = true;
    filterSettings.filterCustomProfit = true;
    filterSettings.filterAccountBound = true;
    filterSettings.filterNotAccountBound = true;
    filterSettings.filterNoSell = true;
    filterSettings.filterNotNoSell = true;
    filterSettings.filterKnownByApi = true;
    filterSettings.filterUnknownByApi = true;
    filterSettings.filterMinPriceGold = 0;
    filterSettings.filterMinPriceSilver = 0;
    filterSettings.filterMinPriceCopper = 0;
    filterSettings.filterMaxPriceGold = 0;
    filterSettings.filterMaxPriceSilver = 0;
    filterSettings.filterMaxPriceCopper = 0;
    filterSettings.filterMinQuantity = 0;
    filterSettings.filterMaxQuantity = 0;
    filterSettings.itemRarityFilterMin = 0;
    filterSettings.maxHistoryItems = 0;
    uint64_t currentVersion = s_ItemsStateVersion.load(std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> cacheLock(s_FilteredItemsCacheMutex);
        if (s_FilteredItemsCache.hasSettings &&
            s_FilteredItemsCache.itemsVersion == currentVersion &&
            s_FilteredItemsCache.lastFilterSettings == filterSettings)
        {
            return s_FilteredItemsCache.result; // Cache hit
        }
    }

    std::vector<std::pair<int, Stat>> filtered;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);

        std::vector<std::pair<int, Stat>> candidates;
        candidates.reserve(s_Items.size());

        for (const auto& [id, stat] : s_Items)
        {
            // Don't filter out items with count 0 - allow them to be shown
            // if (stat.count == 0) continue;

            Stat copy = stat;
            copy.isIgnored = (ignoredSnapshot.count(id) > 0) || (sessionIgnoredSnapshot.count(id) > 0) || (skipOnceSnapshot.count(id) > 0);
            if (PassesFilterImpl(copy, filterSettings))
                candidates.emplace_back(id, std::move(copy));
        }

        size_t limit = static_cast<size_t>(filterSettings.maxHistoryItems);

        if (limit > 0 && candidates.size() > limit)
        {
            filtered.assign(std::make_move_iterator(candidates.end() - limit),
                            std::make_move_iterator(candidates.end()));
        }
        else
        {
            filtered = std::move(candidates);
        }
    }

    // Update cache
    {
        std::lock_guard<std::mutex> cacheLock(s_FilteredItemsCacheMutex);
        s_FilteredItemsCache.itemsVersion = currentVersion;
        s_FilteredItemsCache.hasSettings = true;
        s_FilteredItemsCache.lastFilterSettings = filterSettings;
        s_FilteredItemsCache.result = filtered;
    }

    return filtered;
}

std::vector<std::pair<int, Stat>> ItemTracker::GetFilteredCurrencies()
{
    // Get ignored snapshot BEFORE acquiring s_Mutex (same deadlock prevention as GetFilteredItems)
    std::set<int> ignoredSnapshot = IgnoredItemsManager::GetIgnoredCurrencies();
    std::set<int> sessionIgnoredSnapshot = SessionIgnoreManager::GetIgnoredCurrencies();
    std::set<int> skipOnceSnapshot = SkipOnceManager::GetSkippedCurrencies();

    // Get filter settings snapshot BEFORE acquiring s_Mutex to avoid circular deadlock
    // Note: We disable favorite filtering for GetFilteredCurrencies to avoid conflicts with
    // the Drops tab's custom filter system. The Drops tab uses SortFilterOptions which
    // has its own excludeFavorites/excludeNonFavorites flags.
    // We also disable currency-specific filters to show all currencies in the Drops tab.
    FilterSettings filterSettings = FilterSettings::FromGlobal();
    filterSettings.filterFavorite = true; // Always allow favorites
    filterSettings.filterNotFavorite = true; // Always allow non-favorites
    filterSettings.filterIgnored = true; // Managed separately via excludeIgnored in SortFilterOptions
    filterSettings.filterNotIgnored = true; // Prevent global filter from hiding everything
    filterSettings.filterKnownByApi = true;
    filterSettings.filterUnknownByApi = true;
    filterSettings.filterKarma = true;
    filterSettings.filterLaurel = true;
    filterSettings.filterGem = true;
    filterSettings.filterFractalRelic = true;
    filterSettings.filterBadgeOfHonor = true;
    filterSettings.filterGuildCommendation = true;
    filterSettings.filterTransmutationCharge = true;
    filterSettings.filterSpiritShards = true;
    filterSettings.filterUnboundMagic = true;
    filterSettings.filterVolatileMagic = true;
    filterSettings.filterAirshipParts = true;
    filterSettings.filterGeode = true;
    filterSettings.filterLeyLineCrystals = true;
    filterSettings.filterTradeContracts = true;
    filterSettings.filterElegyMosaic = true;
    filterSettings.filterUncommonCoins = true;
    filterSettings.filterAstralAcclaim = true;
    filterSettings.filterPristineFractalRelics = true;
    filterSettings.filterUnstableFractalEssence = true;
    filterSettings.filterMagnetiteShards = true;
    filterSettings.filterGaetingCrystals = true;
    filterSettings.filterProphetShards = true;
    filterSettings.filterGreenProphetShards = true;
    filterSettings.filterWvWSkirmishTickets = true;
    filterSettings.filterProofsOfHeroics = true;
    filterSettings.filterPvpLeagueTickets = true;
    filterSettings.filterAscendedShardsOfGlory = true;
    filterSettings.filterResearchNotes = true;
    filterSettings.filterTyrianDefenseSeal = true;
    filterSettings.filterTestimonyOfDesertHeroics = true;
    filterSettings.filterTestimonyOfJadeHeroics = true;
    filterSettings.filterTestimonyOfCastoranHeroics = true;
    filterSettings.filterLegendaryInsight = true;
    filterSettings.filterTalesOfDungeonDelving = true;
    filterSettings.filterImperialFavor = true;
    filterSettings.filterCanachCoins = true;
    filterSettings.filterAncientCoin = true;
    filterSettings.filterUnusualCoin = true;
    filterSettings.filterJadeSliver = true;
    filterSettings.filterStaticCharge = true;
    filterSettings.filterPinchOfStardust = true;
    filterSettings.filterCalcifiedGasp = true;
    filterSettings.filterUrsusOblige = true;
    filterSettings.filterGaetingCrystalJanthir = true;
    filterSettings.filterAntiquatedDucat = true;
    filterSettings.filterAetherRichSap = true;
    filterSettings.filterMinQuantity = 0;
    filterSettings.filterMaxQuantity = 0;
    uint64_t currentVersion = s_ItemsStateVersion.load(std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> cacheLock(s_FilteredCurrenciesCacheMutex);
        if (s_FilteredCurrenciesCache.hasSettings &&
            s_FilteredCurrenciesCache.itemsVersion == currentVersion &&
            s_FilteredCurrenciesCache.lastFilterSettings == filterSettings)
        {
            return s_FilteredCurrenciesCache.result; // Cache hit
        }
    }

    auto currencies = GetCurrenciesCopy();
    std::vector<std::pair<int, Stat>> filtered;
    filtered.reserve(currencies.size());
    for (auto& [id, st] : currencies)
    {
        if (id == 44602 || id == 67027 || id == 89409)
            continue;
        // Don't filter out currencies with count 0 - allow them to be shown
        // if (st.count == 0 && id != 1) continue;

        st.isIgnored = (ignoredSnapshot.count(id) > 0) || (sessionIgnoredSnapshot.count(id) > 0) || (skipOnceSnapshot.count(id) > 0);
        if (PassesFilterImpl(st, filterSettings))
            filtered.emplace_back(id, std::move(st));
    }

    // Update cache
    {
        std::lock_guard<std::mutex> cacheLock(s_FilteredCurrenciesCacheMutex);
        s_FilteredCurrenciesCache.itemsVersion = currentVersion;
        s_FilteredCurrenciesCache.hasSettings = true;
        s_FilteredCurrenciesCache.lastFilterSettings = filterSettings;
        s_FilteredCurrenciesCache.result = filtered;
    }

    return filtered;
}

// Search functionality
std::vector<std::pair<int, Stat>> ItemTracker::GetSearchedItems(const std::string& searchTerm)
{
    auto items = GetFilteredItems();
    if (searchTerm.empty())
        return items;

    std::vector<std::pair<int, Stat>> searched;
    searched.reserve(items.size());
    for (auto& [id, stat] : items)
        if (SearchManager::MatchesSearch(stat.details.name, searchTerm))
            searched.emplace_back(id, std::move(stat));

    return searched;
}

std::vector<std::pair<int, Stat>> ItemTracker::GetSearchedCurrencies(const std::string& searchTerm)
{
    auto currencies = GetFilteredCurrencies();
    if (searchTerm.empty())
        return currencies;

    std::vector<std::pair<int, Stat>> searched;
    searched.reserve(currencies.size());
    for (auto& [id, stat] : currencies)
        if (SearchManager::MatchesSearchCurrency(stat.details.name, searchTerm))
            searched.emplace_back(id, std::move(stat));

    return searched;
}

// ============================================================================
//  View-Getter (0-copy hot path)
//  Uses thread_local storage + atomic version check. On cache hit:
//    0 mutex locks · 0 copies · 1 atomic load
//  Only when version or sort-mode actually changes:
//    1 mutex lock + 1 copy from shared cache to thread_local buffer.
// ============================================================================

const std::vector<std::pair<int, Stat>>& ItemTracker::GetFilteredItemsView()
{
    static thread_local uint64_t s_LastVersion = 0;
    static thread_local std::vector<std::pair<int, Stat>> s_Local;
    static thread_local bool s_Initialized = false;

    uint64_t ver = s_ItemsStateVersion.load(std::memory_order_relaxed);
    if (s_Initialized && s_LastVersion == ver) return s_Local;

    s_Local    = ItemTracker::GetFilteredItems(); // hits shared cache on 2nd miss usually
    s_LastVersion = ver;
    s_Initialized = true;
    return s_Local;
}

const std::vector<std::pair<int, Stat>>& ItemTracker::GetFilteredCurrenciesView()
{
    static thread_local uint64_t s_LastVersion = 0;
    static thread_local std::vector<std::pair<int, Stat>> s_Local;
    static thread_local bool s_Initialized = false;

    uint64_t ver = s_ItemsStateVersion.load(std::memory_order_relaxed);
    if (s_Initialized && s_LastVersion == ver) return s_Local;

    s_Local       = ItemTracker::GetFilteredCurrencies();
    s_LastVersion = ver;
    s_Initialized = true;
    return s_Local;
}

const std::vector<std::pair<int, Stat>>& ItemTracker::GetFavoriteItemsView()
{
    static thread_local uint64_t s_LastVersion = 0;
    static thread_local std::vector<std::pair<int, Stat>> s_Local;
    static thread_local bool s_Initialized = false;

    uint64_t ver = s_ItemsStateVersion.load(std::memory_order_relaxed);
    if (s_Initialized && s_LastVersion == ver) return s_Local;

    s_Local       = ItemTracker::GetFavoriteItems();
    s_LastVersion = ver;
    s_Initialized = true;
    return s_Local;
}

const std::vector<std::pair<int, Stat>>& ItemTracker::GetFavoriteCurrenciesView()
{
    static thread_local uint64_t s_LastVersion = 0;
    static thread_local std::vector<std::pair<int, Stat>> s_Local;
    static thread_local bool s_Initialized = false;

    uint64_t ver = s_ItemsStateVersion.load(std::memory_order_relaxed);
    if (s_Initialized && s_LastVersion == ver) return s_Local;

    s_Local       = ItemTracker::GetFavoriteCurrencies();
    s_LastVersion = ver;
    s_Initialized = true;
    return s_Local;
}

const std::vector<std::pair<int, Stat>>& ItemTracker::GetSortedItemsView(SortMode mode, const SortFilterOptions& filter)
{
    // Key = (itemsVersion · 64 + sortMode). Mode fits in 8 bits.
    // Include filter options in key for proper caching
    static thread_local uint64_t s_LastKey = 0;
    static thread_local SortFilterOptions s_LastFilter = {};
    static thread_local std::vector<std::pair<int, Stat>> s_Local;
    static thread_local bool s_Initialized = false;

    uint64_t ver  = s_ItemsStateVersion.load(std::memory_order_relaxed);
    uint64_t key  = (ver << 8) | (static_cast<uint64_t>(mode) & 0xFF);
    
    // Include filter options in cache key
    uint64_t filterKey = (static_cast<uint64_t>(filter.excludeIgnored) << 0) |
                         (static_cast<uint64_t>(filter.excludeZeroCount) << 1) |
                         (static_cast<uint64_t>(filter.excludeCurrencies) << 2) |
                         (static_cast<uint64_t>(filter.rarityFilterMin) << 3) |
                         (static_cast<uint64_t>(filter.excludeFavorites) << 4) |
                         (static_cast<uint64_t>(filter.excludeNonFavorites) << 5);
    key |= (filterKey << 16);

    if (s_Initialized && s_LastKey == key && s_LastFilter.excludeIgnored == filter.excludeIgnored &&
        s_LastFilter.excludeZeroCount == filter.excludeZeroCount &&
        s_LastFilter.excludeCurrencies == filter.excludeCurrencies &&
        s_LastFilter.rarityFilterMin == filter.rarityFilterMin &&
        s_LastFilter.searchTerm == filter.searchTerm &&
        s_LastFilter.excludeFavorites == filter.excludeFavorites &&
        s_LastFilter.excludeNonFavorites == filter.excludeNonFavorites) return s_Local;

    // Get sorted items and apply filters
    s_Local = ItemTracker::GetSortedItems(mode);
    
    // Apply filters
    if (filter.excludeIgnored || filter.excludeZeroCount || filter.excludeCurrencies || filter.rarityFilterMin > 0 || !filter.searchTerm.empty() || filter.excludeFavorites || filter.excludeNonFavorites)
    {
        std::vector<std::pair<int, Stat>> filtered;
        filtered.reserve(s_Local.size());

        for (auto& [id, st] : s_Local)
        {
            // Exclude ignored items
            if (filter.excludeIgnored && st.isIgnored) continue;

            // Exclude zero count items
            if (filter.excludeZeroCount && st.count == 0) continue;

            // Exclude currencies
            if (filter.excludeCurrencies && st.IsCurrency()) continue;

            // Exclude favorites
            if (filter.excludeFavorites && st.isFavorite) continue;

            // Exclude non-favorites
            if (filter.excludeNonFavorites && !st.isFavorite) continue;

            // Rarity filter
            if (filter.rarityFilterMin > 0)
            {
                std::string rarity = st.details.loaded ? st.details.rarity : "";
                int rarityRank = 0;
                if (rarity == "Junk") rarityRank = 1;
                else if (rarity == "Basic") rarityRank = 2;
                else if (rarity == "Fine") rarityRank = 3;
                else if (rarity == "Masterwork") rarityRank = 4;
                else if (rarity == "Rare") rarityRank = 5;
                else if (rarity == "Exotic") rarityRank = 6;
                else if (rarity == "Ascended") rarityRank = 7;
                else if (rarity == "Legendary") rarityRank = 8;
                if (rarityRank < filter.rarityFilterMin) continue;
            }

            // Search term filter (case-insensitive)
            if (!filter.searchTerm.empty())
            {
                std::string itemName = st.details.loaded ? st.details.name : ("Item #" + std::to_string(id));
                std::string searchTermLower = filter.searchTerm;
                std::string itemNameLower = itemName;
                std::transform(searchTermLower.begin(), searchTermLower.end(), searchTermLower.begin(), ::tolower);
                std::transform(itemNameLower.begin(), itemNameLower.end(), itemNameLower.begin(), ::tolower);
                if (itemNameLower.find(searchTermLower) == std::string::npos) continue;
            }

            filtered.push_back({id, st});
        }

        s_Local = std::move(filtered);
    }
    
    s_LastKey = key;
    s_LastFilter = filter;
    s_Initialized = true;
    return s_Local;
}

const std::vector<std::pair<int, Stat>>& ItemTracker::GetSortedCurrenciesView(SortMode mode, const SortFilterOptions& filter)
{
    static thread_local uint64_t s_LastKey = 0;
    static thread_local SortFilterOptions s_LastFilter = {};
    static thread_local std::vector<std::pair<int, Stat>> s_Local;
    static thread_local bool s_Initialized = false;

    uint64_t ver  = s_ItemsStateVersion.load(std::memory_order_relaxed);
    uint64_t key  = (ver << 8) | (static_cast<uint64_t>(mode) & 0xFF);
    
    // Include filter options in cache key
    uint64_t filterKey = (static_cast<uint64_t>(filter.excludeIgnored) << 0) |
                         (static_cast<uint64_t>(filter.excludeZeroCount) << 1) |
                         (static_cast<uint64_t>(filter.excludeCurrencies) << 2) |
                         (static_cast<uint64_t>(filter.rarityFilterMin) << 3) |
                         (static_cast<uint64_t>(filter.excludeFavorites) << 4) |
                         (static_cast<uint64_t>(filter.excludeNonFavorites) << 5);
    key |= (filterKey << 16);

    if (s_Initialized && s_LastKey == key && s_LastFilter.excludeIgnored == filter.excludeIgnored &&
        s_LastFilter.excludeZeroCount == filter.excludeZeroCount &&
        s_LastFilter.excludeCurrencies == filter.excludeCurrencies &&
        s_LastFilter.rarityFilterMin == filter.rarityFilterMin &&
        s_LastFilter.searchTerm == filter.searchTerm &&
        s_LastFilter.excludeFavorites == filter.excludeFavorites &&
        s_LastFilter.excludeNonFavorites == filter.excludeNonFavorites) return s_Local;

    // Get sorted currencies and apply filters
    s_Local = ItemTracker::GetSortedCurrencies(mode);

    // Apply filters (currencies typically don't have rarity, so ignore that filter)
    if (filter.excludeIgnored || filter.excludeZeroCount || filter.excludeCurrencies || !filter.searchTerm.empty() || filter.excludeFavorites || filter.excludeNonFavorites)
    {
        std::vector<std::pair<int, Stat>> filtered;
        filtered.reserve(s_Local.size());
        
        for (auto& [id, st] : s_Local)
        {
            // Exclude ignored currencies
            if (filter.excludeIgnored && st.isIgnored) continue;

            // Exclude zero count currencies
            if (filter.excludeZeroCount && st.count == 0) continue;

            // Exclude favorites
            if (filter.excludeFavorites && st.isFavorite) continue;

            // Exclude non-favorites
            if (filter.excludeNonFavorites && !st.isFavorite) continue;

            // Search term filter (case-insensitive)
            if (!filter.searchTerm.empty())
            {
                std::string currencyName = st.details.loaded ? st.details.name : ("Currency #" + std::to_string(id));
                std::string searchTermLower = filter.searchTerm;
                std::string currencyNameLower = currencyName;
                std::transform(searchTermLower.begin(), searchTermLower.end(), searchTermLower.begin(), ::tolower);
                std::transform(currencyNameLower.begin(), currencyNameLower.end(), currencyNameLower.begin(), ::tolower);
                if (currencyNameLower.find(searchTermLower) == std::string::npos) continue;
            }

            filtered.push_back({id, st});
        }

        s_Local = std::move(filtered);
    }
    
    s_LastKey = key;
    s_LastFilter = filter;
    s_Initialized = true;
    return s_Local;
}

// Multi-Sort implementation
std::vector<std::pair<int, Stat>> ItemTracker::GetSortedItems(SortMode mode, bool)
{
    bool favoritesFirst;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        favoritesFirst = g_Settings.itemsFavoritesFirst; // per-tab setting (Drops > Settings)
    }
    auto items = GetFilteredItems();
    std::vector<std::pair<int, Stat>> sorted(items.begin(), items.end());

    std::sort(sorted.begin(), sorted.end(), [mode, favoritesFirst](const auto& a, const auto& b) {
        const auto& statA = a.second;
        const auto& statB = b.second;

        // Favorites first if enabled
        if (favoritesFirst)
        {
            if (statA.isFavorite != statB.isFavorite)
                return statA.isFavorite > statB.isFavorite;
        }

        switch (mode)
        {
            case SortMode::PriceDesc:
                return GetStatProfit(statA) > GetStatProfit(statB);
            case SortMode::PriceAsc:
                return GetStatProfit(statA) < GetStatProfit(statB);
            case SortMode::CountDesc:
                return std::abs(statA.count) > std::abs(statB.count);
            case SortMode::CountAsc:
                return std::abs(statA.count) < std::abs(statB.count);
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
    bool favoritesFirst;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        favoritesFirst = g_Settings.currenciesFavoritesFirst; // per-tab setting (Drops > Settings)
    }
    auto currencies = GetFilteredCurrencies();
    std::vector<std::pair<int, Stat>> sorted(currencies.begin(), currencies.end());

    std::sort(sorted.begin(), sorted.end(), [mode, favoritesFirst](const auto& a, const auto& b) {
        const auto& statA = a.second;
        const auto& statB = b.second;

        // Favorites first if enabled
        if (favoritesFirst)
        {
            if (statA.isFavorite != statB.isFavorite)
                return statA.isFavorite > statB.isFavorite;
        }

        switch (mode)
        {
            case SortMode::PriceDesc:
                return GetStatProfit(statA) > GetStatProfit(statB);
            case SortMode::PriceAsc:
                return GetStatProfit(statA) < GetStatProfit(statB);
            case SortMode::CountDesc:
                return std::abs(statA.count) > std::abs(statB.count);
            case SortMode::CountAsc:
                return std::abs(statA.count) < std::abs(statB.count);
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
    // If ignored, return 0 profit
    if (stat.isIgnored)
        return 0;
    
    // Use custom profit if set
    if (stat.HasCustomProfit())
        return CustomProfitManager::GetCustomProfit(stat.apiId) * stat.count;

    // Coins are counted directly
    if (stat.IsCoin())
        return stat.count;

    // Salvage kits: Calculate cost per use and subtract from profit
    if (s_SalvageKits.find(stat.apiId) != s_SalvageKits.end())
    {
        const auto& kitInfo = s_SalvageKits[stat.apiId];
        
        // Only infinite kits have cost per use - finite kits are paid upfront
        if (kitInfo.infinite)
        {
            // Infinite kits use the predefined cost per use
            long long costPerUse = kitInfo.costPerUse;
            // Cost is negative (expense), so return negative value
            return costPerUse * stat.count;
        }
        
        // Finite kits: costs already paid at purchase, no per-use cost
        return 0;
    }

    // Calculate all possible sell values per unit using helper functions
    long long vendorPrice = CanSellToVendor(stat.details) ? (long long)stat.details.vendorValue : 0;
    long long tpSellPrice = CanSellOnTp(stat.details) ? TpSellProceedsPerUnitCopper(stat.details) : 0;

    long long maxPrice = std::max(vendorPrice, tpSellPrice);

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

std::string ItemTracker::ExportFavoritesToJson()
{
    nlohmann::json exportData;
    exportData["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

    nlohmann::json itemsArray = nlohmann::json::array();
    auto items = GetFavoriteItems();
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
    auto currencies = GetFavoriteCurrencies();
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
    // Snapshot ignored state BEFORE acquiring s_Mutex (deadlock prevention)
    std::set<int> ignoredItems        = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> ignoredCurrencies    = IgnoredItemsManager::GetIgnoredCurrencies();
    std::set<int> sessionIgnoredItems  = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> sessionIgnoredCurrencies = SessionIgnoreManager::GetIgnoredCurrencies();
    std::set<int> skipOnceItems        = SkipOnceManager::GetSkippedItems();
    std::set<int> skipOnceCurrencies   = SkipOnceManager::GetSkippedCurrencies();

    std::map<int, Stat> itemsCopy, currenciesCopy;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        itemsCopy      = s_Items;
        currenciesCopy = s_Currencies;
    }

    long long total = 0;

    for (auto& [id, stat] : itemsCopy)
    {
        // Always use live ignored state, not the stale copy in stat.isIgnored
        stat.isIgnored = (ignoredItems.count(id) > 0) || (sessionIgnoredItems.count(id) > 0) || (skipOnceItems.count(id) > 0);
        if (PassesFilter(stat))
            total += GetStatProfit(stat);
    }

    for (auto& [id, stat] : currenciesCopy)
    {
        stat.isIgnored = (ignoredCurrencies.count(id) > 0) || (sessionIgnoredCurrencies.count(id) > 0) || (skipOnceCurrencies.count(id) > 0);
        if (PassesFilter(stat))
            total += GetStatProfit(stat);
    }

    return total;
}

void ItemTracker::UpdateProfitHistory()
{
    auto now = std::chrono::system_clock::now();
    
    // Check interval first without holding any locks
    {
        std::lock_guard<std::mutex> lock(s_ProfitHistoryMutex);
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - s_LastHistoryUpdate);
        
        if (duration.count() < HISTORY_UPDATE_INTERVAL_SECONDS)
            return; // Not time to update yet
        
        s_LastHistoryUpdate = now;
    }
    
    // Get data outside of profit history lock to maintain lock order: 3 → 5 → 2
    auto sessionDuration = GetSessionDuration();
    
    if (sessionDuration.count() < 1)
        return;
    
    // Calculate profit outside of lock to avoid deadlock
    long long totalProfit = CalcTotalCustomProfit();
    
    // Read profit goal settings BEFORE acquiring s_ProfitHistoryMutex to prevent
    // any lock-order inversion (s_ProfitHistoryMutex → SettingsMutex) —
    // future-safe, in case any outer caller ever holds SettingsMutex and calls us.
    bool notifyProfitGoal;
    int  profitGoalAmount;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        notifyProfitGoal = g_Settings.notifyProfitGoal;
        profitGoalAmount = g_Settings.profitGoalAmount;
    }

    // Now acquire profit history lock at the end
    {
        std::lock_guard<std::mutex> lock(s_ProfitHistoryMutex);

        if (notifyProfitGoal && !s_ProfitGoalReached)
        {
            if (totalProfit >= static_cast<long long>(profitGoalAmount))
            {
                char msg[256];
                int gold = profitGoalAmount / 10000;
                snprintf(msg, sizeof(msg), Localization::GetText("profit_goal_reached_msg"), gold);
                UINotifications::AddGenericNotification(Localization::GetText("profit_goal_reached_title"), msg, "", "Legendary", true);
                s_ProfitGoalReached = true;
            }
        }

        double hours = static_cast<double>(sessionDuration.count()) / 3600.0;
        long long currentProfitPerHour = 0;
        if (hours > 0.0001) // Prevent division by near-zero or overflow
            currentProfitPerHour = static_cast<long long>(totalProfit / hours);
        
        // Add new entry
        ProfitHistoryEntry entry;
        entry.timestamp = now;
        entry.profitPerHour = currentProfitPerHour;
        
        s_ProfitHistory.push_back(entry);

        // Keep only last MAX_HISTORY_ENTRIES entries
        while (s_ProfitHistory.size() > MAX_HISTORY_ENTRIES)
            s_ProfitHistory.pop_front();
    }
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

std::vector<std::pair<std::chrono::system_clock::time_point, long long>> ItemTracker::GetProfitHistory()
{
    std::lock_guard<std::mutex> lock(s_ProfitHistoryMutex);
    std::vector<std::pair<std::chrono::system_clock::time_point, long long>> result;
    for (const auto& entry : s_ProfitHistory)
    {
        result.push_back({entry.timestamp, entry.profitPerHour});
    }
    return result;
}

long long ItemTracker::TpSellProceedsPerUnitCopper(const ApiDetails& d)
{
    if (d.accountBound || d.tpSellPrice <= 0) return 0;
    // Apply 15% TP fee (85/100)
    return static_cast<long long>(d.tpSellPrice * 85.0 / 100.0);
}

long long ItemTracker::TpBuyProceedsPerUnitCopper(const ApiDetails& d)
{
    if (d.accountBound || d.tpBuyPrice <= 0) return 0;
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
    // Snapshot ignored state BEFORE acquiring s_Mutex (deadlock prevention)
    std::set<int> ignoredItems       = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> sessionIgnoredItems = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> skipOnceItems       = SkipOnceManager::GetSkippedItems();

    std::map<int, Stat> itemsCopy;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        itemsCopy = s_Items;
    }

    long long total = 0;
    for (auto& [id, stat] : itemsCopy)
    {
        bool isIgnored = (ignoredItems.count(id) > 0) || (sessionIgnoredItems.count(id) > 0) || (skipOnceItems.count(id) > 0);
        if (!stat.details.loaded || isIgnored) continue;
        long long per = TpSellProceedsPerUnitCopper(stat.details);
        if (per > 0) total += stat.count * per;
    }
    return total;
}

long long ItemTracker::CalcTotalTpInstantProfit()
{
    // Snapshot ignored state BEFORE acquiring s_Mutex (deadlock prevention)
    std::set<int> ignoredItems       = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> sessionIgnoredItems = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> skipOnceItems       = SkipOnceManager::GetSkippedItems();

    std::map<int, Stat> itemsCopy;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        itemsCopy = s_Items;
    }

    long long total = 0;
    for (auto& [id, stat] : itemsCopy)
    {
        bool isIgnored = (ignoredItems.count(id) > 0) || (sessionIgnoredItems.count(id) > 0) || (skipOnceItems.count(id) > 0);
        if (!stat.details.loaded || isIgnored) continue;
        long long per = TpBuyProceedsPerUnitCopper(stat.details);
        if (per > 0) total += stat.count * per;
    }
    return total;
}

long long ItemTracker::CalcTotalVendorProfit()
{
    // Snapshot ignored state BEFORE acquiring s_Mutex (deadlock prevention)
    std::set<int> ignoredItems       = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> sessionIgnoredItems = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> skipOnceItems       = SkipOnceManager::GetSkippedItems();

    std::map<int, Stat> itemsCopy;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        itemsCopy = s_Items;
    }

    long long total = 0;
    for (auto& [id, stat] : itemsCopy)
    {
        bool isIgnored = (ignoredItems.count(id) > 0) || (sessionIgnoredItems.count(id) > 0) || (skipOnceItems.count(id) > 0);
        if (!stat.details.loaded || isIgnored) continue;
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

// === Persistence Functions ===

static std::unique_ptr<SaveSnapshot> CreateSaveSnapshot(const char* addonDir)
{
    auto snap = std::make_unique<SaveSnapshot>();
    snap->dataPath = std::string(addonDir) + "\\farming_data.json";

    snap->timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    {
        std::lock_guard<std::mutex> ssLock(s_SessionStartMutex);
        snap->sessionStart = std::chrono::system_clock::to_time_t(s_SessionStart);
    }

    snap->magicFind = s_MagicFind.load();

    // Global Lock Order: 1. s_PersistentMutex, 4. s_SessionDropsMutex, 5. s_Mutex
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);

        for (const auto& [id, stat] : s_Items)
        {
            PersistedStatSaveView sv;
            sv.count = stat.count;
            sv.isFavorite = stat.isFavorite;
            sv.lastMagicFind = stat.lastMagicFind;
            snap->items.emplace(id, sv);
        }

        for (const auto& [id, stat] : s_Currencies)
        {
            PersistedStatSaveView sv;
            sv.count = stat.count;
            sv.isFavorite = stat.isFavorite;
            sv.lastMagicFind = stat.lastMagicFind;
            snap->currencies.emplace(id, sv);
        }

        snap->sessionDrops = s_SessionDrops;
        snap->persistentFavoriteItems = s_PersistentFavoriteItems;
        snap->persistentFavoriteCurrencies = s_PersistentFavoriteCurrencies;
    }

    snap->ignoredItems = IgnoredItemsManager::GetIgnoredItems();
    snap->ignoredCurrencies = IgnoredItemsManager::GetIgnoredCurrencies();
    snap->customProfits = CustomProfitManager::GetAllCustomProfitsDetailed();
    snap->pinnedItemsJson = PinnedItemsManager::ExportToJson();

    return snap;
}

void ItemTracker::SaveData(const char* addonDir)
{
    if (!addonDir)
        return;

    if (!s_SaveWorkerShutdown.load(std::memory_order_acquire))
    {
        auto snap = CreateSaveSnapshot(addonDir);
        {
            std::lock_guard<std::mutex> lk(s_SaveQueueMutex);
            s_PendingSave = std::move(snap);
        }
        s_SaveQueueCv.notify_one();
    }
    else
    {
        SaveDataImmediate(addonDir);
    }
}

void ItemTracker::SaveDataImmediate(const char* addonDir)
{
    if (!addonDir)
        return;

    auto snap = CreateSaveSnapshot(addonDir);
    WriteSnapshotToDisk(*snap);
}

void ItemTracker::LoadData(const char* addonDir)
{
    if (!addonDir)
        return;

    std::string dataPath = std::string(addonDir) + "\\farming_data.json";
    
    // Bump version before loading to invalidate any cached data
    s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
    
    std::ifstream file(dataPath);
    if (!file.is_open())
    {
        if (APIDefs) APIDefs->Log(LOGL_INFO, "FarmingTracker", "No existing farming data found (normal for first start).");
        return;
    }

    try
    {
        nlohmann::json data;
        file >> data;
        file.close();
        
        if (APIDefs) APIDefs->Log(LOGL_INFO, "FarmingTracker", "Loading farming data...");

        // Clear existing persistent stores before loading from file to ensure a clean state
        {
            std::lock_guard<std::mutex> pLock(s_PersistentMutex);
            s_PersistentFavoriteItems.clear();
            s_PersistentFavoriteCurrencies.clear();
        }
        IgnoredItemsManager::ClearAll();
        CustomProfitManager::ClearAll();
        PinnedItemsManager::ClearAll();

        // Load session start if available
        if (data.contains("sessionStart"))
        {
            std::lock_guard<std::mutex> sessionLock(s_SessionStartMutex);
            time_t sessionStartT = data["sessionStart"].get<time_t>();
            s_SessionStart = std::chrono::system_clock::from_time_t(sessionStartT);
        }

        // Load Magic Find
        if (data.contains("magicFind"))
        {
            s_MagicFind.store(data["magicFind"].get<int>());
        }

        // Load ignored items
        if (data.contains("ignoredItems") && data["ignoredItems"].is_array())
        {
            for (const auto& idJson : data["ignoredItems"])
                IgnoredItemsManager::IgnoreItem(idJson.get<int>());
        }
        if (data.contains("ignoredCurrencies") && data["ignoredCurrencies"].is_array())
        {
            for (const auto& idJson : data["ignoredCurrencies"])
                IgnoredItemsManager::IgnoreCurrency(idJson.get<int>());
        }

        // Load favorites (Load these BEFORE items/currencies so they can be applied)
        if (data.contains("favoriteItems") && data["favoriteItems"].is_array())
        {
            std::lock_guard<std::mutex> pLock(s_PersistentMutex);
            for (const auto& idJson : data["favoriteItems"])
                s_PersistentFavoriteItems.insert(idJson.get<int>());
        }
        if (data.contains("favoriteCurrencies") && data["favoriteCurrencies"].is_array())
        {
            std::lock_guard<std::mutex> pLock(s_PersistentMutex);
            for (const auto& idJson : data["favoriteCurrencies"])
                s_PersistentFavoriteCurrencies.insert(idJson.get<int>());
        }

        // Load custom profits
        if (data.contains("customProfits") && data["customProfits"].is_object())
        {
            for (auto it = data["customProfits"].begin(); it != data["customProfits"].end(); ++it)
            {
                int id = std::stoi(it.key());
                if (it.value().is_object())
                {
                    long long profit = it.value().value("profit", 0LL);
                    StatType type = static_cast<StatType>(it.value().value("type", static_cast<int>(StatType::Item)));
                    CustomProfitManager::SetCustomProfit(id, profit, type);
                }
                else if (it.value().is_number())
                {
                    // Backward compatibility
                    long long profit = it.value().get<long long>();
                    CustomProfitManager::SetCustomProfit(id, profit, StatType::Item);
                }
            }
        }

        // Load pinned items
        if (data.contains("pinnedItems"))
        {
            PinnedItemsManager::ImportFromJson(data["pinnedItems"]);
        }

        // Load items
        if (data.contains("items") && data["items"].is_array())
        {
            // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
            std::lock_guard<std::mutex> pLock(s_PersistentMutex);
            std::lock_guard<std::mutex> lock(s_Mutex);
            
            for (const auto& itemJson : data["items"])
            {
                int apiId = itemJson["apiId"].get<int>();
                long long count = itemJson["count"].get<long long>();
                int lastMF = itemJson.value("lastMagicFind", -1);
                
                // Check persistent store for favorite status (pLock already held)
                bool isFavorite = itemJson.value("isFavorite", false);
                if (s_PersistentFavoriteItems.count(apiId) > 0)
                    isFavorite = true;

                Stat s;
                s.apiId = apiId;
                s.type = StatType::Item;
                s.count = count;
                s.isFavorite = isFavorite;
                s.lastMagicFind = lastMF;
                s_Items[apiId] = s;
                
                // Ensure it's in the persistent store if it was marked favorite in the item list (migration)
                if (isFavorite)
                {
                    s_PersistentFavoriteItems.insert(apiId);
                }
            }
        }

        // Load currencies
        if (data.contains("currencies") && data["currencies"].is_array())
        {
            // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
            std::lock_guard<std::mutex> pLock(s_PersistentMutex);
            std::lock_guard<std::mutex> lock(s_Mutex);
            
            for (const auto& currencyJson : data["currencies"])
            {
                int apiId = currencyJson["apiId"].get<int>();
                long long count = currencyJson["count"].get<long long>();
                int lastMF = currencyJson.value("lastMagicFind", -1);

                // Check persistent store for favorite status (pLock already held)
                bool isFavorite = currencyJson.value("isFavorite", false);
                if (s_PersistentFavoriteCurrencies.count(apiId) > 0)
                    isFavorite = true;

                Stat s;
                s.apiId = apiId;
                s.type = StatType::Currency;
                s.count = count;
                s.isFavorite = isFavorite;
                s.lastMagicFind = lastMF;
                s_Currencies[apiId] = s;

                // Ensure it's in the persistent store if it was marked favorite in the item list (migration)
                if (isFavorite)
                {
                    s_PersistentFavoriteCurrencies.insert(apiId);
                }
            }
        }

        // Load session drops (for Timeline tab)
        if (data.contains("sessionDrops") && data["sessionDrops"].is_array())
        {
            std::lock_guard<std::mutex> lock(s_SessionDropsMutex);
            s_SessionDrops.clear();
            for (const auto& dropJson : data["sessionDrops"])
            {
                SessionHistory::DropEntry drop;
                drop.itemId = dropJson.value("itemId", 0);
                drop.itemName = dropJson.value("itemName", "");
                drop.iconUrl = dropJson.value("iconUrl", "");
                drop.isCurrency = dropJson.value("isCurrency", false);
                drop.rarity = dropJson.value("rarity", "");
                drop.count = dropJson.value("count", 0);
                drop.totalValue = dropJson.value("totalValue", 0LL);
                drop.magicFind = dropJson.value("magicFind", -1);
                drop.timestamp = dropJson.value("timestamp", "");
                drop.characterName = dropJson.value("characterName", "");
                s_SessionDrops.push_back(drop);
            }
            s_SessionDropsVersion.fetch_add(1, std::memory_order_relaxed);
        }
        
        if (APIDefs) APIDefs->Log(LOGL_INFO, "FarmingTracker", "Farming data loaded successfully.");
    }
    catch (const std::exception& e)
    {
        if (APIDefs) APIDefs->Log(LOGL_CRITICAL, "FarmingTracker", ("Failed to parse farming data: " + std::string(e.what())).c_str());
    }
    catch (...)
    {
        if (APIDefs) APIDefs->Log(LOGL_CRITICAL, "FarmingTracker", "Unknown error loading farming data!");
    }
}

void ItemTracker::ImportFromJson(const nlohmann::json& j)
{
    if (!j.is_object()) return;

    std::lock_guard<std::mutex> lock(s_Mutex);

    // Import items
    if (j.contains("items") && j["items"].is_array())
    {
        for (const auto& itemJson : j["items"])
        {
            if (!itemJson.contains("id")) continue;
            int id = itemJson["id"].get<int>();

            auto it = s_Items.find(id);
            if (it == s_Items.end())
            {
                Stat s;
                s.apiId = id;
                s.type = StatType::Item;
                s.count = 0;
                s_Items[id] = s;
                it = s_Items.find(id);
            }

            Stat& st = it->second;
            if (itemJson.contains("count")) st.count = itemJson["count"].get<long long>();
            if (itemJson.contains("isFavorite")) st.isFavorite = itemJson["isFavorite"].get<bool>();
            if (itemJson.contains("isIgnored")) st.isIgnored = itemJson["isIgnored"].get<bool>();
            if (itemJson.contains("customProfit")) CustomProfitManager::SetCustomProfit(id, itemJson["customProfit"].get<long long>(), StatType::Item);
        }
    }

    // Import currencies
    if (j.contains("currencies") && j["currencies"].is_array())
    {
        for (const auto& currencyJson : j["currencies"])
        {
            if (!currencyJson.contains("id")) continue;
            int id = currencyJson["id"].get<int>();

            auto it = s_Currencies.find(id);
            if (it == s_Currencies.end())
            {
                Stat s;
                s.apiId = id;
                s.type = StatType::Currency;
                s.count = 0;
                s_Currencies[id] = s;
                it = s_Currencies.find(id);
            }

            Stat& st = it->second;
            if (currencyJson.contains("count")) st.count = currencyJson["count"].get<long long>();
            if (currencyJson.contains("isFavorite")) st.isFavorite = currencyJson["isFavorite"].get<bool>();
            if (currencyJson.contains("isIgnored")) st.isIgnored = currencyJson["isIgnored"].get<bool>();
            if (currencyJson.contains("customProfit")) CustomProfitManager::SetCustomProfit(id, currencyJson["customProfit"].get<long long>(), StatType::Currency);
        }
    }
}

void ItemTracker::ImportFavoritesFromJson(const nlohmann::json& j)
{
    std::set<int> newFavoriteItems;
    std::set<int> newFavoriteCurrencies;

    if (j.is_object())
    {
        if (j.contains("favoriteItems") && j["favoriteItems"].is_array())
        {
            for (const auto& v : j["favoriteItems"])
            {
                if (v.is_number_integer())
                    newFavoriteItems.insert(v.get<int>());
            }
        }
        if (j.contains("favoriteCurrencies") && j["favoriteCurrencies"].is_array())
        {
            for (const auto& v : j["favoriteCurrencies"])
            {
                if (v.is_number_integer())
                    newFavoriteCurrencies.insert(v.get<int>());
            }
        }

        if (newFavoriteItems.empty() && newFavoriteCurrencies.empty() && j.contains("favorites") && j["favorites"].is_array())
        {
            for (const auto& v : j["favorites"])
            {
                if (v.is_number_integer())
                    newFavoriteItems.insert(v.get<int>());
            }
        }
    }
    else if (j.is_array())
    {
        for (const auto& v : j)
        {
            if (v.is_number_integer())
                newFavoriteItems.insert(v.get<int>());
        }
    }
    else
    {
        return;
    }

    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        s_PersistentFavoriteItems = newFavoriteItems;
        s_PersistentFavoriteCurrencies = newFavoriteCurrencies;
    }

    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        for (auto& [id, st] : s_Items)
            st.isFavorite = (newFavoriteItems.count(id) > 0);
        for (auto& [id, st] : s_Currencies)
            st.isFavorite = (newFavoriteCurrencies.count(id) > 0);
    }

    for (int id : newFavoriteItems)
        IgnoredItemsManager::UnignoreItem(id);
    for (int id : newFavoriteCurrencies)
        IgnoredItemsManager::UnignoreCurrency(id);
}

std::vector<int> ItemTracker::CollectPendingItemIds()
{
    // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);

    std::set<int> ids;
    for (auto& [id, st] : s_Items)
    {
        // Only load data for items that are not loaded yet
        if (!st.details.loaded)
            ids.insert(id);
    }
    
    // Also include persistent items (favorites, ignored, custom profit)
    for (int id : s_PersistentFavoriteItems)
    {
        auto it = s_Items.find(id);
        if (it == s_Items.end() || !it->second.details.loaded)
            ids.insert(id);
    }
    
    // Ignored items
    auto ignoredItems = IgnoredItemsManager::GetIgnoredItems();
    for (int id : ignoredItems)
    {
        auto it = s_Items.find(id);
        if (it == s_Items.end() || !it->second.details.loaded)
            ids.insert(id);
    }

    // Custom profits
    auto allCustomProfits = CustomProfitManager::GetAllCustomProfitsDetailed();
    for (const auto& [id, entry] : allCustomProfits)
    {
        if (entry.type == StatType::Item)
        {
            auto it = s_Items.find(id);
            if (it == s_Items.end() || !it->second.details.loaded)
                ids.insert(id);
        }
    }

    // Also load data for salvage kits that are tracked as currencies
    for (auto& [id, st] : s_Currencies)
    {
        if (s_SalvageKits.find(id) != s_SalvageKits.end() && !st.details.loaded)
            ids.insert(id);
    }
    return std::vector<int>(ids.begin(), ids.end());
}

bool ItemTracker::NeedCurrencyTable()
{
    // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    // Check current session currencies
    for (auto& [id, st] : s_Currencies)
    {
        if (id == 1) continue;
        if (!st.details.loaded) return true;
    }

    // Check persistent currencies
    for (int id : s_PersistentFavoriteCurrencies)
    {
        if (id == 1) continue;
        auto it = s_Currencies.find(id);
        if (it == s_Currencies.end() || !it->second.details.loaded)
            return true;
    }

    auto ignoredCurrencies = IgnoredItemsManager::GetIgnoredCurrencies();
    for (int id : ignoredCurrencies)
    {
        if (id == 1) continue;
        auto it = s_Currencies.find(id);
        if (it == s_Currencies.end() || !it->second.details.loaded)
            return true;
    }

    auto allCustomProfits = CustomProfitManager::GetAllCustomProfitsDetailed();
    for (const auto& [id, entry] : allCustomProfits)
    {
        if (entry.type == StatType::Currency && id != 1)
        {
            auto it = s_Currencies.find(id);
            if (it == s_Currencies.end() || !it->second.details.loaded)
                return true;
        }
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

void ItemTracker::ApplyItemsFromApi(const std::vector<int>& requestedIds, const json& itemsArray, const json& pricesArray)
{
    if (!itemsArray.is_array() || !pricesArray.is_array()) return;

    {
        // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);

        bool modified = false;

        std::set<int> receivedItemIds;
        for (auto& item : itemsArray)
        {
            if (!item.contains("id")) continue;
            int id = item["id"].get<int>();
            receivedItemIds.insert(id);

            // Check if this is already a currency (skip if so!)
            if (s_Currencies.find(id) != s_Currencies.end())
            {
                // Already a currency, don't add to items
                continue;
            }

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
                if (curIt == s_Currencies.end())
                {
                    // Create the currency entry if it's a salvage kit we want to track
                    Stat s;
                    s.apiId = id;
                    s.type = StatType::Currency;
                    s.count = 0;
                    s_Currencies[id] = s;
                    st = &s_Currencies[id];
                    modified = true;
                }
                else
                {
                    st = &curIt->second;
                }
            }
            else
            {
                // This is an item from the API that isn't in our session yet.
                // We create it to store its details (name, icon) for persistent displays (Favorites/Ignored).
                Stat s;
                s.apiId = id;
                s.type = StatType::Item;
                s.count = 0;
                s_Items[id] = s;
                st = &s_Items[id];
                modified = true;

                // Re-apply persistent flags
                st->isFavorite = s_PersistentFavoriteItems.count(id) > 0;
                st->isIgnored = IgnoredItemsManager::IsItemIgnored(id);
            }

            if (!st) continue;

            st->details.name        = item.value("name", "");
            st->details.description = item.value("description", "");
            st->details.vendorValue = item.value("vendor_value", 0);
            st->details.rarity      = item.value("rarity", std::string());
            st->details.level       = item.value("level", 0);
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

                // Extract upgrade component type if applicable
                if (t == "UpgradeComponent" && item.contains("details") && item["details"].contains("type") && item["details"]["type"].is_string())
                {
                    st->details.upgradeComponentType = item["details"]["type"].get<std::string>();
                }
            }

            // Mark for notification — will be processed after locks are released
            if (st->notificationPending)
                st->notificationPending = true; // already set, keep it
            // Details are now loaded; allow notification to fire
            st->notificationPending = true;
        }

        // --- Handle missing IDs (those that weren't returned by the API) ---
        for (int id : requestedIds)
        {
            if (receivedItemIds.count(id) > 0) continue;

            // If ID was requested but not returned, it's likely invalid for the /v2/items endpoint.
            // We keep it as not loaded so we keep trying to load from API in case it becomes available later.
            auto it = s_Items.find(id);
            if (it != s_Items.end())
            {
                it->second.details.loaded = false; // Keep trying to load from API
                it->second.details.knownByApi = false;
                
                // Check if this ID might actually be a currency (like the user's ID 45 example)
                if (s_Currencies.find(id) != s_Currencies.end())
                {
                    it->second.details.name = "ID " + std::to_string(id) + " is a Currency, not an Item!";
                }
                else
                {
                    it->second.details.name = "Unknown Item (" + std::to_string(id) + ")";
                }
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

            if (!st)
            {
                // Item not found in tracker - might be a price for an item we don't have
                continue;
            }

            bool hasSellPrice = false;
            bool hasBuyPrice = false;

            if (pr.contains("sells") && pr["sells"].contains("unit_price"))
            {
                st->details.tpSellPrice = pr["sells"]["unit_price"].get<int>();
                hasSellPrice = true;
            }

            if (pr.contains("buys") && pr["buys"].contains("unit_price"))
            {
                st->details.tpBuyPrice = pr["buys"]["unit_price"].get<int>();
                hasBuyPrice = true;
            }

            // Log if item has no prices but is in inventory
            if (st->count > 0 && !hasSellPrice && !hasBuyPrice)
            {
                static int loggedCount = 0;
                if (loggedCount < 5)
                {
                    Gw2Api::Log("Item " + std::to_string(id) + " (" + st->details.name + ") has no TP prices (no buy/sell orders)", "warning");
                    loggedCount++;
                }
            }

            // Prices updated — re-mark for notification
            st->notificationPending = true;
        }

        if (modified)
            s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
    } // ← All locks released here

    // --- Backfill loot log entries that had unknown price (-1) ---
    // Done AFTER lock release to avoid deadlock with LootLogger mutex.
    // Items that were dropped before API data arrived have sellPriceTp == -1;
    // now that we have prices, update them so the Loot Log shows correct values.
    {
        std::map<int, long long> backfillPrices;
        {
            std::lock_guard<std::mutex> lock(s_Mutex);
            for (auto& pr : pricesArray)
            {
                if (!pr.contains("id")) continue;
                int id = pr["id"].get<int>();
                auto it = s_Items.find(id);
                if (it == s_Items.end()) continue;
                long long up = TpSellProceedsPerUnitCopper(it->second.details);
                if (up <= 0)
                    up = CanSellToVendor(it->second.details) ? (long long)it->second.details.vendorValue : 0;
                if (up > 0)
                    backfillPrices[id] = up;
            }
        }
        if (!backfillPrices.empty())
        {
            std::lock_guard<std::mutex> lootLock(LootLogger::GetSessionEntriesMutex());
            auto& entries = LootLogger::GetSessionEntriesRef();
            for (auto& e : entries)
            {
                if (e.sellPriceTp >= 0) continue; // already known
                auto it = backfillPrices.find(e.itemId);
                if (it != backfillPrices.end())
                    e.sellPriceTp = it->second;
            }
        }
    }

    // Process notifications AFTER releasing all locks to avoid deadlock with UINotifications
    ProcessPendingNotifications();
}

void ItemTracker::ClearItemDetails()
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    // Clear all item details
    for (auto& [id, st] : s_Items)
    {
        st.details.loaded = false;
        st.details.name.clear();
        st.details.description.clear();
        st.details.iconUrl.clear();
        st.details.vendorValue = 0;
        st.details.tpBuyPrice = 0;
        st.details.tpSellPrice = 0;
        st.details.noSell = false;
        st.details.accountBound = false;
        st.details.rarity.clear();
        st.details.itemType = ItemType::Unknown;
        st.details.knownByApi = false;
        st.notificationPending = false; // Clear pending notifications on load
    }

    // Clear all currency details
    for (auto& [id, st] : s_Currencies)
    {
        st.details.loaded = false;
        st.details.name.clear();
        st.details.description.clear();
        st.details.iconUrl.clear();
        st.details.vendorValue = 0;
        st.details.tpBuyPrice = 0;
        st.details.tpSellPrice = 0;
        st.details.noSell = false;
        st.details.accountBound = false;
        st.details.rarity.clear();
        st.details.itemType = ItemType::Unknown;
        st.details.knownByApi = false;
        st.notificationPending = false; // Clear pending notifications on load
    }
}

void ItemTracker::ForceReloadAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (auto& [id, st] : s_Items)
    {
        if (st.count != 0)
            st.details.loaded = false;
    }
    for (auto& [id, st] : s_Currencies)
    {
        if (st.count != 0)
            st.details.loaded = false;
    }
}

void ItemTracker::ApplyCurrencyTable(const json& currenciesArray)
{
    if (!currenciesArray.is_array()) return;

    // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);

    bool modified = false;

    for (auto& c : currenciesArray)
    {
        if (!c.contains("id")) continue;
        int id = c["id"].get<int>();
        
        // Remove this ID from items map if present (so currencies aren't shown in items tab!)
        s_Items.erase(id);
        
        auto it = s_Currencies.find(id);
        
        Stat* st = nullptr;
        if (it == s_Currencies.end())
        {
            // Only add currencies that are persistent (favorites, ignored, custom profit)
            // or if they are basic coins (ID 1)
            bool isPersistent = false;
            if (s_PersistentFavoriteCurrencies.count(id) > 0) isPersistent = true;
            if (IgnoredItemsManager::IsCurrencyIgnored(id)) isPersistent = true;
            if (CustomProfitManager::HasCustomProfit(id) && CustomProfitManager::GetType(id) == StatType::Currency) isPersistent = true;
            
            if (id == 1 || isPersistent)
            {
                Stat s;
                s.apiId = id;
                s.type = StatType::Currency;
                s.count = 0;
                s_Currencies[id] = s;
                st = &s_Currencies[id];
                modified = true;

                // Re-apply flags
                st->isFavorite = s_PersistentFavoriteCurrencies.count(id) > 0;
                st->isIgnored = IgnoredItemsManager::IsCurrencyIgnored(id);
            }
        }
        else
        {
            st = &it->second;
        }

        if (!st) continue;

        st->details.name = c.value("name", "");
        st->details.description = c.value("description", "");
        if (c.contains("icon") && c["icon"].is_string())
            st->details.iconUrl = BuildIconUrl(c["icon"].get<std::string>());
        st->details.loaded = true;
        st->details.knownByApi = true;
    }

    if (modified)
        s_ItemsStateVersion.fetch_add(1, std::memory_order_relaxed);
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

std::pair<int, Stat> ItemTracker::GetBestDropTotalValue()
{
    // Get ignored snapshots BEFORE locking s_Mutex
    std::set<int> ignoredSnapshot = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> sessionIgnoredSnapshot = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> skipOnceSnapshot = SkipOnceManager::GetSkippedItems();
    
    std::lock_guard<std::mutex> lock(s_Mutex);

    std::pair<int, Stat> bestDrop = { 0, Stat() };
    long long maxTotalProfit = LLONG_MIN;

    for (const auto& [id, stat] : s_Items)
    {
        if (stat.count == 0) continue;
        if (ignoredSnapshot.count(id) > 0 || sessionIgnoredSnapshot.count(id) > 0 || skipOnceSnapshot.count(id) > 0) continue;

        long long totalProfit = GetStatProfit(stat);
        // If no drop is selected yet or the current profit is higher, take this one
        if (bestDrop.first == 0 || totalProfit > maxTotalProfit)
        {
            maxTotalProfit = totalProfit;
            bestDrop = { id, stat };
        }
    }

    return bestDrop;
}

std::pair<int, Stat> ItemTracker::GetBestDrop()
{
    // Get ignored snapshots BEFORE locking s_Mutex
    std::set<int> ignoredSnapshot = IgnoredItemsManager::GetIgnoredItems();
    std::set<int> sessionIgnoredSnapshot = SessionIgnoreManager::GetIgnoredItems();
    std::set<int> skipOnceSnapshot = SkipOnceManager::GetSkippedItems();
    
    std::lock_guard<std::mutex> lock(s_Mutex);

    std::pair<int, Stat> bestDrop = { 0, Stat() };
    long long maxUnitProfit = LLONG_MIN;

    for (const auto& [id, stat] : s_Items)
    {
        if (stat.count == 0) continue;
        if (ignoredSnapshot.count(id) > 0 || sessionIgnoredSnapshot.count(id) > 0 || skipOnceSnapshot.count(id) > 0) continue;
        
        // Calculate unit profit using GetStatProfit(), which already handles all cases!
        long long totalProfit = GetStatProfit(stat);
        long long unitProfit = totalProfit / stat.count;

        // If no drop is selected yet or the current profit is higher, take this one
        if (bestDrop.first == 0 || unitProfit > maxUnitProfit)
        {
            maxUnitProfit = unitProfit;
            bestDrop = { id, stat };
        }
    }

    return bestDrop;
}
