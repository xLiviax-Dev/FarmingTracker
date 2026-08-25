#include "gw2_fetcher.h"
#include "gw2_api.h"
#include "item_tracker.h"
#include "settings.h"
#include "shared.h"

#include "../include/nlohmann/json.hpp"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace Gw2Fetcher;

static std::atomic<bool> s_Shutdown{ true };
static std::atomic<bool> s_WorkerFinished{ false };
static std::thread       s_Thread;
static std::mutex        s_CvMutex;
static std::condition_variable s_Cv;
static bool              s_FirstRun = true; // reset to true on each Init()
static std::atomic<bool> s_Wake{ false };
static std::atomic<bool> s_ForceRefresh{ false };
static std::atomic<Gw2Status> s_Status{ Gw2Status::Disconnected };
static std::atomic<int> s_ReconnectCount{ 0 };
static Gw2Status s_LastLoggedStatus{ Gw2Status::Disconnected };

static void WorkerLoop()
{
    static std::map<std::string, nlohmann::json> s_CurrencyJsonCache; // Cache per language
    static std::string    s_LastApiKey;
    static std::string    s_LastLanguage;
    static auto           s_LastPriceUpdate = std::chrono::steady_clock::now();
    static int            s_BackoffLevel = 0; // 0 = no backoff

    while (!s_Shutdown.load())
    {
        // Exponential Backoff: increase wait time if we hit errors
        int waitMs = 800;
        if (s_BackoffLevel > 0) {
            // Wait 2^level seconds, max 1 hour (3600s)
            int backoffSec = (std::min)(3600, (int)std::pow(2, s_BackoffLevel));
            waitMs = backoffSec * 1000;
            Gw2Api::Log("API error backoff active - Waiting " + std::to_string(backoffSec) + "s", "warning");
        }

        std::unique_lock<std::mutex> lk(s_CvMutex);
        s_Cv.wait_for(lk, std::chrono::milliseconds(waitMs), [] {
            return s_Shutdown.load() || s_Wake.exchange(false);
        });
        lk.unlock();

        if (s_Shutdown.load())
            break;

        // Check price update interval
        auto now = std::chrono::steady_clock::now();
        auto elapsedMin = std::chrono::duration_cast<std::chrono::minutes>(now - s_LastPriceUpdate).count();
        // Cache isFirstRun BEFORE resetting s_FirstRun, so we can decide later whether ForceReloadAll is needed
        bool isFirstRun = s_FirstRun;
        bool forceUpdate = isFirstRun || s_ForceRefresh.exchange(false); // Force update on first run or manual refresh
        s_FirstRun = false;

        std::string token;
        int priceUpdateIntervalMin;
        std::string currentLanguage;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            token = g_Settings.gw2ApiKey;
            priceUpdateIntervalMin = g_Settings.priceUpdateIntervalMin;
            currentLanguage = g_Settings.language;
        }

        // BUGFIX: ItemTracker::ForceReloadAll() resets ALL item details.loaded to false,
        // which causes every UI name to show "Loading..." for a few seconds.
        // This MUST NOT happen after every single map change (s_ForceRefresh from entry.cpp!).
        // Only reload details when truly necessary: FirstRun, Key change or Language change.
        bool shouldReloadAllDetails =
            isFirstRun
            || (token != s_LastApiKey)
            || (currentLanguage != s_LastLanguage);

        if (forceUpdate)
        {
            s_Status.store(Gw2Status::Connecting);
            Gw2Api::Log(std::string("Force update triggered") +
                (isFirstRun ? " (FirstRun)" : " (MapChange/KeyChange/Manual)") +
                " - Refreshing all data", "info");
            s_CurrencyJsonCache.clear();
            if (shouldReloadAllDetails)
                ItemTracker::ForceReloadAll();

            nlohmann::json tokenInfo;
            std::string err;
            if (Gw2Api::GetJson("/v2/tokeninfo", token, tokenInfo, err))
            {
                Gw2Api::Log("API Key validated successfully: " + tokenInfo.value("name", "unnamed"), "info");
                
                // Check permissions
                std::string permErr;
                if (!Gw2Api::CheckPermissions(token, permErr))
                    Gw2Api::Log("API Key is missing permissions! " + permErr, "error");
                else
                    Gw2Api::Log("API Key permissions validated (inventories, progression)", "info");
            }
            else
                Gw2Api::Log("API Key validation failed: " + err, "error");
        }

        if (!SettingsManager::IsGw2ApiKeyPlausible(token))
        {
            s_Status.store(Gw2Status::Disconnected);
            if (s_LastLoggedStatus != Gw2Status::Disconnected)
            {
                Gw2Api::Log("Disconnected - No valid API key", "error");
                s_LastLoggedStatus = Gw2Status::Disconnected;
            }
            continue;
        }

        if (token != s_LastApiKey)
        {
            s_LastApiKey = token;
            s_CurrencyJsonCache.clear();
            s_LastLanguage.clear();
            s_ReconnectCount.store(0);
            s_BackoffLevel = 0; // Reset backoff on new key
            Gw2Api::Log("Connecting to GW2 API with new API key", "info");
            forceUpdate = true;
        }

        if (currentLanguage != s_LastLanguage)
        {
            // Clear old cache entries if we have more than 3 languages cached to save memory
            if (s_CurrencyJsonCache.size() > 3) s_CurrencyJsonCache.clear();
            
            s_LastLanguage = currentLanguage;
            Gw2Api::Log("Language changed, clearing currency cache", "info");
            forceUpdate = true;
        }

        bool needCurrencyTable = ItemTracker::NeedCurrencyTable();
        std::vector<int> pending = ItemTracker::CollectPendingItemIds();

        // Throttle only when there is nothing to fetch right now.
        // Always fetch pending items immediately on first start, regardless of interval
        if (!forceUpdate &&
            elapsedMin < priceUpdateIntervalMin &&
            !needCurrencyTable &&
            pending.empty())
        {
            continue;
        }

        // If we have pending items to load, fetch them immediately even if interval hasn't passed
        // This ensures icons and details are available right after game start
        if (!pending.empty() && elapsedMin < priceUpdateIntervalMin)
        {
            // Don't update s_LastPriceUpdate for pending items - keep regular schedule intact
        }
        else if (elapsedMin >= priceUpdateIntervalMin)
        {
            s_LastPriceUpdate = now;
        }

        // Set status to Connected if we have a valid API key
        s_Status.store(Gw2Status::Connected);
        if (s_LastLoggedStatus != Gw2Status::Connected)
        {
            Gw2Api::Log("Connected - Valid API key present", "data");
            s_LastLoggedStatus = Gw2Status::Connected;
        }

        // --- ITEMS FIRST (Priority): Fetch item metadata immediately to make ---
        // --- names/rarities/icons available fast in Drops/Overview/Items tabs. ---
        // --- Currency table is loaded afterwards, so it no longer blocks items. ---
        if (!pending.empty())
        {
            nlohmann::json items, prices;
            std::string err;
            if (!Gw2Api::FetchItemsMany(pending, token, items, prices, err))
            {
                Gw2Api::Log("Failed to fetch item data: " + err, "error");
                s_Status.store(Gw2Status::Error);
                s_ReconnectCount.fetch_add(1);
                s_BackoffLevel = (std::min)(12, s_BackoffLevel + 1); // Max 2^12 = 4096s
            }
            else
            {
                ItemTracker::ApplyItemsFromApi(pending, items, prices);
                s_Status.store(Gw2Status::Connected);
                s_ReconnectCount.store(0);
                s_BackoffLevel = 0; // Success!
            }
        }

        // --- CURRENCIES SECOND: Now load the currency table (no longer blocks items). ---
        // ApplyCurrencyTable may move entries from s_Items to s_Currencies; if that
        // happens we take care of it on the next worker tick.
        if (needCurrencyTable)
        {
            auto& cache = s_CurrencyJsonCache[currentLanguage];
            if (!cache.is_array())
            {
                std::string err;
                if (Gw2Api::FetchCurrenciesAll(token, cache, err))
                {
                    if (!cache.is_array())
                        cache = nlohmann::json::array();
                    s_Status.store(Gw2Status::Connected);
                    s_ReconnectCount.store(0);
                    s_BackoffLevel = 0; // Success! Reset backoff
                    Gw2Api::Log("Connected - Currency data fetched successfully", "data");
                }
                else
                {
                    Gw2Api::Log("Failed to fetch currency data: " + err, "error");
                    cache = nlohmann::json();
                    s_Status.store(Gw2Status::Error);
                    s_ReconnectCount.fetch_add(1);
                    s_BackoffLevel = (std::min)(12, s_BackoffLevel + 1); // Max 2^12 = 4096s
                    if (s_LastLoggedStatus != Gw2Status::Error)
                    {
                        Gw2Api::Log("Error - Failed to fetch currency data", "error");
                        s_LastLoggedStatus = Gw2Status::Error;
                    }
                }
            }
            if (cache.is_array())
            {
                ItemTracker::ApplyCurrencyTable(cache);
            }
        }
    }

    s_Status.store(Gw2Status::Disconnected);
    s_WorkerFinished.store(true, std::memory_order_release);
}

void Gw2Fetcher::Init()
{
    if (s_Thread.joinable())
        return;

    s_Shutdown.store(false);
    s_WorkerFinished.store(false, std::memory_order_release);
    s_FirstRun = true; // ensure force-refresh on first iteration after restart
    s_Thread = std::thread(WorkerLoop);
}

void Gw2Fetcher::Shutdown()
{
    s_Shutdown.store(true);
    s_Wake.store(true);
    s_Cv.notify_all();

    if (s_Thread.joinable())
    {
        // Don't block the main thread for 35s. 
        // We detached the thread if it takes too long, but we must cleanup handles first.
        Gw2Api::Shutdown();

        // Wait a reasonable amount of time (max 2s) for the thread to notice shutdown
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (!s_WorkerFinished.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (s_WorkerFinished.load(std::memory_order_acquire))
        {
            s_Thread.join();
        }
        else
        {
            Gw2Api::Log("GW2 fetcher thread didn't shutdown quickly — detaching", "warning");
            s_Thread.detach();
        }
    }
}

void Gw2Fetcher::UpdateApiKey()
{
    Gw2Api::Log("Manual GW2 API refresh requested", "info");
    s_ForceRefresh.store(true);
    s_Wake.store(true);
    s_Cv.notify_one();
}

void Gw2Fetcher::NotifyDrfActivity()
{
    s_Wake.store(true);
    s_Cv.notify_one();
}

Gw2Status Gw2Fetcher::GetStatus()
{
    return s_Status.load();
}

int Gw2Fetcher::GetReconnectCount()
{
    return s_ReconnectCount.load();
}
