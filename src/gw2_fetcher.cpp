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

static std::atomic<bool> s_Shutdown{ true };
static std::thread       s_Thread;
static std::mutex        s_CvMutex;
static std::condition_variable s_Cv;
static std::atomic<bool> s_Wake{ false };
static std::atomic<Gw2Fetcher::Gw2Status> s_Status{ Gw2Fetcher::Gw2Status::Disconnected };
static std::atomic<int> s_ReconnectCount{ 0 };
static Gw2Fetcher::Gw2Status s_LastLoggedStatus{ Gw2Fetcher::Gw2Status::Disconnected };

static void WorkerLoop()
{
    static nlohmann::json s_CurrencyJsonCache;
    static std::string    s_LastApiKey;

    while (!s_Shutdown.load())
    {
        std::unique_lock<std::mutex> lk(s_CvMutex);
        s_Cv.wait_for(lk, std::chrono::milliseconds(800), [] {
            return s_Shutdown.load() || s_Wake.exchange(false);
        });
        lk.unlock();

        if (s_Shutdown.load())
            break;

        std::string token = g_Settings.gw2ApiKey;
        if (!SettingsManager::IsGw2ApiKeyPlausible(token))
        {
            s_Status.store(Gw2Fetcher::Gw2Status::Disconnected);
            if (s_LastLoggedStatus != Gw2Fetcher::Gw2Status::Disconnected)
            {
                Gw2Api::Log("Disconnected - No valid API key", "error");
                s_LastLoggedStatus = Gw2Fetcher::Gw2Status::Disconnected;
            }
            continue;
        }

        if (token != s_LastApiKey)
        {
            s_LastApiKey     = token;
            s_CurrencyJsonCache = nlohmann::json();
            s_ReconnectCount.store(0);
            Gw2Api::Log("Connecting to GW2 API with new API key", "info");
        }

        // Set status to Connected if we have a valid API key
        s_Status.store(Gw2Fetcher::Gw2Status::Connected);
        if (s_LastLoggedStatus != Gw2Fetcher::Gw2Status::Connected)
        {
            Gw2Api::Log("Connected - Valid API key present", "data");
            s_LastLoggedStatus = Gw2Fetcher::Gw2Status::Connected;
        }

        if (ItemTracker::NeedCurrencyTable())
        {
            if (!s_CurrencyJsonCache.is_array())
            {
                std::string err;
                if (Gw2Api::FetchCurrenciesAll(token, s_CurrencyJsonCache, err))
                {
                    if (!s_CurrencyJsonCache.is_array())
                        s_CurrencyJsonCache = nlohmann::json::array();
                    s_Status.store(Gw2Fetcher::Gw2Status::Connected);
                    s_ReconnectCount.store(0);
                    Gw2Api::Log("Connected - Currency data fetched successfully", "data");
                }
                else
                {
                    Gw2Api::Log("Failed to fetch currency data: " + err, "error");
                    s_CurrencyJsonCache = nlohmann::json();
                    s_Status.store(Gw2Fetcher::Gw2Status::Error);
                    s_ReconnectCount.fetch_add(1);
                    if (s_LastLoggedStatus != Gw2Fetcher::Gw2Status::Error)
                    {
                        Gw2Api::Log("Error - Failed to fetch currency data", "error");
                        s_LastLoggedStatus = Gw2Fetcher::Gw2Status::Error;
                    }
                }
            }
            if (s_CurrencyJsonCache.is_array())
                ItemTracker::ApplyCurrencyTable(s_CurrencyJsonCache);
        }

        std::vector<int> pending = ItemTracker::CollectPendingItemIds();
        if (pending.empty())
            continue;

        nlohmann::json items, prices;
        std::string err;
        if (!Gw2Api::FetchItemsMany(pending, token, items, prices, err))
        {
            Gw2Api::Log("Failed to fetch item data: " + err, "error");
            s_Status.store(Gw2Fetcher::Gw2Status::Error);
            s_ReconnectCount.fetch_add(1);
            continue;
        }

        ItemTracker::ApplyItemsFromApi(items, prices);
        s_Status.store(Gw2Fetcher::Gw2Status::Connected);
        Gw2Api::Log("Connected - Item data fetched successfully", "data");
    }
}

void Gw2Fetcher::Init()
{
    if (s_Thread.joinable())
        return;

    s_Shutdown.store(false);
    s_Thread = std::thread(WorkerLoop);
}

void Gw2Fetcher::Shutdown()
{
    s_Shutdown.store(true);
    s_Wake.store(true);
    s_Cv.notify_all();
    
    if (s_Thread.joinable())
    {
        s_Thread.join(); // Wait for thread to finish
    }
}

void Gw2Fetcher::UpdateApiKey()
{
    s_Wake.store(true);
    s_Cv.notify_one();
}

void Gw2Fetcher::NotifyDrfActivity()
{
    s_Wake.store(true);
    s_Cv.notify_one();
}

Gw2Fetcher::Gw2Status Gw2Fetcher::GetStatus()
{
    return s_Status.load();
}

int Gw2Fetcher::GetReconnectCount()
{
    return s_ReconnectCount.load();
}
