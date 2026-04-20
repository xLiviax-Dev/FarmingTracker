// ---------------------------------------------------------------------------
// drf_client.cpp
//   DRF WebSocket client using Windows WinHTTP WebSocket API.
//
// Protocol recap:
//     1. Connect to  wss://drf.rs/ws
//     2. Send "Bearer <token>"  as a TEXT frame
//     3. Receive JSON frames:
//        - kind == "data"           -> farming drop, parse and forward
//        - kind == "session_update" -> map enter/leave, ignore
//     4. If server closes with "no valid session provided" -> auth failed
//     5. On any error -> reconnect with back-off: 0, 2, 5, 10, 20, 30s
// ---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include "drf_client.h"
#include "item_tracker.h"
#include "settings.h"
#include "../include/nlohmann/json.hpp"
#include "shared.h"   // APIDefs for logging
#include "gw2_fetcher.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <sstream>
#include <functional>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------
static std::function<void(DrfStatus)> s_OnStatus;
static std::atomic<DrfStatus>         s_Status{ DrfStatus::Disconnected };
static std::atomic<bool>              s_Shutdown{ false };
static std::atomic<int>               s_ReconnectCount{ 0 };
static std::thread                    s_WorkerThread;
static std::mutex                     s_ConnectMutex;

// WinHTTP handles for forced shutdown
static HINTERNET                       s_hSession = nullptr;
static HINTERNET                       s_hConnect = nullptr;
static HINTERNET                       s_hWebSocket = nullptr;
static std::mutex                      s_HandleMutex;

// Helpers to signal the worker to reconnect with a new token
static std::atomic<bool>              s_ReconnectRequested{ false };
static std::string                    s_PendingToken;
static std::mutex                     s_TokenMutex;

// Debug logging
static std::vector<DrfLogEntry>       s_Logs;
static std::mutex                     s_LogMutex;
static constexpr size_t               MAX_LOG_ENTRIES = 100;

// Max currencies per drop (more = DRF bug / invalid message)
static constexpr int MAX_CURRENCIES_PER_DROP = 10;

// ---------------------------------------------------------------------------
static void SetStatus(DrfStatus status)
{
    s_Status.store(status);
    if (s_OnStatus) s_OnStatus(status);
}

// ---------------------------------------------------------------------------
// Debug logging implementation
// ---------------------------------------------------------------------------
void DrfClient::Log(const std::string& message, const std::string& type)
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    DrfLogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.message = message;
    entry.type = type;
    s_Logs.push_back(entry);

    // Keep only the last MAX_LOG_ENTRIES entries
    if (s_Logs.size() > MAX_LOG_ENTRIES)
        s_Logs.erase(s_Logs.begin());
}

std::vector<DrfLogEntry> DrfClient::GetLogs()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    return s_Logs;
}

void DrfClient::ClearLogs()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    s_Logs.clear();
}

// ---------------------------------------------------------------------------
// Parse a "data" kind JSON message and forward drops to ItemTracker
// ---------------------------------------------------------------------------
static void HandleDataMessage(const std::string& jsonText)
{
    try
    {
        auto j = json::parse(jsonText);
        auto& payload = j.at("payload");
        auto& drop    = payload.at("drop");

        std::map<int, long long> items, currencies;

        if (drop.contains("items"))
            for (auto& [k, v] : drop["items"].items())
                items[std::stoi(k)] = v.get<long long>();

        if (drop.contains("curr"))
        {
            for (auto& [k, v] : drop["curr"].items())
                currencies[std::stoi(k)] = v.get<long long>();

            // Ignore invalid drop (DRF wallet-snapshot bug)
            if ((int)currencies.size() > MAX_CURRENCIES_PER_DROP)
                return;
        }

        ItemTracker::AddDrop(items, currencies);
        Gw2Fetcher::NotifyDrfActivity();

        // Log the drop
        std::stringstream ss;
        ss << "Drop: " << items.size() << " items, " << currencies.size() << " currencies";
        DrfClient::Log(ss.str(), "data");
    }
    catch (...) { /* malformed message, skip */ }
}

// ---------------------------------------------------------------------------
// One connection attempt + receive loop.
// Returns true if we should retry, false if we should stop (auth failed/shutdown).
// ---------------------------------------------------------------------------
static bool RunConnection(const std::string& token)
{
    DrfClient::Log("Connecting to DRF...", "info");
    SetStatus(DrfStatus::Connecting);

    // Debug: Log connection attempt
    if (APIDefs)
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "DRF: Connecting to wss://drf.rs/ws");

    // Open WinHTTP session
    HINTERNET hSession = WinHttpOpen(
        L"FarmingTracker/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to create WinHTTP session");
        DrfClient::Log("Failed to create WinHTTP session", "error");
        SetStatus(DrfStatus::Error);
        return true;
    }

    // Store handles for forced shutdown
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        s_hSession = hSession;
    }

    // Connect to drf.rs:443
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"drf.rs",
        INTERNET_DEFAULT_HTTPS_PORT,
        0);
    if (!hConnect) {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to connect to drf.rs:443");
        DrfClient::Log("Failed to connect to drf.rs:443", "error");
        WinHttpCloseHandle(hSession);
        SetStatus(DrfStatus::Error);
        return true;
    }

    // Store hConnect handle
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        s_hConnect = hConnect;
    }

    // Open WebSocket upgrade request
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        L"/ws",
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to create WebSocket request");
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        SetStatus(DrfStatus::Error);
        return true;
    }

    // Perform WebSocket upgrade
    WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0);

    BOOL bResult = WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!bResult || !WinHttpReceiveResponse(hRequest, nullptr))
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to send WebSocket upgrade request");
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        SetStatus(DrfStatus::Disconnected);
        return true; // retry
    }

    HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
    WinHttpCloseHandle(hRequest); // no longer needed after upgrade

    if (!hWebSocket)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to complete WebSocket upgrade");
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        SetStatus(DrfStatus::Disconnected);
        return true;
    }

    // Store hWebSocket handle
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        s_hWebSocket = hWebSocket;
    }

    // Send authentication: "Bearer <token>"
    std::string authMsg = "Bearer " + token;
    if (APIDefs)
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "DRF: Sending authentication token");
    
    DWORD sendResult = WinHttpWebSocketSend(
        hWebSocket,
        WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
        (PVOID)authMsg.c_str(),
        (DWORD)authMsg.size());

    if (sendResult != ERROR_SUCCESS)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to send authentication token");
        WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
        WinHttpCloseHandle(hWebSocket);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        SetStatus(DrfStatus::Disconnected);
        return true;
    }

    if (APIDefs)
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "DRF: Successfully connected and authenticated");
    DrfClient::Log("Connected to DRF", "info");
    SetStatus(DrfStatus::Connected);
    s_ReconnectCount.store(0);

    // ---------------------------------------------------------------------------
    // Receive loop
    // ---------------------------------------------------------------------------
    std::vector<BYTE> buffer(40960);      // 40 KB – large enough for big drops
    std::string       assembled;
    bool              shouldRetry = true;

    while (!s_Shutdown.load() && !s_ReconnectRequested.load())
    {
        DWORD                            bytesRead = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE   bufType   = WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE;

        DWORD recvResult = WinHttpWebSocketReceive(
            hWebSocket,
            buffer.data(),
            (DWORD)buffer.size(),
            &bytesRead,
            &bufType);

        if (recvResult != ERROR_SUCCESS)
        {
            // Connection lost
            SetStatus(DrfStatus::Disconnected);
            shouldRetry = true;
            break;
        }

        if (bufType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
        {
            // Server closed the connection.
            // Check close reason: "no valid session provided" = auth failed.
            USHORT closeStatus = 0;
            char   closeReason[256]{};
            DWORD  reasonLen = 0;
            WinHttpWebSocketQueryCloseStatus(
                hWebSocket, &closeStatus, closeReason, sizeof(closeReason) - 1, &reasonLen);

            // Check for buffer overflow before creating string
            if (reasonLen > sizeof(closeReason) - 1)
                reasonLen = sizeof(closeReason) - 1;

            std::string reason(closeReason, reasonLen);
            if (reason.find("no valid session") != std::string::npos)
            {
                DrfClient::Log("Authentication failed: " + reason, "error");
                SetStatus(DrfStatus::AuthFailed);
                shouldRetry = false; // don't reconnect automatically, user must fix token
            }
            else
            {
                SetStatus(DrfStatus::Disconnected);
                shouldRetry = true;
            }
            break;
        }

        // Accumulate fragments
        assembled.append(reinterpret_cast<char*>(buffer.data()), bytesRead);

        bool isLastFragment =
            (bufType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE) ||
            (bufType == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE);

        if (!isLastFragment)
            continue;

        // We have a complete message
        std::string msg = std::move(assembled);
        assembled.clear();

        // Robust check: skip "session_update" messages (map enter/leave)
        try {
            json msgJson = json::parse(msg);
            if (msgJson.contains("kind") && msgJson["kind"].get<std::string>() == "session_update")
                continue;
        }
        catch (...) {
            // If JSON parsing fails, continue with normal processing
        }

        HandleDataMessage(msg);
    }

    // Cleanup
    WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
    WinHttpCloseHandle(hWebSocket);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Clear handles
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        s_hWebSocket = nullptr;
        s_hConnect = nullptr;
        s_hSession = nullptr;
    }

    return shouldRetry;
}

// ---------------------------------------------------------------------------
// Worker thread – handles reconnect back-off
// ---------------------------------------------------------------------------
static void WorkerThread()
{
    // Reconnect delays:  0, 2, 5, 10, 20, 30, 30, 30 ...…
    static const int kDelays[] = { 0, 2, 5, 10, 20, 30 };

    while (!s_Shutdown.load())
    {
        // Grab current token
        std::string token;
        {
            std::lock_guard<std::mutex> lock(s_TokenMutex);
            token = s_PendingToken;
            s_ReconnectRequested.store(false);
        }

        // Don't attempt connection with empty / invalid token
        if (token.empty())
        {
            SetStatus(DrfStatus::Disconnected);
            // Wait until a new token arrives
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        bool shouldRetry = RunConnection(token);

        if (s_Shutdown.load())
            break;

        // Auth failed: same token will always fail — wait for Connect() / new token (or unload).
        if (!shouldRetry)
        {
            while (!s_Shutdown.load() && !s_ReconnectRequested.load())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Back-off delay
        int tries   = s_ReconnectCount.fetch_add(1) + 1;
        int delayIdx = (tries < 6) ? (tries - 1) : 5;
        int delaySec = kDelays[delayIdx];

        SetStatus(DrfStatus::Reconnecting);

        // Sleep in 100ms chunks so we can react to shutdown/reconnect signals
        for (int i = 0; i < delaySec * 10 && !s_Shutdown.load() && !s_ReconnectRequested.load(); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    SetStatus(DrfStatus::Disconnected);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void DrfClient::Init(std::function<void(DrfStatus)> onStatusChange)
{
    s_OnStatus = std::move(onStatusChange);
    s_Shutdown.store(false);
    s_WorkerThread = std::thread(WorkerThread);
}

void DrfClient::Connect(const std::string& token)
{
    {
        std::lock_guard<std::mutex> lock(s_TokenMutex);
        s_PendingToken = token;
    }
    s_ReconnectRequested.store(true);
    s_ReconnectCount.store(0);
}

void DrfClient::Shutdown()
{
    s_Shutdown.store(true);
    s_ReconnectRequested.store(true); // wake the worker

    // Force close WebSocket connection to prevent DLL lock
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        if (s_hWebSocket)
        {
            WinHttpWebSocketClose(s_hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
            WinHttpCloseHandle(s_hWebSocket);
            s_hWebSocket = nullptr;
        }
        if (s_hConnect)
        {
            WinHttpCloseHandle(s_hConnect);
            s_hConnect = nullptr;
        }
        if (s_hSession)
        {
            WinHttpCloseHandle(s_hSession);
            s_hSession = nullptr;
        }
    }

    // Give the thread a moment to exit gracefully
    if (s_WorkerThread.joinable())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Force detach if thread doesn't finish (WebSocket receive may be blocking)
        if (s_WorkerThread.joinable())
        {
            s_WorkerThread.detach();
        }
    }
}

DrfStatus DrfClient::GetStatus()
{
    return s_Status.load();
}

int DrfClient::GetReconnectCount()
{
    return s_ReconnectCount.load();
}
