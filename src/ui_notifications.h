#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "item_tracker.h"

namespace UINotifications
{
    struct Notification
    {
        int itemId;
        std::string itemName;
        std::string iconUrl;
        std::string rarity;
        long long value;
        std::string specialText; // e.g., "Pre-Cursor drop!"
        std::chrono::system_clock::time_point startTime;
        float fadeAmount = 0.0f;
        bool isPrecursor = false;
        bool isInfusion = false;
    };

    // Initialize the notification system
    void Init();

    // Shutdown the notification system
    void Shutdown();

    // Render the notifications (called in every frame)
    void Render();

    // Add a new notification
    void AddNotification(int itemId, const Stat& stat, const std::string& specialText = "");

    // Add a generic notification (for Profit Goal, Reset Warning, etc.)
    void AddGenericNotification(const std::string& title, const std::string& message, const std::string& iconUrl = "", const std::string& rarity = "Basic", bool isAlert = false);

    // Audio system
    void PlayNotificationSound(bool isPrecursor, bool isInfusion, bool isAlert = false);
    void SetVolume(float volume);

    // Clear all notifications
    void ClearAll();
}
