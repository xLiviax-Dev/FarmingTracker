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
#include "auto_reset.h"
#include "gw2_fetcher.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "search_manager.h"
#include "localization.h"
#include "item_tracker.h"

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();

static AddonDefinition_t s_AddonDef{};

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
    s_AddonDef.Signature        = 20240001;  // Positive signature for non-Raidcore addons
    s_AddonDef.APIVersion       = NEXUS_API_VERSION;
    s_AddonDef.Name             = "Farming Tracker";
    s_AddonDef.Version.Major    = 0;
    s_AddonDef.Version.Minor    = 0;
    s_AddonDef.Version.Build    = 1;
    s_AddonDef.Version.Revision = 0;
    s_AddonDef.Author           = "Community";
    s_AddonDef.Description      = "Tracks farmed items and currencies in real-time via DRF (drf.rs).";
    s_AddonDef.Load             = AddonLoad;
    s_AddonDef.Unload           = AddonUnload;
    s_AddonDef.Flags            = AF_None;
    s_AddonDef.Provider         = UP_GitHub;
    s_AddonDef.UpdateLink       = "https://github.com/xLiviax-Dev/FarmingTracker/releases";
    return &s_AddonDef;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            // DisableThreadLibraryCalls entfernt, um korrektes DLL-Entladen zu ermöglichen
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

void AddonLoad(AddonAPI_t* aApi)
{
    if (!aApi || !aApi->ImguiContext || !aApi->ImguiMalloc || !aApi->ImguiFree)
        return;

    APIDefs = aApi;

    // Set up ImGui context (REQUIRED for rendering) — ImGui sources must match Nexus’s version.
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

    AutoReset::OnAddonLoad();

    // Load persisted farming data
    ItemTracker::LoadData(addonDir);

    DrfClient::Init([](DrfStatus s) { /* Status change callback - unused */ });

    // Load token from active account
    std::string activeDrfToken;
    if (!g_Settings.accounts.empty() && g_Settings.currentAccountIndex >= 0 && g_Settings.currentAccountIndex < g_Settings.accounts.size())
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

    if (SettingsManager::IsTokenValid(activeDrfToken))
        DrfClient::Connect(activeDrfToken);

    Gw2Fetcher::Init();

    UI::Init();

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
        
        Gw2Fetcher::Shutdown();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "GW2 fetcher shutdown complete");
        
        DrfClient::Shutdown();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "DRF client shutdown complete");
        
        AutoReset::OnAddonUnload();
        APIDefs->Log(LOGL_INFO, "FarmingTracker", "Auto reset shutdown complete");
        
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
