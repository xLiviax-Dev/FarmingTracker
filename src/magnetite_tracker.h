#pragma once

// ---------------------------------------------------------------------------
// magnetite_tracker.h
// Weekly Magnetite Shard tracker.
//
// Two-channel approach:
//   1. DRF (real-time, primary)   — every incoming shard drop is counted
//      immediately and added to magnetiteWeeklyEarned.
//   2. API fallback on map change — queries /v2/account/wallet when leaving
//      a raid/encounter map and uses the wallet delta ONLY to cover drops
//      that DRF missed.  Purchases are never counted.
//
// Anti-double-count logic:
//   At each API check we snapshot magnetiteWeeklyEarned into
//   magnetiteWeeklyEarnedAtLastCheck.  The wallet delta is compared against
//   the DRF delta since the last check:
//
//     drfDelta   = magnetiteWeeklyEarned - magnetiteWeeklyEarnedAtLastCheck
//     walletDelta = newWalletTotal - lastWalletTotal  (only if > 0)
//     missedByDrf = max(0, walletDelta - drfDelta)
//
//   Only missedByDrf is added — ensuring purchases and already-counted
//   drops are never added twice.
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

    // Raid & Strike Mission map IDs (MumbleLink map_id).
    // API fallback is only triggered when leaving one of these maps.
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

        // Visions of Eternity
        1609,  // Lichtung des Waechters (Kela encounter) — confirmed in-game
        // TODO: add further VoE encounter map IDs when released
    };

    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    // Call once on addon load to initialise state from saved settings.
    void Init();

    // Called by DRF channel: add 'amount' shards earned this week.
    // Only positive values are accepted (purchases are ignored).
    // Large deltas (>30) are assumed to be sales/trades and are ignored.
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

} // namespace MagnetiteTracker
