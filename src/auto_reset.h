#pragma once

#include <string>

// AutomaticReset enum order (numeric values in settings.json).
enum class AutomaticResetMode : int
{
    Never                    = 0,
    OnAddonLoad              = 1,
    OnDailyReset             = 2,
    OnWeeklyReset            = 3,
    OnWeeklyNaWvwReset       = 4,
    OnWeeklyEuWvwReset       = 5,
    OnWeeklyMapBonusReset    = 6,
    MinutesAfterShutdown     = 7,
};

namespace AutoReset
{
    // Call after SettingsManager::Init, before UI/DRF. May reset counts once.
    void OnAddonLoad();

    // Call before final Save on unload (mode MinutesAfterShutdown).
    void OnAddonUnload();

    // Each frame from main render (cheap): scheduled daily/weekly resets while game is running.
    void Tick();

    // After manual Reset button (and after ItemTracker::Reset).
    void OnManualReset();

    // When automatic reset mode changes in settings.
    void RefreshSchedule();

    std::string GetNextResetDisplayUtc();
}
