#include "magnetite_tracker.h"
#include "settings.h"
#include "gw2_api.h"
#include "shared.h"

#include <mutex>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace
{
    std::mutex s_Mutex;

    // Returns current UTC time as ISO-8601 string: "2026-05-17T12:31:04Z"
    std::string UtcNowIso()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }

    // Parse ISO-8601 UTC string back to time_point.
    // Returns false if parsing fails.
    bool ParseIsoUtc(const std::string& s,
                     std::chrono::system_clock::time_point& out)
    {
        if (s.size() < 19) return false;
        std::tm tm{};
        std::istringstream ss(s);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (ss.fail()) return false;
#ifdef _WIN32
        out = std::chrono::system_clock::from_time_t(_mkgmtime(&tm));
#else
        out = std::chrono::system_clock::from_time_t(timegm(&tm));
#endif
        return true;
    }

    // Returns true if the cooldown period since the last API check has elapsed.
    bool CooldownElapsed()
    {
        std::string lastCheck;
        int cooldownMin;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            lastCheck   = g_Settings.magnetiteLastApiCheckUtc;
            cooldownMin = g_Settings.magnetiteApiCheckCooldownMin;
        }

        if (lastCheck.empty()) return true; // Never checked before

        std::chrono::system_clock::time_point lastPt;
        if (!ParseIsoUtc(lastCheck, lastPt)) return true;

        auto elapsed = std::chrono::system_clock::now() - lastPt;
        return elapsed >= std::chrono::minutes(cooldownMin);
    }

    // Query /v2/account/wallet, find currency id 28, return its value.
    // Returns -1 on failure.
    int FetchWalletTotal(const std::string& apiToken)
    {
        nlohmann::json wallet;
        std::string err;
        if (!Gw2Api::GetJson("/v2/account/wallet", apiToken, wallet, err))
        {
            Gw2Api::Log("MagnetiteTracker: wallet fetch failed: " + err, "error");
            return -1;
        }

        if (!wallet.is_array()) return -1;

        for (auto& entry : wallet)
        {
            if (entry.value("id", -1) == MagnetiteTracker::CURRENCY_ID)
                return entry.value("value", -1);
        }

        return -1; // Currency not found in wallet
    }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
namespace MagnetiteTracker
{

void Init()
{
    // Nothing extra needed — state is already loaded from settings.json
    // by SettingsManager::Load() before Init() is called.
    Gw2Api::Log("MagnetiteTracker: initialised — weekly earned: "
        + std::to_string(GetWeeklyEarned()) + " / "
        + std::to_string(WEEKLY_CAP), "info");
}

std::string UtcToLocal(const std::string& utcIso)
{
    std::chrono::system_clock::time_point tp;
    if (!ParseIsoUtc(utcIso, tp))
        return "";

    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void OnDrfShardsEarned(int amount)
{
    if (amount <= 0) return; // Ignore purchases / negative deltas

    // Only count shards if player is on a raid/strike/encounter map
    int currentMapId = GetCurrentMapId();
    if (RAID_STRIKE_MAP_IDS.find(currentMapId) == RAID_STRIKE_MAP_IDS.end())
    {
        Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
            + " shards ignored (not on raid/encounter map, map_id=" + std::to_string(currentMapId) + ")", "debug");
        return;
    }

    // Ignore large deltas (>30) - assumed to be sales/trades (Mini Pets, etc.)
    // Max raid boss drop is 26 shards, so anything >30 is likely not a raid drop
    if (amount > 30)
    {
        Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
            + " shards ignored (likely sale/trade, >30 threshold)", "debug");
        return;
    }

    std::lock_guard<std::mutex> lock(s_Mutex);

    bool enabled;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        enabled = g_Settings.enableMagnetiteTracker;
    }
    if (!enabled) return;

    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        g_Settings.magnetiteWeeklyEarned =
            std::min(g_Settings.magnetiteWeeklyEarned + amount, WEEKLY_CAP);
    }

    SettingsManager::Save();

    Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
        + " shards  (total this week: "
        + std::to_string(GetWeeklyEarned()) + ")", "data");
}

void OnMapChange(int prevMapId, const std::string& apiToken)
{
    Gw2Api::Log("MagnetiteTracker: OnMapChange called, prevMapId=" + std::to_string(prevMapId), "info");

    // Only trigger for raid/strike maps
    if (RAID_STRIKE_MAP_IDS.find(prevMapId) == RAID_STRIKE_MAP_IDS.end())
    {
        Gw2Api::Log("MagnetiteTracker: prevMapId not in raid/strike list", "debug");
        return;
    }

    bool enabled;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        enabled = g_Settings.enableMagnetiteTracker;
    }
    if (!enabled)
    {
        Gw2Api::Log("MagnetiteTracker: tracker not enabled", "debug");
        return;
    }

    if (!CooldownElapsed())
    {
        Gw2Api::Log("MagnetiteTracker: map change from raid map "
            + std::to_string(prevMapId)
            + " — skipping API check (cooldown active)", "debug");
        return;
    }

    Gw2Api::Log("MagnetiteTracker: map change from raid/strike map "
        + std::to_string(prevMapId) + " — querying wallet...", "info");

    int newTotal = FetchWalletTotal(apiToken);
    if (newTotal < 0)
    {
        // Update timestamp even on failure to avoid hammering on errors
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        g_Settings.magnetiteLastApiCheckUtc = UtcNowIso();
        SettingsManager::Save();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);

        int lastTotal   = g_Settings.magnetiteLastWalletTotal;
        int drfNow      = g_Settings.magnetiteWeeklyEarned;
        int drfAtCheck  = g_Settings.magnetiteWeeklyEarnedAtLastCheck;

        // Always update the baseline and DRF snapshot for next time
        g_Settings.magnetiteLastWalletTotal         = newTotal;
        g_Settings.magnetiteWeeklyEarnedAtLastCheck = drfNow;
        g_Settings.magnetiteLastApiCheckUtc         = UtcNowIso();

        if (lastTotal < 0)
        {
            // First check ever — just record the baseline, no additions
            Gw2Api::Log("MagnetiteTracker: baseline wallet total = "
                + std::to_string(newTotal), "info");
        }
        else
        {
            int walletDelta = newTotal - lastTotal;

            if (walletDelta <= 0)
            {
                // Wallet stayed flat or decreased (purchases / no activity)
                if (walletDelta < 0)
                    Gw2Api::Log("MagnetiteTracker: wallet -" + std::to_string(-walletDelta)
                        + " (shards spent — ignored)", "debug");
                else
                    Gw2Api::Log("MagnetiteTracker: wallet unchanged", "debug");
            }
            else
            {
                // Wallet increased — work out how much DRF already counted
                int drfDelta    = drfNow - drfAtCheck;          // what DRF saw
                int missedByDrf = std::max(0, walletDelta - drfDelta); // gap

                if (missedByDrf > 0)
                {
                    int canAdd = std::max(0, WEEKLY_CAP - drfNow);
                    int toAdd  = std::min(missedByDrf, canAdd);

                    g_Settings.magnetiteWeeklyEarned += toAdd;

                    Gw2Api::Log("MagnetiteTracker: API fallback +" + std::to_string(toAdd)
                        + " shards missed by DRF"
                        + "  (walletDelta=" + std::to_string(walletDelta)
                        + "  drfDelta="     + std::to_string(drfDelta)   + ")"
                        + "  total this week: "
                        + std::to_string(g_Settings.magnetiteWeeklyEarned), "data");
                }
                else
                {
                    Gw2Api::Log("MagnetiteTracker: DRF already covered all "
                        + std::to_string(walletDelta)
                        + " shards — no API correction needed", "debug");
                }
            }
        }
    }

    SettingsManager::Save();
}

void OnWeeklyReset()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        g_Settings.magnetiteWeeklyEarned            = 0;
        g_Settings.magnetiteWeeklyEarnedAtLastCheck = 0;
        g_Settings.magnetiteLastApiCheckUtc.clear();
        // NOTE: magnetiteLastWalletTotal is NOT reset — it is a running total
    }
    SettingsManager::Save();
    Gw2Api::Log("MagnetiteTracker: weekly reset — counter cleared", "info");
}

int GetWeeklyEarned()
{
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    return g_Settings.magnetiteWeeklyEarned;
}

} // namespace MagnetiteTracker
