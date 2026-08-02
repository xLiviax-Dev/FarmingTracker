#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <vector>

#include "../include/nlohmann/json.hpp"

namespace Gw2Api
{
    // HTTPS GET https://api.guildwars2.com + pathQuery (must start with /), appends access_token.
    bool GetJson(const std::string& pathAndQuery, const std::string& accessToken, nlohmann::json& out, std::string& error);

    bool FetchItemsMany(const std::vector<int>& ids, const std::string& token,
                        nlohmann::json& itemsOut, nlohmann::json& pricesOut, std::string& error);

    bool FetchCurrenciesAll(const std::string& token, nlohmann::json& currenciesOut, std::string& error);
    bool FetchMaterials(const std::string& token, nlohmann::json& materialsOut, std::string& error);
    bool FetchMaterialStorage(const std::string& token, nlohmann::json& materialsOut, std::string& error);
    bool FetchWallet(const std::string& token, nlohmann::json& walletOut, std::string& error);
    bool FetchBank(const std::string& token, nlohmann::json& bankOut, std::string& error);
    bool FetchInventory(const std::string& token, const std::string& characterName, nlohmann::json& bagsOut, std::string& error);
    bool FetchSharedInventory(const std::string& token, nlohmann::json& sharedOut, std::string& error);
    bool FetchCharacters(const std::string& token, nlohmann::json& charactersOut, std::string& error);

    bool FetchAccountName(const std::string& token, std::string& accountName, std::string& error);

    // Checks if the token has the required permissions (inventories, progression)
    bool CheckPermissions(const std::string& token, std::string& error);

    // Returns the GW2 API language code for the current UI language (e.g. "de", "en")
    std::string GetLanguageCode();
    struct Gw2ApiLogEntry
    {
        std::chrono::system_clock::time_point timestamp;
        std::string message;
        std::string type; // "info", "error", "request", etc.
    };

    void Log(const std::string& message, const std::string& type = "info");
    std::vector<Gw2ApiLogEntry> GetLogs();
    void ClearLogs();
    int GetRequestCount();
    void ResetRequestCount();
    void Shutdown();
}
