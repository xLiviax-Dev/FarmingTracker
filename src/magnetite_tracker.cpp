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
#include <thread>
#include <condition_variable>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace
{
    std::mutex s_Mutex;

    // Worker thread stuff
    std::atomic<bool> s_Shutdown{true};
    std::atomic<bool> s_WorkerFinished{false};
    std::thread s_WorkerThread;
    std::mutex s_CvMutex;
    std::condition_variable s_Cv;
    std::atomic<bool> s_WakeWorker{false};
    std::string s_PendingApiToken; // Token queued for fetching (protected by s_CvMutex)

    // API discrepancy state
    std::atomic<bool> s_HasApiDiscrepancy{false};
    std::atomic<int>  s_LastApiDiscrepancy{0};

    // ===================================================================
    // NEW: Context tracking to disambiguate overlapping amounts.
    //
    // Core problem: some values mean different things depending on WHEN
    // they arrive.  Without context we cannot distinguish:
    //
    //   amount 10 -> Vale Guardian (BASE, count)
    //             -> CM bonus +10 on same boss just after base (EXEMPT, do NOT count)
    //
    //   amount  1 -> Boss failure at 75% (count)
    //             -> Grand Frost Legion chest drop (EXEMPT)
    //             -> Forging Steel event shard (EXEMPT)
    // ===================================================================

    // Timestamp (steady_clock ms) of the most recent BASE-encounter drop
    // that was counted (8/10/12/14/16).  Any "10" delta within
    // CM_CONTEXT_WINDOW_MS of this timestamp is treated as a CM bonus
    // (+10 weekly-exempt) rather than another base-10 encounter.
    std::mutex s_ContextMutex;         // protects lastBaseCountedMs, lastFailureMs
    uint64_t   s_LastBaseCountedMs = 0;
    uint64_t   s_LastFailureCountedMs = 0;

    uint64_t SteadyClockNowMs()
    {
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count());
    }

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
        std::string error;
        if (!Gw2Api::GetJson("/v2/account/wallet", apiToken, wallet, error))
        {
            Gw2Api::Log("MagnetiteTracker: wallet fetch failed: " + error, "error");
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

            Gw2Api::Log("MagnetiteTracker: querying wallet...", "info");

            int newTotal = FetchWalletTotal(apiToken);
            if (newTotal < 0)
            {
                // Update timestamp even on failure to avoid hammering on errors
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                g_Settings.magnetiteLastApiCheckUtc = UtcNowIso();
                BackgroundJobs::EnqueueDebouncedSettingsSave();
                continue;
            }

            {
                // Lock Order: SettingsMutex -> s_Mutex (consistent deadlock avoidance)
                std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
                std::lock_guard<std::mutex> lock(s_Mutex);

                int lastTotal   = g_Settings.magnetiteLastWalletTotal;
                int drfNow      = g_Settings.magnetiteWeeklyEarned;
                int drfAtCheck  = g_Settings.magnetiteWeeklyEarnedAtLastCheck;

                // Always update the baseline and DRF snapshot for next time
                g_Settings.magnetiteLastWalletTotal         = newTotal;
                g_Settings.magnetiteWeeklyEarnedAtLastCheck = drfNow;
                g_Settings.magnetiteLastApiCheckUtc         = UtcNowIso();

                if (lastTotal < 0)
                {
                    Gw2Api::Log("MagnetiteTracker: baseline wallet total = "
                        + std::to_string(newTotal), "info");
                }
                else
                {
                    int walletDelta = newTotal - lastTotal;
                    int drfDelta    = drfNow - drfAtCheck;

                    if (walletDelta <= 0)
                    {
                        if (walletDelta < 0)
                            Gw2Api::Log("MagnetiteTracker: wallet -" + std::to_string(-walletDelta)
                                + " (shards spent — ignored)", "debug");
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

                            Gw2Api::Log("MagnetiteTracker: API discrepancy WARNING — "
                                "wallet gained " + std::to_string(walletDelta)
                                + " but DRF only counted " + std::to_string(drfDelta)
                                + "  (gap=" + std::to_string(discrepancy)
                                + "). Gap NOT auto-added (likely includes CMs/chests/minis/"
                                  "achievements). Please verify manually.",
                                "warning");
                        }
                        else if (discrepancy > 0)
                        {
                            s_HasApiDiscrepancy.store(false);
                            s_LastApiDiscrepancy.store(discrepancy);

                            Gw2Api::Log("MagnetiteTracker: walletDelta="
                                + std::to_string(walletDelta)
                                + "  drfDelta=" + std::to_string(drfDelta)
                                + "  small discrepancy=" + std::to_string(discrepancy)
                                + " shards (within chest/event noise — ignored)",
                                "debug");
                        }
                        else if (discrepancy < 0)
                        {
                            s_HasApiDiscrepancy.store(false);
                            s_LastApiDiscrepancy.store(0);

                            Gw2Api::Log("MagnetiteTracker: drfDelta="
                                + std::to_string(drfDelta)
                                + " exceeds walletDelta=" + std::to_string(walletDelta)
                                + "  (DRF ahead by " + std::to_string(-discrepancy)
                                + "). No auto-correction applied.",
                                "warning");
                        }
                        else
                        {
                            s_HasApiDiscrepancy.store(false);
                            s_LastApiDiscrepancy.store(0);

                            Gw2Api::Log("MagnetiteTracker: DRF and wallet agree perfectly — "
                                + std::to_string(walletDelta) + " shards delta matched",
                                "debug");
                        }
                    }
                }
            }

            BackgroundJobs::EnqueueDebouncedSettingsSave();
        }

        s_WorkerFinished.store(true, std::memory_order_release);
    }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
namespace MagnetiteTracker
{

void Init()
{
    if (!s_WorkerThread.joinable())
    {
        s_Shutdown.store(false);
        s_WorkerFinished.store(false, std::memory_order_release);
        s_WorkerThread = std::thread(WorkerLoop);
    }

    s_HasApiDiscrepancy.store(false);
    s_LastApiDiscrepancy.store(0);

    {
        std::lock_guard<std::mutex> lk(s_ContextMutex);
        s_LastBaseCountedMs = 0;
        s_LastFailureCountedMs = 0;
    }

    Gw2Api::Log("MagnetiteTracker: initialised — weekly earned: "
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
        while (!s_WorkerFinished.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (s_WorkerFinished.load(std::memory_order_acquire))
        {
            s_WorkerThread.join();
        }
        else
        {
            Gw2Api::Log("MagnetiteTracker: worker thread didn't shutdown quickly — detaching", "warning");
            s_WorkerThread.detach();
        }
    }
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
    if (amount <= 0) return;

    // =======================================================================
    // STAGE 1 — Map check: only react on raid/strike maps.
    // =======================================================================
    int currentMapId = GetCurrentMapId();
    if (RAID_STRIKE_MAP_IDS.find(currentMapId) == RAID_STRIKE_MAP_IDS.end())
    {
        Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
            + " shards ignored (not on raid/encounter map, map_id=" + std::to_string(currentMapId) + ")", "debug");
        return;
    }

    // =======================================================================
    // STAGE 2 — Enable + explicit exclusion list.
    // =======================================================================
    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        if (!g_Settings.enableMagnetiteTracker) return;
    }

    if (EXPLICITLY_EXCLUDED.find(amount) != EXPLICITLY_EXCLUDED.end())
    {
        Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
            + " shards ignored (amount on explicit weekly-exempt list)", "debug");
        return;
    }

    // =======================================================================
    // STAGE 3 — Categorize into:  BASE ENCOUNTER  /  FAILURE TIER  /  OTHER.
    // =======================================================================
    const bool isBaseCandidate =
        (VALID_BASE_ENCOUNTERS.find(amount) != VALID_BASE_ENCOUNTERS.end());

    const bool isFailureCandidate =
        (VALID_FAILURE_TIERS.find(amount) != VALID_FAILURE_TIERS.end());

    if (!isBaseCandidate && !isFailureCandidate)
    {
        Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
            + " shards ignored (not a valid base-encounter or failure-tier amount — "
              "CM/chest/mini/salvage/achievement)",
            "debug");
        return;
    }

    // =======================================================================
    // STAGE 4 — Context-based disambiguation.
    //
    //   The key case we solve here:
    //     CM boss kill -> two DRF deltas within ~5-20s of each other:
    //       [BASE amount  (8/10/12/14/16)]  -> count
    //       [CM bonus 10]                    -> MUST NOT count
    //
    //   Similarly, amount 1/3/5 can be failure tiers (count) OR chest/event
    //   drops (do not count).  We rate-limit failures to de-noise.
    // =======================================================================

    uint64_t nowMs = SteadyClockNowMs();
    bool countIt = false;
    std::string reason;

    if (isBaseCandidate && amount == 10)
    {
        // -------------------------------------------------------------------
        // Ambiguous "10 shards":  either a VALE GUARDIAN-level BASE kill
        //                         or a CM BONUS +10 fired just after a base.
        // Rule:
        //   If a VALID BASE (8/12/14/16) was counted in the last
        //   CM_CONTEXT_WINDOW_MS milliseconds, assume CM bonus -> EXEMPT.
        //   If the last base counted was itself a 10 (two base-10 bosses
        //   back-to-back, e.g. split maps), require > WINDOW gap.
        //   If no recent base at all -> it's a real base-10 encounter.
        // -------------------------------------------------------------------
        uint64_t lastBaseMs;
        {
            std::lock_guard<std::mutex> lk(s_ContextMutex);
            lastBaseMs = s_LastBaseCountedMs;
        }

        if (lastBaseMs != 0 &&
            (nowMs - lastBaseMs) <= static_cast<uint64_t>(CM_CONTEXT_WINDOW_MS))
        {
            Gw2Api::Log("MagnetiteTracker: DRF +10 shards IGNORED — looks like CM +10 bonus "
                "(base-encounter counted " + std::to_string((nowMs - lastBaseMs) / 1000)
                + "s ago, within context window)", "info");
            return;
        }

        countIt = true;
        reason  = "base-10 encounter (no CM context)";
    }
    else if (isBaseCandidate)
    {
        // Base 8/12/14/16 — no ambiguity (CM bonus is always exactly 10,
        // never 8/12/14/16 on its own).  Count unconditionally.
        countIt = true;
        reason  = "base encounter (8/12/14/16)";
    }
    else if (isFailureCandidate)
    {
        // -------------------------------------------------------------------
        // Failure tiers 1 / 3 / 5.
        //
        // Ambiguity: same numbers appear as:
        //   - Stone Summit Chest (5)              [Forging Steel strike]
        //   - Grand Frost Legion Chest (1-3)      [Cold War strike]
        //   - Bonus chests after kills
        //   - Generic event shards
        //
        // We can't read DRF event names in this function, so we use a
        // strong heuristic: failures never happen more often than ~12s
        // apart (time to run back, pull, engage, die).  Chest drops are
        // typically spaced out OR clustered AFTER a base kill.
        //
        // Additional safeguard: if a base-encounter was counted within
        // the last 10 seconds, a failure-tier 1/3/5 is almost certainly a
        // CHEST drop after the kill -> ignore.
        // -------------------------------------------------------------------
        uint64_t lastBaseMs, lastFailureMs;
        {
            std::lock_guard<std::mutex> lk(s_ContextMutex);
            lastBaseMs    = s_LastBaseCountedMs;
            lastFailureMs = s_LastFailureCountedMs;
        }

        // (a) Within 15s of a BASE kill -> post-kill chest noise, ignore.
        if (lastBaseMs != 0 && (nowMs - lastBaseMs) <= 15000ULL)
        {
            Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
                + " shards IGNORED (failure-tier amount within 15s of base kill "
                  "— classified as post-kill chest drop)", "info");
            return;
        }

        // (b) Rate-limit failures: no two counted within FAILURE_MIN_INTERVAL_S.
        if (lastFailureMs != 0 &&
            (nowMs - lastFailureMs) < static_cast<uint64_t>(FAILURE_MIN_INTERVAL_S * 1000ULL))
        {
            Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
                + " shards IGNORED (failure-tier rate-limit — last counted "
                + std::to_string((nowMs - lastFailureMs) / 1000) + "s ago)", "debug");
            return;
        }

        countIt = true;
        reason  = "failure-tier (anti-spam passed, not within base-kill window)";
    }

    // =======================================================================
    // STAGE 5 — Apply the count.
    // =======================================================================
    if (!countIt) return;

    {
        std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
        std::lock_guard<std::mutex> lock(s_Mutex);
        g_Settings.magnetiteWeeklyEarned =
            std::min(g_Settings.magnetiteWeeklyEarned + amount, WEEKLY_CAP);

        // Record the context timestamp so future disambiguation works.
        // We take s_ContextMutex HERE (after SettingsMutex -> s_Mutex, before unlock).
        std::lock_guard<std::mutex> ctxLock(s_ContextMutex);
        if (isBaseCandidate)
            s_LastBaseCountedMs = nowMs;
        else if (isFailureCandidate)
            s_LastFailureCountedMs = nowMs;
    }

    BackgroundJobs::EnqueueDebouncedSettingsSave();

    Gw2Api::Log("MagnetiteTracker: DRF +" + std::to_string(amount)
        + " shards counted [" + reason + "]  (weekly: "
        + std::to_string(GetWeeklyEarned()) + " / " + std::to_string(WEEKLY_CAP) + ")",
        "data");
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
    if (!enabled) return;

    // Queue the request
    {
        std::lock_guard<std::mutex> lk(s_CvMutex);
        s_PendingApiToken = apiToken;
    }

    // Wake the worker
    s_WakeWorker.store(true);
    s_Cv.notify_one();
}

void OnWeeklyReset()
{
    // Global Lock Order: SettingsMutex -> s_Mutex  (then s_ContextMutex after)
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    g_Settings.magnetiteWeeklyEarned            = 0;
    g_Settings.magnetiteWeeklyEarnedAtLastCheck = 0;
    g_Settings.magnetiteLastApiCheckUtc.clear();

    // Fix: also reset wallet baseline so first post-reset API check builds a
    // NEW baseline instead of computing delta across the reset boundary.
    g_Settings.magnetiteLastWalletTotal         = -1;

    s_HasApiDiscrepancy.store(false);
    s_LastApiDiscrepancy.store(0);

    // Context timestamps also cleared so new week starts with a clean slate.
    {
        std::lock_guard<std::mutex> ctxLock(s_ContextMutex);
        s_LastBaseCountedMs = 0;
        s_LastFailureCountedMs = 0;
    }

    BackgroundJobs::EnqueueDebouncedSettingsSave();
    Gw2Api::Log("MagnetiteTracker: weekly reset — counter, wallet baseline, context cleared", "info");
}

int GetWeeklyEarned()
{
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    return g_Settings.magnetiteWeeklyEarned;
}

void SetWeeklyEarned(int amount)
{
    std::lock_guard<std::recursive_mutex> sLock(Settings::s_SettingsMutex);
    std::lock_guard<std::mutex> lock(s_Mutex);
    g_Settings.magnetiteWeeklyEarned = (std::max)(0, (std::min)(amount, WEEKLY_CAP));
    g_Settings.magnetiteWeeklyEarnedAtLastCheck = g_Settings.magnetiteWeeklyEarned;
    BackgroundJobs::EnqueueDebouncedSettingsSave();
    Gw2Api::Log("MagnetiteTracker: manual update — weekly earned set to " + std::to_string(GetWeeklyEarned()), "info");
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

} // namespace MagnetiteTracker
