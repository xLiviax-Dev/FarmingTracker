#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../include/nexus/Nexus.h"

// Global pointer to the Nexus API - set in AddonLoad, cleared in AddonUnload
extern AddonAPI_t* APIDefs;

#include <windows.h>
#include <functional>
HMODULE GetModule();
int GetCurrentMapId();
bool IsInCombat();

// ---------------------------------------------------------------------------
// Background Job Queue (single shared worker thread, FIFO)
// ---------------------------------------------------------------------------
// Offloads synchronous heavy work (JSON building, file IO, API calls) from
// the render / DRF event threads to a single dedicated worker. Jobs are
// executed strictly in order; use this for one-shot tasks (session save,
// backup creation, debounced settings persistence, ...).
// For periodic "drop all but last" tasks (ItemTracker::SaveData), keep the
// existing specialized single-slot worker instead.
namespace BackgroundJobs
{
    using JobFn = std::function<void()>;

    void Init();
    // Flushes all currently queued jobs (up to a 3s deadline) then joins the
    // worker. Any jobs still pending after the deadline are dropped.
    void Shutdown();

    // Enqueue a job (FIFO). The call is never blocking for long.
    void Enqueue(JobFn fn);

    // Debounced variant for SettingsManager::Save(). Multiple rapid calls
    // collapse to a single actual save in the worker (avoid per-shard file
    // IO). Safe to call from any thread.
    void EnqueueDebouncedSettingsSave();
}
