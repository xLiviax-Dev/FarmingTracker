#pragma once
#include <string>
#include <map>

// ---------------------------------------------------------------------------
// Settings → <GW2>/addons/FarmingTracker/settings.json
// automaticResetMode enum order (0-7), do not reorder.
// ---------------------------------------------------------------------------

struct Settings
{
    std::string drfToken;

    // 0 Never, 1 OnAddonLoad, 2 Daily UTC, 3 Weekly Mon 07:30 UTC, 4 NA WvW Sat 02 UTC,
    // 5 EU WvW Fri 18 UTC, 6 Map bonus Thu 20 UTC, 7 Minutes after shutdown
    int automaticResetMode = 1;

    int minutesUntilResetAfterShutdown = 30;

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
    float miniWindowWidth = 200.0f;
    float miniWindowHeight = 150.0f;

    // Main Window Settings
    bool mainWindowClickThrough = false;
    float mainWindowPosX = 100.0f;
    float mainWindowPosY = 100.0f;
    float mainWindowWidth = 800.0f;
    float mainWindowHeight = 600.0f;

    // Tab Settings
    int activeTab = 0; // 0=Summary, 1=Items, 2=Currencies, 3=Profit, 4=Debug

    // Visual Enhancement Settings
    bool enableGradientBackgrounds = false;

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
    bool filterIgnored = false;
    bool filterNotIgnored = true;

    // Range Filters
    bool showRangeFilters = true;
    int filterMinPrice = 0;
    int filterMaxPrice = 0;
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
    
    // Custom Profit System
    bool enableCustomProfit = false;
    
    // Search
    bool enableSearch = false;
    std::string searchTerm = "";
    
    // Ignored Items
    bool enableIgnoredItems = false;
    
    // Advanced UI Settings
    bool showAdvancedSettings = false;
    int negativeCountIconOpacity = 77;
    int countBackgroundOpacity = 0;

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

    // Grid View Settings
    bool enableGridView = false;

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
};

extern Settings g_Settings;

namespace SettingsManager
{
    void Init(const char* addonDir);
    void Load();
    void Save();
    bool IsTokenValid(const std::string& token);
    bool IsGw2ApiKeyPlausible(const std::string& key);
}
