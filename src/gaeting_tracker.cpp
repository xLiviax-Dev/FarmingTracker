#include "gaeting_tracker.h"
#include "settings.h"
#include "gw2_api.h"
#include "shared.h"

#include <mutex>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <condition_variable>

namespace
{
    std::mutex s_Mutex;

    std::atomic<bool> s_Shutdown{true};
    std::thread s_WorkerThread;
    std::mutex s_CvMutex;
    std::condition_variable s_Cv;
    std::atomic<bool> s_WakeWorker{false};
    std::string s_PendingApiToken;

    std::atomic<bool> s_HasApiDiscrepancy{false};
    std::atomic<int>  s_LastApiDiscrepancy{0};

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

    bool ParseIsoUtc(const std::string& s, std::chrono::system_clock::time_point& out)
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

    bool CooldownElapsed()
    {
        std::string lastCheck;
        int cooldownMin;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            lastCheck   = g_Settings.gaetingLastApiCheckUtc;
            cooldownMin = g_Settings.gaetingApiCheckCooldownMin;
        }
        if (lastCheck.empty()) return true;

        std::chrono::system_clock::time_point lastPt;
        if (!ParseIsoUtc(lastCheck, lastPt)) return true;

        auto elapsed = std::chrono::system_clock::now() - lastPt;
        return elapsed >= std::chrono::minutes(cooldownMin);
    }

    int FetchWalletTotal(const std::string& apiToken)
    {
        nlohmann::json wallet;
        std::string error;
        if (!Gw2Api::GetJson("/v2/account/wallet", apiToken, wallet, error))
        {
            Gw2Api::Log("GaetingTracker: wallet fetch failed: " + error, "error");
            return -1;
        }
        if (!wallet.is_array()) return -1;

        for (auto& entry : wallet)
        {
            if (entry.value("id", -1) == GaetingTracker::CURRENCY_ID)
                return entry.value("value", -1);
        }
        return -1;
    }

    bool IsValidWeeklyAmount(int amount)
    {
        return GaetingTracker::VALID_WEEKLY_AMOUNTS.find(amount)
            != GaetingTracker::VALID_WEEKLY_AMOUNTS.end();
    }

    void WorkerLoop()
    {
        while (!s_Shutdown.load())
        {
            std::unique_lock<std::mutex> lk(s_CvMutex);
            s_Cv.wait(lk, [] { return s_Shutdown.load() || s_WakeWorker.exchange(false); });
            lk.unlock();

            if (s_Shutdown.load()) break;

            std::string apiToken;
            {
                std::lock_guard<std::mutex> lk(s_CvMutex);
                apiToken = s_PendingApiToken;
            }
            if (apiToken.empty()) continue;

            if (!CooldownElapsed()) continue;

            Gw2Api::Log("GaetingTracker: querying wallet...", "info");

            int newTotal = FetchWalletTotal(apiToken);
            if (newTotal < 0)
            {
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                g_Settings.gaetingLastApiCheckUtc = UtcNowIso();
                BackgroundJobs::EnqueueDebouncedSettingsSave();
                continue;
            }

            {
                // Lock Order: SettingsMutex -> s_Mutex (consistent with Magnetite)
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                std::lock_guard<std::mutex> lock(s_Mutex);

                int lastTotal  = g_Settings.gaetingLastWalletTotal;
                int drfNow     = g_Settings.gaetingWeeklyEarned;
                int drfAtCheck = g_Settings.gaetingWeeklyEarnedAtLastCheck;

                g_Settings.gaetingLastWalletTotal         = newTotal;
                g_Settings.gaetingWeeklyEarnedAtLastCheck = drfNow;
                g_Settings.gaetingLastApiCheckUtc         = UtcNowIso();

                if (lastTotal < 0)
                {
                    Gw2Api::Log("GaetingTracker: baseline wallet total = "
                        + std::to_string(newTotal), "info");
                }
                else
                {
                    int walletDelta = newTotal - lastTotal;
                    int drfDelta    = drfNow - drfAtCheck;

                    if (walletDelta <= 0)
                    {
                        if (walletDelta < 0)
                            Gw2Api::Log("GaetingTracker: wallet -" + std::to_string(-walletDelta)
                                + " (spent — ignored)", "debug");
                        s_HasApiDiscrepancy.store(false);
                        s_LastApiDiscrepancy.store(0);
                    }
                    else
                    {
                        int discrepancy = walletDelta - drfDelta;
                        if (discrepancy >= 10)
                        {
                            s_HasApiDiscrepancy.store(true);
                            s_LastApiDiscrepancy.store(discrepancy);

                            Gw2Api::Log("GaetingTracker: API discrepancy WARNING — wallet gained "
                                + std::to_string(walletDelta) + " but DRF counted "
                                + std::to_string(drfDelta) + " (gap=" + std::to_string(discrepancy)
                                + "). NOT auto-added (likely CM/mini/salvage). Verify manually.",
                                "warning");
                        }
                        else if (discrepancy > 0)
                        {
                            s_HasApiDiscrepancy.store(false);
                            s_LastApiDiscrepancy.store(discrepancy);

                            Gw2Api::Log("GaetingTracker: walletDelta=" + std::to_string(walletDelta)
                                + " drfDelta=" + std::to_string(drfDelta)
                                + " small discrepancy=" + std::to_string(discrepancy)
                                + " (chest/event noise — ignored)", "debug");
                        }
                        else if (discrepancy < 0)
                        {
                            s_HasApiDiscrepancy.store(false);
                            s_LastApiDiscrepancy.store(0);

                            Gw2Api::Log("GaetingTracker: drfDelta=" + std::to_string(drfDelta)
                                + " exceeds walletDelta=" + std::to_string(walletDelta)
                                + " (DRF ahead by " + std::to_string(-discrepancy)
                                + "). No auto-correction.", "warning");
                        }
                        else
                        {
                            s_HasApiDiscrepancy.store(false);
                            s_LastApiDiscrepancy.store(0);

                            Gw2Api::Log("GaetingTracker: DRF and wallet agree perfectly — delta="
                                + std::to_string(walletDelta), "debug");
                        }
                    }
                }
            }

            BackgroundJobs::EnqueueDebouncedSettingsSave();
        }
    }
} // anon

namespace GaetingTracker
{

void Init()
{
    if (!s_WorkerThread.joinable())
    {
        s_Shutdown.store(false);
        s_WorkerThread = std::thread(WorkerLoop);
    }
    s_HasApiDiscrepancy.store(false);
    s_LastApiDiscrepancy.store(0);
    Gw2Api::Log("GaetingTracker: initialised — weekly earned: "
        + std::to_string(GetWeeklyEarned()) + " / "
        + std::to_string(WEEKLY_CAP), "info");
}

void Shutdown()
{
    s_Shutdown.store(true);
    s_WakeWorker.store(true);
    s_Cv.notify_all();

    if (s_WorkerThread.joinable())
    {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (s_WorkerThread.joinable() && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (s_WorkerThread.joinable())
        {
            Gw2Api::Log("GaetingTracker: worker thread didn't shutdown — detaching", "warning");
            s_WorkerThread.detach();
        }
    }
}

std::string UtcToLocal(const std::string& utcIso)
{
    std::chrono::system_clock::time_point tp;
    if (!ParseIsoUtc(utcIso, tp)) return "";
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

void OnDrfCrystalsEarned(int amount)
{
    if (amount <= 0) return;

    int currentMapId = GetCurrentMapId();
    if (VOE_RAID_MAP_IDS.find(currentMapId) == VOE_RAID_MAP_IDS.end())
    {
        Gw2Api::Log("GaetingTracker: DRF +" + std::to_string(amount)
            + " crystals ignored (not on VoE map, map_id=" + std::to_string(currentMapId) + ")", "debug");
        return;
    }

    // Strict filter — weekly-counted valid amounts only:
    //   ignored = 22 (12+10 CM), 40 (mini trade), 52 (12+40 mini-direct), 60 (salvage)
    if (!IsValidWeeklyAmount(amount))
    {
        Gw2Api::Log("GaetingTracker: DRF +" + std::to_string(amount)
            + " crystals ignored (not a valid weekly-counted reward — CM/mini/salvage/chest)", "debug");
        return;
    }

    bool enabled;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        enabled = g_Settings.enableGaetingTracker;
    }
    if (!enabled) return;

    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);
        g_Settings.gaetingWeeklyEarned =
            (std::min)(g_Settings.gaetingWeeklyEarned + amount, WEEKLY_CAP);
    }

    BackgroundJobs::EnqueueDebouncedSettingsSave();
    Gw2Api::Log("GaetingTracker: DRF +" + std::to_string(amount)
        + " crystals (total this week: " + std::to_string(GetWeeklyEarned()) + ")", "data");
}

void OnMapChange(int prevMapId, const std::string& apiToken)
{
    Gw2Api::Log("GaetingTracker: OnMapChange called, prevMapId=" + std::to_string(prevMapId), "info");
    if (VOE_RAID_MAP_IDS.find(prevMapId) == VOE_RAID_MAP_IDS.end())
    {
        Gw2Api::Log("GaetingTracker: prevMapId not in VoE raid list", "debug");
        return;
    }
    bool enabled;
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        enabled = g_Settings.enableGaetingTracker;
    }
    if (!enabled) return;

    {
        std::lock_guard<std::mutex> lk(s_CvMutex);
        s_PendingApiToken = apiToken;
    }
    s_WakeWorker.store(true);
    s_Cv.notify_one();
}

void OnWeeklyReset()
{
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    g_Settings.gaetingWeeklyEarned              = 0;
    g_Settings.gaetingWeeklyEarnedAtLastCheck   = 0;
    g_Settings.gaetingLastApiCheckUtc.clear();
    g_Settings.gaetingLastWalletTotal           = -1;
    s_HasApiDiscrepancy.store(false);
    s_LastApiDiscrepancy.store(0);
    BackgroundJobs::EnqueueDebouncedSettingsSave();
    Gw2Api::Log("GaetingTracker: weekly reset — counter cleared, wallet baseline reset", "info");
}

int GetWeeklyEarned()
{
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    return g_Settings.gaetingWeeklyEarned;
}

void SetWeeklyEarned(int amount)
{
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    g_Settings.gaetingWeeklyEarned = (std::max)(0, (std::min)(amount, WEEKLY_CAP));
    g_Settings.gaetingWeeklyEarnedAtLastCheck = g_Settings.gaetingWeeklyEarned;
    BackgroundJobs::EnqueueDebouncedSettingsSave();
    Gw2Api::Log("GaetingTracker: manual update — weekly earned set to " + std::to_string(GetWeeklyEarned()), "info");
}

bool HasApiDiscrepancy()
{
    return s_HasApiDiscrepancy.load();
}

int GetLastApiDiscrepancy()
{
    return s_LastApiDiscrepancy.load();
}

void ClearApiDiscrepancy()
{
    s_HasApiDiscrepancy.store(false);
    s_LastApiDiscrepancy.store(0);
}

} // namespace GaetingTracker
