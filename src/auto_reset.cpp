#include "auto_reset.h"
#include "settings.h"
#include "item_tracker.h"
#include "shared.h"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdio>

namespace
{
    using Clock = std::chrono::system_clock;

    Clock::time_point UtcNow() { return Clock::now(); }

    bool ParseIsoUtc(const std::string& s, Clock::time_point& out)
    {
        if (s.size() < 20)
            return false;
        std::tm tm{};
        int y = 0, mo = 0, d = 0, h = 0, mi = 0, sec = 0;
        if (sscanf_s(s.c_str(), "%d-%d-%dT%d:%d:%d",
                &y, &mo, &d, &h, &mi, &sec) != 6)
            return false;
        
        // Validate ranges to prevent integer overflow
        if (y < 1900 || y > 2100 || mo < 1 || mo > 12 || d < 1 || d > 31 ||
            h < 0 || h > 23 || mi < 0 || mi > 59 || sec < 0 || sec > 59)
            return false;
        
        tm.tm_year = y - 1900;
        tm.tm_mon  = mo - 1;
        tm.tm_mday = d;
        tm.tm_hour = h;
        tm.tm_min  = mi;
        tm.tm_sec  = sec;
        time_t tt = _mkgmtime(&tm);
        if (tt == -1)
            return false;
        out = Clock::from_time_t(tt);
        return true;
    }

    std::string ToIsoUtc(Clock::time_point tp)
    {
        time_t t = Clock::to_time_t(tp);
        struct tm utc{};
        gmtime_s(&utc, &t);
        time_t check = _mkgmtime(&utc);
        if (check == -1)
            return ""; // Invalid time
        std::ostringstream oss;
        oss << std::setfill('0')
            << (utc.tm_year + 1900) << '-' << std::setw(2) << (utc.tm_mon + 1) << '-'
            << std::setw(2) << utc.tm_mday << 'T'
            << std::setw(2) << utc.tm_hour << ':'
            << std::setw(2) << utc.tm_min << ':'
            << std::setw(2) << utc.tm_sec << 'Z';
        return oss.str();
    }

    Clock::time_point StartOfUtcDay(Clock::time_point tp)
    {
        time_t t = Clock::to_time_t(tp);
        struct tm utc{};
        gmtime_s(&utc, &t);
        utc.tm_hour = 0;
        utc.tm_min  = 0;
        utc.tm_sec  = 0;
        time_t day0 = _mkgmtime(&utc);
        return Clock::from_time_t(day0);
    }

    // .NET DayOfWeek matches tm_wday: Sun=0 .. Sat=6
    Clock::time_point NextWeeklyUtc(Clock::time_point nowUtc, int resetWday, int hour, int minute)
    {
        auto dayStart = StartOfUtcDay(nowUtc);
        time_t t = Clock::to_time_t(dayStart);
        struct tm utc{};
        gmtime_s(&utc, &t);
        int wday = utc.tm_wday;
        int daysUntil = (resetWday - wday + 7) % 7;
        Clock::time_point candidate = dayStart + std::chrono::hours(24 * daysUntil)
            + std::chrono::hours(hour) + std::chrono::minutes(minute);
        if (nowUtc < candidate)
            return candidate;
        return candidate + std::chrono::hours(24 * 7);
    }

    Clock::time_point ComputeNextResetUtc(Clock::time_point fromUtc, int mode, int minutesAfterShutdown)
    {
        switch (mode)
        {
        case 0: // Never
        case 1: // OnAddonLoad
            return (Clock::time_point::max)();
        case 7: // MinutesAfterShutdown
            return fromUtc + std::chrono::minutes(minutesAfterShutdown);
        case 2: // Daily 00:00 UTC — next calendar day midnight
            return StartOfUtcDay(fromUtc) + std::chrono::hours(24);
        case 3: // Monday 07:30 UTC
            return NextWeeklyUtc(fromUtc, 1, 7, 30);
        case 4: // Saturday 02:00 UTC (NA WvW)
            return NextWeeklyUtc(fromUtc, 6, 2, 0);
        case 5: // Friday 18:00 UTC (EU WvW)
            return NextWeeklyUtc(fromUtc, 5, 18, 0);
        case 6: // Thursday 20:00 UTC (Map bonus)
            return NextWeeklyUtc(fromUtc, 4, 20, 0);
        case 8: // Custom days (1-28 days)
        {
            int days = g_Settings.customResetDays;
            if (days < 1) days = 1;
            if (days > 28) days = 28;
            return StartOfUtcDay(fromUtc) + std::chrono::hours(24 * days);
        }
        default:
            return (Clock::time_point::max)();
        }
    }

    void UpdateNextResetDateTime()
    {
        int mode = g_Settings.automaticResetMode;
        if (mode == 0 || mode == 1)
        {
            g_Settings.nextResetDateTimeUtc.clear();
            return;
        }
        g_Settings.nextResetDateTimeUtc =
            ToIsoUtc(ComputeNextResetUtc(UtcNow(), mode, g_Settings.minutesUntilResetAfterShutdown));
    }

    static bool s_IsFirstAddonLoad = true;
    static Clock::time_point s_LastResetTime = (Clock::time_point::min)();

    bool ShouldResetNowOnLoad()
    {
        int mode = g_Settings.automaticResetMode;
        Clock::time_point next{};
        const bool haveNext = !g_Settings.nextResetDateTimeUtc.empty()
            && ParseIsoUtc(g_Settings.nextResetDateTimeUtc, next);

        const bool isPastReset = haveNext && (UtcNow() >= next);
        const bool isAddonStart = s_IsFirstAddonLoad;
        s_IsFirstAddonLoad = false;

        // Check if we already reset today (within the same reset period)
        const bool alreadyResetToday = (s_LastResetTime != (Clock::time_point::min)()) &&
            (UtcNow() - s_LastResetTime < std::chrono::hours(24));

        switch (mode)
        {
        case 0: return false;
        case 1: return isAddonStart;
        case 7: return isAddonStart && isPastReset;
        default:
            // Only reset if past reset time AND haven't reset today
            if (isPastReset && !alreadyResetToday)
            {
                s_LastResetTime = UtcNow();
                return true;
            }
            return false;
        }
    }
}

void AutoReset::OnAddonLoad()
{
    if (g_Settings.nextResetDateTimeUtc.empty())
        UpdateNextResetDateTime();

    if (ShouldResetNowOnLoad())
    {
        ItemTracker::Reset();
        // Clear persisted data after reset
        const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
        ItemTracker::ClearPersistedData(addonDir);
    }

    UpdateNextResetDateTime();
}

void AutoReset::OnAddonUnload()
{
    if (g_Settings.automaticResetMode != static_cast<int>(AutomaticResetMode::MinutesAfterShutdown))
        return;
    g_Settings.nextResetDateTimeUtc = ToIsoUtc(
        UtcNow() + std::chrono::minutes(g_Settings.minutesUntilResetAfterShutdown));
}

void AutoReset::Tick()
{
    int mode = g_Settings.automaticResetMode;
    if (mode <= static_cast<int>(AutomaticResetMode::OnAddonLoad)
        || mode == static_cast<int>(AutomaticResetMode::MinutesAfterShutdown))
        return;

    Clock::time_point next{};
    if (!ParseIsoUtc(g_Settings.nextResetDateTimeUtc, next))
        return;

    if (UtcNow() < next)
        return;

    ItemTracker::Reset();
    // Clear persisted data after reset
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
    ItemTracker::ClearPersistedData(addonDir);
    UpdateNextResetDateTime();
    SettingsManager::Save();
}

void AutoReset::OnManualReset()
{
    UpdateNextResetDateTime();
    SettingsManager::Save();
}

void AutoReset::RefreshSchedule()
{
    UpdateNextResetDateTime();
    SettingsManager::Save();
}

std::string AutoReset::GetNextResetDisplayUtc()
{
    if (g_Settings.nextResetDateTimeUtc.empty())
        return "—";
    return g_Settings.nextResetDateTimeUtc;
}
