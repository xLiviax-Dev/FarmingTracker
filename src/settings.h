#pragma once
#include <string>
#include <map>
#include <vector>
#include <mutex>
#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#else
#include "../include/nlohmann/json.hpp"
#endif

// ---------------------------------------------------------------------------
// Settings → <GW2>/addons/FarmingTracker/settings.json
// automaticResetMode enum order (0-7), do not reorder.
// ---------------------------------------------------------------------------

struct Account
{
    std::string name;
    std::string drfToken;
    std::string gw2ApiKey;
};

struct Settings
{
    // Language (stored as string for JSON serialization)
    std::string language = "English";
    
    // Multi-Account System
    std::vector<Account> accounts;
    int currentAccountIndex = 0;

    // Legacy single account fields (for backwards compatibility)
    std::string drfToken;

    // 0 Never, 1 OnAddonLoad, 2 Daily UTC, 3 Weekly Mon 07:30 UTC, 4 NA WvW Sat 02 UTC,
    // 5 EU WvW Fri 18 UTC, 6 Map bonus Thu 20 UTC, 7 Minutes after shutdown, 8 Custom days
    int automaticResetMode = 2;

    int minutesUntilResetAfterShutdown = 30;
    int customResetDays = 1; // For custom reset mode (1-28 days)

    // GW2 official API key (account) - for item/currency/commerce data
    std::string gw2ApiKey;

    // Hotkey for toggling main window
    std::string toggleHotkey = "CTRL+F";

    // Hotkey for toggling mini window
    std::string miniWindowToggleHotkey = "CTRL+SHIFT+M";

    // Hotkey for reset action (empty = disabled)
    std::string resetHotkey = "CTRL+SHIFT+R";

    // Next automatic reset instant (ISO-8601 UTC). Managed by AutoReset.
    std::string nextResetDateTimeUtc;

    // Set to true after a manual reset so the next OnAddonLoad does not auto-reset again.
    bool manualResetPending = false;

    int  iconSize         = 36;
    bool showRarityBorder   = true;
    float rarityBorderSize = 3.0f;

    // Gradient Background Settings
    bool enableGradientBackgrounds = false;
    float gradientTopColor[4] = {0.95f, 0.95f, 1.0f, 1.0f};
    float gradientBottomColor[4] = {0.85f, 0.85f, 0.95f, 1.0f};

    // Sparkline Settings
    bool showProfitSparkline = true;

    // Summaries Settings
    bool enableSummariesInProfitTab = true;

    // Best Drop Highlight Settings
    bool enableBestDropHighlight = true;
    bool enableBestDropInMiniWindow = false;
    bool miniWindowShowBestDropTotalValue = false; // New: Show best drop by total session value

    // Mini Window (Overlay Widget) Settings
    bool showMiniWindow = false;
    bool miniWindowShowProfit = true;
    bool miniWindowShowProfitPerHour = true;
    bool miniWindowShowTradingProfitSell = false;
    bool miniWindowShowTradingProfitInstant = false;
    bool miniWindowShowTotalItems = false;
    bool miniWindowShowSessionDuration = false;
    bool miniWindowClickThrough = false;
    bool miniWindowHideTitleBar = false;
    bool miniWindowLocked = false;
    float miniWindowPosX = 50.0f;
    float miniWindowPosY = 50.0f;
    float miniWindowWidth = 350.0f;
    float miniWindowHeight = 200.0f;

    // Main Window Settings
    bool showMainWindow = true;
    bool mainWindowClickThrough = false;
    float mainWindowPosX = 100.0f;
    float mainWindowPosY = 100.0f;
    float mainWindowWidth = 800.0f;
    float mainWindowHeight = 600.0f;

    // Tab Settings
    int activeTab = 0; // 0=Drops, 1=Loot Filter, 2=Session History, 3=Custom Profit, 4=Debug, 5=Loot Log, 6=Timeline

    // Main Tab Visibility
    bool enableDashboardTab = true;
    bool enableDropsTab = false;
    bool enableSessionHistoryTab = false;
    bool lockTabOrder = false;
    bool enableTimelineTab = true;
    std::vector<std::string> mainTabOrder = {"dashboard", "timeline", "drops", "loot_filter", "loot_log", "session_history", "custom_profit", "debug"};

    // 0 = |count| desc, 1 = |count| asc, 2 = api id asc, 3 = api id desc, 4 = name A–Z
    int itemSortMode = 0;

    // Drops Settings
    bool itemsEnableGridView = true;
    bool itemsFavoritesFirst = false;
    bool itemsGroupByRarity = true;
    bool itemsShowRarityAsTabs = false;
    bool itemsGroupByCategory = false;
    bool itemsShowGroupAsTabs = false;

    bool currenciesEnableGridView = true;
    bool currenciesFavoritesFirst = false;
    bool currenciesGroupByRarity = true;
    bool currenciesShowRarityAsTabs = false;
    bool currenciesGroupByCategory = false;
    bool currenciesShowGroupAsTabs = false;

    std::string itemsSavePath = "";
    std::string currenciesSavePath = "";
    std::string liveLogCustomPath = "";

    // 0 = all, otherwise minimum rarity rank (1=Basic+, 2=Fine+, … see RarityRank in ui)
    int itemRarityFilterMin = 0;

    bool showItemIcons = true;

    // Group by Rarity
    bool groupByRarity = false;
    bool showRarityAsTabs = false;

    // Group by Category (Type)
    bool groupByType = false;
    bool showTypeAsTabs = false;
    bool currencyGroupByCategory = false;
    bool currencyShowAsTabs = false;

    // Tooltips
    bool enableTooltips = true;

    // Window Opacity
    float mainWindowOpacity = 1.0f;  // 0.0 to 1.0
    float miniWindowOpacity = 1.0f;   // 0.0 to 1.0

    // Custom Accent Color (RGB, 0.0 to 1.0)
    float accentColorR = 0.369f; // #5E331B
    float accentColorG = 0.2f;
    float accentColorB = 0.106f;

    // Session History
    bool enableSessionHistory = false;
    int maxSessionHistory = 20;  // 1 to 50
    bool overwriteSessionHistory = true;  // If true, oldest session is deleted when limit is reached

    std::string lastResetTimestamp;

    // === Magnetite Shard Weekly Tracker ===
    bool        enableMagnetiteTracker           = true;
    int         magnetiteWeeklyEarned            = 0;     // Shards earned this week via DRF (resets Monday 07:30 UTC)
    int         magnetiteWeeklyEarnedAtLastCheck = 0;     // DRF counter snapshot at time of last wallet API check
    int         magnetiteLastWalletTotal         = -1;    // Last known wallet total (-1 = unknown)
    std::string magnetiteLastApiCheckUtc;                 // ISO-8601 UTC of last wallet API check
    int         magnetiteApiCheckCooldownMin     = 10;    // Minutes between wallet API checks on map change

    // === Loot Log ===
    bool        enableLootLog           = true;
    int         lootLogFormat           = 0;      // 0=CSV, 1=JSON, 2=Both
    std::string lootLogFolder;                    // empty = default (addonDir/loot-logs/)
    int         lootLogMaxDays          = 30;     // days before old logs are deleted (0=never)
    bool        lootLogItems            = true;   // log item drops
    bool        lootLogCurrencies       = true;   // log currency drops
    bool        lootLogIncludeMap       = true;   // include map_id + map_name column
    bool        lootLogIncludeBuffs     = true;   // include active_buffs column
    // Active buff IDs to include (farming-relevant only).
    // Populated with defaults on first load; user can add/remove in Settings.
    std::vector<int> lootLogBuffWhitelist;

    // === Ignored Tab Settings ===
    
    // Rarity Toggle States (auto-ignore future items of this rarity)
    bool ignoredRarityToggleJunk = false;
    bool ignoredRarityToggleBasic = false;
    bool ignoredRarityToggleFine = false;
    bool ignoredRarityToggleMasterwork = false;
    bool ignoredRarityToggleRare = false;
    bool ignoredRarityToggleExotic = false;
    bool ignoredRarityToggleAscended = false;
    bool ignoredRarityToggleLegendary = false;
    
    // Group/View Options
    bool ignoredGroupByRarity = false;
    bool ignoredShowRarityAsTabs = false;

    // === Advanced Features ===
    
    // Extended Filtering
    bool filterSellableToVendor = true;
    bool filterSellableOnTp = true;
    bool filterCustomProfit = true;
    bool filterKnownByApi = true;
    bool filterUnknownByApi = true;

    // Additional Filters
    bool filterAccountBound = true;
    bool filterNotAccountBound = true;
    bool filterNoSell = true;
    bool filterNotNoSell = true;
    bool filterFavorite = true;
    bool filterNotFavorite = true;
    bool filterIgnored = false;
    bool filterNotIgnored = true;

    // Range Filters
    bool showRangeFilters = true;
    int filterMinPriceGold = 0;
    int filterMinPriceSilver = 0;
    int filterMinPriceCopper = 0;
    int filterMaxPriceGold = 0;
    int filterMaxPriceSilver = 0;
    int filterMaxPriceCopper = 0;
    int filterMinQuantity = 0;
    int filterMaxQuantity = 0;

    // Item Type Filters
    bool filterTypeArmor = true;
    bool filterTypeWeapon = true;
    bool filterTypeTrinket = true;
    bool filterTypeGizmo = true;
    bool filterTypeCraftingMaterial = true;
    bool filterTypeConsumable = true;
    bool filterTypeGatheringTool = true;
    bool filterTypeBag = true;
    bool filterTypeContainer = true;
    bool filterTypeMiniPet = true;
    bool filterTypeGizmoContainer = true;
    bool filterTypeBackpack = true;
    bool filterTypeUpgradeComponent = true;
    bool filterTypeTool = true;
    bool filterTypeTrophy = true;
    bool filterTypeUnlock = true;
    
    // Currency Filters
    bool filterKarma = true;
    bool filterLaurel = true;
    bool filterGem = true;
    bool filterFractalRelic = true;
    bool filterBadgeOfHonor = true;
    bool filterGuildCommendation = true;
    bool filterTransmutationCharge = true;
    bool filterSpiritShards = true;
    bool filterUnboundMagic = true;
    bool filterVolatileMagic = true;
    bool filterAirshipParts = true;
    bool filterGeode = true;
    bool filterLeyLineCrystals = true;
    bool filterTradeContracts = true;
    bool filterElegyMosaic = true;
    bool filterUncommonCoins = true;
    bool filterAstralAcclaim = true;
    bool filterPristineFractalRelics = true;
    bool filterUnstableFractalEssence = true;
    bool filterMagnetiteShards = true;
    bool filterGaetingCrystals = true;
    bool filterProphetShards = true;
    bool filterGreenProphetShards = true;
    bool filterWvWSkirmishTickets = true;
    bool filterProofsOfHeroics = true;
    bool filterPvpLeagueTickets = true;
    bool filterAscendedShardsOfGlory = true;
    bool filterResearchNotes = true;
    bool filterTyrianDefenseSeal = true;
    bool filterTestimonyOfDesertHeroics = true;
    bool filterTestimonyOfJadeHeroics = true;
    bool filterTestimonyOfCastoranHeroics = true;
    bool filterLegendaryInsight = true;
    bool filterTalesOfDungeonDelving = true;
    bool filterImperialFavor = true;
    bool filterCanachCoins = true;
    bool filterAncientCoin = true;
    bool filterUnusualCoin = true;
    bool filterJadeSliver = true;
    bool filterStaticCharge = true;
    bool filterPinchOfStardust = true;
    bool filterCalcifiedGasp = true;
    bool filterUrsusOblige = true;
    bool filterGaetingCrystalJanthir = true;
    bool filterAntiquatedDucat = true;
    bool filterAetherRichSap = true;
    
    // Custom Profit System
    bool enableCustomProfit = true;
    
    // Search
    bool enableSearch = false;
    std::string searchTerm = "";
    
    // Ignored Items
    bool enableIgnoredItems = false;

    // Performance Settings
    bool enableIconCache = true;
    int iconCacheMaxIcons = 1000;
    int maxHistoryItems = 500; // 0 = unlimited
    int priceUpdateIntervalMin = 5; // 5 to 15 minutes
    bool disableComplexVisualsOnLowPerf = false;

    // Count Display Settings
    int countTextColor = 0xFFFFFF; // White
    int countBackgroundColor = 0x000000; // Black
    int countFontSize = 20; // Size20
    int countHorizontalAlignment = 2; // Right
    
    // Profit Labels
    std::string profitLabelText = "Profit";
    std::string profitPerHourLabelText = "Profit per hour";
    int profitWindowDisplayMode = 0; // 0=ProfitAndProfitPerHour, 1=ProfitOnly, 2=ProfitPerHourOnly

    // Favorites System
    bool enableFavorites = true;
    bool favoritesFirst = false;

    // Notification Settings
    bool enableNotifications = false;
    bool showNotificationSetup = false;
    float notificationPosX = 1600.0f;
    float notificationPosY = 800.0f;
    float notificationWidth = 300.0f;
    float notificationHeight = 0.0f; // 0 = auto
    float notificationDuration = 5.0f;
    bool notificationEnableMinValue = true;
    float notificationMinValueGold = 1.0f;
    bool notificationEnableMinRarity = true;
    int notificationMinRarity = 5; // Exotic+
    bool notificationCombineValueAndRarity = false;
    bool notificationIncludeNonProfit = true;
    bool notificationPrecursorAlert = true;
    bool notificationInfusionAlert = true;
    bool notificationIncludeAgonyInfusions = false;
    bool notificationStacking = true;

    // Audio Settings
    bool notificationPlaySound = false;
    float notificationVolume = 0.5f; // Global volume (acts as multiplier or default)
    float notificationVolumeStandard = 0.5f;
    float notificationVolumePrecursor = 0.5f;
    float notificationVolumeInfusion = 0.5f;
    float notificationVolumeAlert = 0.5f;
    std::string soundPathStandard = ""; // Empty = use embedded
    std::string soundPathPrecursor = "";
    std::string soundPathInfusion = "";
    std::string soundPathAlert = "";

    // Notification Triggers (Old merged into new)
    bool notifyProfitGoal = false;
    int profitGoalAmount = 1000000; // 100g in copper coins
    bool notifyResetWarning = false;
    int resetWarningMinutes = 5;
    bool notifySessionComplete = false;
    int sessionCompleteHours = 4;

    bool enableFavoriteTextColor = true;
    float favoriteTextColor[4] = {1.0f, 0.8f, 0.4f, 1.0f}; // Gold
    bool enableFavoriteRowColor = true;
    float favoriteRowColor[4] = {0.3f, 0.25f, 0.15f, 1.0f}; // Dark gold

    // Grid View Settings
    bool enableGridViewItems = false;
    bool enableGridViewCurrencies = false;
    bool enableGridViewSummary = false;
    bool enableGridViewSummaryFavorites = false;
    bool enableGridViewSummaryCurrencies = false;
    bool enableGridViewSummaryItems = false;
    int gridIconSize = 48;
    int gridIconSizeCurrencies = 48;

    // Timeline Tab Settings
    int timelineIconSizeItems = 40;
    int timelineIconSizeCurrencies = 26;

    // UI State Settings
    bool showTopItems = true;
    bool showTopCurrencies = true;
    
    // Debug Features
    bool enableDebugTab = false;
    bool useFakeDrfServer = false;

    // API Timeouts (in milliseconds)
    int gw2ApiConnectTimeout = 15000;
    int gw2ApiReceiveTimeout = 30000;

    // Salvage Kits Settings
    struct SalvageKitSetting
    {
        bool enabled = false;
        bool useKarma = false; // false = Gold, true = Karma
    };

    // Salvage Kit settings by Item ID
    std::map<int, SalvageKitSetting> salvageKitSettings;

    // === Settings Profile System ===
    struct SettingsProfile
    {
        std::string name;
        std::string profileData; // JSON string of settings
    };
    std::vector<SettingsProfile> settingsProfiles;
    int currentProfileIndex = -1; // -1 = no profile selected (use default settings)


    // === Automatic Backups ===
    bool enableAutoBackups = true;
    int maxBackupCount = 5; // Maximum number of backups to keep
    int backupFrequency = 1; // Backup frequency (0 = manual only, 1 = daily, 2 = weekly)
    std::string autoBackupPath = ""; // Empty = addon directory

    // UI Reset Flag (not persisted)
    bool resetTableSettings = false;

    static std::recursive_mutex s_SettingsMutex;
};

extern Settings g_Settings;

namespace SettingsManager
{
    void Init(const char* addonDir);
    void Load();
    void Save();
    bool IsTokenValid(const std::string& token);
    bool IsGw2ApiKeyPlausible(const std::string& key);
    void ExportToFile(const std::string& filePath);
    void ImportFromFile(const std::string& filePath);
    void ImportFilterSettings(const nlohmann::json& j);
    nlohmann::json ExportFilterSettings();
    void ResetToDefaults();

    // JSON Serialization (Internal use and Backup/Restore)
    nlohmann::json ToSettingsJson(const Settings& settings);
    void FromSettingsJson(const nlohmann::json& j, Settings& settings);

    // Profile Management
    void CreateProfile(const std::string& name);
    void ApplyProfile(int index);
    void DeleteProfile(int index);
    void UpdateProfile(int index);
    int GetProfileCount();

    // Get current account's DRF token
    std::string GetCurrentDrfToken();
    std::string GetCurrentGw2ApiKey();
}
