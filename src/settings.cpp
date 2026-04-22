#include "settings.h"
#include "shared.h"
#include "../include/nlohmann/json.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "crypt32.lib")

#include <fstream>
#include <regex>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vector>

using json = nlohmann::json;

Settings g_Settings;
static std::string s_SettingsPath;

// Secure encryption using Windows CryptProtectData
static std::string SimpleEncrypt(const std::string& data, const std::string&)
{
    if (data.empty()) return data;

    DATA_BLOB inBlob, outBlob;
    inBlob.pbData = (BYTE*)data.c_str();
    inBlob.cbData = static_cast<DWORD>(data.size());

    if (!CryptProtectData(&inBlob, L"FarmingTracker", NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &outBlob))
    {
        return data; // Fallback to plaintext if encryption fails
    }

    std::string encrypted((char*)outBlob.pbData, outBlob.cbData);
    LocalFree(outBlob.pbData);
    return encrypted;
}

static std::string SimpleDecrypt(const std::string& encrypted, const std::string&)
{
    if (encrypted.empty()) return encrypted;

    DATA_BLOB inBlob, outBlob;
    inBlob.pbData = (BYTE*)encrypted.c_str();
    inBlob.cbData = static_cast<DWORD>(encrypted.size());

    if (!CryptUnprotectData(&inBlob, NULL, NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &outBlob))
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

static void ClampSettings()
{
    g_Settings.automaticResetMode = std::clamp(g_Settings.automaticResetMode, 0, 7);
    g_Settings.minutesUntilResetAfterShutdown =
        std::clamp(g_Settings.minutesUntilResetAfterShutdown, 1, 24 * 60);
    g_Settings.iconSize = std::clamp(g_Settings.iconSize, 16, 128);
    g_Settings.gridIconSize = std::clamp(g_Settings.gridIconSize, 16, 128);
    g_Settings.itemSortMode = std::clamp(g_Settings.itemSortMode, 0, 4);
    g_Settings.itemRarityFilterMin = std::clamp(g_Settings.itemRarityFilterMin, 0, 7);

    // Clamp other settings
    g_Settings.countFontSize = std::clamp(g_Settings.countFontSize, 10, 40);
    g_Settings.countHorizontalAlignment = std::clamp(g_Settings.countHorizontalAlignment, 0, 2);
    g_Settings.profitWindowDisplayMode = std::clamp(g_Settings.profitWindowDisplayMode, 0, 2);
    
    // Clamp API timeouts (1 second to 2 minutes)
    g_Settings.gw2ApiConnectTimeout = std::clamp(g_Settings.gw2ApiConnectTimeout, 1000, 120000);
    g_Settings.gw2ApiReceiveTimeout = std::clamp(g_Settings.gw2ApiReceiveTimeout, 1000, 120000);
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

        // Load language
        if (j.contains("language")) g_Settings.language = j["language"].get<std::string>();

        // Load multi-account system
        if (j.contains("accounts") && j["accounts"].is_array())
        {
            g_Settings.accounts.clear();
            for (const auto& accJson : j["accounts"])
            {
                Account acc;
                if (accJson.contains("name")) acc.name = accJson["name"].get<std::string>();
                if (accJson.contains("drfToken")) {
                    std::string encryptedToken = accJson["drfToken"].get<std::string>();
                    acc.drfToken = SimpleDecrypt(FromHexString(encryptedToken), "FarmingTracker2024");
                }
                if (accJson.contains("gw2ApiKey")) {
                    std::string encryptedKey = accJson["gw2ApiKey"].get<std::string>();
                    acc.gw2ApiKey = SimpleDecrypt(FromHexString(encryptedKey), "FarmingTracker2024");
                }
                g_Settings.accounts.push_back(acc);
            }
        }
        if (j.contains("currentAccountIndex")) g_Settings.currentAccountIndex = j["currentAccountIndex"].get<int>();

        // Backwards compatibility: if no accounts exist but legacy tokens do, migrate to account system
        if (g_Settings.accounts.empty())
        {
            Account defaultAccount;
            defaultAccount.name = "Default";
            if (j.contains("drfToken")) {
                std::string encryptedToken = j["drfToken"].get<std::string>();
                defaultAccount.drfToken = SimpleDecrypt(FromHexString(encryptedToken), "FarmingTracker2024");
            }
            if (j.contains("gw2ApiKey")) {
                std::string encryptedKey = j["gw2ApiKey"].get<std::string>();
                defaultAccount.gw2ApiKey = SimpleDecrypt(FromHexString(encryptedKey), "FarmingTracker2024");
            }
            g_Settings.accounts.push_back(defaultAccount);
            g_Settings.currentAccountIndex = 0;
        }

        // Load encrypted tokens (legacy, for backwards compatibility)
        if (j.contains("drfToken")) {
            std::string encryptedToken = j["drfToken"].get<std::string>();
            g_Settings.drfToken = SimpleDecrypt(FromHexString(encryptedToken), "FarmingTracker2024");
        }
        if (j.contains("automaticResetMode")) g_Settings.automaticResetMode = j["automaticResetMode"].get<int>();
        if (j.contains("minutesUntilReset"))  g_Settings.minutesUntilResetAfterShutdown = j["minutesUntilReset"].get<int>();
        if (j.contains("gw2ApiKey")) {
            std::string encryptedKey = j["gw2ApiKey"].get<std::string>();
            g_Settings.gw2ApiKey = SimpleDecrypt(FromHexString(encryptedKey), "FarmingTracker2024");
        }
        if (j.contains("toggleHotkey")) g_Settings.toggleHotkey = j["toggleHotkey"].get<std::string>();
        if (j.contains("miniWindowToggleHotkey")) g_Settings.miniWindowToggleHotkey = j["miniWindowToggleHotkey"].get<std::string>();
        if (j.contains("nextResetDateTimeUtc")) g_Settings.nextResetDateTimeUtc = j["nextResetDateTimeUtc"].get<std::string>();
        if (j.contains("showMiniWindow")) g_Settings.showMiniWindow = j["showMiniWindow"].get<bool>();
        if (j.contains("miniWindowShowProfit")) g_Settings.miniWindowShowProfit = j["miniWindowShowProfit"].get<bool>();
        if (j.contains("miniWindowShowProfitPerHour")) g_Settings.miniWindowShowProfitPerHour = j["miniWindowShowProfitPerHour"].get<bool>();
        if (j.contains("miniWindowShowTradingProfitSell")) g_Settings.miniWindowShowTradingProfitSell = j["miniWindowShowTradingProfitSell"].get<bool>();
        if (j.contains("miniWindowShowTradingProfitInstant")) g_Settings.miniWindowShowTradingProfitInstant = j["miniWindowShowTradingProfitInstant"].get<bool>();
        if (j.contains("miniWindowShowTotalItems")) g_Settings.miniWindowShowTotalItems = j["miniWindowShowTotalItems"].get<bool>();
        if (j.contains("miniWindowShowSessionDuration")) g_Settings.miniWindowShowSessionDuration = j["miniWindowShowSessionDuration"].get<bool>();
        if (j.contains("miniWindowClickThrough")) g_Settings.miniWindowClickThrough = j["miniWindowClickThrough"].get<bool>();
        if (j.contains("mainWindowClickThrough")) g_Settings.mainWindowClickThrough = j["mainWindowClickThrough"].get<bool>();
        if (j.contains("miniWindowPosX")) g_Settings.miniWindowPosX = j["miniWindowPosX"].get<float>();
        if (j.contains("miniWindowPosY")) g_Settings.miniWindowPosY = j["miniWindowPosY"].get<float>();
        if (j.contains("miniWindowWidth")) g_Settings.miniWindowWidth = j["miniWindowWidth"].get<float>();
        if (j.contains("miniWindowHeight")) g_Settings.miniWindowHeight = j["miniWindowHeight"].get<float>();
        if (j.contains("mainWindowPosX")) g_Settings.mainWindowPosX = j["mainWindowPosX"].get<float>();
        if (j.contains("mainWindowPosY")) g_Settings.mainWindowPosY = j["mainWindowPosY"].get<float>();
        if (j.contains("mainWindowWidth")) g_Settings.mainWindowWidth = j["mainWindowWidth"].get<float>();
        if (j.contains("mainWindowHeight")) g_Settings.mainWindowHeight = j["mainWindowHeight"].get<float>();
        if (j.contains("activeTab")) g_Settings.activeTab = j["activeTab"].get<int>();
        if (j.contains("enableGradientBackgrounds")) g_Settings.enableGradientBackgrounds = j["enableGradientBackgrounds"].get<bool>();
        if (j.contains("gradientTopColor"))
        {
            auto arr = j["gradientTopColor"];
            if (arr.is_array() && arr.size() == 4)
            {
                for (int i = 0; i < 4; i++)
                    g_Settings.gradientTopColor[i] = arr[i].get<float>();
            }
        }
        if (j.contains("gradientBottomColor"))
        {
            auto arr = j["gradientBottomColor"];
            if (arr.is_array() && arr.size() == 4)
            {
                for (int i = 0; i < 4; i++)
                    g_Settings.gradientBottomColor[i] = arr[i].get<float>();
            }
        }
        if (j.contains("iconSize"))           g_Settings.iconSize           = j["iconSize"].get<int>();
        if (j.contains("gridIconSize"))       g_Settings.gridIconSize       = j["gridIconSize"].get<int>();
        if (j.contains("showRarityBorder"))   g_Settings.showRarityBorder   = j["showRarityBorder"].get<bool>();
        if (j.contains("itemSortMode"))       g_Settings.itemSortMode       = j["itemSortMode"].get<int>();
        if (j.contains("itemRarityFilterMin")) g_Settings.itemRarityFilterMin = j["itemRarityFilterMin"].get<int>();
        if (j.contains("showItemIcons"))      g_Settings.showItemIcons      = j["showItemIcons"].get<bool>();
        if (j.contains("lastResetTimestamp")) g_Settings.lastResetTimestamp = j["lastResetTimestamp"].get<std::string>();
        
        // === Advanced Settings ===
        // Extended Filtering
        if (j.contains("filterSellableToVendor")) g_Settings.filterSellableToVendor = j["filterSellableToVendor"].get<bool>();
        if (j.contains("filterSellableOnTp")) g_Settings.filterSellableOnTp = j["filterSellableOnTp"].get<bool>();
        if (j.contains("filterCustomProfit")) g_Settings.filterCustomProfit = j["filterCustomProfit"].get<bool>();
        if (j.contains("filterKnownByApi")) g_Settings.filterKnownByApi = j["filterKnownByApi"].get<bool>();
        if (j.contains("filterUnknownByApi")) g_Settings.filterUnknownByApi = j["filterUnknownByApi"].get<bool>();

        // Additional Filters
        if (j.contains("filterAccountBound")) g_Settings.filterAccountBound = j["filterAccountBound"].get<bool>();
        if (j.contains("filterNotAccountBound")) g_Settings.filterNotAccountBound = j["filterNotAccountBound"].get<bool>();
        if (j.contains("filterNoSell")) g_Settings.filterNoSell = j["filterNoSell"].get<bool>();
        if (j.contains("filterNotNoSell")) g_Settings.filterNotNoSell = j["filterNotNoSell"].get<bool>();
        if (j.contains("filterFavorite")) g_Settings.filterFavorite = j["filterFavorite"].get<bool>();
        if (j.contains("filterNotFavorite")) g_Settings.filterNotFavorite = j["filterNotFavorite"].get<bool>();
        if (j.contains("filterIgnored")) g_Settings.filterIgnored = j["filterIgnored"].get<bool>();
        if (j.contains("filterNotIgnored")) g_Settings.filterNotIgnored = j["filterNotIgnored"].get<bool>();

        // Range Filters
        if (j.contains("showRangeFilters")) g_Settings.showRangeFilters = j["showRangeFilters"].get<bool>();
        if (j.contains("filterMinPrice")) g_Settings.filterMinPrice = j["filterMinPrice"].get<int>();
        if (j.contains("filterMaxPrice")) g_Settings.filterMaxPrice = j["filterMaxPrice"].get<int>();
        if (j.contains("filterMinQuantity")) g_Settings.filterMinQuantity = j["filterMinQuantity"].get<int>();
        if (j.contains("filterMaxQuantity")) g_Settings.filterMaxQuantity = j["filterMaxQuantity"].get<int>();

        // Item Type Filters
        if (j.contains("filterTypeArmor")) g_Settings.filterTypeArmor = j["filterTypeArmor"].get<bool>();
        if (j.contains("filterTypeWeapon")) g_Settings.filterTypeWeapon = j["filterTypeWeapon"].get<bool>();
        if (j.contains("filterTypeTrinket")) g_Settings.filterTypeTrinket = j["filterTypeTrinket"].get<bool>();
        if (j.contains("filterTypeGizmo")) g_Settings.filterTypeGizmo = j["filterTypeGizmo"].get<bool>();
        if (j.contains("filterTypeCraftingMaterial")) g_Settings.filterTypeCraftingMaterial = j["filterTypeCraftingMaterial"].get<bool>();
        if (j.contains("filterTypeConsumable")) g_Settings.filterTypeConsumable = j["filterTypeConsumable"].get<bool>();
        if (j.contains("filterTypeGatheringTool")) g_Settings.filterTypeGatheringTool = j["filterTypeGatheringTool"].get<bool>();
        if (j.contains("filterTypeBag")) g_Settings.filterTypeBag = j["filterTypeBag"].get<bool>();
        if (j.contains("filterTypeContainer")) g_Settings.filterTypeContainer = j["filterTypeContainer"].get<bool>();
        if (j.contains("filterTypeMiniPet")) g_Settings.filterTypeMiniPet = j["filterTypeMiniPet"].get<bool>();
        if (j.contains("filterTypeGizmoContainer")) g_Settings.filterTypeGizmoContainer = j["filterTypeGizmoContainer"].get<bool>();
        if (j.contains("filterTypeBackpack")) g_Settings.filterTypeBackpack = j["filterTypeBackpack"].get<bool>();
        if (j.contains("filterTypeUpgradeComponent")) g_Settings.filterTypeUpgradeComponent = j["filterTypeUpgradeComponent"].get<bool>();
        if (j.contains("filterTypeTool")) g_Settings.filterTypeTool = j["filterTypeTool"].get<bool>();
        if (j.contains("filterTypeTrophy")) g_Settings.filterTypeTrophy = j["filterTypeTrophy"].get<bool>();
        if (j.contains("filterTypeUnlock")) g_Settings.filterTypeUnlock = j["filterTypeUnlock"].get<bool>();
        
        // Currency Filters
        if (j.contains("filterKarma")) g_Settings.filterKarma = j["filterKarma"].get<bool>();
        if (j.contains("filterLaurel")) g_Settings.filterLaurel = j["filterLaurel"].get<bool>();
        if (j.contains("filterGem")) g_Settings.filterGem = j["filterGem"].get<bool>();
        if (j.contains("filterFractalRelic")) g_Settings.filterFractalRelic = j["filterFractalRelic"].get<bool>();
        if (j.contains("filterBadgeOfHonor")) g_Settings.filterBadgeOfHonor = j["filterBadgeOfHonor"].get<bool>();
        if (j.contains("filterGuildCommendation")) g_Settings.filterGuildCommendation = j["filterGuildCommendation"].get<bool>();
        if (j.contains("filterTransmutationCharge")) g_Settings.filterTransmutationCharge = j["filterTransmutationCharge"].get<bool>();
        if (j.contains("filterSpiritShards")) g_Settings.filterSpiritShards = j["filterSpiritShards"].get<bool>();
        if (j.contains("filterUnboundMagic")) g_Settings.filterUnboundMagic = j["filterUnboundMagic"].get<bool>();
        if (j.contains("filterVolatileMagic")) g_Settings.filterVolatileMagic = j["filterVolatileMagic"].get<bool>();
        if (j.contains("filterAirshipParts")) g_Settings.filterAirshipParts = j["filterAirshipParts"].get<bool>();
        if (j.contains("filterGeode")) g_Settings.filterGeode = j["filterGeode"].get<bool>();
        if (j.contains("filterLeyLineCrystals")) g_Settings.filterLeyLineCrystals = j["filterLeyLineCrystals"].get<bool>();
        if (j.contains("filterTradeContracts")) g_Settings.filterTradeContracts = j["filterTradeContracts"].get<bool>();
        if (j.contains("filterElegyMosaic")) g_Settings.filterElegyMosaic = j["filterElegyMosaic"].get<bool>();
        if (j.contains("filterUncommonCoins")) g_Settings.filterUncommonCoins = j["filterUncommonCoins"].get<bool>();
        if (j.contains("filterAstralAcclaim")) g_Settings.filterAstralAcclaim = j["filterAstralAcclaim"].get<bool>();
        if (j.contains("filterPristineFractalRelics")) g_Settings.filterPristineFractalRelics = j["filterPristineFractalRelics"].get<bool>();
        if (j.contains("filterUnstableFractalEssence")) g_Settings.filterUnstableFractalEssence = j["filterUnstableFractalEssence"].get<bool>();
        if (j.contains("filterMagnetiteShards")) g_Settings.filterMagnetiteShards = j["filterMagnetiteShards"].get<bool>();
        if (j.contains("filterGaetingCrystals")) g_Settings.filterGaetingCrystals = j["filterGaetingCrystals"].get<bool>();
        if (j.contains("filterProphetShards")) g_Settings.filterProphetShards = j["filterProphetShards"].get<bool>();
        if (j.contains("filterGreenProphetShards")) g_Settings.filterGreenProphetShards = j["filterGreenProphetShards"].get<bool>();
        if (j.contains("filterWvWSkirmishTickets")) g_Settings.filterWvWSkirmishTickets = j["filterWvWSkirmishTickets"].get<bool>();
        if (j.contains("filterProofsOfHeroics")) g_Settings.filterProofsOfHeroics = j["filterProofsOfHeroics"].get<bool>();
        if (j.contains("filterPvpLeagueTickets")) g_Settings.filterPvpLeagueTickets = j["filterPvpLeagueTickets"].get<bool>();
        if (j.contains("filterAscendedShardsOfGlory")) g_Settings.filterAscendedShardsOfGlory = j["filterAscendedShardsOfGlory"].get<bool>();
        if (j.contains("filterResearchNotes")) g_Settings.filterResearchNotes = j["filterResearchNotes"].get<bool>();
        if (j.contains("filterTyrianDefenseSeal")) g_Settings.filterTyrianDefenseSeal = j["filterTyrianDefenseSeal"].get<bool>();
        if (j.contains("filterTestimonyOfDesertHeroics")) g_Settings.filterTestimonyOfDesertHeroics = j["filterTestimonyOfDesertHeroics"].get<bool>();
        if (j.contains("filterTestimonyOfJadeHeroics")) g_Settings.filterTestimonyOfJadeHeroics = j["filterTestimonyOfJadeHeroics"].get<bool>();
        if (j.contains("filterTestimonyOfCastoranHeroics")) g_Settings.filterTestimonyOfCastoranHeroics = j["filterTestimonyOfCastoranHeroics"].get<bool>();
        if (j.contains("filterLegendaryInsight")) g_Settings.filterLegendaryInsight = j["filterLegendaryInsight"].get<bool>();
        if (j.contains("filterTalesOfDungeonDelving")) g_Settings.filterTalesOfDungeonDelving = j["filterTalesOfDungeonDelving"].get<bool>();
        if (j.contains("filterImperialFavor")) g_Settings.filterImperialFavor = j["filterImperialFavor"].get<bool>();
        if (j.contains("filterCanachCoins")) g_Settings.filterCanachCoins = j["filterCanachCoins"].get<bool>();
        if (j.contains("filterAncientCoin")) g_Settings.filterAncientCoin = j["filterAncientCoin"].get<bool>();
        if (j.contains("filterUnusualCoin")) g_Settings.filterUnusualCoin = j["filterUnusualCoin"].get<bool>();
        if (j.contains("filterJadeSliver")) g_Settings.filterJadeSliver = j["filterJadeSliver"].get<bool>();
        if (j.contains("filterStaticCharge")) g_Settings.filterStaticCharge = j["filterStaticCharge"].get<bool>();
        if (j.contains("filterPinchOfStardust")) g_Settings.filterPinchOfStardust = j["filterPinchOfStardust"].get<bool>();
        if (j.contains("filterCalcifiedGasp")) g_Settings.filterCalcifiedGasp = j["filterCalcifiedGasp"].get<bool>();
        if (j.contains("filterUrsusOblige")) g_Settings.filterUrsusOblige = j["filterUrsusOblige"].get<bool>();
        if (j.contains("filterGaetingCrystalJanthir")) g_Settings.filterGaetingCrystalJanthir = j["filterGaetingCrystalJanthir"].get<bool>();
        if (j.contains("filterAntiquatedDucat")) g_Settings.filterAntiquatedDucat = j["filterAntiquatedDucat"].get<bool>();
        if (j.contains("filterAetherRichSap")) g_Settings.filterAetherRichSap = j["filterAetherRichSap"].get<bool>();

        // Custom Profit System
        if (j.contains("enableCustomProfit")) g_Settings.enableCustomProfit = j["enableCustomProfit"].get<bool>();
        
        // Search
        if (j.contains("enableSearch")) g_Settings.enableSearch = j["enableSearch"].get<bool>();
        if (j.contains("searchTerm")) g_Settings.searchTerm = j["searchTerm"].get<std::string>();
        
        // Ignored Items
        if (j.contains("enableIgnoredItems")) g_Settings.enableIgnoredItems = j["enableIgnoredItems"].get<bool>();

        // Advanced UI Settings
        if (j.contains("showAdvancedSettings")) g_Settings.showAdvancedSettings = j["showAdvancedSettings"].get<bool>();

        // Count Display Settings
        if (j.contains("countTextColor")) g_Settings.countTextColor = j["countTextColor"].get<int>();
        if (j.contains("countBackgroundColor")) g_Settings.countBackgroundColor = j["countBackgroundColor"].get<int>();
        if (j.contains("countFontSize")) g_Settings.countFontSize = j["countFontSize"].get<int>();
        if (j.contains("countHorizontalAlignment")) g_Settings.countHorizontalAlignment = j["countHorizontalAlignment"].get<int>();
        
        // Profit Labels
        if (j.contains("profitLabelText")) g_Settings.profitLabelText = j["profitLabelText"].get<std::string>();
        if (j.contains("profitPerHourLabelText")) g_Settings.profitPerHourLabelText = j["profitPerHourLabelText"].get<std::string>();
        if (j.contains("profitWindowDisplayMode")) g_Settings.profitWindowDisplayMode = j["profitWindowDisplayMode"].get<int>();

        // Favorites System
        if (j.contains("enableFavorites")) g_Settings.enableFavorites = j["enableFavorites"].get<bool>();

        // Grid View Settings
        if (j.contains("enableGridView")) g_Settings.enableGridView = j["enableGridView"].get<bool>();

        // UI State Settings
        if (j.contains("showTopItems")) g_Settings.showTopItems = j["showTopItems"].get<bool>();
        if (j.contains("showTopCurrencies")) g_Settings.showTopCurrencies = j["showTopCurrencies"].get<bool>();
        
        // Debug Features
        if (j.contains("enableDebugTab")) g_Settings.enableDebugTab = j["enableDebugTab"].get<bool>();
        if (j.contains("useFakeDrfServer")) g_Settings.useFakeDrfServer = j["useFakeDrfServer"].get<bool>();

        // API Timeouts
        if (j.contains("gw2ApiConnectTimeout")) g_Settings.gw2ApiConnectTimeout = j["gw2ApiConnectTimeout"].get<int>();
        if (j.contains("gw2ApiReceiveTimeout")) g_Settings.gw2ApiReceiveTimeout = j["gw2ApiReceiveTimeout"].get<int>();

        // Salvage Kits Settings
        if (j.contains("salvageKitSettings") && j["salvageKitSettings"].is_object())
        {
            for (auto& [idStr, kitSetting] : j["salvageKitSettings"].items())
            {
                try
                {
                    int id = std::stoi(idStr);
                    Settings::SalvageKitSetting setting;
                    if (kitSetting.contains("enabled")) setting.enabled = kitSetting["enabled"].get<bool>();
                    if (kitSetting.contains("useKarma")) setting.useKarma = kitSetting["useKarma"].get<bool>();
                    g_Settings.salvageKitSettings[id] = setting;
                }
                catch (...)
                {
                    // Invalid item ID, skip this entry
                    if (APIDefs)
                        APIDefs->Log(LOGL_WARNING, "FarmingTracker", ("Invalid salvage kit ID in settings: " + idStr).c_str());
                }
            }
        }
    }
    catch (...)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "Failed to parse settings.json");
    }

    ClampSettings();
}

void SettingsManager::Save()
{
    if (s_SettingsPath.empty())
        return;

    if (APIDefs)
        APIDefs->Log(LOGL_INFO, "FarmingTracker", ("Saving settings to: " + s_SettingsPath).c_str());

    ClampSettings();

    json j;
    // Save language
    j["language"]              = g_Settings.language;

    // Save multi-account system
    j["accounts"] = nlohmann::json::array();
    for (const auto& acc : g_Settings.accounts)
    {
        nlohmann::json accJson;
        accJson["name"] = acc.name;
        accJson["drfToken"] = ToHexString(SimpleEncrypt(acc.drfToken, "FarmingTracker2024"));
        accJson["gw2ApiKey"] = ToHexString(SimpleEncrypt(acc.gw2ApiKey, "FarmingTracker2024"));
        j["accounts"].push_back(accJson);
    }
    j["currentAccountIndex"] = g_Settings.currentAccountIndex;

    // Save encrypted tokens (legacy, for backwards compatibility)
    j["drfToken"]              = ToHexString(SimpleEncrypt(g_Settings.drfToken, "FarmingTracker2024"));
    j["automaticResetMode"]    = g_Settings.automaticResetMode;
    j["minutesUntilReset"]     = g_Settings.minutesUntilResetAfterShutdown;
    j["gw2ApiKey"]             = ToHexString(SimpleEncrypt(g_Settings.gw2ApiKey, "FarmingTracker2024"));
    j["toggleHotkey"]           = g_Settings.toggleHotkey;
    j["miniWindowToggleHotkey"] = g_Settings.miniWindowToggleHotkey;
    j["nextResetDateTimeUtc"]  = g_Settings.nextResetDateTimeUtc;
    j["showMiniWindow"]        = g_Settings.showMiniWindow;
    j["miniWindowShowProfit"]  = g_Settings.miniWindowShowProfit;
    j["miniWindowShowProfitPerHour"] = g_Settings.miniWindowShowProfitPerHour;
    j["miniWindowShowTradingProfitSell"] = g_Settings.miniWindowShowTradingProfitSell;
    j["miniWindowShowTradingProfitInstant"] = g_Settings.miniWindowShowTradingProfitInstant;
    j["miniWindowShowTotalItems"] = g_Settings.miniWindowShowTotalItems;
    j["miniWindowShowSessionDuration"] = g_Settings.miniWindowShowSessionDuration;
    j["miniWindowClickThrough"] = g_Settings.miniWindowClickThrough;
    j["mainWindowClickThrough"] = g_Settings.mainWindowClickThrough;
    j["miniWindowPosX"]         = g_Settings.miniWindowPosX;
    j["miniWindowPosY"]         = g_Settings.miniWindowPosY;
    j["miniWindowWidth"]        = g_Settings.miniWindowWidth;
    j["miniWindowHeight"]       = g_Settings.miniWindowHeight;
    j["mainWindowPosX"]         = g_Settings.mainWindowPosX;
    j["mainWindowPosY"]         = g_Settings.mainWindowPosY;
    j["mainWindowWidth"]        = g_Settings.mainWindowWidth;
    j["mainWindowHeight"]       = g_Settings.mainWindowHeight;
    j["activeTab"]              = g_Settings.activeTab;
    j["enableGradientBackgrounds"] = g_Settings.enableGradientBackgrounds;
    j["gradientTopColor"] = nlohmann::json::array();
    for (int i = 0; i < 4; i++)
        j["gradientTopColor"].push_back(g_Settings.gradientTopColor[i]);
    j["gradientBottomColor"] = nlohmann::json::array();
    for (int i = 0; i < 4; i++)
        j["gradientBottomColor"].push_back(g_Settings.gradientBottomColor[i]);
    j["iconSize"]              = g_Settings.iconSize;
    j["gridIconSize"]          = g_Settings.gridIconSize;
    j["showRarityBorder"]      = g_Settings.showRarityBorder;
    j["itemSortMode"]          = g_Settings.itemSortMode;
    j["itemRarityFilterMin"]   = g_Settings.itemRarityFilterMin;
    j["showItemIcons"]         = g_Settings.showItemIcons;
    j["lastResetTimestamp"]    = g_Settings.lastResetTimestamp;
    
    // === Advanced Settings ===
    // Extended Filtering
    j["filterSellableToVendor"] = g_Settings.filterSellableToVendor;
    j["filterSellableOnTp"]    = g_Settings.filterSellableOnTp;
    j["filterCustomProfit"]    = g_Settings.filterCustomProfit;
    j["filterKnownByApi"]      = g_Settings.filterKnownByApi;
    j["filterUnknownByApi"]    = g_Settings.filterUnknownByApi;

    // Additional Filters
    j["filterAccountBound"]    = g_Settings.filterAccountBound;
    j["filterNotAccountBound"] = g_Settings.filterNotAccountBound;
    j["filterNoSell"]          = g_Settings.filterNoSell;
    j["filterNotNoSell"]       = g_Settings.filterNotNoSell;
    j["filterFavorite"]        = g_Settings.filterFavorite;
    j["filterNotFavorite"]     = g_Settings.filterNotFavorite;
    j["filterIgnored"]         = g_Settings.filterIgnored;
    j["filterNotIgnored"]      = g_Settings.filterNotIgnored;

    // Range Filters
    j["showRangeFilters"]      = g_Settings.showRangeFilters;
    j["filterMinPrice"]        = g_Settings.filterMinPrice;
    j["filterMaxPrice"]        = g_Settings.filterMaxPrice;
    j["filterMinQuantity"]     = g_Settings.filterMinQuantity;
    j["filterMaxQuantity"]     = g_Settings.filterMaxQuantity;

    // Item Type Filters
    j["filterTypeArmor"]       = g_Settings.filterTypeArmor;
    j["filterTypeWeapon"]      = g_Settings.filterTypeWeapon;
    j["filterTypeTrinket"]     = g_Settings.filterTypeTrinket;
    j["filterTypeGizmo"]       = g_Settings.filterTypeGizmo;
    j["filterTypeCraftingMaterial"] = g_Settings.filterTypeCraftingMaterial;
    j["filterTypeConsumable"]  = g_Settings.filterTypeConsumable;
    j["filterTypeGatheringTool"] = g_Settings.filterTypeGatheringTool;
    j["filterTypeBag"]         = g_Settings.filterTypeBag;
    j["filterTypeContainer"]   = g_Settings.filterTypeContainer;
    j["filterTypeMiniPet"]     = g_Settings.filterTypeMiniPet;
    j["filterTypeGizmoContainer"] = g_Settings.filterTypeGizmoContainer;
    j["filterTypeBackpack"]    = g_Settings.filterTypeBackpack;
    j["filterTypeUpgradeComponent"] = g_Settings.filterTypeUpgradeComponent;
    j["filterTypeTool"]        = g_Settings.filterTypeTool;
    j["filterTypeTrophy"]      = g_Settings.filterTypeTrophy;
    j["filterTypeUnlock"]      = g_Settings.filterTypeUnlock;
    
    // Currency Filters
    j["filterKarma"]           = g_Settings.filterKarma;
    j["filterLaurel"]           = g_Settings.filterLaurel;
    j["filterGem"]             = g_Settings.filterGem;
    j["filterFractalRelic"]    = g_Settings.filterFractalRelic;
    j["filterBadgeOfHonor"]    = g_Settings.filterBadgeOfHonor;
    j["filterGuildCommendation"] = g_Settings.filterGuildCommendation;
    j["filterTransmutationCharge"] = g_Settings.filterTransmutationCharge;
    j["filterSpiritShards"]    = g_Settings.filterSpiritShards;
    j["filterUnboundMagic"]    = g_Settings.filterUnboundMagic;
    j["filterVolatileMagic"]   = g_Settings.filterVolatileMagic;
    j["filterAirshipParts"]    = g_Settings.filterAirshipParts;
    j["filterGeode"]           = g_Settings.filterGeode;
    j["filterLeyLineCrystals"] = g_Settings.filterLeyLineCrystals;
    j["filterTradeContracts"]  = g_Settings.filterTradeContracts;
    j["filterElegyMosaic"]     = g_Settings.filterElegyMosaic;
    j["filterUncommonCoins"]   = g_Settings.filterUncommonCoins;
    j["filterAstralAcclaim"]   = g_Settings.filterAstralAcclaim;
    j["filterPristineFractalRelics"] = g_Settings.filterPristineFractalRelics;
    j["filterUnstableFractalEssence"] = g_Settings.filterUnstableFractalEssence;
    j["filterMagnetiteShards"] = g_Settings.filterMagnetiteShards;
    j["filterGaetingCrystals"] = g_Settings.filterGaetingCrystals;
    j["filterProphetShards"]   = g_Settings.filterProphetShards;
    j["filterGreenProphetShards"] = g_Settings.filterGreenProphetShards;
    j["filterWvWSkirmishTickets"] = g_Settings.filterWvWSkirmishTickets;
    j["filterProofsOfHeroics"] = g_Settings.filterProofsOfHeroics;
    j["filterPvpLeagueTickets"] = g_Settings.filterPvpLeagueTickets;
    j["filterAscendedShardsOfGlory"] = g_Settings.filterAscendedShardsOfGlory;
    j["filterResearchNotes"]   = g_Settings.filterResearchNotes;
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

    // Custom Profit System
    j["enableCustomProfit"]    = g_Settings.enableCustomProfit;
    
    // Search
    j["enableSearch"]          = g_Settings.enableSearch;
    j["searchTerm"]            = g_Settings.searchTerm;
    
    // Ignored Items
    j["enableIgnoredItems"]    = g_Settings.enableIgnoredItems;

    // Advanced UI Settings
    j["showAdvancedSettings"]  = g_Settings.showAdvancedSettings;

    // Count Display Settings
    j["countTextColor"]        = g_Settings.countTextColor;
    j["countBackgroundColor"]  = g_Settings.countBackgroundColor;
    j["countFontSize"]         = g_Settings.countFontSize;
    j["countHorizontalAlignment"] = g_Settings.countHorizontalAlignment;
    
    // Profit Labels
    j["profitLabelText"]       = g_Settings.profitLabelText;
    j["profitPerHourLabelText"] = g_Settings.profitPerHourLabelText;
    j["profitWindowDisplayMode"] = g_Settings.profitWindowDisplayMode;

    // Favorites System
    j["enableFavorites"]       = g_Settings.enableFavorites;

    // Grid View Settings
    j["enableGridView"]        = g_Settings.enableGridView;

    // UI State Settings
    j["showTopItems"]          = g_Settings.showTopItems;
    j["showTopCurrencies"]     = g_Settings.showTopCurrencies;
    
    // Debug Features
    j["enableDebugTab"]        = g_Settings.enableDebugTab;
    j["useFakeDrfServer"]      = g_Settings.useFakeDrfServer;

    // API Timeouts
    j["gw2ApiConnectTimeout"]  = g_Settings.gw2ApiConnectTimeout;
    j["gw2ApiReceiveTimeout"]  = g_Settings.gw2ApiReceiveTimeout;

    // Salvage Kits Settings
    json salvageKitsJson;
    for (const auto& [id, setting] : g_Settings.salvageKitSettings)
    {
        json kitJson;
        kitJson["enabled"] = setting.enabled;
        kitJson["useKarma"] = setting.useKarma;
        salvageKitsJson[std::to_string(id)] = kitJson;
    }
    j["salvageKitSettings"] = salvageKitsJson;

    // Create directory if it doesn't exist
    size_t lastSlash = s_SettingsPath.find_last_of("\\/");
    if (lastSlash != std::string::npos)
    {
        std::string directory = s_SettingsPath.substr(0, lastSlash);
        if (APIDefs)
            APIDefs->Log(LOGL_INFO, "FarmingTracker", ("Creating directory: " + directory).c_str());

        // Create directory recursively
        size_t pos = 0;
        bool directoryCreated = false;
        while ((pos = directory.find_first_of("\\/", pos)) != std::string::npos)
        {
            std::string subDir = directory.substr(0, pos);
            if (!subDir.empty())
            {
                if (!CreateDirectoryA(subDir.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
                {
                    if (APIDefs)
                        APIDefs->Log(LOGL_WARNING, "FarmingTracker", ("Failed to create directory: " + subDir).c_str());
                }
                else
                {
                    directoryCreated = true;
                }
            }
            pos++;
        }
        if (!CreateDirectoryA(directory.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            if (APIDefs)
                APIDefs->Log(LOGL_WARNING, "FarmingTracker", ("Failed to create directory: " + directory).c_str());
        }
        else
        {
            directoryCreated = true;
        }

        // If directory creation failed, return early
        if (!directoryCreated && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            if (APIDefs)
                APIDefs->Log(LOGL_WARNING, "FarmingTracker", "Cannot create settings directory, aborting save");
            return;
        }
    }

    std::ofstream file(s_SettingsPath);
    if (file.is_open())
    {
        file << j.dump(4);
        file.flush();
        file.close();
        if (APIDefs)
            APIDefs->Log(LOGL_INFO, "FarmingTracker", "Settings saved successfully");
    }
    else
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "Failed to open settings file for writing");
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
    if (key.size() < 8 || key.size() > 256)
        return false;
    for (unsigned char c : key)
    {
        if (c < 32 || c > 126)
            return false;
    }
    return true;
}
