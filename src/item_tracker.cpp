#include "item_tracker.h"
#include "loot_logger.h"
#include "magnetite_tracker.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "search_manager.h"
#include "settings.h"
#include "session_history.h"
#include "gw2_api.h"
#include "ui_notifications.h"
#include "localization.h"
#include "ui_common.h"
#include "../include/nlohmann/json.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>

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
        if (lowerName.find("precursor") != std::string::npos ||
            lowerDesc.find("precursor") != std::string::npos ||
            (st.details.rarity == "Exotic" && st.details.itemType == ItemType::Weapon &&
             st.details.vendorValue == 0 && !st.details.accountBound))
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
                           bool isIgnored = false, bool isFavorite = false)
{
    auto it = map.find(apiId);
    if (it != map.end())
    {
        it->second.count += delta;
        if (delta > 0)
        {
            it->second.notificationPending = true;
        }
    }
    else
    {
        Stat s;
        s.apiId = apiId;
        s.type  = type;
        s.count = delta;
        if (delta > 0) s.notificationPending = true;

        // Re-apply persistent flags (passed as parameters to avoid deadlock)
        s.isFavorite = isFavorite;
        s.isIgnored = isIgnored;

        map[apiId] = s;
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
    // Global Lock Order: 1. s_PersistentMutex, 4. s_SessionDropsMutex, 5. s_Mutex
    // All locks released before ProcessPendingNotifications to avoid recursive s_Mutex deadlock.
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

        for (auto& [id, delta] : items)
        {
            SessionHistory::DropEntry drop;
            drop.itemId = id;
            drop.itemName = ""; // Will be filled when saving
            drop.isCurrency = false;
            drop.rarity = "";
            drop.count = static_cast<int>(delta);
            drop.totalValue = 0; // Will be calculated when saving
            drop.magicFind = s_MagicFind.load();
            drop.timestamp = timestamp;
            {
                std::lock_guard<std::mutex> lock(UICommon::s_AccountNameMutex);
                drop.characterName = UICommon::s_AccountNameBuf;
            }
            s_SessionDrops.push_back(drop);
        }

        for (auto& [id, delta] : currencies)
        {
            SessionHistory::DropEntry drop;
            drop.itemId = id;
            drop.itemName = ""; // Will be filled when saving
            drop.isCurrency = true;
            drop.rarity = "";
            drop.count = static_cast<int>(delta);
            drop.totalValue = 0; // Will be calculated when saving
            drop.magicFind = s_MagicFind.load();
            drop.timestamp = timestamp;
            {
                std::lock_guard<std::mutex> lock(UICommon::s_AccountNameMutex);
                drop.characterName = UICommon::s_AccountNameBuf;
            }
            s_SessionDrops.push_back(drop);
        }

        for (auto& [id, delta] : items)
        {
            bool isFavorite = s_PersistentFavoriteItems.count(id) > 0;
            bool isIgnored = IgnoredItemsManager::IsItemIgnored(id);
            
            // Check if rarity toggle is active for this item
            // NOTE: use find() NOT operator[] to avoid creating a blank Stat(apiId=0)
            auto existIt = s_Items.find(id);
            if (!isIgnored && existIt != s_Items.end() && existIt->second.details.loaded)
            {
                std::string rarity = existIt->second.details.rarity;
                bool shouldIgnore = false;
                
                {
                    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                    if (rarity == "Junk" && g_Settings.ignoredRarityToggleJunk) shouldIgnore = true;
                    else if (rarity == "Basic" && g_Settings.ignoredRarityToggleBasic) shouldIgnore = true;
                    else if (rarity == "Fine" && g_Settings.ignoredRarityToggleFine) shouldIgnore = true;
                    else if (rarity == "Masterwork" && g_Settings.ignoredRarityToggleMasterwork) shouldIgnore = true;
                    else if (rarity == "Rare" && g_Settings.ignoredRarityToggleRare) shouldIgnore = true;
                    else if (rarity == "Exotic" && g_Settings.ignoredRarityToggleExotic) shouldIgnore = true;
                    else if (rarity == "Ascended" && g_Settings.ignoredRarityToggleAscended) shouldIgnore = true;
                    else if (rarity == "Legendary" && g_Settings.ignoredRarityToggleLegendary) shouldIgnore = true;
                }
                
                if (shouldIgnore)
                {
                    IgnoredItemsManager::IgnoreItem(id);
                    isIgnored = true;
                }
            }
            
            UpdateOrInsert(s_Items, id, delta, StatType::Item, isIgnored, isFavorite);
            s_Items[id].lastMagicFind = s_MagicFind.load();
        }

        for (auto& [id, delta] : currencies)
        {
            bool isFavorite = s_PersistentFavoriteCurrencies.count(id) > 0;
            bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(id);
            UpdateOrInsert(s_Currencies, id, delta, StatType::Currency, isIgnored, isFavorite);
            s_Currencies[id].lastMagicFind = s_MagicFind.load();
        }
    } // ← All locks released here

    // Magnetite Tracker integration
    auto magIt = currencies.find(MagnetiteTracker::CURRENCY_ID);
    if (magIt != currencies.end() && magIt->second > 0)
        MagnetiteTracker::OnDrfShardsEarned(static_cast<int>(magIt->second));

    // Loot Logger — log every drop to file immediately
    // Map name and API token are resolved inside LogDrop (cached).
    {
        int mapId = s_LastKnownMapId.load();
        std::string apiToken;
        {
            std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
            apiToken = g_Settings.gw2ApiKey;
        }

        for (const auto& [id, delta] : items)
        {
            if (delta <= 0) continue;
            auto it = s_Items.find(id);
            
            // Wait for item details to load (adaptive, max 500ms)
            std::string name = "";
            std::string rarity = "";
            std::string itemType = "";
            
            if (it != s_Items.end())
            {
                // Wait up to 500ms for details to load
                auto startTime = std::chrono::steady_clock::now();
                const auto timeout = std::chrono::milliseconds(500);
                
                while (!it->second.details.loaded)
                {
                    auto now = std::chrono::steady_clock::now();
                    if (now - startTime >= timeout)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                if (it->second.details.loaded)
                {
                    name = it->second.details.name;
                    rarity = it->second.details.rarity;
                    itemType = ItemTypeToString(it->second.details.itemType);
                }
                else
                {
                    // Fallback to item ID if name not loaded
                    char idBuf[32];
                    snprintf(idBuf, sizeof(idBuf), "Item #%d", id);
                    name = idBuf;
                }
            }
            else
            {
                char idBuf[32];
                snprintf(idBuf, sizeof(idBuf), "Item #%d", id);
                name = idBuf;
            }
            
            long long price = (it != s_Items.end()) ? GetStatProfit(it->second) : -1;
            LootLogger::LogDrop(id, name, delta, false, itemType, rarity, price,
                                s_LastKnownMapId, LootLogger::ResolveMapName(s_LastKnownMapId, apiToken));
        }

        for (const auto& [id, delta] : currencies)
        {
            if (delta <= 0) continue;
            auto it = s_Currencies.find(id);
            
            // Wait for currency details to load (adaptive, max 500ms)
            std::string name = "";
            
            if (it != s_Currencies.end())
            {
                // Wait up to 500ms for details to load
                auto startTime = std::chrono::steady_clock::now();
                const auto timeout = std::chrono::milliseconds(500);
                
                while (!it->second.details.loaded)
                {
                    auto now = std::chrono::steady_clock::now();
                    if (now - startTime >= timeout)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                if (it->second.details.loaded)
                {
                    name = it->second.details.name;
                }
                else
                {
                    // Fallback to currency ID if name not loaded
                    char idBuf[32];
                    snprintf(idBuf, sizeof(idBuf), "Currency #%d", id);
                    name = idBuf;
                }
            }
            else
            {
                char idBuf[32];
                snprintf(idBuf, sizeof(idBuf), "Currency #%d", id);
                name = idBuf;
            }
            
            LootLogger::LogDrop(id, name, delta, true, "Currency", "", -1,
                                s_LastKnownMapId, LootLogger::ResolveMapName(s_LastKnownMapId, apiToken));
        }
    }

    // Process notifications AFTER releasing all locks to avoid recursive s_Mutex deadlock
    ProcessPendingNotifications();
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

void ItemTracker::SaveCurrentSession()
{
    bool enableSessionHistory;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        enableSessionHistory = g_Settings.enableSessionHistory;
    }

    if (!enableSessionHistory)
        return;

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
    auto startTime = now - duration;
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

    // Get items and collect top drops and rarity counts
    {
        // Global Lock Order: 4. s_SessionDropsMutex, 5. s_Mutex
        std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);
        
        sessionData.totalDrops = static_cast<int>(s_Items.size());

        // Collect top drops (by value)
        std::vector<std::pair<long long, SessionHistory::DropEntry>> drops;
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

        // Populate allDrops with item details
        for (auto& drop : s_SessionDrops)
        {
            // Find item details from current session items
            auto it = s_Items.find(drop.itemId);
            if (it != s_Items.end())
            {
                drop.itemName = it->second.details.loaded ? it->second.details.name : "Unknown";
                drop.iconUrl = it->second.details.loaded ? it->second.details.iconUrl : "";
                drop.rarity = it->second.details.loaded ? it->second.details.rarity : "Unknown";
                drop.totalValue = GetStatProfit(it->second);
            }
            else
            {
                // Check currencies
                auto currIt = s_Currencies.find(drop.itemId);
                if (currIt != s_Currencies.end())
                {
                    drop.itemName = currIt->second.details.loaded ? currIt->second.details.name : "Unknown";
                    drop.iconUrl = currIt->second.details.loaded ? currIt->second.details.iconUrl : "";
                    drop.rarity = currIt->second.details.loaded ? currIt->second.details.rarity : "Unknown";
                    drop.totalValue = GetStatProfit(currIt->second);
                }
            }
            sessionData.allDrops.push_back(drop);
        }

        // Map name (placeholder - would need DRF or GW2 API for actual map)
        sessionData.mapName = "Unknown";
    }

    // Save session
    SessionHistory::SaveSession(sessionData);
}

void ItemTracker::Reset()
{
    // Save session history before resetting
    SaveCurrentSession();

    // Clear active notifications
    UINotifications::ClearAll();

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

    s_Items.clear();
    s_Currencies.clear();
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
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_Items;
}

std::map<int, Stat> ItemTracker::GetCurrenciesCopy()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_Currencies;
}

Stat ItemTracker::GetItemStat(int itemId)
{
    // Snapshot ignored state BEFORE acquiring s_PersistentMutex/s_Mutex
    // to avoid circular lock: s_PersistentMutex -> s_Mutex -> IgnoredItems::s_Mutex
    // vs IgnoreItem: IgnoredItems::s_Mutex -> (no longer calls back)
    bool isIgnored = IgnoredItemsManager::IsItemIgnored(itemId);

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
    bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(currencyId);

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
            if (isItem) s_PersistentFavoriteItems.insert(apiId);
            if (isCurrency) s_PersistentFavoriteCurrencies.insert(apiId);
            // If it's neither yet (e.g. adding from search), we'll add to both and the load logic will sort it out
            if (!isItem && !isCurrency)
            {
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

    std::lock_guard<std::mutex> lock(s_Mutex);

    // Update in items if present
    auto itemIt = s_Items.find(apiId);
    if (itemIt != s_Items.end())
        itemIt->second.isFavorite = favorite;

    // Update in currencies if present
    auto currencyIt = s_Currencies.find(apiId);
    if (currencyIt != s_Currencies.end())
        currencyIt->second.isFavorite = favorite;

    // If adding to favorites, remove from ignored
    if (favorite)
    {
        IgnoredItemsManager::UnignoreItem(apiId);
        IgnoredItemsManager::UnignoreCurrency(apiId);
    }
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

std::map<int, Stat> ItemTracker::GetFavoriteItems()
{
    std::map<int, Stat> favorites;
    std::string searchTerm;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        searchTerm = g_Settings.searchTerm;
    }
    
    std::set<int> persistentIds;
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        persistentIds = s_PersistentFavoriteItems;
    }

    std::lock_guard<std::mutex> lock(s_Mutex);

    // 1. Get current session favorites
    for (const auto& [id, stat] : s_Items)
    {
        if (stat.isFavorite)
        {
            if (searchTerm.empty() || SearchManager::MatchesSearch(stat.details.name, searchTerm))
                favorites[id] = stat;
        }
    }
    
    // 2. Add persistent favorites that are not in the current session yet
    for (int id : persistentIds)
    {
        if (favorites.find(id) == favorites.end())
        {
            Stat s;
            s.apiId = id;
            s.type = StatType::Item;
            s.count = 0;
            s.isFavorite = true;
            
            auto it = s_Items.find(id);
            std::string name = (it != s_Items.end() && it->second.details.loaded) ? it->second.details.name : "";
            
            if (searchTerm.empty() || SearchManager::MatchesSearch(name, searchTerm))
            {
                favorites[id] = s;
            }
        }
    }
    
    return favorites;
}

std::map<int, Stat> ItemTracker::GetFavoriteCurrencies()
{
    std::map<int, Stat> favorites;
    std::string searchTerm;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        searchTerm = g_Settings.searchTerm;
    }
    
    std::set<int> persistentIds;
    {
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
        persistentIds = s_PersistentFavoriteCurrencies;
    }

    std::lock_guard<std::mutex> lock(s_Mutex);

    // 1. Get current session favorites
    for (const auto& [id, stat] : s_Currencies)
    {
        if (stat.isFavorite)
        {
            if (searchTerm.empty() || SearchManager::MatchesSearchCurrency(stat.details.name, searchTerm))
                favorites[id] = stat;
        }
    }
    
    // 2. Add persistent favorites that are not in the current session yet
    for (int id : persistentIds)
    {
        if (favorites.find(id) == favorites.end())
        {
            Stat s;
            s.apiId = id;
            s.type = StatType::Currency;
            s.count = 0;
            s.isFavorite = true;
            
            auto it = s_Currencies.find(id);
            std::string name = (it != s_Currencies.end() && it->second.details.loaded) ? it->second.details.name : "";
            
            if (searchTerm.empty() || SearchManager::MatchesSearchCurrency(name, searchTerm))
            {
                favorites[id] = s;
            }
        }
    }
    
    return favorites;
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
                drop.iconUrl = it->second.details.iconUrl;
                drop.rarity = it->second.details.rarity;
                long long safeCount = (it->second.count != 0) ? it->second.count : 1;
                drop.totalValue = (GetStatProfit(it->second) * drop.count) / safeCount;
            }
        }
        else
        {
            auto it = s_Items.find(drop.itemId);
            if (it != s_Items.end() && it->second.details.loaded)
            {
                drop.itemName = it->second.details.name;
                drop.iconUrl = it->second.details.iconUrl;
                drop.rarity = it->second.details.rarity;
                long long safeCount = (it->second.count != 0) ? it->second.count : 1;
                drop.totalValue = (GetStatProfit(it->second) * drop.count) / safeCount;
            }
        }
    }

    return drops;
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
        // Ignored currencies should not be shown regardless of currency-specific filters
        if (stat.isIgnored) return false;
        
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

std::map<int, Stat> ItemTracker::GetFilteredItems()
{
    // Get ignored snapshot BEFORE acquiring s_Mutex to avoid circular deadlock:
    // GetFilteredItems(s_Mutex) -> IsItemIgnored(IgnoredItems::s_Mutex) vs.
    // IgnoreItem(IgnoredItems::s_Mutex) -> SetFavorite(s_PersistentMutex -> s_Mutex)
    std::set<int> ignoredSnapshot = IgnoredItemsManager::GetIgnoredItems();

    // Get filter settings snapshot BEFORE acquiring s_Mutex to avoid circular deadlock:
    // GetFilteredItems(s_Mutex) -> PassesFilter -> FilterSettings::FromGlobal(s_SettingsMutex)
    FilterSettings filterSettings = FilterSettings::FromGlobal();

    std::map<int, Stat> filtered;
    {
        std::lock_guard<std::mutex> lock(s_Mutex);

        std::vector<std::pair<long long, Stat>> candidates;

        for (const auto& [id, stat] : s_Items)
        {
            if (stat.count == 0) continue;

            Stat copy = stat;
            copy.isIgnored = (ignoredSnapshot.count(id) > 0);
            if (PassesFilterImpl(copy, filterSettings))
                candidates.push_back({0, copy});
        }

        size_t limit = static_cast<size_t>(filterSettings.maxHistoryItems);

        if (limit > 0 && candidates.size() > limit)
        {
            for (size_t i = candidates.size() - limit; i < candidates.size(); ++i)
                filtered[candidates[i].second.apiId] = candidates[i].second;
        }
        else
        {
            for (const auto& cand : candidates)
                filtered[cand.second.apiId] = cand.second;
        }
    }
    return filtered;
}

std::map<int, Stat> ItemTracker::GetFilteredCurrencies()
{
    // Get ignored snapshot BEFORE acquiring s_Mutex (same deadlock prevention as GetFilteredItems)
    std::set<int> ignoredSnapshot = IgnoredItemsManager::GetIgnoredCurrencies();

    // Get filter settings snapshot BEFORE acquiring s_Mutex to avoid circular deadlock
    FilterSettings filterSettings = FilterSettings::FromGlobal();

    auto currencies = GetCurrenciesCopy();
    std::map<int, Stat> filtered;
    for (auto& [id, st] : currencies)
    {
        if (id == 44602 || id == 67027 || id == 89409)
            continue;
        if (st.count == 0 && id != 1) continue;

        st.isIgnored = (ignoredSnapshot.count(id) > 0);
        if (PassesFilterImpl(st, filterSettings))
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
    std::string searchTerm;
    bool favoritesFirst;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        searchTerm    = g_Settings.searchTerm;
        favoritesFirst = g_Settings.itemsFavoritesFirst; // per-tab setting (Drops > Settings)
    }
    auto items = GetSearchedItems(searchTerm);
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
    std::string searchTerm;
    bool favoritesFirst;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        searchTerm    = g_Settings.searchTerm;
        favoritesFirst = g_Settings.currenciesFavoritesFirst; // per-tab setting (Drops > Settings)
    }
    auto currencies = GetSearchedCurrencies(searchTerm);
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
    
    // Now acquire profit history lock at the end
    {
        std::lock_guard<std::mutex> lock(s_ProfitHistoryMutex);
    
        // Check Profit Goal Notification
        bool notifyProfitGoal;
        int  profitGoalAmount;
        {
            std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
            notifyProfitGoal = g_Settings.notifyProfitGoal;
            profitGoalAmount = g_Settings.profitGoalAmount;
        }

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
    std::lock_guard<std::mutex> lock(s_Mutex);
    long long total = 0;
    for (auto& [id, stat] : s_Items)
    {
        if (!stat.details.loaded || IsItemIgnored(id)) continue;
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
        if (!stat.details.loaded || IsItemIgnored(id)) continue;
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
        if (!stat.details.loaded || IsItemIgnored(id)) continue;
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
void ItemTracker::SaveData(const char* addonDir)
{
    if (!addonDir)
        return;

    std::string dataPath = std::string(addonDir) + "\\farming_data.json";
    
    // Global Lock Order: 1. s_PersistentMutex, 4. s_SessionDropsMutex, 5. s_Mutex
    std::lock_guard<std::mutex> pLock(s_PersistentMutex);
    std::lock_guard<std::mutex> dropsLock(s_SessionDropsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);

    nlohmann::json data;
    data["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    data["sessionStart"] = std::chrono::system_clock::to_time_t(s_SessionStart);
    data["magicFind"] = s_MagicFind.load();

    // Save items
    nlohmann::json itemsArray = nlohmann::json::array();
    for (const auto& [id, stat] : s_Items)
    {
        nlohmann::json item;
        item["apiId"] = id;
        item["count"] = stat.count;
        item["isFavorite"] = stat.isFavorite;
        item["lastMagicFind"] = stat.lastMagicFind;
        itemsArray.push_back(item);
    }
    data["items"] = itemsArray;

    // Save currencies
    nlohmann::json currenciesArray = nlohmann::json::array();
    for (const auto& [id, stat] : s_Currencies)
    {
        nlohmann::json currency;
        currency["apiId"] = id;
        currency["count"] = stat.count;
        currency["isFavorite"] = stat.isFavorite;
        currency["lastMagicFind"] = stat.lastMagicFind;
        currenciesArray.push_back(currency);
    }
    data["currencies"] = currenciesArray;

    // Save session drops (for Timeline tab)
    nlohmann::json dropsArray = nlohmann::json::array();
    for (const auto& drop : s_SessionDrops)
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

    // Save ignored items
    nlohmann::json ignoredItemsArray = nlohmann::json::array();
    {
        auto ignoredIds = IgnoredItemsManager::GetIgnoredItems();
        for (int id : ignoredIds)
            ignoredItemsArray.push_back(id);
    }
    data["ignoredItems"] = ignoredItemsArray;

    nlohmann::json ignoredCurrenciesArray = nlohmann::json::array();
    {
        auto ignoredIds = IgnoredItemsManager::GetIgnoredCurrencies();
        for (int id : ignoredIds)
            ignoredCurrenciesArray.push_back(id);
    }
    data["ignoredCurrencies"] = ignoredCurrenciesArray;

    // Save favorites (Persistent stores)
    nlohmann::json favoriteItemsArray = nlohmann::json::array();
    nlohmann::json favoriteCurrenciesArray = nlohmann::json::array();
    for (int id : s_PersistentFavoriteItems)
        favoriteItemsArray.push_back(id);
    for (int id : s_PersistentFavoriteCurrencies)
        favoriteCurrenciesArray.push_back(id);
    
    data["favoriteItems"] = favoriteItemsArray;
    data["favoriteCurrencies"] = favoriteCurrenciesArray;

    // Save custom profits
    nlohmann::json customProfitsJson = nlohmann::json::object();
    auto allCustomProfitsDetailed = CustomProfitManager::GetAllCustomProfitsDetailed();
    for (const auto& [id, entry] : allCustomProfitsDetailed)
    {
        nlohmann::json cp;
        cp["profit"] = entry.customProfitCopper;
        cp["type"] = static_cast<int>(entry.type);
        customProfitsJson[std::to_string(id)] = cp;
    }
    data["customProfits"] = customProfitsJson;

    // Write to file
    std::ofstream file(dataPath);
    if (file.is_open())
    {
        file << data.dump(4);
        file.close();
    }
}

void ItemTracker::LoadData(const char* addonDir)
{
    if (!addonDir)
        return;

    std::string dataPath = std::string(addonDir) + "\\farming_data.json";
    
    std::ifstream file(dataPath);
    if (!file.is_open())
        return;

    try
    {
        nlohmann::json data;
        file >> data;
        file.close();

        // Clear existing persistent stores before loading from file to ensure a clean state
        {
            std::lock_guard<std::mutex> pLock(s_PersistentMutex);
            s_PersistentFavoriteItems.clear();
            s_PersistentFavoriteCurrencies.clear();
        }
        IgnoredItemsManager::ClearAll();
        CustomProfitManager::ClearAll();

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
        }
    }
    catch (...)
    {
        // If loading fails, just continue with empty data
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

void ItemTracker::ApplyItemsFromApi(const json& itemsArray, const json& pricesArray)
{
    if (!itemsArray.is_array() || !pricesArray.is_array()) return;

    {
        // Global Lock Order: 1. s_PersistentMutex, 5. s_Mutex
        std::lock_guard<std::mutex> pLock(s_PersistentMutex);
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
                if (curIt == s_Currencies.end())
                {
                    // Create the currency entry if it's a salvage kit we want to track
                    Stat s;
                    s.apiId = id;
                    s.type = StatType::Currency;
                    s.count = 0;
                    s_Currencies[id] = s;
                    st = &s_Currencies[id];
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
                
                // Re-apply persistent flags
                st->isFavorite = s_PersistentFavoriteItems.count(id) > 0;
                st->isIgnored = IgnoredItemsManager::IsItemIgnored(id);
            }

            if (!st) continue;

            st->details.name        = item.value("name", "");
            st->details.description = item.value("description", "");
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
    } // ← All locks released here

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

    for (auto& c : currenciesArray)
    {
        if (!c.contains("id")) continue;
        int id = c["id"].get<int>();
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

std::pair<int, Stat> ItemTracker::GetBestDrop()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    std::pair<int, Stat> bestDrop = {0, Stat()};
    long long maxProfit = 0;
    
    for (const auto& [id, stat] : s_Items)
    {
        if (stat.count == 0) continue;
        if (!PassesFilter(stat)) continue;
        
        long long profit = GetStatProfit(stat);
        if (profit > maxProfit)
        {
            maxProfit = profit;
            bestDrop = {id, stat};
        }
    }
    
    return bestDrop;
}
