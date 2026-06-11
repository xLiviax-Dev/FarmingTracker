// ---------------------------------------------------------------------------
// entry.cpp – Farming Tracker Nexus Addon entry point
// ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <libloaderapi.h>
#include <thread>
#include <chrono>
#include "../include/nexus/Nexus.h"
#include "../include/imgui/imgui.h"
#include "shared.h"
#include "settings.h"
#include "drf_client.h"
#include "ui.h"
#include "ui_common.h"
#include "ui_tab_icons.h"
#include "ui_info.h"
#include "auto_reset.h"
#include "gw2_fetcher.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "search_manager.h"
#include "localization.h"
#include "item_tracker.h"
#include "session_history.h"
#include "ui_notifications.h"
#include "gw2_api.h"
#include "magnetite_tracker.h"
#include "loot_logger.h"

// ---------------------------------------------------------------------------
// MumbleLink Identity struct (standard GW2 format, delivered by Nexus via
// EV_MUMBLE_IDENTITY_UPDATED with Mumble::Identity* as payload)
// Field types must match Nexus's internal Mumble::Identity exactly.
// ---------------------------------------------------------------------------
struct MumbleIdentity
{
    wchar_t  name[256];
    uint32_t profession;
    uint32_t spec;
    uint32_t race;
    uint32_t map_id;
    uint32_t world_id;
    uint32_t team_color_id;
    bool     commander;
    float    fov;
    uint32_t uisz;
};

// GW2 MumbleLink context struct — the actual current map ID lives here.
// Matches the GW2 MumbleLink context layout exactly.
#pragma pack(push, 1)
struct Gw2MumbleContext
{
    uint8_t  serverAddress[28]; // sockaddr_in or sockaddr_in6
    uint32_t mapId;             // ← actual current map ID
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

static int s_LastMapId = 0;

int GetCurrentMapId()
{
    return s_LastMapId;
}

static void OnMumbleIdentityUpdated(void* aEventArgs)
{
    // Map ID from the Identity struct is unreliable for story instances.
    // Read from the full MumbleLink context instead.
    if (!APIDefs || !APIDefs->DataLink_Get) return;
    auto* mumble = static_cast<LinkedMem*>(APIDefs->DataLink_Get(DL_MUMBLE_LINK));
    if (!mumble) return;
    int newMapId = static_cast<int>(mumble->context.mapId);
    
    APIDefs->Log(LOGL_DEBUG, "FarmingTracker", ("OnMumbleIdentityUpdated: map_id = " + std::to_string(newMapId)).c_str());

    if (newMapId != s_LastMapId && s_LastMapId != 0)
    {
        std::string token;
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            token = g_Settings.gw2ApiKey;
        }
        MagnetiteTracker::OnMapChange(s_LastMapId, token);
    }

    s_LastMapId = newMapId;
    ItemTracker::SetCurrentMapId(newMapId); // keep item_tracker in sync for loot log
}

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();

static AddonDefinition_t s_AddonDef{};
static HMODULE s_Module = NULL;

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
    s_AddonDef.Signature        = 0x0134D681;  // Positive signature for non-Raidcore addons
    s_AddonDef.APIVersion       = NEXUS_API_VERSION;
    s_AddonDef.Name             = "Farming Tracker";
    s_AddonDef.Version.Major    = 2;
    s_AddonDef.Version.Minor    = 0;
    s_AddonDef.Version.Build    = 0;
    s_AddonDef.Version.Revision = 3;
    s_AddonDef.Author           = "Livia.3928";
    s_AddonDef.Description      = "Tracks farmed items and currencies in real-time via DRF (drf.rs).";
    s_AddonDef.Load             = AddonLoad;
    s_AddonDef.Unload           = AddonUnload;
    s_AddonDef.Flags            = AF_None;
    s_AddonDef.Provider         = UP_GitHub;
    s_AddonDef.UpdateLink       = "https://github.com/xLiviax-Dev/FarmingTracker/releases/latest/download/FarmingTracker.dll";
    return &s_AddonDef;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            s_Module = hModule;
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

HMODULE GetModule()
{
    return s_Module;
}

void AddonLoad(AddonAPI_t* aApi)
{
    if (!aApi || !aApi->ImguiContext || !aApi->ImguiMalloc || !aApi->ImguiFree)
        return;

    APIDefs = aApi;

    // Set up ImGui context (REQUIRED for rendering) — ImGui sources must match Nexus's version.
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(APIDefs->ImguiContext));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void*(*)(size_t, void*)>(APIDefs->ImguiMalloc),
        reinterpret_cast<void(*)(void*, void*)>(APIDefs->ImguiFree));

    // Load settings
    const char* addonDir = APIDefs->Paths_GetAddonDirectory("FarmingTracker");
    APIDefs->Log(LOGL_INFO, "FarmingTracker", addonDir ? addonDir : "NULL");
    SettingsManager::Init(addonDir);

    // Initialize localization with saved language
    Localization::SetLanguage(Localization::StringToLanguage(g_Settings.language));

    // Initialize icon disk-cache
    UICommon::InitIconCache(addonDir);
    UITabIcons::Init(addonDir);

    // Initialize info panel icons
    UIInfo::Init(addonDir);

    // Initialize session history BEFORE auto-reset (so sessions can be saved on reset)
    SessionHistory::Init(addonDir);
    SessionHistory::SetEnabled(g_Settings.enableSessionHistory);
    SessionHistory::SetMaxSessions(g_Settings.maxSessionHistory);
    SessionHistory::SetSaveAllItems(true); // Always save timeline data

    // Load persisted farming data FIRST before any reset logic
    ItemTracker::LoadData(addonDir);

    AutoReset::OnAddonLoad();

    DrfClient::Init([](DrfStatus s) { /* Status change callback - unused */ });

    // Load token from active account
    std::string activeDrfToken;
    if (!g_Settings.accounts.empty() && g_Settings.currentAccountIndex >= 0
        && static_cast<size_t>(g_Settings.currentAccountIndex) < g_Settings.accounts.size())
    {
        activeDrfToken = g_Settings.accounts[g_Settings.currentAccountIndex].drfToken;
        // Also update legacy fields for backwards compatibility
        g_Settings.drfToken = activeDrfToken;
        g_Settings.gw2ApiKey = g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey;
    }
    else
    {
        activeDrfToken = g_Settings.drfToken; // Fallback to legacy
    }

    bool tokenValid = SettingsManager::IsTokenValid(activeDrfToken);
    if (tokenValid)
        DrfClient::Connect(activeDrfToken);

    Gw2Fetcher::Init();

    // Subscribe to MumbleLink map change events for Magnetite Tracker
    APIDefs->Events_Subscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityUpdated);

    // Also read current map immediately on load (before first EV_MUMBLE_IDENTITY_UPDATED fires)
    if (APIDefs->DataLink_Get)
    {
        auto* mumble = static_cast<LinkedMem*>(APIDefs->DataLink_Get(DL_MUMBLE_LINK));
        if (mumble)
        {
            int currentMapId = static_cast<int>(mumble->context.mapId);
            APIDefs->Log(LOGL_DEBUG, "FarmingTracker", ("Initial map_id = " + std::to_string(currentMapId)).c_str());
            s_LastMapId = currentMapId;
            ItemTracker::SetCurrentMapId(currentMapId);
        }
    }
    MagnetiteTracker::Init();

    // Loot Logger — must come after SettingsManager::Load()
    {
        const char* aDir = APIDefs->Paths_GetAddonDirectory("FarmingTracker");
        LootLogger::Init(aDir ? aDir : "");
    }

    UI::Init();
    UINotifications::Init();

    APIDefs->Log(LOGL_INFO, "FarmingTracker", "Farming Tracker loaded.");
}

void AddonUnload()
{
    if (!APIDefs)
    {
        // API not available - cannot log
        return;
    }
    
    APIDefs->Log(LOGL_INFO, "FarmingTracker", "=== UNLOAD STARTED ===");

    try 
    {
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "Starting unload process...");

        UI::Shutdown();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "UI shutdown complete");

        // Unsubscribe MumbleLink event before shutting down workers
        APIDefs->Events_Unsubscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityUpdated);

        // Flush and close loot log files
        LootLogger::Shutdown();

        // Gw2Fetcher MUST be shut down before UINotifications:
        // the worker thread calls PlayNotificationSound() which accesses the
        // miniaudio engine; uninitialising audio while the worker is still
        // running causes use-after-free / crash.
        Gw2Fetcher::Shutdown();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "GW2 fetcher shutdown complete");

        DrfClient::Shutdown();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "DRF client shutdown complete");

        UINotifications::Shutdown();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "Notification system shutdown complete");
        
        AutoReset::OnAddonUnload();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "Auto reset shutdown complete");
        
        // Save current session to history before shutdown
        ItemTracker::SaveCurrentSession();
        
        // Save farming data before shutdown
        const char* addonDir = APIDefs->Paths_GetAddonDirectory("FarmingTracker");
        ItemTracker::SaveData(addonDir);
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "Farming data saved");
        
        SettingsManager::Save();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "Settings saved");

        // Nexus manages DLL lifecycle automatically - no manual cleanup needed

        APIDefs->Log(LOGL_INFO, "FarmingTracker", "=== UNLOAD COMPLETED SUCCESSFULLY ===");
    }
    catch (...)
    {
        if (APIDefs)
            APIDefs->Log(LOGL_WARNING, "FarmingTracker", "Exception during unload!");
        // Continue with cleanup even if exception occurred
    }
    
    // Clear API pointer last
    APIDefs = nullptr;
}
