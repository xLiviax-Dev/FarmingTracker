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
