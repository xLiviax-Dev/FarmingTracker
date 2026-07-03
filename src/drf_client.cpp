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
#include "ui_common.h"

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

// WinHTTP handles for forced shutdown
static HINTERNET                       s_hSession = nullptr;
static HINTERNET                       s_hConnect = nullptr;
static HINTERNET                       s_hWebSocket = nullptr;
static std::mutex                      s_HandleMutex;

// Helpers to signal the worker to reconnect with a new token
static std::atomic<bool>              s_ReconnectRequested{ false };
static std::string                    s_PendingToken;
static std::mutex                     s_TokenMutex;

// Rate limiting for ALL connection attempts to prevent server spam
static std::chrono::steady_clock::time_point s_LastConnectionAttempt{};
static constexpr auto                  kMinConnectionInterval = std::chrono::seconds(10);

// Debug logging
static std::deque<DrfLogEntry>        s_Logs;
static std::mutex                     s_LogMutex;
static constexpr size_t               MAX_LOG_ENTRIES = 100;
static std::atomic<bool>              s_LoggedRawSample{ false };

static std::string TruncateForLog(const std::string& s, size_t maxLen)
{
    if (s.size() <= maxLen) return s;
    return s.substr(0, maxLen) + "…";
}

static std::string JsonObjectKeys(const json& j)
{
    if (!j.is_object()) return "";
    std::string out;
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        if (!out.empty()) out += ", ";
        out += it.key();
    }
    return out;
}

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
        s_Logs.pop_front();
}

std::vector<DrfLogEntry> DrfClient::GetLogs()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    return std::vector<DrfLogEntry>(s_Logs.begin(), s_Logs.end());
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

        // Extract character name from DRF data
        if (payload.contains("character") && payload["character"].is_string())
        {
            std::string characterName = payload["character"].get<std::string>();
            {
                std::lock_guard<std::mutex> lock(UICommon::s_AccountNameMutex);
                strncpy_s(UICommon::s_AccountNameBuf, characterName.c_str(), sizeof(UICommon::s_AccountNameBuf));
            }
        }

        if (drop.contains("magic_find") || drop.contains("magicFind") || drop.contains("mf"))
        {
            DrfClient::Log("DRF: Drop contains magic find field(s)", "debug");
        }

        if (drop.contains("mf") && drop["mf"].is_number_integer())
        {
            int mf = drop["mf"].get<int>();
            ItemTracker::SetMagicFind(mf);
            DrfClient::Log("DRF: Magic Find = " + std::to_string(mf), "debug");
        }

        std::map<int, long long> items, currencies;

        if (drop.contains("items"))
            for (auto& [k, v] : drop["items"].items())
                items[std::stoi(k)] = v.get<long long>();

        if (drop.contains("curr"))
        {
            for (auto& [k, v] : drop["curr"].items())
                currencies[std::stoi(k)] = v.get<long long>();
        }

        ItemTracker::AddDrop(items, currencies);
        Gw2Fetcher::NotifyDrfActivity();
        // Reset reconnect count only when we actually receive valid data
        s_ReconnectCount.store(0);

        // Log the drop
        std::stringstream ss;
        ss << "Drop: " << items.size() << " items, " << currencies.size() << " currencies";
        DrfClient::Log(ss.str(), "data");
    }
    catch (...) { /* malformed message, skip */ }
}

// ---------------------------------------------------------------------------
// Ensures minimum time between connection attempts to prevent server spam
// ---------------------------------------------------------------------------
static void WaitForCooldown()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - s_LastConnectionAttempt;
    if (elapsed < kMinConnectionInterval)
    {
        auto remaining = kMinConnectionInterval - elapsed;
        // Sleep in small chunks to allow shutdown/reconnect signals
        auto msRemaining = std::chrono::duration_cast<std::chrono::milliseconds>(remaining).count();
        for (int i = 0; i < msRemaining / 100 && !s_Shutdown.load() && !s_ReconnectRequested.load(); ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    s_LastConnectionAttempt = std::chrono::steady_clock::now();
}

// ---------------------------------------------------------------------------
// One connection attempt + receive loop.
// Returns true if we should retry, false if we should stop (auth failed/shutdown).
// ---------------------------------------------------------------------------
static bool RunConnection(const std::string& token)
{
    // Ensure we don't spam the server with rapid connection attempts
    WaitForCooldown();
    
    // If shutdown or reconnect was requested during cooldown, exit early
    if (s_Shutdown.load() || s_ReconnectRequested.load())
        return true;

    // Cache APIDefs at function entry: the UI thread may set it to nullptr
    // (AddonUnload) while this worker thread is still running.
    auto* apiDefs = APIDefs;

    DrfClient::Log("Connecting to DRF...", "info");
    SetStatus(DrfStatus::Connecting);

    // Debug: Log connection attempt
    if (apiDefs)
        apiDefs->Log(LOGL_INFO, "FarmingTracker", "DRF: Connecting to wss://drf.rs/ws");

    // Open WinHTTP session
    HINTERNET hSession = WinHttpOpen(
        L"FarmingTracker/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) {
        if (apiDefs)
            apiDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to create WinHTTP session");
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
        if (apiDefs)
            apiDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to connect to drf.rs:443");
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
        if (apiDefs)
            apiDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to create WebSocket request");
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
        if (apiDefs)
            apiDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to send WebSocket upgrade request");
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
        if (apiDefs)
            apiDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to complete WebSocket upgrade");
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
    if (apiDefs)
        apiDefs->Log(LOGL_INFO, "FarmingTracker", "DRF: Sending authentication token");
    
    DWORD sendResult = WinHttpWebSocketSend(
        hWebSocket,
        WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
        (PVOID)authMsg.c_str(),
        (DWORD)authMsg.size());

    if (sendResult != ERROR_SUCCESS)
    {
        if (apiDefs)
            apiDefs->Log(LOGL_WARNING, "FarmingTracker", "DRF: Failed to send authentication token");
        WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
        WinHttpCloseHandle(hWebSocket);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        SetStatus(DrfStatus::Disconnected);
        return true;
    }

    if (apiDefs)
        apiDefs->Log(LOGL_INFO, "FarmingTracker", "DRF: Successfully connected and authenticated");
    DrfClient::Log("Connected to DRF", "info");
    SetStatus(DrfStatus::Connected);

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
            // Connection lost - log the error for debugging
            DrfClient::Log("WebSocket receive error: " + std::to_string(recvResult), "error");
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

        // Accumulate fragments — guard against excessively large messages
        assembled.append(reinterpret_cast<char*>(buffer.data()), bytesRead);
        constexpr size_t kMaxAssembledSize = 4 * 1024 * 1024; // 4 MB hard cap
        if (assembled.size() > kMaxAssembledSize)
        {
            DrfClient::Log("DRF: Dropping oversized message (>4 MB), resetting buffer", "warning");
            assembled.clear();
            continue;
        }

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
            std::string kind = msgJson.contains("kind") && msgJson["kind"].is_string() ? msgJson["kind"].get<std::string>() : "";
            if (kind == "session_update")
                continue;

            if (!s_LoggedRawSample.load() && kind == "data")
            {
                s_LoggedRawSample.store(true);
                std::string topKeys     = JsonObjectKeys(msgJson);
                std::string payloadKeys;
                std::string dropKeys;
                if (msgJson.contains("payload") && msgJson["payload"].is_object())
                {
                    payloadKeys = JsonObjectKeys(msgJson["payload"]);
                    if (msgJson["payload"].contains("drop") && msgJson["payload"]["drop"].is_object())
                        dropKeys = JsonObjectKeys(msgJson["payload"]["drop"]);
                }
                DrfClient::Log("DRF sample: top=[" + topKeys + "] payload=[" + payloadKeys + "] drop=[" + dropKeys + "]", "debug");
                DrfClient::Log("DRF raw sample: " + TruncateForLog(msg, 900), "debug");
            }
        }
        catch (...) {
            // If JSON parsing fails, continue with normal processing
        }

        HandleDataMessage(msg);
    }

    // Clear handles first to prevent double-close
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        s_hWebSocket = nullptr;
        s_hConnect = nullptr;
        s_hSession = nullptr;
    }

    // Cleanup local handles only if they're still valid
    if (hWebSocket)
    {
        WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
        WinHttpCloseHandle(hWebSocket);
    }
    if (hConnect)
    {
        WinHttpCloseHandle(hConnect);
    }
    if (hSession)
    {
        WinHttpCloseHandle(hSession);
    }

    return shouldRetry;
}

// ---------------------------------------------------------------------------
// Worker thread – handles reconnect back-off
// ---------------------------------------------------------------------------
static void WorkerThread()
{
    // Reconnect delays:  10, 30, 60, 120, 180, 180, 180 ... (exponential backoff with cap, server-friendly)
    static const int kDelays[] = { 10, 30, 60, 120, 180 };
    static constexpr size_t kNumDelays = sizeof(kDelays) / sizeof(kDelays[0]);

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

        // Back-off delay - with safe index handling
        int tries = s_ReconnectCount.fetch_add(1) + 1;
        size_t delayIdx = static_cast<size_t>(tries - 1);
        if (delayIdx >= kNumDelays)
            delayIdx = kNumDelays - 1;
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
    s_ReconnectCount.store(0); // Reset on init for fresh start
    s_LastConnectionAttempt = std::chrono::steady_clock::time_point{}; // Reset cooldown timer
    s_WorkerThread = std::thread(WorkerThread);
}

void DrfClient::Connect(const std::string& token)
{
    // Always update the last connection attempt time to prevent spamming
    s_LastConnectionAttempt = std::chrono::steady_clock::now();

    DrfClient::Log("Manual DRF reconnect requested", "info");
    
    // Force close existing connection to break the blocking receive loop
    {
        std::lock_guard<std::mutex> lock(s_HandleMutex);
        if (s_hWebSocket)
        {
            WinHttpWebSocketClose(s_hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
            WinHttpCloseHandle(s_hWebSocket);
            s_hWebSocket = nullptr;
        }
    }

    {
        std::lock_guard<std::mutex> lock(s_TokenMutex);
        s_PendingToken = token;
    }
    s_ReconnectRequested.store(true);
    s_ReconnectCount.store(0); // Reset on manual connect only
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
        // Wait longer for graceful shutdown to avoid use-after-free
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // Final attempt to join
        if (s_WorkerThread.joinable())
        {
            s_WorkerThread.join();
        }

        // Only detach as last resort to prevent crash during DLL unload
        if (s_WorkerThread.joinable())
        {
            DrfClient::Log("Warning: Force detaching DRF worker thread - potential resource leak", "warning");
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
