#include "settings.h"
#include "shared.h"
#include "../include/nlohmann/json.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

#include <fstream>
#include <regex>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>

using json = nlohmann::json;

Settings g_Settings;
std::recursive_mutex Settings::s_SettingsMutex;
static std::string s_SettingsPath;

static std::vector<std::string> DefaultMainTabOrder()
{
    return {"dashboard", "timeline", "drops", "loot_filter", "loot_log", "session_history", "custom_profit", "debug"};
}

static void EnsureMainTabOrderValid()
{
    const auto defaults = DefaultMainTabOrder();

    if (g_Settings.mainTabOrder.empty())
        g_Settings.mainTabOrder = defaults;

    g_Settings.mainTabOrder.erase(
        std::remove_if(g_Settings.mainTabOrder.begin(), g_Settings.mainTabOrder.end(),
            [&](const std::string& key)
            {
                return std::find(defaults.begin(), defaults.end(), key) == defaults.end();
            }),
        g_Settings.mainTabOrder.end());

    for (const auto& key : defaults)
    {
        if (std::find(g_Settings.mainTabOrder.begin(), g_Settings.mainTabOrder.end(), key) == g_Settings.mainTabOrder.end())
            g_Settings.mainTabOrder.push_back(key);
    }
}

// Secure encryption using Windows CryptProtectData
static std::string SimpleEncrypt(const std::string& data)
{
    if (data.empty()) return data;

    DATA_BLOB inBlob, outBlob;
    inBlob.pbData = (BYTE*)data.c_str();
    inBlob.cbData = static_cast<DWORD>(data.size());

    // Use a fixed entropy for additional security layer (Nexus/GW2 environment context)
    std::string entropy = "FarmingTracker2024";
    DATA_BLOB entropyBlob;
    entropyBlob.pbData = (BYTE*)entropy.c_str();
    entropyBlob.cbData = static_cast<DWORD>(entropy.size());

    if (!CryptProtectData(&inBlob, L"FarmingTracker", &entropyBlob, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &outBlob))
    {
        return data; // Fallback to plaintext if encryption fails
    }

    std::string encrypted((char*)outBlob.pbData, outBlob.cbData);
    LocalFree(outBlob.pbData);
    return encrypted;
}

static std::string SimpleDecrypt(const std::string& encrypted)
{
    if (encrypted.empty()) return encrypted;

    DATA_BLOB inBlob, outBlob;
    inBlob.pbData = (BYTE*)encrypted.c_str();
    inBlob.cbData = static_cast<DWORD>(encrypted.size());

    std::string entropy = "FarmingTracker2024";
    DATA_BLOB entropyBlob;
    entropyBlob.pbData = (BYTE*)entropy.c_str();
    entropyBlob.cbData = static_cast<DWORD>(entropy.size());

    if (!CryptUnprotectData(&inBlob, NULL, &entropyBlob, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &outBlob))
    {
        return encrypted; // Return as-is if decryption fails
    }

    std::string decrypted((char*)outBlob.pbData, outBlob.cbData);
    LocalFree(outBlob.pbData);
    return decrypted;
}

static std::string ToHexString(const std::string& data)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : data)
    {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

static std::string FromHexString(const std::string& hex)
{
    if (hex.length() % 2 != 0)
        return {}; // Invalid: odd length

    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        char* endPtr = nullptr;
        long byteValue = std::strtol(byteString.c_str(), &endPtr, 16);
        if (endPtr == byteString.c_str() || *endPtr != '\0' || byteValue < 0 || byteValue > 255)
            return {}; // Invalid hex byte
        result += static_cast<char>(byteValue);
    }
    return result;
}

nlohmann::json SettingsManager::ToSettingsJson(const Settings& s)
{
    nlohmann::json j;
    j["settingsVersion"] = s.settingsVersion;
    j["language"] = s.language;
    j["currentAccountIndex"] = s.currentAccountIndex;
    
    j["accounts"] = json::array();
    for (const auto& acc : s.accounts)
    {
        json accJson;
        accJson["name"] = acc.name;
        accJson["drfToken"] = ToHexString(SimpleEncrypt(acc.drfToken));
        accJson["gw2ApiKey"] = ToHexString(SimpleEncrypt(acc.gw2ApiKey));
        j["accounts"].push_back(accJson);
    }

    // Encrypted legacy tokens
    j["drfToken"] = ToHexString(SimpleEncrypt(s.drfToken));
    j["gw2ApiKey"] = ToHexString(SimpleEncrypt(s.gw2ApiKey));

    j["automaticResetMode"] = s.automaticResetMode;
    j["minutesUntilReset"] = s.minutesUntilResetAfterShutdown;
    j["customResetDays"] = s.customResetDays;
    j["toggleHotkey"] = s.toggleHotkey;
    j["miniWindowToggleHotkey"] = s.miniWindowToggleHotkey;
    j["resetHotkey"] = s.resetHotkey;
    j["nextResetDateTimeUtc"] = s.nextResetDateTimeUtc;
    j["manualResetPending"] = s.manualResetPending;
    j["historyIconSize"] = s.historyIconSize;
    j["profitIconSize"] = s.profitIconSize;
    j["itemsIconSize"] = s.itemsIconSize;
    j["showRarityBorder"] = s.showRarityBorder;
    j["rarityBorderSize"] = s.rarityBorderSize;
    j["enableGradientBackgrounds"] = s.enableGradientBackgrounds;
    
    j["gradientTopColor"] = json::array({s.gradientTopColor[0], s.gradientTopColor[1], s.gradientTopColor[2], s.gradientTopColor[3]});
    j["gradientBottomColor"] = json::array({s.gradientBottomColor[0], s.gradientBottomColor[1], s.gradientBottomColor[2], s.gradientBottomColor[3]});
    
    j["showProfitSparkline"] = s.showProfitSparkline;
    j["sparklineColor"] = s.sparklineColor;
    j["enableSummariesInProfitTab"] = s.enableSummariesInProfitTab;
    j["enableBestDropHighlight"] = s.enableBestDropHighlight;
    j["bestDropHighlightColor"] = json::array({s.bestDropHighlightColor[0], s.bestDropHighlightColor[1], s.bestDropHighlightColor[2], s.bestDropHighlightColor[3]});
    j["miniWindowShowBestDropSingle"] = s.miniWindowShowBestDropSingle;
    j["miniWindowShowBestDropTotalValue"] = s.miniWindowShowBestDropTotalValue;
    j["miniWindowShowBestDropIcons"] = s.miniWindowShowBestDropIcons;
    j["miniWindowBestDropIconSize"] = s.miniWindowBestDropIconSize;
    j["showShortIcon"] = s.showShortIcon;
    j["showMiniWindow"] = s.showMiniWindow;
    j["miniWindowVisibilityMode"] = static_cast<int>(s.miniWindowVisibilityMode);
    j["miniWindowShowProfit"] = s.miniWindowShowProfit;
    j["miniWindowShowProfitPerHour"] = s.miniWindowShowProfitPerHour;
    j["miniWindowShowTradingProfitSell"] = s.miniWindowShowTradingProfitSell;
    j["miniWindowShowTradingProfitInstant"] = s.miniWindowShowTradingProfitInstant;
    j["miniWindowShowTotalItems"] = s.miniWindowShowTotalItems;
    j["miniWindowShowSessionDuration"] = s.miniWindowShowSessionDuration;
    j["miniWindowClickThrough"] = s.miniWindowClickThrough;
    j["miniWindowAllowRightClickUnpin"] = s.miniWindowAllowRightClickUnpin;
    j["miniWindowShowMaterialStorageCount"] = s.miniWindowShowMaterialStorageCount;
    j["miniWindowShowWalletCount"] = s.miniWindowShowWalletCount;
    j["miniWindowShowBankCount"] = s.miniWindowShowBankCount;
    j["miniWindowShowInventoryCount"] = s.miniWindowShowInventoryCount;
    j["miniWindowHideCountLabels"] = s.miniWindowHideCountLabels;
    j["miniWindowShortCountLabels"] = s.miniWindowShortCountLabels;
    j["miniWindowHideZeroDropStats"] = s.miniWindowHideZeroDropStats;
    j["miniWindowHideTextLabels"] = s.miniWindowHideTextLabels;
    j["miniWindowHideIcons"] = s.miniWindowHideIcons;
    j["miniWindowPinnedIconSize"] = s.miniWindowPinnedIconSize;
    j["mainWindowFontSize"] = s.mainWindowFontSize;
    j["tabContentFontSize"] = s.tabContentFontSize;
    j["notificationFontSize"] = s.notificationFontSize;
    j["miniWindowHideTitleBar"] = s.miniWindowHideTitleBar;
    j["miniWindowHideBorder"] = s.miniWindowHideBorder;
    j["miniWindowLocked"] = s.miniWindowLocked;
    j["miniWindowPosX"] = s.miniWindowPosX;
    j["miniWindowPosY"] = s.miniWindowPosY;
    j["miniWindowWidth"] = s.miniWindowWidth;
    j["miniWindowHeight"] = s.miniWindowHeight;
    j["miniWindowEnableTextShadow"] = s.miniWindowEnableTextShadow;
    j["miniWindowTextColor"] = s.miniWindowTextColor;
    j["miniWindowFontSize"] = s.miniWindowFontSize;
    j["miniWindowElementOrder"] = s.miniWindowElementOrder;
    j["showMainWindow"] = s.showMainWindow;
    j["mainWindowVisibilityMode"] = static_cast<int>(s.mainWindowVisibilityMode);
    j["mainWindowClickThrough"] = s.mainWindowClickThrough;
    j["mainWindowHideTitleBar"] = s.mainWindowHideTitleBar;
    j["mainWindowPosX"] = s.mainWindowPosX;
    j["mainWindowPosY"] = s.mainWindowPosY;
    j["mainWindowWidth"] = s.mainWindowWidth;
    j["mainWindowHeight"] = s.mainWindowHeight;
    j["activeTab"] = s.activeTab;
    j["enableDashboardTab"] = s.enableDashboardTab;
    j["enableDropsTab"] = s.enableDropsTab;
    j["enableSessionHistoryTab"] = s.enableSessionHistoryTab;
    j["enableTimelineTab"] = s.enableTimelineTab;
    j["lockTabOrder"] = s.lockTabOrder;
    j["mainTabOrder"] = s.mainTabOrder;
    j["itemSortMode"] = s.itemSortMode;
    j["itemRarityFilterMin"] = s.itemRarityFilterMin;
    j["showItemIcons"] = s.showItemIcons;
    j["groupByRarity"] = s.groupByRarity;
    j["showRarityAsTabs"] = s.showRarityAsTabs;

    // Drops Settings
    j["itemsEnableGridView"] = s.itemsEnableGridView;
    j["itemsFavoritesFirst"] = s.itemsFavoritesFirst;
    j["itemsFavoritesAsGrid"] = s.itemsFavoritesAsGrid;
    j["itemsGroupByRarity"] = s.itemsGroupByRarity;
    j["itemsShowRarityAsTabs"] = s.itemsShowRarityAsTabs;
    j["itemsGroupByCategory"] = s.itemsGroupByCategory;
    j["itemsShowGroupAsTabs"] = s.itemsShowGroupAsTabs;
    j["currenciesEnableGridView"] = s.currenciesEnableGridView;
    j["currenciesFavoritesFirst"] = s.currenciesFavoritesFirst;
    j["currenciesFavoritesAsGrid"] = s.currenciesFavoritesAsGrid;
    j["currenciesGroupByCategory"] = s.currenciesGroupByCategory;
    j["currenciesShowGroupAsTabs"] = s.currenciesShowGroupAsTabs;
    j["overviewCurrenciesFirst"] = s.overviewCurrenciesFirst;
    j["overviewEnableGridView"] = s.overviewEnableGridView;
    j["overviewFavoritesAsGrid"] = s.overviewFavoritesAsGrid;
    j["overviewFavoritesIconSize"] = s.overviewFavoritesIconSize;
    j["overviewItemSortMode"] = s.overviewItemSortMode;
    j["overviewItemRarityFilterMin"] = s.overviewItemRarityFilterMin;
    j["overviewGroupByRarity"] = s.overviewGroupByRarity;
    j["overviewShowRarityAsTabs"] = s.overviewShowRarityAsTabs;
    j["overviewGroupByCategory"] = s.overviewGroupByCategory;
    j["overviewShowGroupAsTabs"] = s.overviewShowGroupAsTabs;
    j["itemsSavePath"] = s.itemsSavePath;
    j["currenciesSavePath"] = s.currenciesSavePath;
    j["liveLogCustomPath"] = s.liveLogCustomPath;
    j["currencyGroupByCategory"] = s.currencyGroupByCategory;
    j["currencyShowAsTabs"] = s.currencyShowAsTabs;
    j["mainWindowOpacity"] = s.mainWindowOpacity;
    j["miniWindowOpacity"] = s.miniWindowOpacity;
    j["accentColorR"] = s.accentColorR;
    j["accentColorG"] = s.accentColorG;
    j["accentColorB"] = s.accentColorB;
    j["enableSessionHistory"] = s.enableSessionHistory;
    j["maxSessionHistory"] = s.maxSessionHistory;
    j["overwriteSessionHistory"] = s.overwriteSessionHistory;

    // Magnetite Shard Weekly Tracker
    j["magnetiteTracker"] = {
        {"enabled",                 s.enableMagnetiteTracker},
        {"weeklyEarned",            s.magnetiteWeeklyEarned},
        {"weeklyEarnedAtLastCheck", s.magnetiteWeeklyEarnedAtLastCheck},
        {"lastWalletTotal",         s.magnetiteLastWalletTotal},
        {"lastApiCheckUtc",         s.magnetiteLastApiCheckUtc},
        {"cooldownMin",             s.magnetiteApiCheckCooldownMin},
    };

    // Gaeting Crystal Weekly Tracker
    j["gaetingTracker"] = {
        {"enabled",                 s.enableGaetingTracker},
        {"weeklyEarned",            s.gaetingWeeklyEarned},
        {"weeklyEarnedAtLastCheck", s.gaetingWeeklyEarnedAtLastCheck},
        {"lastWalletTotal",         s.gaetingLastWalletTotal},
        {"lastApiCheckUtc",         s.gaetingLastApiCheckUtc},
        {"cooldownMin",             s.gaetingApiCheckCooldownMin},
    };

    // Loot Log
    j["lootLog"] = {
        {"enabled",       s.enableLootLog},
        {"format",        s.lootLogFormat},
        {"folder",        s.lootLogFolder},
        {"maxDays",       s.lootLogMaxDays},
        {"items",         s.lootLogItems},
        {"currencies",    s.lootLogCurrencies},
        {"includeMap",    s.lootLogIncludeMap},
        {"includeMagicFind", s.lootLogIncludeMagicFind},
    };

    j["filterSellableToVendor"] = s.filterSellableToVendor;
    j["filterSellableOnTp"] = s.filterSellableOnTp;
    j["filterCustomProfit"] = s.filterCustomProfit;
    j["filterKnownByApi"] = s.filterKnownByApi;
    j["filterUnknownByApi"] = s.filterUnknownByApi;
    j["filterAccountBound"] = s.filterAccountBound;
    j["filterNotAccountBound"] = s.filterNotAccountBound;
    j["filterNoSell"] = s.filterNoSell;
    j["filterNotNoSell"] = s.filterNotNoSell;
    j["filterFavorite"] = s.filterFavorite;
    j["filterNotFavorite"] = s.filterNotFavorite;
    j["filterIgnored"] = s.filterIgnored;
    j["filterNotIgnored"] = s.filterNotIgnored;
    j["showRangeFilters"] = s.showRangeFilters;
    j["filterMinPriceGold"] = s.filterMinPriceGold;
    j["filterMinPriceSilver"] = s.filterMinPriceSilver;
    j["filterMinPriceCopper"] = s.filterMinPriceCopper;
    j["filterMaxPriceGold"] = s.filterMaxPriceGold;
    j["filterMaxPriceSilver"] = s.filterMaxPriceSilver;
    j["filterMaxPriceCopper"] = s.filterMaxPriceCopper;
    j["filterMinQuantity"] = s.filterMinQuantity;
    j["filterMaxQuantity"] = s.filterMaxQuantity;
    j["filterTypeArmor"] = s.filterTypeArmor;
    j["filterTypeWeapon"] = s.filterTypeWeapon;
    j["filterTypeTrinket"] = s.filterTypeTrinket;
    j["filterTypeGizmo"] = s.filterTypeGizmo;
    j["filterTypeCraftingMaterial"] = s.filterTypeCraftingMaterial;
    j["filterTypeConsumable"] = s.filterTypeConsumable;
    j["filterTypeGatheringTool"] = s.filterTypeGatheringTool;
    j["filterTypeBag"] = s.filterTypeBag;
    j["filterTypeContainer"] = s.filterTypeContainer;
    j["filterTypeMiniPet"] = s.filterTypeMiniPet;
    j["filterTypeGizmoContainer"] = s.filterTypeGizmoContainer;
    j["filterTypeBackpack"] = s.filterTypeBackpack;
    j["filterTypeUpgradeComponent"] = s.filterTypeUpgradeComponent;
    j["filterTypeTool"] = s.filterTypeTool;
    j["filterTypeTrophy"] = s.filterTypeTrophy;
    j["filterTypeUnlock"] = s.filterTypeUnlock;
    j["filterKarma"] = s.filterKarma;
    j["filterLaurel"] = s.filterLaurel;
    j["filterGem"] = s.filterGem;
    j["filterFractalRelic"] = s.filterFractalRelic;
    j["filterBadgeOfHonor"] = s.filterBadgeOfHonor;
    j["filterGuildCommendation"] = s.filterGuildCommendation;
    j["filterTransmutationCharge"] = s.filterTransmutationCharge;
    j["filterSpiritShards"] = s.filterSpiritShards;
    j["filterUnboundMagic"] = s.filterUnboundMagic;
    j["filterVolatileMagic"] = s.filterVolatileMagic;
    j["filterAirshipParts"] = s.filterAirshipParts;
    j["filterGeode"] = s.filterGeode;
    j["filterLeyLineCrystals"] = s.filterLeyLineCrystals;
    j["filterTradeContracts"] = s.filterTradeContracts;
    j["filterElegyMosaic"] = s.filterElegyMosaic;
    j["filterUncommonCoins"] = s.filterUncommonCoins;
    j["filterAstralAcclaim"] = s.filterAstralAcclaim;
    j["filterPristineFractalRelics"] = s.filterPristineFractalRelics;
    j["filterUnstableFractalEssence"] = s.filterUnstableFractalEssence;
    j["filterMagnetiteShards"] = s.filterMagnetiteShards;
    j["filterGaetingCrystals"] = s.filterGaetingCrystals;
    j["filterProphetShards"] = s.filterProphetShards;
    j["filterGreenProphetShards"] = s.filterGreenProphetShards;
    j["filterWvWSkirmishTickets"] = s.filterWvWSkirmishTickets;
    j["filterProofsOfHeroics"] = s.filterProofsOfHeroics;
    j["filterPvpLeagueTickets"] = s.filterPvpLeagueTickets;
    j["filterAscendedShardsOfGlory"] = s.filterAscendedShardsOfGlory;
    j["filterResearchNotes"] = s.filterResearchNotes;
    j["filterTyrianDefenseSeal"] = s.filterTyrianDefenseSeal;
    j["filterTestimonyOfDesertHeroics"] = s.filterTestimonyOfDesertHeroics;
    j["filterTestimonyOfJadeHeroics"] = s.filterTestimonyOfJadeHeroics;
    j["filterTestimonyOfCastoranHeroics"] = s.filterTestimonyOfCastoranHeroics;
    j["filterLegendaryInsight"] = s.filterLegendaryInsight;
    j["filterTalesOfDungeonDelving"] = s.filterTalesOfDungeonDelving;
    j["filterImperialFavor"] = s.filterImperialFavor;
    j["filterCanachCoins"] = s.filterCanachCoins;
    j["filterAncientCoin"] = s.filterAncientCoin;
    j["filterUnusualCoin"] = s.filterUnusualCoin;
    j["filterJadeSliver"] = s.filterJadeSliver;
    j["filterStaticCharge"] = s.filterStaticCharge;
    j["filterPinchOfStardust"] = s.filterPinchOfStardust;
    j["filterCalcifiedGasp"] = s.filterCalcifiedGasp;
    j["filterUrsusOblige"] = s.filterUrsusOblige;
    j["filterGaetingCrystalJanthir"] = s.filterGaetingCrystalJanthir;
    j["filterAntiquatedDucat"] = s.filterAntiquatedDucat;
    j["filterAetherRichSap"] = s.filterAetherRichSap;
    j["enableCustomProfit"] = s.enableCustomProfit;
    j["enableSearch"] = s.enableSearch;
    j["searchTerm"] = s.searchTerm;
    j["enableIgnoredItems"] = s.enableIgnoredItems;
    j["enableIconCache"] = s.enableIconCache;
    j["iconCacheMaxIcons"] = s.iconCacheMaxIcons;
    j["maxHistoryItems"] = s.maxHistoryItems;
    j["priceUpdateIntervalMin"] = s.priceUpdateIntervalMin;
    j["disableComplexVisualsOnLowPerf"] = s.disableComplexVisualsOnLowPerf;
    j["countTextColor"] = s.countTextColor;
    j["countBackgroundColor"] = s.countBackgroundColor;
    j["countFontSize"] = s.countFontSize;
    j["countHorizontalAlignment"] = s.countHorizontalAlignment;
    j["profitLabelText"] = s.profitLabelText;
    j["profitPerHourLabelText"] = s.profitPerHourLabelText;
    j["profitWindowDisplayMode"] = s.profitWindowDisplayMode;
    j["enableFavorites"] = s.enableFavorites;
    j["favoritesFirst"] = s.favoritesFirst;
    j["enableFavoriteTextColor"] = s.enableFavoriteTextColor;
    j["favoriteTextColor"] = json::array({s.favoriteTextColor[0], s.favoriteTextColor[1], s.favoriteTextColor[2], s.favoriteTextColor[3]});
    j["enableFavoriteRowColor"] = s.enableFavoriteRowColor;
    j["favoriteRowColor"] = json::array({s.favoriteRowColor[0], s.favoriteRowColor[1], s.favoriteRowColor[2], s.favoriteRowColor[3]});
    j["enableGridViewItems"] = s.enableGridViewItems;
    j["enableGridViewCurrencies"] = s.enableGridViewCurrencies;
    j["enableGridViewSummary"] = s.enableGridViewSummary;
    j["enableGridViewSummaryFavorites"] = s.enableGridViewSummaryFavorites;
    j["enableGridViewSummaryCurrencies"] = s.enableGridViewSummaryCurrencies;
    j["enableGridViewSummaryItems"] = s.enableGridViewSummaryItems;
    j["gridIconSize"] = s.gridIconSize;
    j["gridIconSizeCurrencies"] = s.gridIconSizeCurrencies;
    j["timelineIconSizeItems"] = s.timelineIconSizeItems;
    j["timelineIconSizeCurrencies"] = s.timelineIconSizeCurrencies;
    j["showTopItems"] = s.showTopItems;
    j["showTopCurrencies"] = s.showTopCurrencies;
    j["enableDebugTab"] = s.enableDebugTab;
    j["gw2ApiConnectTimeout"] = s.gw2ApiConnectTimeout;
    j["gw2ApiReceiveTimeout"] = s.gw2ApiReceiveTimeout;
    
    j["salvageKitSettings"] = json::object();
    for (const auto& [id, setting] : s.salvageKitSettings)
    {
        json kitJson;
        kitJson["enabled"] = setting.enabled;
        kitJson["useKarma"] = setting.useKarma;
        j["salvageKitSettings"][std::to_string(id)] = kitJson;
    }

    j["settingsProfiles"] = json::array();
    for (const auto& profile : s.settingsProfiles)
    {
        json profileJson;
        profileJson["name"] = profile.name;
        profileJson["profileData"] = profile.profileData;
        j["settingsProfiles"].push_back(profileJson);
    }
    j["currentProfileIndex"] = s.currentProfileIndex;
    j["enableAutoBackups"] = s.enableAutoBackups;
    j["maxBackupCount"] = s.maxBackupCount;
    j["backupFrequency"] = s.backupFrequency;
    j["autoBackupPath"] = s.autoBackupPath;
    j["lastBackupTimestamp"] = s.lastBackupTimestamp;
    j["enableNotifications"] = s.enableNotifications;
    j["showNotificationSetup"] = s.showNotificationSetup;
    j["notificationPosX"] = s.notificationPosX;
    j["notificationPosY"] = s.notificationPosY;
    j["notificationWidth"] = s.notificationWidth;
    j["notificationHeight"] = s.notificationHeight;
    j["notificationDuration"] = s.notificationDuration;
    j["notificationEnableMinValue"] = s.notificationEnableMinValue;
    j["notificationMinValueGold"] = s.notificationMinValueGold;
    j["notificationEnableMinRarity"] = s.notificationEnableMinRarity;
    j["notificationMinRarity"] = s.notificationMinRarity;
    j["notificationCombineValueAndRarity"] = s.notificationCombineValueAndRarity;
    j["notificationIncludeNonProfit"] = s.notificationIncludeNonProfit;
    j["notificationPrecursorAlert"] = s.notificationPrecursorAlert;
    j["notificationInfusionAlert"] = s.notificationInfusionAlert;
    j["notificationIncludeAgonyInfusions"] = s.notificationIncludeAgonyInfusions;
    j["notificationStacking"] = s.notificationStacking;
    j["notificationBlacklist"] = s.notificationBlacklist;
    j["notificationPlaySound"] = s.notificationPlaySound;
    j["notificationVolume"] = s.notificationVolume;
    j["notificationVolumeStandard"] = s.notificationVolumeStandard;
    j["notificationVolumePrecursor"] = s.notificationVolumePrecursor;
    j["notificationVolumeInfusion"] = s.notificationVolumeInfusion;
    j["notificationVolumeAlert"] = s.notificationVolumeAlert;
    j["soundPathStandard"] = s.soundPathStandard;
    j["soundPathPrecursor"] = s.soundPathPrecursor;
    j["soundPathInfusion"] = s.soundPathInfusion;
    j["soundPathAlert"] = s.soundPathAlert;
    j["notifyProfitGoal"] = s.notifyProfitGoal;
    j["profitGoalAmount"] = s.profitGoalAmount;
    j["notifyResetWarning"] = s.notifyResetWarning;
    j["resetWarningMinutes"] = s.resetWarningMinutes;
    j["notifySessionComplete"] = s.notifySessionComplete;
    j["sessionCompleteHours"] = s.sessionCompleteHours;

    return j;
}

void SettingsManager::FromSettingsJson(const nlohmann::json& j, Settings& s)
{
    // Load settings version for migration
    int loadedSettingsVersion = 0; // Default to old version (no settingsVersion field = old settings)
    if (j.contains("settingsVersion")) loadedSettingsVersion = j["settingsVersion"].get<int>();

    if (j.contains("language")) s.language = j["language"].get<std::string>();
    if (j.contains("currentAccountIndex")) s.currentAccountIndex = j["currentAccountIndex"].get<int>();

    if (j.contains("accounts") && j["accounts"].is_array())
    {
        s.accounts.clear();
        for (const auto& accJson : j["accounts"])
        {
            Account acc;
            if (accJson.contains("name")) acc.name = accJson["name"].get<std::string>();
            if (accJson.contains("drfToken")) acc.drfToken = SimpleDecrypt(FromHexString(accJson["drfToken"].get<std::string>()));
            if (accJson.contains("gw2ApiKey")) acc.gw2ApiKey = SimpleDecrypt(FromHexString(accJson["gw2ApiKey"].get<std::string>()));
            s.accounts.push_back(acc);
        }
    }

    // Legacy and backwards compatibility
    if (j.contains("drfToken")) {
        std::string decrypted = SimpleDecrypt(FromHexString(j["drfToken"].get<std::string>()));
        if (!decrypted.empty()) s.drfToken = decrypted;
    }
    if (j.contains("gw2ApiKey")) {
        std::string decrypted = SimpleDecrypt(FromHexString(j["gw2ApiKey"].get<std::string>()));
        if (!decrypted.empty()) s.gw2ApiKey = decrypted;
    }

    if (j.contains("automaticResetMode")) s.automaticResetMode = j["automaticResetMode"].get<int>();
    if (j.contains("minutesUntilReset")) s.minutesUntilResetAfterShutdown = j["minutesUntilReset"].get<int>();
    if (j.contains("customResetDays")) s.customResetDays = j["customResetDays"].get<int>();
    if (j.contains("toggleHotkey")) s.toggleHotkey = j["toggleHotkey"].get<std::string>();
    if (j.contains("miniWindowToggleHotkey")) s.miniWindowToggleHotkey = j["miniWindowToggleHotkey"].get<std::string>();
    if (j.contains("resetHotkey")) s.resetHotkey = j["resetHotkey"].get<std::string>();
    if (j.contains("nextResetDateTimeUtc")) s.nextResetDateTimeUtc = j["nextResetDateTimeUtc"].get<std::string>();
    if (j.contains("manualResetPending")) s.manualResetPending = j["manualResetPending"].get<bool>();
    if (j.contains("historyIconSize")) s.historyIconSize = j["historyIconSize"].get<int>();
    if (j.contains("profitIconSize")) s.profitIconSize = j["profitIconSize"].get<int>();
    if (j.contains("itemsIconSize")) s.itemsIconSize = j["itemsIconSize"].get<int>();
    if (j.contains("iconSize")) { s.itemsIconSize = j["iconSize"].get<int>(); } // Legacy support: migrate iconSize to itemsIconSize
    if (j.contains("showRarityBorder")) s.showRarityBorder = j["showRarityBorder"].get<bool>();
    if (j.contains("rarityBorderSize")) s.rarityBorderSize = j["rarityBorderSize"].get<float>();
    if (j.contains("enableGradientBackgrounds")) s.enableGradientBackgrounds = j["enableGradientBackgrounds"].get<bool>();
    
    if (j.contains("gradientTopColor") && j["gradientTopColor"].is_array() && j["gradientTopColor"].size() == 4)
        for (int i = 0; i < 4; i++) s.gradientTopColor[i] = j["gradientTopColor"][i].get<float>();
    if (j.contains("gradientBottomColor") && j["gradientBottomColor"].is_array() && j["gradientBottomColor"].size() == 4)
        for (int i = 0; i < 4; i++) s.gradientBottomColor[i] = j["gradientBottomColor"][i].get<float>();
    
    if (j.contains("showProfitSparkline")) s.showProfitSparkline = j["showProfitSparkline"].get<bool>();
    if (j.contains("sparklineColor")) s.sparklineColor = j["sparklineColor"].get<int>();
    if (j.contains("enableSummariesInProfitTab")) s.enableSummariesInProfitTab = j["enableSummariesInProfitTab"].get<bool>();
    if (j.contains("enableBestDropHighlight")) s.enableBestDropHighlight = j["enableBestDropHighlight"].get<bool>();
    if (j.contains("bestDropHighlightColor") && j["bestDropHighlightColor"].is_array() && j["bestDropHighlightColor"].size() == 4)
        for (int i = 0; i < 4; i++) s.bestDropHighlightColor[i] = j["bestDropHighlightColor"][i].get<float>();
    if (j.contains("miniWindowShowBestDropSingle")) s.miniWindowShowBestDropSingle = j["miniWindowShowBestDropSingle"].get<bool>();
    if (j.contains("miniWindowShowBestDropTotalValue")) s.miniWindowShowBestDropTotalValue = j["miniWindowShowBestDropTotalValue"].get<bool>();
    if (j.contains("miniWindowShowBestDropIcons")) s.miniWindowShowBestDropIcons = j["miniWindowShowBestDropIcons"].get<bool>();
    if (j.contains("miniWindowBestDropIconSize")) s.miniWindowBestDropIconSize = j["miniWindowBestDropIconSize"].get<int>();
    if (j.contains("showShortIcon")) s.showShortIcon = j["showShortIcon"].get<bool>();
    if (j.contains("showMiniWindow")) s.showMiniWindow = j["showMiniWindow"].get<bool>();
    if (j.contains("miniWindowVisibilityMode")) {
        int val = j["miniWindowVisibilityMode"].get<int>();
        if (val > 1) val = 1; // Convert old InCombat (1) and OutOfCombat (2) to new OutOfCombat (1)
        s.miniWindowVisibilityMode = static_cast<MiniWindowVisibilityMode>(val);
    }
    if (j.contains("miniWindowShowProfit")) s.miniWindowShowProfit = j["miniWindowShowProfit"].get<bool>();
    if (j.contains("miniWindowShowProfitPerHour")) s.miniWindowShowProfitPerHour = j["miniWindowShowProfitPerHour"].get<bool>();
    if (j.contains("miniWindowShowTradingProfitSell")) s.miniWindowShowTradingProfitSell = j["miniWindowShowTradingProfitSell"].get<bool>();
    if (j.contains("miniWindowShowTradingProfitInstant")) s.miniWindowShowTradingProfitInstant = j["miniWindowShowTradingProfitInstant"].get<bool>();
    if (j.contains("miniWindowShowTotalItems")) s.miniWindowShowTotalItems = j["miniWindowShowTotalItems"].get<bool>();
    if (j.contains("miniWindowShowSessionDuration")) s.miniWindowShowSessionDuration = j["miniWindowShowSessionDuration"].get<bool>();
    if (j.contains("miniWindowClickThrough")) s.miniWindowClickThrough = j["miniWindowClickThrough"].get<bool>();
    if (j.contains("miniWindowAllowRightClickUnpin")) s.miniWindowAllowRightClickUnpin = j["miniWindowAllowRightClickUnpin"].get<bool>();
    if (j.contains("miniWindowShowMaterialStorageCount")) s.miniWindowShowMaterialStorageCount = j["miniWindowShowMaterialStorageCount"].get<bool>();
    if (j.contains("miniWindowShowWalletCount")) s.miniWindowShowWalletCount = j["miniWindowShowWalletCount"].get<bool>();
    if (j.contains("miniWindowShowBankCount")) s.miniWindowShowBankCount = j["miniWindowShowBankCount"].get<bool>();
    if (j.contains("miniWindowShowInventoryCount")) s.miniWindowShowInventoryCount = j["miniWindowShowInventoryCount"].get<bool>();
    if (j.contains("miniWindowHideCountLabels")) s.miniWindowHideCountLabels = j["miniWindowHideCountLabels"].get<bool>();
    if (j.contains("miniWindowShortCountLabels")) s.miniWindowShortCountLabels = j["miniWindowShortCountLabels"].get<bool>();
    if (j.contains("miniWindowHideZeroDropStats")) s.miniWindowHideZeroDropStats = j["miniWindowHideZeroDropStats"].get<bool>();
    if (j.contains("miniWindowHideTextLabels")) s.miniWindowHideTextLabels = j["miniWindowHideTextLabels"].get<bool>();
    if (j.contains("miniWindowHideIcons")) s.miniWindowHideIcons = j["miniWindowHideIcons"].get<bool>();
    if (j.contains("miniWindowPinnedIconSize")) s.miniWindowPinnedIconSize = j["miniWindowPinnedIconSize"].get<float>();
    if (j.contains("mainWindowFontSize")) s.mainWindowFontSize = j["mainWindowFontSize"].get<float>();
    if (j.contains("tabContentFontSize")) s.tabContentFontSize = j["tabContentFontSize"].get<float>();
    if (j.contains("notificationFontSize")) s.notificationFontSize = j["notificationFontSize"].get<float>();
    if (j.contains("miniWindowHideTitleBar")) s.miniWindowHideTitleBar = j["miniWindowHideTitleBar"].get<bool>();
    if (j.contains("miniWindowHideBorder")) s.miniWindowHideBorder = j["miniWindowHideBorder"].get<bool>();
    if (j.contains("miniWindowLocked")) s.miniWindowLocked = j["miniWindowLocked"].get<bool>();
    if (j.contains("miniWindowPosX")) s.miniWindowPosX = j["miniWindowPosX"].get<float>();
    if (j.contains("miniWindowPosY")) s.miniWindowPosY = j["miniWindowPosY"].get<float>();
    if (j.contains("miniWindowWidth")) s.miniWindowWidth = j["miniWindowWidth"].get<float>();
    if (j.contains("miniWindowHeight")) s.miniWindowHeight = j["miniWindowHeight"].get<float>();
    if (j.contains("miniWindowEnableTextShadow")) s.miniWindowEnableTextShadow = j["miniWindowEnableTextShadow"].get<bool>();
    if (j.contains("miniWindowTextColor")) s.miniWindowTextColor = j["miniWindowTextColor"].get<int>();
    if (j.contains("miniWindowFontSize")) s.miniWindowFontSize = j["miniWindowFontSize"].get<float>();
    if (j.contains("miniWindowElementOrder")) s.miniWindowElementOrder = j["miniWindowElementOrder"].get<std::vector<std::string>>();
    if (j.contains("showMainWindow")) s.showMainWindow = j["showMainWindow"].get<bool>();
    if (j.contains("mainWindowVisibilityMode")) s.mainWindowVisibilityMode = static_cast<MainWindowVisibilityMode>(j["mainWindowVisibilityMode"].get<int>());
    if (j.contains("mainWindowClickThrough")) s.mainWindowClickThrough = j["mainWindowClickThrough"].get<bool>();
    if (j.contains("mainWindowHideTitleBar")) s.mainWindowHideTitleBar = j["mainWindowHideTitleBar"].get<bool>();
    if (j.contains("mainWindowPosX")) s.mainWindowPosX = j["mainWindowPosX"].get<float>();
    if (j.contains("mainWindowPosY")) s.mainWindowPosY = j["mainWindowPosY"].get<float>();
    if (j.contains("mainWindowWidth")) s.mainWindowWidth = j["mainWindowWidth"].get<float>();
    if (j.contains("mainWindowHeight")) s.mainWindowHeight = j["mainWindowHeight"].get<float>();
    if (j.contains("activeTab")) s.activeTab = j["activeTab"].get<int>();
    if (j.contains("enableDashboardTab")) s.enableDashboardTab = j["enableDashboardTab"].get<bool>();
    if (j.contains("enableDropsTab")) s.enableDropsTab = j["enableDropsTab"].get<bool>();
    if (j.contains("enableSessionHistoryTab")) s.enableSessionHistoryTab = j["enableSessionHistoryTab"].get<bool>();
    if (j.contains("enableTimelineTab")) s.enableTimelineTab = j["enableTimelineTab"].get<bool>();
    if (j.contains("lockTabOrder")) s.lockTabOrder = j["lockTabOrder"].get<bool>();
    if (j.contains("mainTabOrder") && j["mainTabOrder"].is_array()) s.mainTabOrder = j["mainTabOrder"].get<std::vector<std::string>>();
    if (j.contains("itemSortMode")) s.itemSortMode = j["itemSortMode"].get<int>();
    if (j.contains("itemRarityFilterMin")) s.itemRarityFilterMin = j["itemRarityFilterMin"].get<int>();
    if (j.contains("showItemIcons")) s.showItemIcons = j["showItemIcons"].get<bool>();
    if (j.contains("groupByRarity")) s.groupByRarity = j["groupByRarity"].get<bool>();
    if (j.contains("showRarityAsTabs")) s.showRarityAsTabs = j["showRarityAsTabs"].get<bool>();

    // Drops Settings
    if (j.contains("itemsEnableGridView")) s.itemsEnableGridView = j["itemsEnableGridView"].get<bool>();
    s.itemsFavoritesFirst = true; // Force enabled, no user control
    if (j.contains("itemsFavoritesAsGrid")) s.itemsFavoritesAsGrid = j["itemsFavoritesAsGrid"].get<bool>();
    if (j.contains("itemsGroupByRarity")) s.itemsGroupByRarity = j["itemsGroupByRarity"].get<bool>();
    if (j.contains("itemsShowRarityAsTabs")) s.itemsShowRarityAsTabs = j["itemsShowRarityAsTabs"].get<bool>();
    if (j.contains("itemsGroupByCategory")) s.itemsGroupByCategory = j["itemsGroupByCategory"].get<bool>();
    if (j.contains("itemsShowGroupAsTabs")) s.itemsShowGroupAsTabs = j["itemsShowGroupAsTabs"].get<bool>();
    if (j.contains("currenciesEnableGridView")) s.currenciesEnableGridView = j["currenciesEnableGridView"].get<bool>();
    s.currenciesFavoritesFirst = true; // Force enabled, no user control
    if (j.contains("currenciesFavoritesAsGrid")) s.currenciesFavoritesAsGrid = j["currenciesFavoritesAsGrid"].get<bool>();
    if (j.contains("currenciesGroupByCategory")) s.currenciesGroupByCategory = j["currenciesGroupByCategory"].get<bool>();
    if (j.contains("currenciesShowGroupAsTabs")) s.currenciesShowGroupAsTabs = j["currenciesShowGroupAsTabs"].get<bool>();
    s.overviewCurrenciesFirst = true; // Force enabled, no user control
    if (j.contains("overviewEnableGridView")) s.overviewEnableGridView = j["overviewEnableGridView"].get<bool>();
    if (j.contains("overviewFavoritesAsGrid")) s.overviewFavoritesAsGrid = j["overviewFavoritesAsGrid"].get<bool>();
    if (j.contains("overviewFavoritesIconSize")) s.overviewFavoritesIconSize = j["overviewFavoritesIconSize"].get<int>();
    if (j.contains("overviewItemSortMode")) s.overviewItemSortMode = j["overviewItemSortMode"].get<int>();
    if (j.contains("overviewItemRarityFilterMin")) s.overviewItemRarityFilterMin = j["overviewItemRarityFilterMin"].get<int>();
    if (j.contains("overviewGroupByRarity")) s.overviewGroupByRarity = j["overviewGroupByRarity"].get<bool>();
    if (j.contains("overviewShowRarityAsTabs")) s.overviewShowRarityAsTabs = j["overviewShowRarityAsTabs"].get<bool>();
    if (j.contains("overviewGroupByCategory")) s.overviewGroupByCategory = j["overviewGroupByCategory"].get<bool>();
    if (j.contains("overviewShowGroupAsTabs")) s.overviewShowGroupAsTabs = j["overviewShowGroupAsTabs"].get<bool>();
    if (j.contains("itemsSavePath")) s.itemsSavePath = j["itemsSavePath"].get<std::string>();
    if (j.contains("currenciesSavePath")) s.currenciesSavePath = j["currenciesSavePath"].get<std::string>();
    if (j.contains("liveLogCustomPath")) s.liveLogCustomPath = j["liveLogCustomPath"].get<std::string>();
    if (j.contains("currencyGroupByCategory")) s.currencyGroupByCategory = j["currencyGroupByCategory"].get<bool>();
    if (j.contains("currencyShowAsTabs")) s.currencyShowAsTabs = j["currencyShowAsTabs"].get<bool>();
    if (j.contains("mainWindowOpacity")) s.mainWindowOpacity = j["mainWindowOpacity"].get<float>();
    if (j.contains("miniWindowOpacity")) s.miniWindowOpacity = j["miniWindowOpacity"].get<float>();
    if (j.contains("accentColorR")) s.accentColorR = j["accentColorR"].get<float>();
    if (j.contains("accentColorG")) s.accentColorG = j["accentColorG"].get<float>();
    if (j.contains("accentColorB")) s.accentColorB = j["accentColorB"].get<float>();
    if (j.contains("enableSessionHistory")) s.enableSessionHistory = j["enableSessionHistory"].get<bool>();
    if (j.contains("maxSessionHistory")) s.maxSessionHistory = j["maxSessionHistory"].get<int>();
    if (j.contains("overwriteSessionHistory")) s.overwriteSessionHistory = j["overwriteSessionHistory"].get<bool>();

    // Loot Log
    if (j.contains("lootLog"))
    {
        const auto& ll = j["lootLog"];
        s.enableLootLog       = ll.value("enabled",      true);
        s.lootLogFormat       = ll.value("format",       0);
        s.lootLogFolder       = ll.value("folder",       std::string{});
        s.lootLogMaxDays      = ll.value("maxDays",      30);
        s.lootLogItems        = ll.value("items",        true);
        s.lootLogCurrencies   = ll.value("currencies",   true);
        s.lootLogIncludeMap   = ll.value("includeMap",   true);
        s.lootLogIncludeMagicFind = ll.value("includeMagicFind", false);
    }

    // Magnetite Shard Weekly Tracker
    if (j.contains("magnetiteTracker"))
    {
        auto& mt = j["magnetiteTracker"];
        s.enableMagnetiteTracker           = mt.value("enabled",                 false);
        s.magnetiteWeeklyEarned            = mt.value("weeklyEarned",            0);
        s.magnetiteWeeklyEarnedAtLastCheck = mt.value("weeklyEarnedAtLastCheck", 0);
        s.magnetiteLastWalletTotal         = mt.value("lastWalletTotal",         -1);
        s.magnetiteLastApiCheckUtc         = mt.value("lastApiCheckUtc",         std::string{});
        s.magnetiteApiCheckCooldownMin     = mt.value("cooldownMin",             10);
    }

    // Gaeting Crystal Weekly Tracker
    if (j.contains("gaetingTracker"))
    {
        auto& gt = j["gaetingTracker"];
        s.enableGaetingTracker             = gt.value("enabled",                 false);
        s.gaetingWeeklyEarned              = gt.value("weeklyEarned",            0);
        s.gaetingWeeklyEarnedAtLastCheck   = gt.value("weeklyEarnedAtLastCheck", 0);
        s.gaetingLastWalletTotal           = gt.value("lastWalletTotal",         -1);
        s.gaetingLastApiCheckUtc           = gt.value("lastApiCheckUtc",         std::string{});
        s.gaetingApiCheckCooldownMin       = gt.value("cooldownMin",             10);
    }
    if (j.contains("filterSellableToVendor")) s.filterSellableToVendor = j["filterSellableToVendor"].get<bool>();
    if (j.contains("filterSellableOnTp")) s.filterSellableOnTp = j["filterSellableOnTp"].get<bool>();
    if (j.contains("filterCustomProfit")) s.filterCustomProfit = j["filterCustomProfit"].get<bool>();
    if (j.contains("filterKnownByApi")) s.filterKnownByApi = j["filterKnownByApi"].get<bool>();
    if (j.contains("filterUnknownByApi")) s.filterUnknownByApi = j["filterUnknownByApi"].get<bool>();
    if (j.contains("filterAccountBound")) s.filterAccountBound = j["filterAccountBound"].get<bool>();
    if (j.contains("filterNotAccountBound")) s.filterNotAccountBound = j["filterNotAccountBound"].get<bool>();
    if (j.contains("filterNoSell")) s.filterNoSell = j["filterNoSell"].get<bool>();
    if (j.contains("filterNotNoSell")) s.filterNotNoSell = j["filterNotNoSell"].get<bool>();
    if (j.contains("filterFavorite")) s.filterFavorite = j["filterFavorite"].get<bool>();
    if (j.contains("filterNotFavorite")) s.filterNotFavorite = j["filterNotFavorite"].get<bool>();
    if (j.contains("filterIgnored")) s.filterIgnored = j["filterIgnored"].get<bool>();
    if (j.contains("filterNotIgnored")) s.filterNotIgnored = j["filterNotIgnored"].get<bool>();
    if (j.contains("showRangeFilters")) s.showRangeFilters = j["showRangeFilters"].get<bool>();
    if (j.contains("filterMinPriceGold")) s.filterMinPriceGold = j["filterMinPriceGold"].get<int>();
    if (j.contains("filterMinPriceSilver")) s.filterMinPriceSilver = j["filterMinPriceSilver"].get<int>();
    if (j.contains("filterMinPriceCopper")) s.filterMinPriceCopper = j["filterMinPriceCopper"].get<int>();
    if (j.contains("filterMaxPriceGold")) s.filterMaxPriceGold = j["filterMaxPriceGold"].get<int>();
    if (j.contains("filterMaxPriceSilver")) s.filterMaxPriceSilver = j["filterMaxPriceSilver"].get<int>();
    if (j.contains("filterMaxPriceCopper")) s.filterMaxPriceCopper = j["filterMaxPriceCopper"].get<int>();
    if (j.contains("filterMinQuantity")) s.filterMinQuantity = j["filterMinQuantity"].get<int>();
    if (j.contains("filterMaxQuantity")) s.filterMaxQuantity = j["filterMaxQuantity"].get<int>();
    if (j.contains("filterTypeArmor")) s.filterTypeArmor = j["filterTypeArmor"].get<bool>();
    if (j.contains("filterTypeWeapon")) s.filterTypeWeapon = j["filterTypeWeapon"].get<bool>();
    if (j.contains("filterTypeTrinket")) s.filterTypeTrinket = j["filterTypeTrinket"].get<bool>();
    if (j.contains("filterTypeGizmo")) s.filterTypeGizmo = j["filterTypeGizmo"].get<bool>();
    if (j.contains("filterTypeCraftingMaterial")) s.filterTypeCraftingMaterial = j["filterTypeCraftingMaterial"].get<bool>();
    if (j.contains("filterTypeConsumable")) s.filterTypeConsumable = j["filterTypeConsumable"].get<bool>();
    if (j.contains("filterTypeGatheringTool")) s.filterTypeGatheringTool = j["filterTypeGatheringTool"].get<bool>();
    if (j.contains("filterTypeBag")) s.filterTypeBag = j["filterTypeBag"].get<bool>();
    if (j.contains("filterTypeContainer")) s.filterTypeContainer = j["filterTypeContainer"].get<bool>();
    if (j.contains("filterTypeMiniPet")) s.filterTypeMiniPet = j["filterTypeMiniPet"].get<bool>();
    if (j.contains("filterTypeGizmoContainer")) s.filterTypeGizmoContainer = j["filterTypeGizmoContainer"].get<bool>();
    if (j.contains("filterTypeBackpack")) s.filterTypeBackpack = j["filterTypeBackpack"].get<bool>();
    if (j.contains("filterTypeUpgradeComponent")) s.filterTypeUpgradeComponent = j["filterTypeUpgradeComponent"].get<bool>();
    if (j.contains("filterTypeTool")) s.filterTypeTool = j["filterTypeTool"].get<bool>();
    if (j.contains("filterTypeTrophy")) s.filterTypeTrophy = j["filterTypeTrophy"].get<bool>();
    if (j.contains("filterTypeUnlock")) s.filterTypeUnlock = j["filterTypeUnlock"].get<bool>();
    if (j.contains("filterKarma")) s.filterKarma = j["filterKarma"].get<bool>();
    if (j.contains("filterLaurel")) s.filterLaurel = j["filterLaurel"].get<bool>();
    if (j.contains("filterGem")) s.filterGem = j["filterGem"].get<bool>();
    if (j.contains("filterFractalRelic")) s.filterFractalRelic = j["filterFractalRelic"].get<bool>();
    if (j.contains("filterBadgeOfHonor")) s.filterBadgeOfHonor = j["filterBadgeOfHonor"].get<bool>();
    if (j.contains("filterGuildCommendation")) s.filterGuildCommendation = j["filterGuildCommendation"].get<bool>();
    if (j.contains("filterTransmutationCharge")) s.filterTransmutationCharge = j["filterTransmutationCharge"].get<bool>();
    if (j.contains("filterSpiritShards")) s.filterSpiritShards = j["filterSpiritShards"].get<bool>();
    if (j.contains("filterUnboundMagic")) s.filterUnboundMagic = j["filterUnboundMagic"].get<bool>();
    if (j.contains("filterVolatileMagic")) s.filterVolatileMagic = j["filterVolatileMagic"].get<bool>();
    if (j.contains("filterAirshipParts")) s.filterAirshipParts = j["filterAirshipParts"].get<bool>();
    if (j.contains("filterGeode")) s.filterGeode = j["filterGeode"].get<bool>();
    if (j.contains("filterLeyLineCrystals")) s.filterLeyLineCrystals = j["filterLeyLineCrystals"].get<bool>();
    if (j.contains("filterTradeContracts")) s.filterTradeContracts = j["filterTradeContracts"].get<bool>();
    if (j.contains("filterElegyMosaic")) s.filterElegyMosaic = j["filterElegyMosaic"].get<bool>();
    if (j.contains("filterUncommonCoins")) s.filterUncommonCoins = j["filterUncommonCoins"].get<bool>();
    if (j.contains("filterAstralAcclaim")) s.filterAstralAcclaim = j["filterAstralAcclaim"].get<bool>();
    if (j.contains("filterPristineFractalRelics")) s.filterPristineFractalRelics = j["filterPristineFractalRelics"].get<bool>();
    if (j.contains("filterUnstableFractalEssence")) s.filterUnstableFractalEssence = j["filterUnstableFractalEssence"].get<bool>();
    if (j.contains("filterMagnetiteShards")) s.filterMagnetiteShards = j["filterMagnetiteShards"].get<bool>();
    if (j.contains("filterGaetingCrystals")) s.filterGaetingCrystals = j["filterGaetingCrystals"].get<bool>();
    if (j.contains("filterProphetShards")) s.filterProphetShards = j["filterProphetShards"].get<bool>();
    if (j.contains("filterGreenProphetShards")) s.filterGreenProphetShards = j["filterGreenProphetShards"].get<bool>();
    if (j.contains("filterWvWSkirmishTickets")) s.filterWvWSkirmishTickets = j["filterWvWSkirmishTickets"].get<bool>();
    if (j.contains("filterProofsOfHeroics")) s.filterProofsOfHeroics = j["filterProofsOfHeroics"].get<bool>();
    if (j.contains("filterPvpLeagueTickets")) s.filterPvpLeagueTickets = j["filterPvpLeagueTickets"].get<bool>();
    if (j.contains("filterAscendedShardsOfGlory")) s.filterAscendedShardsOfGlory = j["filterAscendedShardsOfGlory"].get<bool>();
    if (j.contains("filterResearchNotes")) s.filterResearchNotes = j["filterResearchNotes"].get<bool>();
    if (j.contains("filterTyrianDefenseSeal")) s.filterTyrianDefenseSeal = j["filterTyrianDefenseSeal"].get<bool>();
    if (j.contains("filterTestimonyOfDesertHeroics")) s.filterTestimonyOfDesertHeroics = j["filterTestimonyOfDesertHeroics"].get<bool>();
    if (j.contains("filterTestimonyOfJadeHeroics")) s.filterTestimonyOfJadeHeroics = j["filterTestimonyOfJadeHeroics"].get<bool>();
    if (j.contains("filterTestimonyOfCastoranHeroics")) s.filterTestimonyOfCastoranHeroics = j["filterTestimonyOfCastoranHeroics"].get<bool>();
    if (j.contains("filterLegendaryInsight")) s.filterLegendaryInsight = j["filterLegendaryInsight"].get<bool>();
    if (j.contains("filterTalesOfDungeonDelving")) s.filterTalesOfDungeonDelving = j["filterTalesOfDungeonDelving"].get<bool>();
    if (j.contains("filterImperialFavor")) s.filterImperialFavor = j["filterImperialFavor"].get<bool>();
    if (j.contains("filterCanachCoins")) s.filterCanachCoins = j["filterCanachCoins"].get<bool>();
    if (j.contains("filterAncientCoin")) s.filterAncientCoin = j["filterAncientCoin"].get<bool>();
    if (j.contains("filterUnusualCoin")) s.filterUnusualCoin = j["filterUnusualCoin"].get<bool>();
    if (j.contains("filterJadeSliver")) s.filterJadeSliver = j["filterJadeSliver"].get<bool>();
    if (j.contains("filterStaticCharge")) s.filterStaticCharge = j["filterStaticCharge"].get<bool>();
    if (j.contains("filterPinchOfStardust")) s.filterPinchOfStardust = j["filterPinchOfStardust"].get<bool>();
    if (j.contains("filterCalcifiedGasp")) s.filterCalcifiedGasp = j["filterCalcifiedGasp"].get<bool>();
    if (j.contains("filterUrsusOblige")) s.filterUrsusOblige = j["filterUrsusOblige"].get<bool>();
    if (j.contains("filterGaetingCrystalJanthir")) s.filterGaetingCrystalJanthir = j["filterGaetingCrystalJanthir"].get<bool>();
    if (j.contains("filterAntiquatedDucat")) s.filterAntiquatedDucat = j["filterAntiquatedDucat"].get<bool>();
    if (j.contains("filterAetherRichSap")) s.filterAetherRichSap = j["filterAetherRichSap"].get<bool>();
    if (j.contains("enableCustomProfit")) s.enableCustomProfit = j["enableCustomProfit"].get<bool>();
    if (j.contains("enableSearch")) s.enableSearch = j["enableSearch"].get<bool>();
    if (j.contains("searchTerm")) s.searchTerm = j["searchTerm"].get<std::string>();
    if (j.contains("enableIgnoredItems")) s.enableIgnoredItems = j["enableIgnoredItems"].get<bool>();
    if (j.contains("enableIconCache")) s.enableIconCache = j["enableIconCache"].get<bool>();
    if (j.contains("iconCacheMaxIcons"))
    {
        s.iconCacheMaxIcons = j["iconCacheMaxIcons"].get<int>();
        // Migration: If settings version < 3, force unlimited (0)
        if (loadedSettingsVersion < 3) {
            s.iconCacheMaxIcons = 0;
        }
        // Clamp immediately on load (same rules as ValidateAll) so legacy/invalid values never propagate.
        if (s.iconCacheMaxIcons < 0) s.iconCacheMaxIcons = 0;
        else if (s.iconCacheMaxIcons == 0) { /* unlimited */ }
        else if (s.iconCacheMaxIcons < 2000) s.iconCacheMaxIcons = 2000;
        else if (s.iconCacheMaxIcons > 5000) s.iconCacheMaxIcons = 0;
    }
    if (j.contains("maxHistoryItems")) s.maxHistoryItems = j["maxHistoryItems"].get<int>();
    if (j.contains("priceUpdateIntervalMin")) s.priceUpdateIntervalMin = j["priceUpdateIntervalMin"].get<int>();
    if (j.contains("disableComplexVisualsOnLowPerf")) s.disableComplexVisualsOnLowPerf = j["disableComplexVisualsOnLowPerf"].get<bool>();
    if (j.contains("countTextColor")) s.countTextColor = j["countTextColor"].get<int>();
    if (j.contains("countBackgroundColor")) s.countBackgroundColor = j["countBackgroundColor"].get<int>();
    if (j.contains("countFontSize")) s.countFontSize = j["countFontSize"].get<int>();
    if (j.contains("countHorizontalAlignment")) s.countHorizontalAlignment = j["countHorizontalAlignment"].get<int>();
    if (j.contains("profitLabelText")) s.profitLabelText = j["profitLabelText"].get<std::string>();
    if (j.contains("profitPerHourLabelText")) s.profitPerHourLabelText = j["profitPerHourLabelText"].get<std::string>();
    if (j.contains("profitWindowDisplayMode")) s.profitWindowDisplayMode = j["profitWindowDisplayMode"].get<int>();
    if (j.contains("enableFavorites")) s.enableFavorites = j["enableFavorites"].get<bool>();
    if (j.contains("favoritesFirst")) s.favoritesFirst = j["favoritesFirst"].get<bool>();
    if (j.contains("enableFavoriteTextColor")) s.enableFavoriteTextColor = j["enableFavoriteTextColor"].get<bool>();
    if (j.contains("favoriteTextColor") && j["favoriteTextColor"].is_array() && j["favoriteTextColor"].size() == 4)
        for (int i = 0; i < 4; i++) s.favoriteTextColor[i] = j["favoriteTextColor"][i].get<float>();
    if (j.contains("enableFavoriteRowColor")) s.enableFavoriteRowColor = j["enableFavoriteRowColor"].get<bool>();
    if (j.contains("favoriteRowColor") && j["favoriteRowColor"].is_array() && j["favoriteRowColor"].size() == 4)
        for (int i = 0; i < 4; i++) s.favoriteRowColor[i] = j["favoriteRowColor"][i].get<float>();
    if (j.contains("enableGridViewItems")) s.enableGridViewItems = j["enableGridViewItems"].get<bool>();
    if (j.contains("enableGridViewCurrencies")) s.enableGridViewCurrencies = j["enableGridViewCurrencies"].get<bool>();
    if (j.contains("enableGridViewSummary")) s.enableGridViewSummary = j["enableGridViewSummary"].get<bool>();
    if (j.contains("enableGridViewSummaryFavorites")) s.enableGridViewSummaryFavorites = j["enableGridViewSummaryFavorites"].get<bool>();
    if (j.contains("enableGridViewSummaryCurrencies")) s.enableGridViewSummaryCurrencies = j["enableGridViewSummaryCurrencies"].get<bool>();
    if (j.contains("enableGridViewSummaryItems")) s.enableGridViewSummaryItems = j["enableGridViewSummaryItems"].get<bool>();
    if (j.contains("gridIconSize")) s.gridIconSize = j["gridIconSize"].get<int>();
    if (j.contains("gridIconSizeCurrencies")) s.gridIconSizeCurrencies = j["gridIconSizeCurrencies"].get<int>();
    if (j.contains("timelineIconSizeItems")) s.timelineIconSizeItems = j["timelineIconSizeItems"].get<int>();
    if (j.contains("timelineIconSizeCurrencies")) s.timelineIconSizeCurrencies = j["timelineIconSizeCurrencies"].get<int>();
    if (j.contains("showTopItems")) s.showTopItems = j["showTopItems"].get<bool>();
    if (j.contains("showTopCurrencies")) s.showTopCurrencies = j["showTopCurrencies"].get<bool>();
    if (j.contains("enableDebugTab")) s.enableDebugTab = j["enableDebugTab"].get<bool>();
    if (j.contains("gw2ApiConnectTimeout")) s.gw2ApiConnectTimeout = j["gw2ApiConnectTimeout"].get<int>();
    if (j.contains("gw2ApiReceiveTimeout")) s.gw2ApiReceiveTimeout = j["gw2ApiReceiveTimeout"].get<int>();
    
    if (j.contains("salvageKitSettings") && j["salvageKitSettings"].is_object())
    {
        for (auto it = j["salvageKitSettings"].begin(); it != j["salvageKitSettings"].end(); ++it)
        {
            try {
                int id = std::stoi(it.key());
                Settings::SalvageKitSetting sks;
                if (it.value().contains("enabled")) sks.enabled = it.value()["enabled"].get<bool>();
                if (it.value().contains("useKarma")) sks.useKarma = it.value()["useKarma"].get<bool>();
                s.salvageKitSettings[id] = sks;
            } catch (...) {}
        }
    }

    if (j.contains("settingsProfiles") && j["settingsProfiles"].is_array())
    {
        s.settingsProfiles.clear();
        for (const auto& profileJson : j["settingsProfiles"])
        {
            Settings::SettingsProfile profile;
            if (profileJson.contains("name")) profile.name = profileJson["name"].get<std::string>();
            if (profileJson.contains("profileData")) profile.profileData = profileJson["profileData"].get<std::string>();
            s.settingsProfiles.push_back(profile);
        }
    }
    if (j.contains("currentProfileIndex")) s.currentProfileIndex = j["currentProfileIndex"].get<int>();
    if (j.contains("enableAutoBackups")) s.enableAutoBackups = j["enableAutoBackups"].get<bool>();
    if (j.contains("maxBackupCount")) s.maxBackupCount = j["maxBackupCount"].get<int>();
    if (j.contains("backupFrequency")) s.backupFrequency = j["backupFrequency"].get<int>();
    if (j.contains("autoBackupPath")) s.autoBackupPath = j["autoBackupPath"].get<std::string>();
    if (j.contains("lastBackupTimestamp")) s.lastBackupTimestamp = j["lastBackupTimestamp"].get<std::string>();
    if (j.contains("enableNotifications")) s.enableNotifications = j["enableNotifications"].get<bool>();
    if (j.contains("showNotificationSetup")) s.showNotificationSetup = j["showNotificationSetup"].get<bool>();
    if (j.contains("notificationPosX")) s.notificationPosX = j["notificationPosX"].get<float>();
    if (j.contains("notificationPosY")) s.notificationPosY = j["notificationPosY"].get<float>();
    if (j.contains("notificationWidth")) s.notificationWidth = j["notificationWidth"].get<float>();
    if (j.contains("notificationHeight")) s.notificationHeight = j["notificationHeight"].get<float>();
    if (j.contains("notificationDuration")) s.notificationDuration = j["notificationDuration"].get<float>();
    if (j.contains("notificationEnableMinValue")) s.notificationEnableMinValue = j["notificationEnableMinValue"].get<bool>();
    if (j.contains("notificationMinValueGold")) s.notificationMinValueGold = j["notificationMinValueGold"].get<float>();
    if (j.contains("notificationEnableMinRarity")) s.notificationEnableMinRarity = j["notificationEnableMinRarity"].get<bool>();
    if (j.contains("notificationMinRarity")) s.notificationMinRarity = j["notificationMinRarity"].get<int>();
    if (j.contains("notificationCombineValueAndRarity")) s.notificationCombineValueAndRarity = j["notificationCombineValueAndRarity"].get<bool>();
    if (j.contains("notificationIncludeNonProfit")) s.notificationIncludeNonProfit = j["notificationIncludeNonProfit"].get<bool>();
    if (j.contains("notificationPrecursorAlert")) s.notificationPrecursorAlert = j["notificationPrecursorAlert"].get<bool>();
    if (j.contains("notificationInfusionAlert")) s.notificationInfusionAlert = j["notificationInfusionAlert"].get<bool>();
    if (j.contains("notificationIncludeAgonyInfusions")) s.notificationIncludeAgonyInfusions = j["notificationIncludeAgonyInfusions"].get<bool>();
    if (j.contains("notificationStacking")) s.notificationStacking = j["notificationStacking"].get<bool>();
    if (j.contains("notificationBlacklist")) s.notificationBlacklist = j["notificationBlacklist"].get<std::vector<int>>();
    if (j.contains("notificationPlaySound")) s.notificationPlaySound = j["notificationPlaySound"].get<bool>();
    if (j.contains("notificationVolume")) s.notificationVolume = j["notificationVolume"].get<float>();
    if (j.contains("notificationVolumeStandard")) s.notificationVolumeStandard = j["notificationVolumeStandard"].get<float>();
    if (j.contains("notificationVolumePrecursor")) s.notificationVolumePrecursor = j["notificationVolumePrecursor"].get<float>();
    if (j.contains("notificationVolumeInfusion")) s.notificationVolumeInfusion = j["notificationVolumeInfusion"].get<float>();
    if (j.contains("notificationVolumeAlert")) s.notificationVolumeAlert = j["notificationVolumeAlert"].get<float>();
    if (j.contains("soundPathStandard")) s.soundPathStandard = j["soundPathStandard"].get<std::string>();
    if (j.contains("soundPathPrecursor")) s.soundPathPrecursor = j["soundPathPrecursor"].get<std::string>();
    if (j.contains("soundPathInfusion")) s.soundPathInfusion = j["soundPathInfusion"].get<std::string>();
    if (j.contains("soundPathAlert")) s.soundPathAlert = j["soundPathAlert"].get<std::string>();
    if (j.contains("notifyProfitGoal")) s.notifyProfitGoal = j["notifyProfitGoal"].get<bool>();
    if (j.contains("profitGoalAmount")) s.profitGoalAmount = j["profitGoalAmount"].get<int>();
    if (j.contains("notifyResetWarning")) s.notifyResetWarning = j["notifyResetWarning"].get<bool>();
    if (j.contains("resetWarningMinutes")) s.resetWarningMinutes = j["resetWarningMinutes"].get<int>();
    if (j.contains("notifySessionComplete")) s.notifySessionComplete = j["notifySessionComplete"].get<bool>();
    if (j.contains("sessionCompleteHours")) s.sessionCompleteHours = j["sessionCompleteHours"].get<int>();
}

static void SaveSettingsInternal(const std::string& filePath)
{
    if (filePath.empty()) return;

    try {
        std::filesystem::path p(filePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (...) {}

    json j = SettingsManager::ToSettingsJson(g_Settings);
    std::ofstream file(filePath);
    if (file.is_open())
    {
        file << j.dump(4);
        file.close();
    }
}

static void ClampSettings()
{
    g_Settings.automaticResetMode = std::clamp(g_Settings.automaticResetMode, 0, 7);
    g_Settings.minutesUntilResetAfterShutdown =
        std::clamp(g_Settings.minutesUntilResetAfterShutdown, 1, 24 * 60);
    g_Settings.historyIconSize = std::clamp(g_Settings.historyIconSize, 16, 96);
    g_Settings.profitIconSize = std::clamp(g_Settings.profitIconSize, 16, 96);
    g_Settings.itemsIconSize = std::clamp(g_Settings.itemsIconSize, 16, 96);
    g_Settings.gridIconSize = std::clamp(g_Settings.gridIconSize, 16, 128);
    g_Settings.gridIconSizeCurrencies = std::clamp(g_Settings.gridIconSizeCurrencies, 16, 96);
    g_Settings.itemSortMode = std::clamp(g_Settings.itemSortMode, 0, 9);
    g_Settings.itemRarityFilterMin = std::clamp(g_Settings.itemRarityFilterMin, 0, 7);
    g_Settings.mainWindowOpacity = std::clamp(g_Settings.mainWindowOpacity, 0.0f, 1.0f);
    g_Settings.miniWindowOpacity = std::clamp(g_Settings.miniWindowOpacity, 0.0f, 1.0f);
    g_Settings.accentColorR = std::clamp(g_Settings.accentColorR, 0.0f, 1.0f);
    g_Settings.accentColorG = std::clamp(g_Settings.accentColorG, 0.0f, 1.0f);
    g_Settings.accentColorB = std::clamp(g_Settings.accentColorB, 0.0f, 1.0f);
    g_Settings.maxSessionHistory = std::clamp(g_Settings.maxSessionHistory, 1, 50);

    // Clamp other settings
    g_Settings.countFontSize = std::clamp(g_Settings.countFontSize, 10, 40);
    g_Settings.countHorizontalAlignment = std::clamp(g_Settings.countHorizontalAlignment, 0, 2);
    g_Settings.profitWindowDisplayMode = std::clamp(g_Settings.profitWindowDisplayMode, 0, 2);
    
    // Clamp API timeouts (1 second to 2 minutes)
    g_Settings.gw2ApiConnectTimeout = std::clamp(g_Settings.gw2ApiConnectTimeout, 1000, 120000);
    g_Settings.gw2ApiReceiveTimeout = std::clamp(g_Settings.gw2ApiReceiveTimeout, 1000, 120000);

    // Icon cache size clamping rules:
    //   0 = unlimited (valid, passthrough)
    //   1..1999 → clamp up to 2000 (minimum enforced value)
    //   2000..5000 → ok, passthrough
    //   5001+ → clamp to 0 (unlimited). The UI intentionally jumps to unlimited at the slider's upper bound.
    if (g_Settings.iconCacheMaxIcons < 0)
        g_Settings.iconCacheMaxIcons = 0;
    else if (g_Settings.iconCacheMaxIcons == 0)
        { /* unlimited, keep */ }
    else if (g_Settings.iconCacheMaxIcons < 2000)
        g_Settings.iconCacheMaxIcons = 2000;
    else if (g_Settings.iconCacheMaxIcons > 5000)
        g_Settings.iconCacheMaxIcons = 0;
}

void SettingsManager::Init(const char* addonDir)
{
    if (!addonDir || !addonDir[0])
        return;
    s_SettingsPath = std::string(addonDir) + "\\settings.json";
    if (APIDefs)
        APIDefs->Log(LOGL_INFO, "FarmingTracker", s_SettingsPath.c_str());
    Load();
}

void SettingsManager::Load()
{
    if (s_SettingsPath.empty())
        return;

    std::ifstream file(s_SettingsPath);
    if (!file.is_open())
    {
        if (APIDefs)
            APIDefs->Log(LOGL_INFO, "FarmingTracker", "Settings file not found, creating new one");
        Save();
        return;
    }

    try
    {
        json j;
        file >> j;

        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        SettingsManager::FromSettingsJson(j, g_Settings);

        // Backwards compatibility: if no accounts exist but legacy tokens do, migrate to account system
        if (g_Settings.accounts.empty() && (!g_Settings.drfToken.empty() || !g_Settings.gw2ApiKey.empty()))
        {
            Account defaultAccount;
            defaultAccount.name = "Default";
            defaultAccount.drfToken = g_Settings.drfToken;
            defaultAccount.gw2ApiKey = g_Settings.gw2ApiKey;
            g_Settings.accounts.push_back(defaultAccount);
            g_Settings.currentAccountIndex = 0;
        }
    }
    catch (...)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "Failed to parse settings.json");
    }

    EnsureMainTabOrderValid();
    ClampSettings();
}

void SettingsManager::Save()
{
    if (s_SettingsPath.empty())
        return;

    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    ClampSettings();
    EnsureMainTabOrderValid();

    json j = ToSettingsJson(g_Settings);

    // Create directory if it doesn't exist
    std::filesystem::path p(s_SettingsPath);
    if (p.has_parent_path()) {
        try {
            std::filesystem::create_directories(p.parent_path());
        } catch (...) {}
    }

    std::ofstream file(s_SettingsPath);
    if (file.is_open())
    {
        file << j.dump(4);
        file.close();
    }
}

bool SettingsManager::IsTokenValid(const std::string& token)
{
    if (token.empty()) return false;
    static const std::regex uuidRegex(
        R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})",
        std::regex::icase);
    return std::regex_match(token, uuidRegex);
}

bool SettingsManager::IsGw2ApiKeyPlausible(const std::string& key)
{
    if (key.empty()) return false;

    // A GW2 API key must have exactly 9 blocks separated by hyphens.
    // Example: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXXXXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    static const std::regex keyRegex(R"(^[0-9A-F]+(-[0-9A-F]+){8}$)", std::regex::icase);
    return std::regex_match(key, keyRegex);
}

void SettingsManager::ExportToFile(const std::string& filePath)
{
    if (filePath.empty()) return;

    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    ClampSettings();
    EnsureMainTabOrderValid();

    json j = ToSettingsJson(g_Settings);

    std::filesystem::path p(filePath);
    if (p.has_parent_path()) {
        try {
            std::filesystem::create_directories(p.parent_path());
        } catch (...) {}
    }

    std::ofstream file(filePath);
    if (file.is_open())
    {
        file << j.dump(4);
        file.close();
    }
}

void SettingsManager::ImportFromFile(const std::string& filePath)
{
    if (filePath.empty()) return;

    std::ifstream file(filePath);
    if (!file.is_open()) return;

    try
    {
        json j;
        file >> j;

        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        SettingsManager::FromSettingsJson(j, g_Settings);
    }
    catch (...) {}

    EnsureMainTabOrderValid();
    ClampSettings();
    Save(); // Save imported settings to main file
}

void SettingsManager::ImportFilterSettings(const nlohmann::json& j)
{
    const json* src = &j;
    if (j.is_object() && j.contains("filters") && j["filters"].is_object())
        src = &j["filters"];

    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);

    auto setBool = [&](const char* key, bool& dst)
    {
        if (src->contains(key) && (*src)[key].is_boolean())
            dst = (*src)[key].get<bool>();
    };
    auto setInt = [&](const char* key, int& dst)
    {
        if (src->contains(key) && (*src)[key].is_number_integer())
            dst = (*src)[key].get<int>();
    };

    setBool("filterSellableToVendor", g_Settings.filterSellableToVendor);
    setBool("filterSellableOnTp", g_Settings.filterSellableOnTp);
    setBool("filterCustomProfit", g_Settings.filterCustomProfit);
    setBool("filterKnownByApi", g_Settings.filterKnownByApi);
    setBool("filterUnknownByApi", g_Settings.filterUnknownByApi);

    setBool("filterAccountBound", g_Settings.filterAccountBound);
    setBool("filterNotAccountBound", g_Settings.filterNotAccountBound);
    setBool("filterNoSell", g_Settings.filterNoSell);
    setBool("filterNotNoSell", g_Settings.filterNotNoSell);
    setBool("filterFavorite", g_Settings.filterFavorite);
    setBool("filterNotFavorite", g_Settings.filterNotFavorite);
    setBool("filterIgnored", g_Settings.filterIgnored);
    setBool("filterNotIgnored", g_Settings.filterNotIgnored);

    setInt("filterMinPriceGold", g_Settings.filterMinPriceGold);
    setInt("filterMinPriceSilver", g_Settings.filterMinPriceSilver);
    setInt("filterMinPriceCopper", g_Settings.filterMinPriceCopper);
    setInt("filterMaxPriceGold", g_Settings.filterMaxPriceGold);
    setInt("filterMaxPriceSilver", g_Settings.filterMaxPriceSilver);
    setInt("filterMaxPriceCopper", g_Settings.filterMaxPriceCopper);
    setInt("filterMinQuantity", g_Settings.filterMinQuantity);
    setInt("filterMaxQuantity", g_Settings.filterMaxQuantity);

    setBool("filterTypeArmor", g_Settings.filterTypeArmor);
    setBool("filterTypeWeapon", g_Settings.filterTypeWeapon);
    setBool("filterTypeTrinket", g_Settings.filterTypeTrinket);
    setBool("filterTypeGizmo", g_Settings.filterTypeGizmo);
    setBool("filterTypeCraftingMaterial", g_Settings.filterTypeCraftingMaterial);
    setBool("filterTypeConsumable", g_Settings.filterTypeConsumable);
    setBool("filterTypeGatheringTool", g_Settings.filterTypeGatheringTool);
    setBool("filterTypeBag", g_Settings.filterTypeBag);
    setBool("filterTypeContainer", g_Settings.filterTypeContainer);
    setBool("filterTypeMiniPet", g_Settings.filterTypeMiniPet);
    setBool("filterTypeGizmoContainer", g_Settings.filterTypeGizmoContainer);
    setBool("filterTypeBackpack", g_Settings.filterTypeBackpack);
    setBool("filterTypeUpgradeComponent", g_Settings.filterTypeUpgradeComponent);
    setBool("filterTypeTool", g_Settings.filterTypeTool);
    setBool("filterTypeTrophy", g_Settings.filterTypeTrophy);
    setBool("filterTypeUnlock", g_Settings.filterTypeUnlock);

    setBool("filterKarma", g_Settings.filterKarma);
    setBool("filterLaurel", g_Settings.filterLaurel);
    setBool("filterGem", g_Settings.filterGem);
    setBool("filterFractalRelic", g_Settings.filterFractalRelic);
    setBool("filterBadgeOfHonor", g_Settings.filterBadgeOfHonor);
    setBool("filterGuildCommendation", g_Settings.filterGuildCommendation);
    setBool("filterTransmutationCharge", g_Settings.filterTransmutationCharge);
    setBool("filterSpiritShards", g_Settings.filterSpiritShards);
    setBool("filterUnboundMagic", g_Settings.filterUnboundMagic);
    setBool("filterVolatileMagic", g_Settings.filterVolatileMagic);
    setBool("filterAirshipParts", g_Settings.filterAirshipParts);
    setBool("filterGeode", g_Settings.filterGeode);
    setBool("filterLeyLineCrystals", g_Settings.filterLeyLineCrystals);
    setBool("filterTradeContracts", g_Settings.filterTradeContracts);
    setBool("filterElegyMosaic", g_Settings.filterElegyMosaic);
    setBool("filterUncommonCoins", g_Settings.filterUncommonCoins);
    setBool("filterAstralAcclaim", g_Settings.filterAstralAcclaim);
    setBool("filterPristineFractalRelics", g_Settings.filterPristineFractalRelics);
    setBool("filterUnstableFractalEssence", g_Settings.filterUnstableFractalEssence);
    setBool("filterMagnetiteShards", g_Settings.filterMagnetiteShards);
    setBool("filterGaetingCrystals", g_Settings.filterGaetingCrystals);
    setBool("filterProphetShards", g_Settings.filterProphetShards);
    setBool("filterGreenProphetShards", g_Settings.filterGreenProphetShards);
    setBool("filterWvWSkirmishTickets", g_Settings.filterWvWSkirmishTickets);
    setBool("filterProofsOfHeroics", g_Settings.filterProofsOfHeroics);
    setBool("filterPvpLeagueTickets", g_Settings.filterPvpLeagueTickets);
    setBool("filterAscendedShardsOfGlory", g_Settings.filterAscendedShardsOfGlory);
    setBool("filterResearchNotes", g_Settings.filterResearchNotes);
    setBool("filterTyrianDefenseSeal", g_Settings.filterTyrianDefenseSeal);
    setBool("filterTestimonyOfDesertHeroics", g_Settings.filterTestimonyOfDesertHeroics);
    setBool("filterTestimonyOfJadeHeroics", g_Settings.filterTestimonyOfJadeHeroics);
    setBool("filterTestimonyOfCastoranHeroics", g_Settings.filterTestimonyOfCastoranHeroics);
    setBool("filterLegendaryInsight", g_Settings.filterLegendaryInsight);
    setBool("filterTalesOfDungeonDelving", g_Settings.filterTalesOfDungeonDelving);
    setBool("filterImperialFavor", g_Settings.filterImperialFavor);
    setBool("filterCanachCoins", g_Settings.filterCanachCoins);
    setBool("filterAncientCoin", g_Settings.filterAncientCoin);
    setBool("filterUnusualCoin", g_Settings.filterUnusualCoin);
    setBool("filterJadeSliver", g_Settings.filterJadeSliver);
    setBool("filterStaticCharge", g_Settings.filterStaticCharge);
    setBool("filterPinchOfStardust", g_Settings.filterPinchOfStardust);
    setBool("filterCalcifiedGasp", g_Settings.filterCalcifiedGasp);
    setBool("filterUrsusOblige", g_Settings.filterUrsusOblige);
    setBool("filterGaetingCrystalJanthir", g_Settings.filterGaetingCrystalJanthir);
    setBool("filterAntiquatedDucat", g_Settings.filterAntiquatedDucat);
    setBool("filterAetherRichSap", g_Settings.filterAetherRichSap);

    ClampSettings();
    Save();
}

nlohmann::json SettingsManager::ExportFilterSettings()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);

    nlohmann::json j;
    j["filterSellableToVendor"] = g_Settings.filterSellableToVendor;
    j["filterSellableOnTp"] = g_Settings.filterSellableOnTp;
    j["filterCustomProfit"] = g_Settings.filterCustomProfit;
    j["filterKnownByApi"] = g_Settings.filterKnownByApi;
    j["filterUnknownByApi"] = g_Settings.filterUnknownByApi;

    j["filterAccountBound"] = g_Settings.filterAccountBound;
    j["filterNotAccountBound"] = g_Settings.filterNotAccountBound;
    j["filterNoSell"] = g_Settings.filterNoSell;
    j["filterNotNoSell"] = g_Settings.filterNotNoSell;
    j["filterFavorite"] = g_Settings.filterFavorite;
    j["filterNotFavorite"] = g_Settings.filterNotFavorite;
    j["filterIgnored"] = g_Settings.filterIgnored;
    j["filterNotIgnored"] = g_Settings.filterNotIgnored;

    j["filterMinPriceGold"] = g_Settings.filterMinPriceGold;
    j["filterMinPriceSilver"] = g_Settings.filterMinPriceSilver;
    j["filterMinPriceCopper"] = g_Settings.filterMinPriceCopper;
    j["filterMaxPriceGold"] = g_Settings.filterMaxPriceGold;
    j["filterMaxPriceSilver"] = g_Settings.filterMaxPriceSilver;
    j["filterMaxPriceCopper"] = g_Settings.filterMaxPriceCopper;
    j["filterMinQuantity"] = g_Settings.filterMinQuantity;
    j["filterMaxQuantity"] = g_Settings.filterMaxQuantity;

    j["filterTypeArmor"] = g_Settings.filterTypeArmor;
    j["filterTypeWeapon"] = g_Settings.filterTypeWeapon;
    j["filterTypeTrinket"] = g_Settings.filterTypeTrinket;
    j["filterTypeGizmo"] = g_Settings.filterTypeGizmo;
    j["filterTypeCraftingMaterial"] = g_Settings.filterTypeCraftingMaterial;
    j["filterTypeConsumable"] = g_Settings.filterTypeConsumable;
    j["filterTypeGatheringTool"] = g_Settings.filterTypeGatheringTool;
    j["filterTypeBag"] = g_Settings.filterTypeBag;
    j["filterTypeContainer"] = g_Settings.filterTypeContainer;
    j["filterTypeMiniPet"] = g_Settings.filterTypeMiniPet;
    j["filterTypeGizmoContainer"] = g_Settings.filterTypeGizmoContainer;
    j["filterTypeBackpack"] = g_Settings.filterTypeBackpack;
    j["filterTypeUpgradeComponent"] = g_Settings.filterTypeUpgradeComponent;
    j["filterTypeTool"] = g_Settings.filterTypeTool;
    j["filterTypeTrophy"] = g_Settings.filterTypeTrophy;
    j["filterTypeUnlock"] = g_Settings.filterTypeUnlock;

    j["filterKarma"] = g_Settings.filterKarma;
    j["filterLaurel"] = g_Settings.filterLaurel;
    j["filterGem"] = g_Settings.filterGem;
    j["filterFractalRelic"] = g_Settings.filterFractalRelic;
    j["filterBadgeOfHonor"] = g_Settings.filterBadgeOfHonor;
    j["filterGuildCommendation"] = g_Settings.filterGuildCommendation;
    j["filterTransmutationCharge"] = g_Settings.filterTransmutationCharge;
    j["filterSpiritShards"] = g_Settings.filterSpiritShards;
    j["filterUnboundMagic"] = g_Settings.filterUnboundMagic;
    j["filterVolatileMagic"] = g_Settings.filterVolatileMagic;
    j["filterAirshipParts"] = g_Settings.filterAirshipParts;
    j["filterGeode"] = g_Settings.filterGeode;
    j["filterLeyLineCrystals"] = g_Settings.filterLeyLineCrystals;
    j["filterTradeContracts"] = g_Settings.filterTradeContracts;
    j["filterElegyMosaic"] = g_Settings.filterElegyMosaic;
    j["filterUncommonCoins"] = g_Settings.filterUncommonCoins;
    j["filterAstralAcclaim"] = g_Settings.filterAstralAcclaim;
    j["filterPristineFractalRelics"] = g_Settings.filterPristineFractalRelics;
    j["filterUnstableFractalEssence"] = g_Settings.filterUnstableFractalEssence;
    j["filterMagnetiteShards"] = g_Settings.filterMagnetiteShards;
    j["filterGaetingCrystals"] = g_Settings.filterGaetingCrystals;
    j["filterProphetShards"] = g_Settings.filterProphetShards;
    j["filterGreenProphetShards"] = g_Settings.filterGreenProphetShards;
    j["filterWvWSkirmishTickets"] = g_Settings.filterWvWSkirmishTickets;
    j["filterProofsOfHeroics"] = g_Settings.filterProofsOfHeroics;
    j["filterPvpLeagueTickets"] = g_Settings.filterPvpLeagueTickets;
    j["filterAscendedShardsOfGlory"] = g_Settings.filterAscendedShardsOfGlory;
    j["filterResearchNotes"] = g_Settings.filterResearchNotes;
    j["filterTyrianDefenseSeal"] = g_Settings.filterTyrianDefenseSeal;
    j["filterTestimonyOfDesertHeroics"] = g_Settings.filterTestimonyOfDesertHeroics;
    j["filterTestimonyOfJadeHeroics"] = g_Settings.filterTestimonyOfJadeHeroics;
    j["filterTestimonyOfCastoranHeroics"] = g_Settings.filterTestimonyOfCastoranHeroics;
    j["filterLegendaryInsight"] = g_Settings.filterLegendaryInsight;
    j["filterTalesOfDungeonDelving"] = g_Settings.filterTalesOfDungeonDelving;
    j["filterImperialFavor"] = g_Settings.filterImperialFavor;
    j["filterCanachCoins"] = g_Settings.filterCanachCoins;
    j["filterAncientCoin"] = g_Settings.filterAncientCoin;
    j["filterUnusualCoin"] = g_Settings.filterUnusualCoin;
    j["filterJadeSliver"] = g_Settings.filterJadeSliver;
    j["filterStaticCharge"] = g_Settings.filterStaticCharge;
    j["filterPinchOfStardust"] = g_Settings.filterPinchOfStardust;
    j["filterCalcifiedGasp"] = g_Settings.filterCalcifiedGasp;
    j["filterUrsusOblige"] = g_Settings.filterUrsusOblige;
    j["filterGaetingCrystalJanthir"] = g_Settings.filterGaetingCrystalJanthir;
    j["filterAntiquatedDucat"] = g_Settings.filterAntiquatedDucat;
    j["filterAetherRichSap"] = g_Settings.filterAetherRichSap;

    return j;
}

void SettingsManager::ResetToDefaults()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    g_Settings = Settings();
    Save();
}

Settings SettingsManager::GetSnapshot()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    return g_Settings;
}

void SettingsManager::CreateProfile(const std::string& name)
{
    if (name.empty()) return;

    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    
    Settings::SettingsProfile profile;
    profile.name = name;
    
    // Serialize current settings to JSON string for this profile
    json j = SettingsManager::ToSettingsJson(g_Settings);
    profile.profileData = j.dump();
    
    g_Settings.settingsProfiles.push_back(profile);
    g_Settings.currentProfileIndex = static_cast<int>(g_Settings.settingsProfiles.size()) - 1;
    
    Save();
}

void SettingsManager::ApplyProfile(int index)
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    
    if (index < 0 || index >= static_cast<int>(g_Settings.settingsProfiles.size()))
        return;
        
    try {
        json j = json::parse(g_Settings.settingsProfiles[index].profileData);
        SettingsManager::FromSettingsJson(j, g_Settings);
        g_Settings.currentProfileIndex = index;
        Save();
    } catch (...) {}
}

void SettingsManager::UpdateProfile(int index)
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    
    if (index < 0 || index >= static_cast<int>(g_Settings.settingsProfiles.size()))
        return;
        
    json j = SettingsManager::ToSettingsJson(g_Settings);
    g_Settings.settingsProfiles[index].profileData = j.dump();
    
    Save();
}

void SettingsManager::DeleteProfile(int index)
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    
    if (index < 0 || index >= static_cast<int>(g_Settings.settingsProfiles.size()))
        return;
        
    g_Settings.settingsProfiles.erase(g_Settings.settingsProfiles.begin() + index);
    
    if (g_Settings.currentProfileIndex == index)
        g_Settings.currentProfileIndex = -1;
    else if (g_Settings.currentProfileIndex > index)
        g_Settings.currentProfileIndex--;
        
    Save();
}

int SettingsManager::GetProfileCount()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    return static_cast<int>(g_Settings.settingsProfiles.size());
}

std::string SettingsManager::GetCurrentDrfToken()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    if (g_Settings.currentAccountIndex >= 0 && g_Settings.currentAccountIndex < static_cast<int>(g_Settings.accounts.size()))
    {
        return g_Settings.accounts[g_Settings.currentAccountIndex].drfToken;
    }
    return g_Settings.drfToken; // Legacy fallback
}

std::string SettingsManager::GetCurrentGw2ApiKey()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    if (g_Settings.currentAccountIndex >= 0 && g_Settings.currentAccountIndex < static_cast<int>(g_Settings.accounts.size()))
    {
        return g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey;
    }
    return g_Settings.gw2ApiKey; // Legacy fallback
}
