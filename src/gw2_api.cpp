#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include "gw2_api.h"
#include "shared.h"   // APIDefs for logging
#include "settings.h"

#include <sstream>
#include <deque>
#include <mutex>
#include <chrono>

// Debug logging
static std::deque<Gw2Api::Gw2ApiLogEntry> s_Logs;
static std::mutex s_LogMutex;
static constexpr size_t MAX_LOG_ENTRIES = 100;

// Request count
static std::atomic<int> s_RequestCount{0};

// Persistent WinHttp Handles for reuse
static HINTERNET s_hSession = nullptr;
static HINTERNET s_hConnect = nullptr;
static std::mutex s_HttpMutex;

namespace
{
    std::wstring Utf8ToWide(const std::string& s)
    {
        if (s.empty()) return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        if (n == 0) return {}; // Conversion failed
        std::wstring w(n, 0);
        int result = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
        if (result == 0) return {}; // Conversion failed
        return w;
    }

    void CloseHttpHandles()
    {
        std::lock_guard<std::mutex> lock(s_HttpMutex);
        if (s_hConnect) { WinHttpCloseHandle(s_hConnect); s_hConnect = nullptr; }
        if (s_hSession) { WinHttpCloseHandle(s_hSession); s_hSession = nullptr; }
    }

    bool HttpsGet(const std::wstring& host, INTERNET_PORT port, const std::wstring& pathQuery,
                  std::string& outBody, std::string& error)
    {
        outBody.clear();
        error.clear();

        std::lock_guard<std::mutex> lock(s_HttpMutex);

        if (!s_hSession)
        {
            s_hSession = WinHttpOpen(L"FarmingTracker-GW2API/1.1",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!s_hSession) { error = "WinHttpOpen"; return false; }

            // Set timeouts from settings
            DWORD connectTimeoutMs, receiveTimeoutMs;
            {
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                connectTimeoutMs = static_cast<DWORD>(g_Settings.gw2ApiConnectTimeout);
                receiveTimeoutMs = static_cast<DWORD>(g_Settings.gw2ApiReceiveTimeout);
            }
            WinHttpSetOption(s_hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &connectTimeoutMs, sizeof(connectTimeoutMs));
            WinHttpSetOption(s_hSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &receiveTimeoutMs, sizeof(receiveTimeoutMs));
        }

        if (!s_hConnect)
        {
            s_hConnect = WinHttpConnect(s_hSession, host.c_str(), port, 0);
            if (!s_hConnect) { error = "WinHttpConnect"; return false; }
        }

        HINTERNET hRequest = WinHttpOpenRequest(s_hConnect, L"GET", pathQuery.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);
        if (!hRequest) { error = "WinHttpOpenRequest"; return false; }

        BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        
        if (!ok || !WinHttpReceiveResponse(hRequest, nullptr))
        {
            error = "WinHttpSend/Receive (" + std::to_string(GetLastError()) + ")";
            WinHttpCloseHandle(hRequest);
            // If connection failed, reset connect handle to try a fresh one next time
            WinHttpCloseHandle(s_hConnect);
            s_hConnect = nullptr;
            return false;
        }

        DWORD status = 0;
        DWORD sz = sizeof(status);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);

        std::string body;
        DWORD dwSize = 0;
        do
        {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;
            std::vector<char> buf(dwSize);
            DWORD downloaded = 0;
            if (!WinHttpReadData(hRequest, buf.data(), dwSize, &downloaded)) break;
            body.append(buf.data(), downloaded);
        } while (dwSize > 0);

        WinHttpCloseHandle(hRequest);

        if (status != 200 && status != 206)
        {
            error = "HTTP " + std::to_string(status) + " " + body.substr(0, 200);
            return false;
        }

        if (body.empty())
        {
            error = "Empty response body";
            return false;
        }

        outBody = std::move(body);
        return true;
    }

    std::string JoinIds(const std::vector<int>& ids, size_t begin, size_t end)
    {
        std::ostringstream oss;
        for (size_t i = begin; i < end; ++i)
        {
            if (i > begin) oss << ',';
            oss << ids[i];
        }
        return oss.str();
    }

} // anonymous namespace

std::string Gw2Api::GetLanguageCode()
{
    std::string language;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        language = g_Settings.language;
    }
    if (language == "German")     return "de";
    if (language == "French")     return "fr";
    if (language == "Spanish")    return "es";
    if (language == "Chinese")    return "zh";
    if (language == "Czech")      return "cs";
    if (language == "Italian")    return "it";
    if (language == "Polish")     return "pl";
    if (language == "Portuguese") return "pt";
    if (language == "Russian")    return "ru";
    return "en";
}

bool Gw2Api::GetJson(const std::string& pathAndQuery, const std::string& accessToken,
                     nlohmann::json& out, std::string& error)
{
    s_RequestCount.fetch_add(1);

    std::string path = pathAndQuery;
    // Only append token if one is provided
    if (!accessToken.empty() && path.find("access_token=") == std::string::npos)
    {
        path += (path.find('?') == std::string::npos ? '?' : '&');
        path += "access_token=" + accessToken;
    }

    std::wstring wpath = Utf8ToWide(path);
    std::string body;
    if (!HttpsGet(L"api.guildwars2.com", INTERNET_DEFAULT_HTTPS_PORT, wpath, body, error))
        return false;

    try
    {
        out = nlohmann::json::parse(body);
        return true;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        return false;
    }
}

bool Gw2Api::FetchItemsMany(const std::vector<int>& ids, const std::string& token,
                            nlohmann::json& itemsOut, nlohmann::json& pricesOut, std::string& error)
{
    itemsOut  = nlohmann::json::array();
    pricesOut = nlohmann::json::array();

    Log("Fetching " + std::to_string(ids.size()) + " items", "request");

    std::string langCode = GetLanguageCode();

    constexpr size_t kBatch = 200;
    constexpr int kRateLimitDelayMs = 100; // 100ms delay between batches to avoid rate limits
    for (size_t i = 0; i < ids.size(); i += kBatch)
    {
        size_t j = (std::min)(i + kBatch, ids.size());
        std::string idlist = JoinIds(ids, i, j);
        nlohmann::json jItems, jPrices;

        std::string q = "/v2/items?ids=" + idlist + "&lang=" + langCode;
        if (!GetJson(q, token, jItems, error))
        {
            // If the error is a 404 (all IDs invalid), we treat it as success but with empty results.
            // This allows ApplyItemsFromApi to mark those IDs as unknown and stop the retry loop.
            if (error.find("404") != std::string::npos || error.find("invalid") != std::string::npos)
            {
                jItems = nlohmann::json::array();
                jPrices = nlohmann::json::array();
            }
            else
            {
                Log("Failed to fetch items: " + error, "error");
                return false;
            }
        }
        else
        {
            if (jItems.is_array())
                for (auto& el : jItems) itemsOut.push_back(el);

            q = "/v2/commerce/prices?ids=" + idlist;
            if (!GetJson(q, token, jPrices, error))
            {
                // Check if error is due to HTTP 206 (Partial Content) - this is not an error
                if (error.find("HTTP 206") != std::string::npos)
                {
                    // HTTP 206 is normal for batch requests, continue processing
                    if (jPrices.is_array())
                        for (auto& el : jPrices) pricesOut.push_back(el);
                }
                else
                {
                    // Real error - skip this batch
                    // HTTP 404 with "all ids provided are invalid" is normal for non-tradable items
                    if (error.find("HTTP 404") != std::string::npos && error.find("all ids provided are invalid") != std::string::npos)
                    {
                        Log("Items not tradable on TP (no prices available)", "data");
                    }
                    else
                    {
                        Log("Failed to fetch prices for batch: " + error, "warning");
                    }
                    error.clear();
                }
            }
            else
            {
                if (jPrices.is_array())
                    for (auto& el : jPrices) pricesOut.push_back(el);
            }
        }

        // Rate limiting: delay between batches
        if (i + kBatch < ids.size())
            Sleep(kRateLimitDelayMs);
    }

    Log("Fetched " + std::to_string(itemsOut.size()) + " items, " + std::to_string(pricesOut.size()) + " prices", "request");
    return true;
}

bool Gw2Api::FetchCurrenciesAll(const std::string& token, nlohmann::json& currenciesOut, std::string& error)
{
    std::string langCode = GetLanguageCode();
    return GetJson("/v2/currencies?ids=all&lang=" + langCode, token, currenciesOut, error);
}

bool Gw2Api::FetchMaterials(const std::string& token, nlohmann::json& materialsOut, std::string& error)
{
    // /v2/materials returns a list of all item IDs that can go into material storage
    // This endpoint does not require authentication, but we pass token for consistency
    return GetJson("/v2/materials", token, materialsOut, error);
}

bool Gw2Api::FetchMaterialStorage(const std::string& token, nlohmann::json& materialsOut, std::string& error)
{
    return GetJson("/v2/account/materials", token, materialsOut, error);
}

bool Gw2Api::FetchWallet(const std::string& token, nlohmann::json& walletOut, std::string& error)
{
    return GetJson("/v2/account/wallet", token, walletOut, error);
}

bool Gw2Api::FetchBank(const std::string& token, nlohmann::json& bankOut, std::string& error)
{
    return GetJson("/v2/account/bank", token, bankOut, error);
}

bool Gw2Api::FetchInventory(const std::string& token, const std::string& characterName, nlohmann::json& bagsOut, std::string& error)
{
    // Character name must be URL-encoded for spaces.
    std::string encoded;
    encoded.reserve(characterName.size());
    for (unsigned char c : characterName)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.')
            encoded += static_cast<char>(c);
        else if (c == ' ')
            encoded += "%20";
        else
        {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", c);
            encoded += buf;
        }
    }
    return GetJson("/v2/characters/" + encoded + "/inventory", token, bagsOut, error);
}

bool Gw2Api::FetchSharedInventory(const std::string& token, nlohmann::json& sharedOut, std::string& error)
{
    return GetJson("/v2/account/inventory", token, sharedOut, error);
}

bool Gw2Api::FetchCharacters(const std::string& token, nlohmann::json& charactersOut, std::string& error)
{
    return GetJson("/v2/characters?ids=all", token, charactersOut, error);
}

// Debug logging implementation
void Gw2Api::Log(const std::string& message, const std::string& type)
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    Gw2Api::Gw2ApiLogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.message = message;
    entry.type = type;
    s_Logs.push_back(entry);

    // Keep only the last MAX_LOG_ENTRIES entries
    if (s_Logs.size() > MAX_LOG_ENTRIES)
        s_Logs.pop_front();
}

std::vector<Gw2Api::Gw2ApiLogEntry> Gw2Api::GetLogs()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    return std::vector<Gw2Api::Gw2ApiLogEntry>(s_Logs.begin(), s_Logs.end());
}

void Gw2Api::ClearLogs()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    s_Logs.clear();
}

int Gw2Api::GetRequestCount()
{
    return s_RequestCount.load();
}

void Gw2Api::ResetRequestCount()
{
    s_RequestCount.store(0);
}

void Gw2Api::Shutdown()
{
    CloseHttpHandles();
}

bool Gw2Api::CheckPermissions(const std::string& token, std::string& error)
{
    nlohmann::json j;
    if (!GetJson("/v2/tokeninfo", token, j, error))
        return false;

    if (!j.contains("permissions") || !j["permissions"].is_array())
    {
        error = "Tokeninfo response is missing permissions array";
        return false;
    }

    bool hasInventories = false;
    bool hasProgression = false;

    for (auto& p : j["permissions"])
    {
        if (p.is_string())
        {
            std::string perm = p.get<std::string>();
            if (perm == "inventories") hasInventories = true;
            if (perm == "progression") hasProgression = true;
        }
    }

    if (!hasInventories || !hasProgression)
    {
        error = "Missing required permissions: ";
        if (!hasInventories) error += "inventories ";
        if (!hasProgression) error += "progression";
        return false;
    }

    return true;
}

bool Gw2Api::FetchAccountName(const std::string& token, std::string& accountName, std::string& error)
{
    accountName.clear();
    error.clear();

    nlohmann::json accountJson;
    if (!GetJson("/v2/account", token, accountJson, error))
    {
        return false;
    }

    if (accountJson.contains("name"))
    {
        accountName = accountJson["name"].get<std::string>();
        return true;
    }

    error = "Account name not found in response";
    return false;
}
