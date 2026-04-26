// ---------------------------------------------------------------------------
// ui.cpp – ImGui rendering for the Farming Tracker Nexus addon
// Refactored: Only contains Init, Shutdown, ProcessKeybind, RenderShortcut, RenderMainWindow
// ---------------------------------------------------------------------------
#include "ui.h"
#include "ui_common.h"
#include "ui_summary.h"
#include "ui_items.h"
#include "ui_currencies.h"
#include "ui_profit.h"
#include "ui_favorites.h"
#include "ui_ignored.h"
#include "ui_filter.h"
#include "ui_custom_profit.h"
#include "ui_debug.h"
#include "ui_settings.h"
#include "ui_mini_window.h"
#include "shared.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "gw2_fetcher.h"
#include "auto_reset.h"
#include "localization.h"
#include "../include/nexus/Nexus.h"
#include "../include/imgui/imgui.h"

#include <string>
#include <algorithm>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Forward declaration
static void ProcessKeybind(const char* aIdentifier, bool aIsRelease);
static void RenderShortcut();

static void RenderMainWindow()
{
    if (!UICommon::s_ShowMainWindow) return;

    AutoReset::Tick();

    ImGui::SetNextWindowPos(ImVec2(g_Settings.mainWindowPosX, g_Settings.mainWindowPosY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(g_Settings.mainWindowWidth, g_Settings.mainWindowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 220), ImVec2(3840, 2160));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (g_Settings.mainWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Farming Tracker##FT_Main", &UICommon::s_ShowMainWindow, flags))
    {
        // Gradient background if enabled
        if (g_Settings.enableGradientBackgrounds)
        {
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            ImVec4 topColor = ImVec4(g_Settings.gradientTopColor[0], g_Settings.gradientTopColor[1], g_Settings.gradientTopColor[2], g_Settings.gradientTopColor[3]);
            ImVec4 bottomColor = ImVec4(g_Settings.gradientBottomColor[0], g_Settings.gradientBottomColor[1], g_Settings.gradientBottomColor[2], g_Settings.gradientBottomColor[3]);

            drawList->AddRectFilledMultiColor(
                windowPos,
                ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                ImGui::ColorConvertFloat4ToU32(topColor),
                ImGui::ColorConvertFloat4ToU32(topColor),
                ImGui::ColorConvertFloat4ToU32(bottomColor),
                ImGui::ColorConvertFloat4ToU32(bottomColor)
            );
        }

        // Save window position and size
        ImVec2 pos = ImGui::GetWindowPos();
        g_Settings.mainWindowPosX = pos.x;
        g_Settings.mainWindowPosY = pos.y;
        ImVec2 size = ImGui::GetWindowSize();
        g_Settings.mainWindowWidth = size.x;
        g_Settings.mainWindowHeight = size.y;

        DrfStatus status = DrfClient::GetStatus();
        ImGui::TextColored(UICommon::StatusColor(status), "DRF: %s", UICommon::StatusText(status));
        ImGui::SameLine();

        const char* gw2StatusText = Localization::GetText("status_unknown");
        ImVec4 gw2StatusColor = ImVec4(1.f, 1.f, 1.f, 1.f);
        switch (Gw2Fetcher::GetStatus())
        {
            case Gw2Fetcher::Gw2Status::Disconnected: gw2StatusText = Localization::GetText("status_disconnected"); gw2StatusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.f); break;
            case Gw2Fetcher::Gw2Status::Connected: gw2StatusText = Localization::GetText("status_connected"); gw2StatusColor = ImVec4(0.1f, 0.8f, 0.1f, 1.f); break;
            case Gw2Fetcher::Gw2Status::Error: gw2StatusText = Localization::GetText("status_error"); gw2StatusColor = ImVec4(0.8f, 0.1f, 0.1f, 1.f); break;
        }
        ImGui::TextColored(gw2StatusColor, "GW2: %s", gw2StatusText);
        ImGui::SameLine();

        // Session Duration
        auto duration = ItemTracker::GetSessionDuration();
        ImGui::Text("Time: %s", UICommon::FormatDuration(duration.count()).c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Current farming session duration");
        ImGui::SameLine();

        if (ImGui::Button(Localization::GetText("reset_button")))
        {
            ItemTracker::Reset();
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
            ItemTracker::ClearPersistedData(addonDir);
            AutoReset::OnManualReset();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(Localization::GetText("reset_tooltip"));

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::BeginTabBar("FT_Tabs"))
        {
            if (ImGui::BeginTabItem(Localization::GetText("tab_summary")))
            {
                g_Settings.activeTab = 0;
                UISummary::RenderSummaryTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(Localization::GetText("tab_items")))
            {
                g_Settings.activeTab = 1;
                UIItems::RenderItemsTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(Localization::GetText("tab_currencies")))
            {
                g_Settings.activeTab = 2;
                UICurrencies::RenderCurrenciesTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(Localization::GetText("tab_profit")))
            {
                g_Settings.activeTab = 3;
                UIProfit::RenderProfitTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(Localization::GetText("tab_favorites")))
            {
                g_Settings.activeTab = 4;
                UIFavorites::RenderFavoritesTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(Localization::GetText("tab_ignored")))
            {
                g_Settings.activeTab = 5;
                UIIgnored::RenderIgnoredTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(Localization::GetText("tab_filter")))
            {
                g_Settings.activeTab = 6;
                UIFilter::RenderFilterTab();
                ImGui::EndTabItem();
            }

            if (g_Settings.enableCustomProfit && ImGui::BeginTabItem(Localization::GetText("tab_custom_profit")))
            {
                g_Settings.activeTab = 7;
                UICustomProfit::RenderCustomProfitTab();
                ImGui::EndTabItem();
            }

            if (g_Settings.enableDebugTab && ImGui::BeginTabItem(Localization::GetText("tab_debug")))
            {
                g_Settings.activeTab = 8;
                UIDebug::RenderDebugTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}

static void RenderShortcut()
{
    ImGui::Checkbox("Show Farming Tracker", &UICommon::s_ShowMainWindow);
}

static void ProcessKeybind(const char* aIdentifier, bool aIsRelease)
{
    if (!aIsRelease && strcmp(aIdentifier, "FT_TOGGLE_MAIN") == 0)
        UICommon::s_ShowMainWindow = !UICommon::s_ShowMainWindow;
    if (!aIsRelease && strcmp(aIdentifier, "FT_TOGGLE_MINI") == 0)
        g_Settings.showMiniWindow = !g_Settings.showMiniWindow;
}

void UI::Init()
{
    if (!APIDefs) return;

    const char* dir = APIDefs->Paths_GetAddonDirectory("FarmingTracker");
    if (dir)
    {
        std::string qa  = std::string(dir) + "\\qa_icon.png";
        std::string qah = std::string(dir) + "\\qa_icon_hover.png";
        if (GetFileAttributesA(qa.c_str()) != INVALID_FILE_ATTRIBUTES)
            APIDefs->Textures_GetOrCreateFromFile("ICON_FT", qa.c_str());
        if (GetFileAttributesA(qah.c_str()) != INVALID_FILE_ATTRIBUTES)
            APIDefs->Textures_GetOrCreateFromFile("ICON_FT_HOVER", qah.c_str());
    }

    APIDefs->GUI_Register(RT_Render, RenderMainWindow);
    APIDefs->GUI_Register(RT_Render, UIMiniWindow::RenderMiniWindow);
    APIDefs->GUI_Register(RT_OptionsRender, UISettings::RenderOptions);

    APIDefs->QuickAccess_Add(
        "QA_FT",
        "ICON_FT",
        "ICON_FT_HOVER",
        "FT_TOGGLE_MAIN",
        "Farming Tracker");

    APIDefs->QuickAccess_AddContextMenu("QAS_FT", "QA_FT", RenderShortcut);

    APIDefs->InputBinds_RegisterWithString("FT_TOGGLE_MAIN", ProcessKeybind, g_Settings.toggleHotkey.c_str());
    APIDefs->InputBinds_RegisterWithString("FT_TOGGLE_MINI", ProcessKeybind, g_Settings.miniWindowToggleHotkey.c_str());
}

void UI::Shutdown()
{
    if (!APIDefs) return;

    APIDefs->GUI_Deregister(RenderMainWindow);
    APIDefs->GUI_Deregister(UIMiniWindow::RenderMiniWindow);
    APIDefs->GUI_Deregister(UISettings::RenderOptions);
    APIDefs->QuickAccess_Remove("QA_FT");
    APIDefs->QuickAccess_RemoveContextMenu("QAS_FT");
    APIDefs->InputBinds_Deregister("FT_TOGGLE_MAIN");
    APIDefs->InputBinds_Deregister("FT_TOGGLE_MINI");
}
