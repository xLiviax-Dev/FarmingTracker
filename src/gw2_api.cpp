#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include "gw2_api.h"
#include "shared.h"   // APIDefs for logging
#include "settings.h"

#include <sstream>
#include <vector>
#include <mutex>
#include <chrono>

// Debug logging
static std::vector<Gw2Api::Gw2ApiLogEntry> s_Logs;
static std::mutex s_LogMutex;
static constexpr size_t MAX_LOG_ENTRIES = 100;

// Request count
static std::atomic<int> s_RequestCount{0};

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

    bool HttpsGet(const std::wstring& host, INTERNET_PORT port, const std::wstring& pathQuery,
                  std::string& outBody, std::string& error)
    {
        outBody.clear();
        error.clear();

        HINTERNET hSession = WinHttpOpen(L"FarmingTracker-GW2API/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) { error = "WinHttpOpen"; return false; }

        // Set timeouts from settings
        DWORD connectTimeoutMs = static_cast<DWORD>(g_Settings.gw2ApiConnectTimeout);
        DWORD receiveTimeoutMs = static_cast<DWORD>(g_Settings.gw2ApiReceiveTimeout);
        WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &connectTimeoutMs, sizeof(connectTimeoutMs));
        WinHttpSetOption(hSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &receiveTimeoutMs, sizeof(receiveTimeoutMs));

        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); error = "WinHttpConnect"; return false; }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", pathQuery.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); error = "WinHttpOpenRequest"; return false; }

        BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        if (!ok || !WinHttpReceiveResponse(hRequest, nullptr))
        {
            error = "WinHttpSend/Receive";
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
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
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        if (status != 200)
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
}

bool Gw2Api::GetJson(const std::string& pathAndQuery, const std::string& accessToken,
                     nlohmann::json& out, std::string& error)
{
    s_RequestCount.fetch_add(1);

    std::string path = pathAndQuery;
    if (path.find("access_token=") == std::string::npos)
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

    constexpr size_t kBatch = 200;
    constexpr int kRateLimitDelayMs = 100; // 100ms delay between batches to avoid rate limits
    for (size_t i = 0; i < ids.size(); i += kBatch)
    {
        size_t j = (std::min)(i + kBatch, ids.size());
        std::string idlist = JoinIds(ids, i, j);
        nlohmann::json jItems, jPrices;

        std::string q = "/v2/items?ids=" + idlist;
        if (!GetJson(q, token, jItems, error))
        {
            Log("Failed to fetch items: " + error, "error");
            return false;
        }

        q = "/v2/commerce/prices?ids=" + idlist;
        if (!GetJson(q, token, jPrices, error))
        {
            jPrices = nlohmann::json::array();
            error.clear();
        }

        if (jItems.is_array())
            for (auto& el : jItems) itemsOut.push_back(el);
        if (jPrices.is_array())
            for (auto& el : jPrices) pricesOut.push_back(el);

        // Rate limiting: delay between batches
        if (i + kBatch < ids.size())
            Sleep(kRateLimitDelayMs);
    }

    Log("Fetched " + std::to_string(itemsOut.size()) + " items, " + std::to_string(pricesOut.size()) + " prices", "request");
    return true;
}

bool Gw2Api::FetchCurrenciesAll(const std::string& token, nlohmann::json& currenciesOut, std::string& error)
{
    return GetJson("/v2/currencies?ids=all", token, currenciesOut, error);
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
        s_Logs.erase(s_Logs.begin());
}

std::vector<Gw2Api::Gw2ApiLogEntry> Gw2Api::GetLogs()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    return s_Logs;
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
