#pragma once
#include <string>
#include <map>
#include <vector>

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
    int automaticResetMode = 1;

    int minutesUntilResetAfterShutdown = 30;
    int customResetDays = 1; // For custom reset mode (1-28 days)

    // GW2 official API key (account) - for item/currency/commerce data
    std::string gw2ApiKey;

    // Hotkey for toggling main window
    std::string toggleHotkey = "CTRL+SHIFT+F";

    // Hotkey for toggling mini window
    std::string miniWindowToggleHotkey = "CTRL+SHIFT+M";

    // Next automatic reset instant (ISO-8601 UTC). Managed by AutoReset.
    std::string nextResetDateTimeUtc;

    int  iconSize         = 32;
    bool showRarityBorder   = true;
    float rarityBorderSize = 3.0f;

    // Gradient Background Settings
    bool enableGradientBackgrounds = false;
    float gradientTopColor[4] = {0.95f, 0.95f, 1.0f, 1.0f};
    float gradientBottomColor[4] = {0.85f, 0.85f, 0.95f, 1.0f};

    // Mini Window (Overlay Widget) Settings
    bool showMiniWindow = false;
    bool miniWindowShowProfit = true;
    bool miniWindowShowProfitPerHour = true;
    bool miniWindowShowTradingProfitSell = false;
    bool miniWindowShowTradingProfitInstant = false;
    bool miniWindowShowTotalItems = false;
    bool miniWindowShowSessionDuration = false;
    bool miniWindowClickThrough = false;
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
    int activeTab = 0; // 0=Summary, 1=Items, 2=Currencies, 3=Profit, 4=Debug

    // 0 = |count| desc, 1 = |count| asc, 2 = api id asc, 3 = api id desc, 4 = name A–Z
    int itemSortMode = 0;

    // 0 = all, otherwise minimum rarity rank (1=Basic+, 2=Fine+, … see RarityRank in ui)
    int itemRarityFilterMin = 0;

    bool showItemIcons = true;

    std::string lastResetTimestamp;

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
    bool filterIgnored = true;
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
    bool enableCustomProfit = false;
    
    // Search
    bool enableSearch = false;
    std::string searchTerm = "";
    
    // Ignored Items
    bool enableIgnoredItems = false;
    bool showIgnoredItems = true; // Show ignored items in Items/Currencies tabs

    // Advanced UI Settings
    bool showAdvancedSettings = false;

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
    bool enableFavorites = false;
    bool enableFavoritesTab = false;
    bool favoritesFirst = false;
    bool enableFavoriteTextColor = false;
    float favoriteTextColor[4] = {1.0f, 0.8f, 0.4f, 1.0f}; // Gold
    bool enableFavoriteRowColor = false;
    float favoriteRowColor[4] = {0.3f, 0.25f, 0.15f, 1.0f}; // Dark gold

    // Ignored Tab
    bool enableIgnoredTab = false;

    // Grid View Settings
    bool enableGridViewItems = false;
    bool enableGridViewCurrencies = false;
    int gridIconSize = 48;
    int gridIconSizeCurrencies = 48;

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

    // === Notification Settings ===
    bool enableNotifications = false;
    bool notifyProfitGoal = false;
    int profitGoalAmount = 1000000; // 100g in copper coins
    bool notifyResetWarning = true;
    int resetWarningMinutes = 5;
    bool notifySessionComplete = false;
    int sessionCompleteHours = 4;
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
    void ResetToDefaults();

    // Profile Management
    void CreateProfile(const std::string& name);
    void ApplyProfile(int index);
    void DeleteProfile(int index);
    void UpdateProfile(int index);
    int GetProfileCount();
}
