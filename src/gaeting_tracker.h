#pragma once

// ---------------------------------------------------------------------------
// gaeting_tracker.h
// Weekly Gaeting Crystal tracker (Visions of Eternity raids).
//
// Two-channel approach (mirrors MagnetiteTracker):
//   1. DRF (real-time, primary)   — every incoming crystal drop is counted
//      immediately and added to gaetingWeeklyEarned. Only crystal amounts
//      matching a known "encounter reward" or "failure reward" are counted.
//      Challenge Mode (+10 per boss, once per boss per week), mini trades
//      (+40 each), and salvage (+60 per asc. item) are explicitly excluded.
//   2. API fallback on map change — queries /v2/account/wallet when leaving
//      a Gaeting raid/encounter map and compares wallet delta vs DRF delta.
//      A positive gap is only flagged as a warning (never auto-added),
//      because it will almost always include CMs, minis, or salvage.
//
// Valid weekly-counted crystal amounts (per Wiki):
//   Successful encounters : 12  (Guardian's Glade / Kela — more TBA)
//   Unsuccessful attempts : 1, 3, or 5 crystals cumulative based on HP %
//   All other amounts (incl. +10 CM, +40 mini, +52 mini-direct, 60 salvage,
//   vendor chests etc.) are intentionally excluded from the 150 weekly cap.
//
// Currency ID 39 = Gaeting Crystal (confirmed via /v2/currencies)
// Weekly cap     = 150
// Weekly reset   = Monday 07:30 UTC  (same as existing auto-reset mode 3)
// ---------------------------------------------------------------------------

#include <string>
#include <unordered_set>

namespace GaetingTracker
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------
    constexpr int CURRENCY_ID  = 39;   // Gaeting Crystal wallet currency ID
    constexpr int WEEKLY_CAP   = 150;  // Maximum earnable per week

    // Known weekly-counted crystal rewards (base-only + failure tiers).
    // Any DRF delta NOT in this set is explicitly excluded.
    inline const std::unordered_set<int> VALID_WEEKLY_AMOUNTS =
    {
        1,     // failure: below 75% HP
        3,     // failure: below 50% HP
        5,     // failure: below 25% HP
        12,    // success: Guardian's Glade (Kela)
        // TODO: expand when further VoE encounters are released
    };

    // Visions of Eternity / Janthir Wilds raid map IDs (MumbleLink map_id).
    // API fallback is only triggered when leaving one of these maps.
    inline const std::unordered_set<int> VOE_RAID_MAP_IDS =
    {
        1609,  // Guardian's Glade (Lichtung des Waechters) — Kela encounter
        // TODO: add further VoE raid map IDs when released
    };

    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    void Init();
    void Shutdown();

    void OnDrfCrystalsEarned(int amount);
    void OnMapChange(int prevMapId, const std::string& apiToken);
    void OnWeeklyReset();

    int  GetWeeklyEarned();
    void SetWeeklyEarned(int amount);

    std::string UtcToLocal(const std::string& utcIso);

    bool HasApiDiscrepancy();
    int  GetLastApiDiscrepancy();
    void ClearApiDiscrepancy();

} // namespace GaetingTracker
