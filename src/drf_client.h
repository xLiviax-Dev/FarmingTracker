#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <vector>
#include <chrono>

// ---------------------------------------------------------------------------
// Connection status
// ---------------------------------------------------------------------------
enum class DrfStatus
{
    Disconnected,
    Connecting,
    Connected,
    AuthFailed,       // Server rejected our token
    Reconnecting,
    Error
};

// ---------------------------------------------------------------------------
// DRF Log Entry
// ---------------------------------------------------------------------------
struct DrfLogEntry
{
    std::chrono::system_clock::time_point timestamp;
    std::string message;
    std::string type; // "info", "error", "data", etc.
};

// ---------------------------------------------------------------------------
// DRF WebSocket client
//   Uses Windows WinHTTP WebSocket API (Windows 8+, no extra libs needed)
//   Runs the receive loop on a background thread.
// ---------------------------------------------------------------------------
namespace DrfClient
{
    // Call once at startup. Provide the token and a status-change callback.
    void Init(std::function<void(DrfStatus)> onStatusChange);

    // Connect / reconnect with the given token.
    // Safe to call from any thread; cancels any current connection first.
    void Connect(const std::string& token);

    // Graceful shutdown – waits for the background thread to exit.
    void Shutdown();

    // Current status
    DrfStatus GetStatus();

    // For debug display: number of reconnect attempts since last success
    int GetReconnectCount();

    // Debug logging
    void Log(const std::string& message, const std::string& type = "info");
    std::vector<DrfLogEntry> GetLogs();
    void ClearLogs();
}
