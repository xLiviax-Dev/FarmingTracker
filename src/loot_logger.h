#pragma once

// ---------------------------------------------------------------------------
// loot_logger.h
// Real-time loot logging feature.
//
// Every item drop and currency gain is written to disk the moment it happens.
// Supports CSV and/or JSON output, one file per session, one folder per day.
//
// File layout:
//   <lootLogFolder>/
//     2026-05-25/
//       session_001_12-30-45.csv
//       session_001_12-30-45.json   (if format == Both)
//       session_002_15-10-22.csv    (new session after reset)
//     2026-05-26/
//       session_001_09-15-00.csv
//
// Thread safety:
//   All public functions are safe to call from any thread.
//   A dedicated write-thread drains the queue to disk so the caller
//   (UI/DRF/worker) is never blocked by I/O.
// ---------------------------------------------------------------------------

#include <string>
#include <vector>

namespace LootLogger
{
    // -----------------------------------------------------------------------
    // Data types
    // -----------------------------------------------------------------------

    // A single drop event — items and currencies share this struct.
    struct DropEntry
    {
        std::string  timestampUtc;   // ISO-8601 e.g. "2026-05-25T12:31:04Z"
        int          mapId      = 0;
        std::string  mapName;        // resolved from cache or API
        int          itemId     = 0;
        std::string  itemName;
        long long    quantity   = 0;
        std::string  itemType;       // "Item", "Currency"
        std::string  rarity;         // "Fine", "Rare", etc.  (empty for currencies)
        long long    sellPriceTp= -1;// copper, -1 if unknown/not tradeable
        std::vector<std::string> activeBuffs; // farming-relevant buff names
    };

    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    // Call from AddonLoad after SettingsManager::Load().
    // Starts the write thread, rotates old logs, opens the first session file.
    void Init(const std::string& addonDir);

    // Call from AddonUnload — flushes the queue and joins the write thread.
    void Shutdown();

    // -----------------------------------------------------------------------
    // Session control
    // -----------------------------------------------------------------------

    // Starts a new session file (called at Init and after every reset).
    // addonDir is only needed if the log folder has not been set yet.
    void StartNewSession(const std::string& addonDir = "");

    // -----------------------------------------------------------------------
    // Drop ingestion
    // -----------------------------------------------------------------------

    // Log one drop.  Called from ItemTracker::AddDrop() / ApplyItemsFromApi().
    // The call is non-blocking — the entry is placed on the write queue.
    void LogDrop(const DropEntry& entry);

    // Convenience overload: builds a DropEntry from the raw fields that
    // ItemTracker already has available and enqueues it.
    void LogDrop(
        int          itemId,
        const std::string& itemName,
        long long    quantity,
        bool         isCurrency,
        const std::string& itemType,
        const std::string& rarity,
        long long    sellPriceTp,
        int          mapId,
        const std::string& mapName
    );

    // -----------------------------------------------------------------------
    // Buff cache
    // -----------------------------------------------------------------------

    // Called from entry.cpp on MumbleLink map change (same hook as
    // MagnetiteTracker::OnMapChange).  Queries /v2/account/buffs and updates
    // the internal farming-buff cache used by subsequent LogDrop() calls.
    void RefreshBuffCache(const std::string& apiToken);

    // -----------------------------------------------------------------------
    // Map name cache
    // -----------------------------------------------------------------------

    // Returns the cached map name for mapId, or fetches it via the API if
    // not yet known.  Falls back to "map_<id>" if the API call fails.
    std::string ResolveMapName(int mapId, const std::string& apiToken);

    // -----------------------------------------------------------------------
    // UI helpers
    // -----------------------------------------------------------------------

    // Returns a snapshot of entries logged in the current session.
    // Thread-safe read — returns a copy.
    std::vector<DropEntry> GetCurrentSessionEntries();

    // Returns the absolute path of the current session's log file (CSV or
    // JSON depending on format setting).  Empty if logging is disabled.
    std::string GetCurrentSessionPath();

    // Returns the absolute path of the log folder.
    std::string GetLogFolder();

    // -----------------------------------------------------------------------
    // Default buff whitelist
    // -----------------------------------------------------------------------

    // Returns the default list of buff IDs that are farming-relevant.
    // Used to populate g_Settings.lootLogBuffWhitelist on first load.
    // Source: /v2/account/buffs category filtering.
    const std::vector<int>& DefaultBuffWhitelist();

} // namespace LootLogger
