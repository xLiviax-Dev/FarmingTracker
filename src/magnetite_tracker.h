#pragma once

// ---------------------------------------------------------------------------
// magnetite_tracker.h
// Weekly Magnetite Shard tracker.
//
// Two-channel approach:
//   1. DRF (real-time, primary)   — every incoming shard drop is counted
//      immediately and added to magnetiteWeeklyEarned. Only shard amounts
//      matching a known "encounter reward" or "failure reward" are counted.
//      Challenge Mode (+10 per boss), chests, achievements, mini sales,
//      salvage, vendor trades are explicitly excluded from the weekly cap.
//   2. API fallback on map change — queries /v2/account/wallet when leaving
//      a raid/encounter map and compares wallet delta vs DRF delta. If the
//      wallet gained MORE than DRF reported, a WARNING is logged and a flag
//      is set so the UI can prompt for manual correction. The tracker never
//      auto-adds the discrepancy (it would include CMs, chests, minis etc.)
//
// Valid weekly-counted shard amounts (per Wiki):
//   Successful encounters : 8, 10, 12, 14, 16 shards (base only, not CM)
//   Unsuccessful attempts : 1, 3, or 5 shards cumulative based on HP %
//   All other amounts (incl. +10 CM, 40 mini, 60 salvage, chest drops etc.)
//   are intentionally ignored for the weekly counter.
//
// Currency ID 28 = Magnetite Shard (confirmed via /v2/currencies)
// Weekly cap     = 800  (as of February 2026 patch)
// Weekly reset   = Monday 07:30 UTC  (same as existing auto-reset mode 3)
// ---------------------------------------------------------------------------

#include <string>
#include <unordered_set>

namespace MagnetiteTracker
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------
    constexpr int CURRENCY_ID  = 28;   // Magnetite Shard wallet currency ID
    constexpr int WEEKLY_CAP   = 800;  // Maximum earnable per week

    // Wiki-valid BASE encounter amounts (successful kills).
    // These amounts ALWAYS count (on raid/strike maps).
    inline const std::unordered_set<int> VALID_BASE_ENCOUNTERS =
    {
        8,     // short/easy  (Cairn, Gorseval, Mursaat Overseer, Fraenir, ...)
        10,    // medium      (Vale Guardian, Slothasor, Boneskinner, Sabetha, ...)
        12,    // long/hard   (Matthias, Keep Construct, Samarog, Xera, ...)
        14,    // very long   (Deimos, Deimos CM pre-boss?, Old Lion's Court, ...)
        16,    // max base    (Dhuum, Eater of Souls, Qadim, Qadim the Peerless, Harvest Temple, ...)
    };

    // Wiki-valid FAILURE amounts (boss-failure progression tiers).
    // These are rate-limited to avoid confusing them with small chest/event drops.
    inline const std::unordered_set<int> VALID_FAILURE_TIERS =
    {
        1,     // below 75% HP
        3,     // below 50% HP
        5,     // below 25% HP
    };

    // Amounts explicitly EXCLUDED even if they would look like base/failure.
    // Used to short-circuit known weekly-exempt combos before context analysis.
    inline const std::unordered_set<int> EXPLICITLY_EXCLUDED =
    {
        // Voice & Claw: 9 = 8 base + 1 bonus chest (chest is weekly-exempt!)
        // Cold War: 19 is a chest/encounter average mix (not a pure base amount)
        9, 19,
        // Miniature vendor trade (40) / mini-direct boss bonus (40)
        40,
        // Salvaged ascended raid item (60 each)
        60,
        // White Mantle Portal Device
        250,
        // Common achievement amounts that overlap base 10/12
        // (weekly wing clear = 10, full wings done = 20, many return-to tiers)
        20, 25, 15,
    };

    // Context dedup: within this many milliseconds of a BASE encounter drop,
    // an otherwise-valid 10 shard delta is assumed to be a CM +10 bonus and
    // is therefore excluded from the weekly counter.
    // Average raid boss kill -> CM reward fires within 5-20 seconds of the base.
    constexpr int CM_CONTEXT_WINDOW_MS = 45000;

    // Failure anti-spam: minimum seconds between counted failure-tier drops.
    // You can't wipe twice in <12s (time to run back, pull, fail).
    constexpr int FAILURE_MIN_INTERVAL_S = 12;

    // Raid & Strike Mission map IDs (MumbleLink map_id).
    // API fallback is only triggered when leaving one of these maps.
    // NOTE: Visions of Eternity maps (e.g. 1609 Lichtung des Waechters)
    // award Gaeting Crystal, NOT Magnetite Shard, so they are excluded.
    inline const std::unordered_set<int> RAID_STRIKE_MAP_IDS =
    {
        // --- Raids ---
        1062,  // Wing 1: Spirit Vale
        1149,  // Wing 2: Salvation Pass
        1156,  // Wing 3: Stronghold of the Faithful
        1188,  // Wing 4: Bastion of the Penitent
        1264,  // Wing 5: Hall of Chains
        1303,  // Wing 6: Mythwright Gambit
        1323,  // Wing 7: The Key of Ahdashim
        1564,  // Wing 8: Mount Balrior (Janthir Wilds)

        // --- Strike Missions ---
        // Icebrood Saga
        1331, 1332,  // Shiverpeaks Pass
        1340, 1346,  // Voice of the Fallen & Claw of the Fallen
        1341, 1344,  // Fraenir of Jormag
        1339, 1351,  // Boneskinner
        1357, 1359,  // Whisper of Jormag
        1362, 1368,  // Forging Steel
        1374, 1376,  // Cold War

        // End of Dragons
        1432,  // Aetherblade Hideout
        1450,  // Xunlai Jade Junkyard
        1451,  // Kaineng Overlook
        1437,  // Harvest Temple
        1485,  // Old Lion's Court

        // Secrets of the Obscure
        1515,  // Cosmic Observatory
        1520,  // Temple of Febe
    };

    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    // Call once on addon load to initialise state from saved settings.
    void Init();

    // Called by DRF channel: add 'amount' shards earned this week.
    // Only positive values matching VALID_WEEKLY_AMOUNTS on a raid/strike
    // map are counted. CMs, chests, minis, achievements etc. are ignored.
    void OnDrfShardsEarned(int amount);

    // Called on map change: checks cooldown, then triggers wallet API call
    // if the previous map was a raid/strike map.
    // 'prevMapId' = MumbleLink map_id before the change
    // 'apiToken'  = current GW2 API key
    void OnMapChange(int prevMapId, const std::string& apiToken);

    // Called by AutoReset on weekly reset (Monday 07:30 UTC).
    void OnWeeklyReset();

    // Returns current weekly earned count (thread-safe read).
    int GetWeeklyEarned();

    // Convert UTC ISO-8601 string to local time string.
    // Returns empty string on failure.
    std::string UtcToLocal(const std::string& utcIso);

    // Manually update the weekly earned count.
    void SetWeeklyEarned(int amount);

    // Returns true if last API check detected a positive gap between
    // wallet gains and DRF-counted shards (suggests uncounted drops or
    // a discrepancy the user should verify manually).
    bool HasApiDiscrepancy();

    // Returns the last reported DRF/wallet delta discrepancy.
    // 0 if no discrepancy was detected or state was cleared.
    int GetLastApiDiscrepancy();

    // Clears the discrepancy flag after the user has manually corrected it.
    void ClearApiDiscrepancy();

    // Shutdown the worker thread
    void Shutdown();

} // namespace MagnetiteTracker
