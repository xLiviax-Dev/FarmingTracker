#include "auto_reset.h"
#include "magnetite_tracker.h"
#include "loot_logger.h"
#include "settings.h"
#include "item_tracker.h"
#include "shared.h"
#include "backup_restore.h"
#include "ui_notifications.h"
#include "localization.h"
#include <mutex>

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
            << std::setw(2) << utc.tm_sec << " UTC";
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
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
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
        int mode, minutesUntilReset;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            mode = g_Settings.automaticResetMode;
            minutesUntilReset = g_Settings.minutesUntilResetAfterShutdown;
        }
        if (mode == 0 || mode == 1)
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            g_Settings.nextResetDateTimeUtc.clear();
            return;
        }
        std::string newVal = ToIsoUtc(ComputeNextResetUtc(UtcNow(), mode, minutesUntilReset));
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            g_Settings.nextResetDateTimeUtc = newVal;
        }
    }

    static bool s_IsFirstAddonLoad = true;
    static Clock::time_point s_LastResetTime = (Clock::time_point::min)();

    // Reset notification trigger flags — call on manual or auto reset
    static bool s_SessionCompleteTriggered = false;
    static bool s_ResetWarningTriggered    = false;

    void ResetNotificationTriggers()
    {
        s_SessionCompleteTriggered = false;
        s_ResetWarningTriggered    = false;
    }

    bool ShouldResetNowOnLoad()
    {
        int mode;
        std::string nextResetStr;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            mode = g_Settings.automaticResetMode;
            nextResetStr = g_Settings.nextResetDateTimeUtc;
        }
        Clock::time_point next{};
        const bool haveNext = !nextResetStr.empty()
            && ParseIsoUtc(nextResetStr, next);

        const bool isPastReset = haveNext && (UtcNow() >= next);
        const bool isAddonStart = s_IsFirstAddonLoad;
        s_IsFirstAddonLoad = false;

        // Check if we already reset today (within the same reset period)
        const bool alreadyResetToday = (s_LastResetTime != (Clock::time_point::min)()) &&
            (UtcNow() - s_LastResetTime < std::chrono::hours(24));

        switch (mode)
        {
        case 0: return false;
        case 1: 
            // Only reset on first addon start, not on reloads
            if (isAddonStart)
            {
                s_LastResetTime = UtcNow();
                return true;
            }
            return false;
        case 7: 
            // Only reset if past reset time (no 24h restriction for MinutesAfterShutdown mode)
            if (isAddonStart && isPastReset)
            {
                s_LastResetTime = UtcNow();
                return true;
            }
            return false;
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
    bool isEmpty;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        isEmpty = g_Settings.nextResetDateTimeUtc.empty();
    }
    if (isEmpty)
        UpdateNextResetDateTime();

    if (ShouldResetNowOnLoad())
    {
        const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
        ItemTracker::SafeReset();
        
        // Update the next reset time BEFORE saving settings, 
        // so the "past" reset time is not persisted and doesn't trigger again on next start.
        UpdateNextResetDateTime();
        
        ItemTracker::SaveData(addonDir);
        SettingsManager::Save();
    }
    else
    {
        // Even if no reset happened, ensure we have a valid next reset time if missing
        UpdateNextResetDateTime();
    }
}

void AutoReset::OnAddonUnload()
{
    int mode, minutesUntilReset;
    bool isEmpty;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        mode = g_Settings.automaticResetMode;
        minutesUntilReset = g_Settings.minutesUntilResetAfterShutdown;
        isEmpty = g_Settings.nextResetDateTimeUtc.empty();
    }
    if (mode != static_cast<int>(AutomaticResetMode::MinutesAfterShutdown))
        return;

    // Only set reset time if not already set (prevents overriding manual resets)
    if (isEmpty)
    {
        std::string newVal = ToIsoUtc(UtcNow() + std::chrono::minutes(minutesUntilReset));
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        g_Settings.nextResetDateTimeUtc = newVal;
    }
}

void AutoReset::Tick()
{
    // Run backup check
    BackupRestore::Tick();

    int mode, sessionCompleteHours, resetWarningMinutes;
    bool notifySessionComplete, notifyResetWarning;
    std::string nextResetStr;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        mode                 = g_Settings.automaticResetMode;
        notifySessionComplete= g_Settings.notifySessionComplete;
        sessionCompleteHours = g_Settings.sessionCompleteHours;
        notifyResetWarning   = g_Settings.notifyResetWarning;
        resetWarningMinutes  = g_Settings.resetWarningMinutes;
        nextResetStr         = g_Settings.nextResetDateTimeUtc;
    }

    // Session duration notification
    if (notifySessionComplete)
    {
        auto duration = ItemTracker::GetSessionDuration();
        auto targetSeconds = static_cast<long long>(sessionCompleteHours) * 3600;
        
        if (duration.count() >= targetSeconds)
        {
            if (!s_SessionCompleteTriggered)
            {
                char msg[256];
                snprintf(msg, sizeof(msg), Localization::GetText("session_complete_msg"), sessionCompleteHours);
                UINotifications::AddGenericNotification(Localization::GetText("session_complete_title"), msg, "", "Rare", true);
                s_SessionCompleteTriggered = true;
            }
        }
        else
        {
            s_SessionCompleteTriggered = false;
        }
    }

    if (mode <= static_cast<int>(AutomaticResetMode::OnAddonLoad)
        || mode == static_cast<int>(AutomaticResetMode::MinutesAfterShutdown))
        return;

    Clock::time_point next{};
    if (!ParseIsoUtc(nextResetStr, next))
        return;

    auto now = UtcNow();

    // Reset warning notification
    if (notifyResetWarning)
    {
        auto warningTime = next - std::chrono::minutes(resetWarningMinutes);
        if (now >= warningTime && now < next)
        {
            if (!s_ResetWarningTriggered)
            {
                char msg[256];
                snprintf(msg, sizeof(msg), Localization::GetText("reset_warning_msg"), resetWarningMinutes);
                UINotifications::AddGenericNotification(Localization::GetText("reset_warning_title"), msg, "", "Rare", true);
                s_ResetWarningTriggered = true;
            }
        }
        else if (now < warningTime)
        {
            s_ResetWarningTriggered = false;
        }
    }

    if (now < next)
        return;

    // Save favorite settings before reset
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;

    ItemTracker::SafeReset();

    // Weekly tracker resets (Magnetite Shards etc.) — mode 3 = Monday 07:30 UTC
    if (mode == 3)
        MagnetiteTracker::OnWeeklyReset();

    // Loot Logger — start a new session file after each reset
    LootLogger::StartNewSession(addonDir ? addonDir : "");

    // Reset notification trigger flags so they can fire again in the new session
    ResetNotificationTriggers();

    // Persist the reset state immediately to avoid data loss on crash
    ItemTracker::SaveData(addonDir);

    UpdateNextResetDateTime();
    SettingsManager::Save();
    
    // Notification for completed reset
    UINotifications::AddGenericNotification(Localization::GetText("auto_reset_done_title"), Localization::GetText("auto_reset_done_msg"), "", "Fine", false);
}

void AutoReset::OnManualReset()
{
    // Update last reset time to prevent immediate auto-reset
    s_LastResetTime = UtcNow();

    // Reset notification triggers so they can fire again in the new session
    ResetNotificationTriggers();

    // Only update schedule for modes that need it
    int mode;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        mode = g_Settings.automaticResetMode;
    }
    if (mode != 1) // Don't clear nextResetDateTimeUtc for OnAddonLoad mode
    {
        UpdateNextResetDateTime();
    }
    SettingsManager::Save();
}

void AutoReset::RefreshSchedule()
{
    UpdateNextResetDateTime();
    SettingsManager::Save();
}

std::string AutoReset::GetNextResetDisplayUtc()
{
    std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
    if (g_Settings.nextResetDateTimeUtc.empty())
        return "\xe2\x80\x94";

    Clock::time_point next{};
    if (!ParseIsoUtc(g_Settings.nextResetDateTimeUtc, next))
        return "\xe2\x80\x94";

    auto now = UtcNow();
    if (now >= next)
        return "0s";

    auto duration = next - now;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto minutes = seconds / 60;
    auto hours = minutes / 60;
    auto days = hours / 24;

    if (days > 0)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%dd %dh", (int)days, (int)(hours % 24));
        return buf;
    }
    else if (hours > 0)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%dh %dm", (int)hours, (int)(minutes % 60));
        return buf;
    }
    else if (minutes > 0)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%dm", (int)minutes);
        return buf;
    }
    else
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%ds", (int)seconds);
        return buf;
    }
}
