#include "shared.h"
#include "settings.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <deque>
#include <functional>
#include <atomic>
#include <chrono>

AddonAPI_t* APIDefs = nullptr;

#pragma pack(push, 1)
struct Gw2MumbleContext
{
    uint8_t  serverAddress[28];
    uint32_t mapId;
    uint32_t mapType;
    uint32_t shardId;
    uint32_t instance;
    uint32_t buildId;
    uint32_t uiState;
    uint16_t compassWidth;
    uint16_t compassHeight;
    float    compassRotation;
    float    playerX;
    float    playerY;
    float    mapCenterX;
    float    mapCenterY;
    float    mapScale;
    uint32_t processId;
    uint8_t  mountIndex;
};
#pragma pack(pop)

struct LinkedMem
{
    uint32_t        uiVersion;
    uint32_t        uiTick;
    float           fAvatarPosition[3];
    float           fAvatarFront[3];
    float           fAvatarTop[3];
    wchar_t         name[256];
    float           fCameraPosition[3];
    float           fCameraFront[3];
    float           fCameraTop[3];
    wchar_t         identity[256];
    uint32_t        contextLen;
    Gw2MumbleContext context;
    wchar_t         description[2048];
};

bool IsInCombat()
{
    if (!APIDefs || !APIDefs->DataLink_Get) return false;
    auto* mumble = static_cast<LinkedMem*>(APIDefs->DataLink_Get(DL_MUMBLE_LINK));
    if (!mumble) return false;
    return (mumble->context.uiState & 0x40) != 0;
}

// ===========================================================================
// Background Job Queue
// ===========================================================================
namespace BackgroundJobs
{
    static std::atomic<bool>        s_Shutdown{ true };
    static std::atomic<bool>        s_WorkerFinished{ false };
    static std::thread              s_WorkerThread;
    static std::mutex               s_QueueMutex;
    static std::condition_variable  s_Cv;
    static std::deque<JobFn>        s_Queue;

    // Coalescing for debounced settings saves: only keep at most one pending.
    static std::atomic<bool>        s_SettingsSaveScheduled{ false };

    static void WorkerLoop()
    {
        for (;;)
        {
            JobFn job;
            {
                std::unique_lock<std::mutex> lk(s_QueueMutex);
                s_Cv.wait(lk, [] {
                    return !s_Queue.empty() || s_Shutdown.load(std::memory_order_acquire);
                });
                if (s_Shutdown.load(std::memory_order_acquire) && s_Queue.empty())
                {
                    s_WorkerFinished.store(true, std::memory_order_release);
                    return;
                }
                job = std::move(s_Queue.front());
                s_Queue.pop_front();
            }

            if (job)
            {
                try { job(); }
                catch (...) { /* swallow - worker must keep running */ }
            }
        }
    }

    void Init()
    {
        s_Shutdown.store(false, std::memory_order_release);
        s_WorkerFinished.store(false, std::memory_order_release);
        if (!s_WorkerThread.joinable())
            s_WorkerThread = std::thread(WorkerLoop);
    }

    void Shutdown()
    {
        s_Shutdown.store(true, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lk(s_QueueMutex);
            s_Cv.notify_all();
        }

        if (s_WorkerThread.joinable())
        {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            while (!s_WorkerFinished.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));

            if (s_WorkerFinished.load(std::memory_order_acquire))
            {
                s_WorkerThread.join();
            }
            else
            {
                if (APIDefs)
                    APIDefs->Log(LOGL_WARNING, "FarmingTracker", "BackgroundJobs: worker didn't exit within 3s — detaching");
                s_WorkerThread.detach();
            }
        }

        // Drop anything left in the queue (not strictly needed but cleans state
        // in case Init() is ever re-called mid-session).
        {
            std::lock_guard<std::mutex> lk(s_QueueMutex);
            s_Queue.clear();
        }
        s_SettingsSaveScheduled.store(false, std::memory_order_release);
    }

    void Enqueue(JobFn fn)
    {
        if (!fn) return;
        if (s_Shutdown.load(std::memory_order_acquire))
        {
            // Worker already down — run inline (best-effort, only during
            // shutdown when callers have no choice).
            try { fn(); } catch (...) {}
            return;
        }
        {
            std::lock_guard<std::mutex> lk(s_QueueMutex);
            s_Queue.push_back(std::move(fn));
        }
        s_Cv.notify_one();
    }

    void EnqueueDebouncedSettingsSave()
    {
        if (s_Shutdown.load(std::memory_order_acquire))
        {
            // Shutdown path — run synchronously (we want settings persisted).
            SettingsManager::Save();
            return;
        }

        bool expected = false;
        if (!s_SettingsSaveScheduled.compare_exchange_strong(expected, true,
                                                             std::memory_order_acq_rel))
        {
            return; // already scheduled — coalesce
        }

        Enqueue([] {
            // Clear flag first so new pending saves can be scheduled while
            // we're still writing (we don't want to miss the NEXT update
            // because we were still busy with the previous one).
            s_SettingsSaveScheduled.store(false, std::memory_order_release);
            SettingsManager::Save();
        });
    }
}
