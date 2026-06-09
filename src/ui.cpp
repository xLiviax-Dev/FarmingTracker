// ---------------------------------------------------------------------------
// ui.cpp – ImGui rendering for the Farming Tracker Nexus addon
// Refactored: Only contains Init, Shutdown, ProcessKeybind, RenderShortcut, RenderMainWindow
// ---------------------------------------------------------------------------
#include "ui.h"
#include "ui_common.h"
#include "ui_items.h"
#include "ui_currencies.h"
#include "ui_drops.h"
#include "ui_profit.h"
#include "ui_favorites.h"
#include "ui_ignored.h"
#include "ui_filter.h"
#include "ui_loot_filter.h"
#include "ui_custom_profit.h"
#include "ui_debug.h"
#include "ui_loot_log.h"
#include "ui_settings.h"
#include "ui_session_history.h"
#include "ui_timeline.h"
#include "ui_mini_window.h"
#include "ui_notifications.h"
#include "ui_tab_icons.h"
#include "ui_info.h"
#include "shared.h"
#include "settings.h"
#include "item_tracker.h"
#include "session_history.h"
#include "drf_client.h"
#include "gw2_fetcher.h"
#include "auto_reset.h"
#include "localization.h"
#include "resource.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "../include/nexus/Nexus.h"
#include "../include/imgui/imgui.h"

void PushAccentColor()
{
    ImVec4 accentColor(g_Settings.accentColorR, g_Settings.accentColorG, g_Settings.accentColorB, 1.0f);

    // HTML design colors for ImGui tabs
    ImVec4 colTabInactive = ImVec4(0.706f, 0.373f, 0.216f, 0.28f); // rgba(180, 95, 55, 0.28)
    ImVec4 colTabHovered = ImVec4(accentColor.x * 2.0f * 0.72f, accentColor.y * 2.0f * 0.72f, accentColor.z * 2.0f * 0.72f, 1.f); // Accent-based, 30% darker
    ImVec4 colTabActive = ImVec4(accentColor.x * 2.0f, accentColor.y * 2.0f, accentColor.z * 2.0f, 1.f); // Accent-based

    ImGui::PushStyleColor(ImGuiCol_Button, accentColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accentColor.x * 1.1f, accentColor.y * 1.1f, accentColor.z * 1.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accentColor.x * 0.9f, accentColor.y * 0.9f, accentColor.z * 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, accentColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(accentColor.x * 1.1f, accentColor.y * 1.1f, accentColor.z * 1.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(accentColor.x * 0.9f, accentColor.y * 0.9f, accentColor.z * 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Tab, colTabInactive);
    ImGui::PushStyleColor(ImGuiCol_TabHovered, colTabHovered);
    ImGui::PushStyleColor(ImGuiCol_TabActive, colTabActive);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, accentColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(accentColor.x * 1.1f, accentColor.y * 1.1f, accentColor.z * 1.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(accentColor.x * 0.9f, accentColor.y * 0.9f, accentColor.z * 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, accentColor);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(accentColor.x * 1.1f, accentColor.y * 1.1f, accentColor.z * 1.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(accentColor.x * 0.9f, accentColor.y * 0.9f, accentColor.z * 0.9f, 1.0f));
}

void PopAccentColor()
{
    ImGui::PopStyleColor(15);
}

#include <string>
#include <algorithm>
#include <cstring>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Forward declaration
static void ProcessKeybind(const char* aIdentifier, bool aIsRelease);
static void RenderShortcut();

// Helper function: Loads bytes directly from the DLL
static std::vector<unsigned char> GetResourceBytes(int resourceId) {
    // Get the module handle of our own DLL
    HMODULE hMod = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&GetResourceBytes, &hMod);
    
    if (!hMod) return {};

    // Find resource (Type 10 is RT_RCDATA)
    HRSRC hRes = FindResourceA(hMod, MAKEINTRESOURCEA(resourceId), (LPCSTR)10);
    if (!hRes) return {};

    HGLOBAL hData = LoadResource(hMod, hRes);
    if (!hData) return {};
    DWORD size = SizeofResource(hMod, hRes);
    void* pData = LockResource(hData);

    if (!pData || size == 0) return {};
    
    return std::vector<unsigned char>((unsigned char*)pData, (unsigned char*)pData + size);
}

static const std::vector<std::string> DefaultMainTabOrderRuntime()
{
    return {"dashboard", "timeline", "drops", "loot_filter", "loot_log", "session_history", "custom_profit", "debug"};
}

static void EnsureMainTabOrderValidRuntime(std::vector<std::string>& order)
{
    const auto defaults = DefaultMainTabOrderRuntime();

    if (order.empty())
        order = defaults;

    order.erase(
        std::remove_if(order.begin(), order.end(),
            [&](const std::string& key)
            {
                return std::find(defaults.begin(), defaults.end(), key) == defaults.end();
            }),
        order.end());

    for (const auto& key : defaults)
    {
        if (std::find(order.begin(), order.end(), key) == order.end())
            order.push_back(key);
    }
}

static bool IsMainTabEnabled(const std::string& key)
{
    if (key == "dashboard") return g_Settings.enableDashboardTab;
    if (key == "drops") return g_Settings.enableDropsTab;
    if (key == "loot_filter") return true;
    if (key == "session_history") return g_Settings.enableSessionHistoryTab;
    if (key == "timeline") return g_Settings.enableTimelineTab;
    if (key == "custom_profit") return g_Settings.enableCustomProfit;
    if (key == "loot_log") return g_Settings.enableLootLog;
    if (key == "debug") return g_Settings.enableDebugTab;
    return false;
}

static void SafeReset()
{
    ItemTracker::SafeReset();
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
    ItemTracker::SaveData(addonDir);
}

// ---------------------------------------------------------------------------
// Info window state
// ---------------------------------------------------------------------------
static bool s_ShowInfoWindow = false;

static void RenderInfoWindow()
{
    if (!s_ShowInfoWindow) return;

    ImGui::SetNextWindowSize(ImVec2(420, 580), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(360, 400), ImVec2(600, 900));
    ImGui::SetNextWindowBgAlpha(0.97f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("FarmingTracker — About##FT_Info", &s_ShowInfoWindow, flags))
    {
        ImGui::BeginChild("##InfoScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        UIInfo::Render();
        ImGui::EndChild();
    }
    ImGui::End();
}

// ---------------------------------------------------------------------------
// Pill-Tab bar — renders the custom pill+icon tab strip.
// Returns the key of the currently active tab.
// s_ActiveTabKey persists across frames.
// ---------------------------------------------------------------------------
static std::string s_ActiveTabKey = "";
static bool        s_TabKeyRestored = false;

// Maps saved activeTab index back to key on first frame
static void RestoreActiveTabKey()
{
    if (s_TabKeyRestored) return;
    s_TabKeyRestored = true;
    static const std::pair<int,const char*> kIndexToKey[] = {
        {0, "dashboard"}, {1, "drops"}, {2, "loot_filter"},
        {3, "session_history"}, {4, "custom_profit"}, {5, "debug"},
        {6, "loot_log"}, {7, "timeline"}
    };
    for (const auto& p : kIndexToKey)
        if (p.first == g_Settings.activeTab) { s_ActiveTabKey = p.second; return; }
    s_ActiveTabKey = "dashboard"; // fallback
}

static void RenderPillTabBar(const std::vector<std::string>& order)
{
    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    auto clampBright = [](float v) { return v < 0.15f ? 0.15f : v; };

    // HTML design colors
    const ImVec4 colBarBg        = ImVec4(0.055f, 0.055f, 0.055f, 1.f); // #0e0e0e
    const ImVec4 colInactiveBgTop   = ImVec4(0.706f, 0.373f, 0.216f, 0.28f); // rgba(180, 95, 55, 0.28)
    const ImVec4 colInactiveBgBottom = ImVec4(0.412f, 0.176f, 0.098f, 0.42f); // rgba(105, 45, 25, 0.42)
    const ImVec4 colInactiveBorder = ImVec4(0.627f, 0.314f, 0.176f, 0.55f); // rgba(160, 80, 45, 0.55)
    const ImVec4 colInactiveTxt  = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colInactiveIcon = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colHoverBgTop   = ImVec4(acR * 2.0f * 0.72f, acG * 2.0f * 0.72f, acB * 2.0f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverBgMid   = ImVec4(acR * 1.2f * 0.72f, acG * 1.2f * 0.72f, acB * 1.2f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverBgBottom = ImVec4(acR * 0.5f * 0.72f, acG * 0.5f * 0.72f, acB * 0.5f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverBorder = ImVec4(acR * 1.5f * 0.72f, acG * 1.5f * 0.72f, acB * 1.5f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverTxt     = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colHoverIcon    = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colActiveBgTop   = ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.f); // Accent-based
    const ImVec4 colActiveBgMid   = ImVec4(acR * 1.2f, acG * 1.2f, acB * 1.2f, 1.f); // Accent-based
    const ImVec4 colActiveBgBottom = ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.f); // Accent-based
    const ImVec4 colActiveBorder = ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.f); // Accent-based
    const ImVec4 colActiveTxt    = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colActiveIcon   = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colDragBg       = ImVec4(clampBright(acR)*0.40f, clampBright(acG)*0.40f, clampBright(acB)*0.40f, 0.85f);
    const ImVec4 colDropLine     = ImVec4(acR, acG, acB, 1.f);

    const float barPad   = 4.f;
    const float tabPadX  = 10.f;
    const float tabPadY  = 4.f;
    const float iconSz   = 16.f;
    const float iconGap  = 5.f;
    const float tabGap   = 3.f;
    const float tabRound = 5.f;

    // Drag state (persists across frames)
    static std::string s_DragKey      = "";   // key of tab being dragged
    static int         s_DragFromIdx  = -1;   // original index in visible[]
    static int         s_DragToIdx    = -1;   // current drop target index
    static bool        s_Dragging     = false;
    static float       s_DragStartX   = 0.f;  // mouse X position when drag started

    ImDrawList* dl  = ImGui::GetWindowDrawList();
    float lineH     = ImGui::GetTextLineHeight();
    float tabH      = std::max(iconSz, lineH) + tabPadY * 2.f;
    float barH      = tabH + barPad * 2.f;
    ImVec2 mousePos = ImGui::GetMousePos();

    // Build visible tab list
    struct TabEntry { std::string key; std::string labelKey; float btnW; float minX; };
    std::vector<TabEntry> visible;
    for (const auto& key : order)
    {
        if (!IsMainTabEnabled(key)) continue;
        std::string lk;
        if      (key == "dashboard")       lk = "tab_dashboard";
        else if (key == "drops")           lk = "tab_drops";
        else if (key == "loot_filter")     lk = "tab_loot_filter";
        else if (key == "session_history") lk = "tab_session_history";
        else if (key == "timeline")        lk = "tab_timeline";
        else if (key == "custom_profit")   lk = "tab_custom_profit";
        else if (key == "loot_log")        lk = "tab_loot_log";
        else if (key == "debug")           lk = "tab_debug";
        if (lk.empty()) continue;
        float textW = ImGui::CalcTextSize(Localization::GetText(lk.c_str())).x;
        float w     = tabPadX * 2.f + iconSz + iconGap + textW;
        visible.push_back({key, lk, w, 0.f});
    }
    if (visible.empty()) return;

    // Validate active tab
    bool activeFound = false;
    for (const auto& t : visible)
        if (t.key == s_ActiveTabKey) { activeFound = true; break; }
    if (!activeFound)
        s_ActiveTabKey = visible[0].key;

    // Calculate positions
    float barW = barPad * 2.f;
    for (size_t i = 0; i < visible.size(); i++)
    {
        visible[i].minX = barW - barPad; // relative to bar start x
        barW += visible[i].btnW + (i + 1 < visible.size() ? tabGap : 0.f);
    }
    float avail = ImGui::GetContentRegionAvail().x;
    barW = std::min(barW + barPad, avail);

    ImVec2 barStart = ImGui::GetCursorScreenPos();

    // Store absolute positions
    for (auto& t : visible)
        t.minX += barStart.x + barPad;

    // --- Bar background ---
    dl->AddRectFilled(barStart,
                      ImVec2(barStart.x + barW, barStart.y + barH),
                      ImGui::ColorConvertFloat4ToU32(colBarBg),
                      tabRound + barPad);

    // --- Drag logic ---
    bool mouseDown    = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool mousePressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool mouseRel     = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    float cy = barStart.y + barPad;

    // Find which tab the mouse is over
    int hoveredIdx = -1;
    for (int i = 0; i < (int)visible.size(); i++)
    {
        float x0 = visible[i].minX;
        float x1 = x0 + visible[i].btnW;
        if (mousePos.x >= x0 && mousePos.x < x1 &&
            mousePos.y >= cy && mousePos.y < cy + tabH)
        {
            hoveredIdx = i; break;
        }
    }

    // Start drag: mouse pressed on a tab
    if (!g_Settings.lockTabOrder && mousePressed && hoveredIdx >= 0)
    {
        s_DragKey     = visible[hoveredIdx].key;
        s_DragFromIdx = hoveredIdx;
        s_DragToIdx   = hoveredIdx;
        s_Dragging    = false; // not yet dragging, wait for movement
        s_DragStartX  = mousePos.x; // remember start position
    }

    // Detect actual drag movement (> 30px threshold to prevent accidental drags)
    if (!s_DragKey.empty() && mouseDown)
    {
        float moved = std::abs(mousePos.x - s_DragStartX);
        if (moved > 30.f) s_Dragging = true;
    }

    // Update drop target index while dragging
    if (s_Dragging && mouseDown && !s_DragKey.empty())
    {
        // Find insert position: which slot the mouse center falls into
        int newIdx = (int)visible.size() - 1;
        for (int i = 0; i < (int)visible.size(); i++)
        {
            float midX = visible[i].minX + visible[i].btnW * 0.5f;
            if (mousePos.x < midX) { newIdx = i; break; }
        }
        s_DragToIdx = newIdx;
    }

    // Commit reorder on mouse release
    if (mouseRel && s_Dragging && !s_DragKey.empty() && s_DragToIdx != s_DragFromIdx)
    {
        // Apply reorder to g_Settings.mainTabOrder
        // Find dragKey in mainTabOrder and move it
        auto& mo = g_Settings.mainTabOrder;
        auto srcIt = std::find(mo.begin(), mo.end(), s_DragKey);
        if (srcIt != mo.end())
        {
            // Find the key that currently sits at s_DragToIdx in visible[]
            const std::string& targetKey = visible[s_DragToIdx].key;
            auto dstIt = std::find(mo.begin(), mo.end(), targetKey);
            if (dstIt != mo.end())
            {
                // Remove from old position, insert at new position
                std::string dragged = *srcIt;
                mo.erase(srcIt);
                dstIt = std::find(mo.begin(), mo.end(), targetKey);
                if (dstIt != mo.end())
                {
                    if (s_DragToIdx > s_DragFromIdx)
                        mo.insert(std::next(dstIt), dragged);
                    else
                        mo.insert(dstIt, dragged);
                }
                SettingsManager::Save();
            }
        }
    }

    // Clear drag state on release
    if (mouseRel)
    {
        s_DragKey     = "";
        s_DragFromIdx = -1;
        s_DragToIdx   = -1;
        s_Dragging    = false;
        s_DragStartX  = 0.f;
    }

    // --- Render tabs ---
    // First render all non-dragged tabs
    for (int i = 0; i < (int)visible.size(); i++)
    {
        const auto& tab    = visible[i];
        bool isDragged     = (s_Dragging && tab.key == s_DragKey);
        if (isDragged) continue; // Skip dragged tab, render it last

        bool isDropTarget  = (s_Dragging && i == s_DragToIdx && tab.key != s_DragKey);
        bool active        = (tab.key == s_ActiveTabKey);
        bool hovered       = (hoveredIdx == i && !s_Dragging);

        float cx  = tab.minX;
        float drawY = cy;

        ImVec2 btnMin = ImVec2(cx, drawY);
        ImVec2 btnMax = ImVec2(cx + tab.btnW, drawY + tabH);

        // Drop indicator line (before this tab)
        if (isDropTarget && s_DragToIdx < s_DragFromIdx)
        {
            dl->AddLine(ImVec2(cx - 1.f, cy - 2.f),
                        ImVec2(cx - 1.f, cy + tabH + 2.f),
                        ImGui::ColorConvertFloat4ToU32(colDropLine), 2.f);
        }

        // Background with gradient
        if (active)
        {
            ImU32 topColor = ImGui::ColorConvertFloat4ToU32(colActiveBgTop);
            ImU32 bottomColor = ImGui::ColorConvertFloat4ToU32(colActiveBgBottom);
            dl->AddRectFilledMultiColor(btnMin, btnMax, topColor, topColor, bottomColor, bottomColor);
            dl->AddRect(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colActiveBorder), tabRound, 0, 0.5f);
        }
        else if (hovered)
        {
            ImU32 topColor = ImGui::ColorConvertFloat4ToU32(colHoverBgTop);
            ImU32 bottomColor = ImGui::ColorConvertFloat4ToU32(colHoverBgBottom);
            dl->AddRectFilledMultiColor(btnMin, btnMax, topColor, topColor, bottomColor, bottomColor);
            dl->AddRect(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colHoverBorder), tabRound, 0, 0.5f);
        }
        else
        {
            ImU32 topColor = ImGui::ColorConvertFloat4ToU32(colInactiveBgTop);
            ImU32 bottomColor = ImGui::ColorConvertFloat4ToU32(colInactiveBgBottom);
            dl->AddRectFilledMultiColor(btnMin, btnMax, topColor, topColor, bottomColor, bottomColor);
            dl->AddRect(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colInactiveBorder), tabRound, 0, 0.5f);
        }

        // Icon
        float iconX = cx + tabPadX;
        float iconY = drawY + (tabH - iconSz) * 0.5f;
        void* iconTex = UITabIcons::GetIcon(tab.key);
        if (iconTex)
        {
            ImVec4 tint = active ? colActiveIcon
                        : hovered ? colHoverIcon
                        : colInactiveIcon;

            const float glowOffset = 2.0f;
            const float glowAlpha = 0.3f;
            ImU32 glowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(tint.x, tint.y, tint.z, glowAlpha));

            for (int gi = 1; gi <= 2; gi++)
            {
                float offset = glowOffset * gi;
                dl->AddImage((ImTextureID)iconTex,
                             ImVec2(iconX - offset, iconY - offset),
                             ImVec2(iconX + iconSz + offset, iconY + iconSz + offset),
                             ImVec2(0,0), ImVec2(1,1), glowColor);
            }

            dl->AddImage((ImTextureID)iconTex,
                         ImVec2(iconX, iconY),
                         ImVec2(iconX + iconSz, iconY + iconSz),
                         ImVec2(0,0), ImVec2(1,1),
                         ImGui::ColorConvertFloat4ToU32(tint));
        }

        // Label
        const ImVec4& lblCol = active ? colActiveTxt
                             : hovered ? colHoverTxt
                             : colInactiveTxt;
        float textX = iconX + iconSz + iconGap;
        float textY = drawY + (tabH - lineH) * 0.5f;
        dl->AddText(ImVec2(textX, textY),
                    ImGui::ColorConvertFloat4ToU32(lblCol),
                    Localization::GetText(tab.labelKey.c_str()));

        // Drop indicator line (after this tab)
        if (isDropTarget && s_DragToIdx >= s_DragFromIdx)
        {
            float lx = cx + tab.btnW + 1.f;
            dl->AddLine(ImVec2(lx, cy - 2.f),
                        ImVec2(lx, cy + tabH + 2.f),
                        ImGui::ColorConvertFloat4ToU32(colDropLine), 2.f);
        }

        // InvisibleButton for click (only when not dragging)
        ImGui::SetCursorScreenPos(ImVec2(cx, cy));
        ImGui::PushID(tab.key.c_str());
        ImGui::InvisibleButton("##pillbtn", ImVec2(tab.btnW, tabH));
        bool clicked = ImGui::IsItemClicked() && !s_Dragging;
        ImGui::PopID();

        if (clicked)
        {
            s_ActiveTabKey = tab.key;
            if      (tab.key == "dashboard")       g_Settings.activeTab = 0;
            else if (tab.key == "drops")           g_Settings.activeTab = 1;
            else if (tab.key == "loot_filter")     g_Settings.activeTab = 2;
            else if (tab.key == "session_history") g_Settings.activeTab = 3;
            else if (tab.key == "custom_profit")   g_Settings.activeTab = 4;
            else if (tab.key == "debug")           g_Settings.activeTab = 5;
            else if (tab.key == "loot_log")        g_Settings.activeTab = 6;
            else if (tab.key == "timeline")        g_Settings.activeTab = 7;
            SettingsManager::Save();
        }
    }

    // Render dragged tab last (foreground)
    if (s_Dragging && !s_DragKey.empty())
    {
        for (int i = 0; i < (int)visible.size(); i++)
        {
            const auto& tab = visible[i];
            if (tab.key != s_DragKey) continue;

            bool active = (tab.key == s_ActiveTabKey);

            float cx = mousePos.x - tab.btnW * 0.5f;
            float drawY = mousePos.y - tabH * 0.5f;

            ImVec2 btnMin = ImVec2(cx, drawY);
            ImVec2 btnMax = ImVec2(cx + tab.btnW, drawY + tabH);

            dl->AddRectFilled(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colDragBg), tabRound);

            float iconX = cx + tabPadX;
            float iconY = drawY + (tabH - iconSz) * 0.5f;
            void* iconTex = UITabIcons::GetIcon(tab.key);
            if (iconTex)
            {
                ImVec4 tint = colActiveTxt;
                dl->AddImage((ImTextureID)iconTex,
                             ImVec2(iconX, iconY),
                             ImVec2(iconX + iconSz, iconY + iconSz),
                             ImVec2(0,0), ImVec2(1,1),
                             ImGui::ColorConvertFloat4ToU32(tint));
            }

            float textX = iconX + iconSz + iconGap;
            float textY = drawY + (tabH - lineH) * 0.5f;
            dl->AddText(ImVec2(textX, textY),
                        ImGui::ColorConvertFloat4ToU32(colActiveTxt),
                        Localization::GetText(tab.labelKey.c_str()));

            break;
        }
    }

    // Cursor past bar
    ImGui::SetCursorScreenPos(ImVec2(barStart.x, barStart.y + barH + 4.f));
}

static void RenderAutoResetTick()
{
    AutoReset::Tick();
}

static void RenderMainWindow()
{
    if (!g_Settings.showMainWindow) return;

    // Render the standalone info window (independent of main window)
    RenderInfoWindow();

    ImGui::SetNextWindowPos(ImVec2(g_Settings.mainWindowPosX, g_Settings.mainWindowPosY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(g_Settings.mainWindowWidth, g_Settings.mainWindowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 220), ImVec2(3840, 2160));
    ImGui::SetNextWindowBgAlpha(g_Settings.mainWindowOpacity);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (g_Settings.mainWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Farming Tracker##FT_Main", &g_Settings.showMainWindow, flags))
    {
        PushAccentColor();

        // Gradient background if enabled
        if (g_Settings.enableGradientBackgrounds && !g_Settings.disableComplexVisualsOnLowPerf)
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

        // --- Top Bar: Pill 1 (Session Duration) | Pill 2 (Last Reset) | Pill 3 (Reset Button) ---
        {
            auto duration = ItemTracker::GetSessionDuration();
            std::string sessionLabel = Localization::GetText("session_duration_label_simple");
            std::string durationStr = sessionLabel + " " + UICommon::FormatDuration(duration.count());

            static std::chrono::steady_clock::time_point s_LastManualResetTime = std::chrono::steady_clock::now();
            auto timeSinceReset = std::chrono::steady_clock::now() - s_LastManualResetTime;
            auto minutesSinceReset = std::chrono::duration_cast<std::chrono::minutes>(timeSinceReset).count();
            auto secondsSinceReset = std::chrono::duration_cast<std::chrono::seconds>(timeSinceReset).count();
            std::string lastResetStr;
            if (secondsSinceReset < 60)
                lastResetStr = std::to_string(secondsSinceReset) + "s";
            else if (minutesSinceReset < 60)
                lastResetStr = std::to_string(minutesSinceReset) + "m";
            else
                lastResetStr = std::to_string(minutesSinceReset / 60) + "h " + std::to_string(minutesSinceReset % 60) + "m";

            std::string nextResetDisplay = AutoReset::GetNextResetDisplayUtc();
            std::string nextResetStr = nextResetDisplay;

            ImDrawList* dl     = ImGui::GetWindowDrawList();
            ImVec2      cursor = ImGui::GetCursorScreenPos();
            float       padX   = 8.0f;
            float       padY   = 3.0f;
            float       rounding = 10.0f;
            float       spacing  = 6.0f;
            ImVec2      textSize1 = ImGui::CalcTextSize(durationStr.c_str());
            std::string lastResetLabel = std::string(Localization::GetText("next_reset_label_simple")) + " " + nextResetStr;
            ImVec2      textSize2 = ImGui::CalcTextSize(lastResetLabel.c_str());
            std::string resetLabel = Localization::GetText("reset_button");
            ImVec2      textSize3 = ImGui::CalcTextSize(resetLabel.c_str());
            float       pillH    = textSize1.y + padY * 2.0f;

            ImU32 colGreenTop    = IM_COL32(42, 154, 42,  255);
            ImU32 colGreenBot    = IM_COL32(10, 58,  10,  255);
            ImU32 colGreenTopHov = IM_COL32(60, 180, 60,  255);
            ImU32 colGreenBotHov = IM_COL32(20, 80,  20,  255);
            ImU32 colGreenBorder = IM_COL32(26, 107, 26,  255);
            ImU32 colGreenBar    = IM_COL32(42, 154, 42,  255);

            // --- Pill 1: Session Duration ---
            float x = cursor.x;
            float y = cursor.y;
            ImVec2 p1Min = ImVec2(x, y);
            ImVec2 p1Max = ImVec2(x + textSize1.x + padX * 2.0f, y + pillH);
            ImGui::SetCursorScreenPos(p1Min);
            ImGui::InvisibleButton("##pill_duration", ImVec2(p1Max.x - p1Min.x, pillH));
            bool p1Hovered = ImGui::IsItemHovered();
            dl->AddRectFilledMultiColor(p1Min, p1Max,
                p1Hovered ? colGreenTopHov : colGreenTop, p1Hovered ? colGreenTopHov : colGreenTop,
                p1Hovered ? colGreenBotHov : colGreenBot, p1Hovered ? colGreenBotHov : colGreenBot);
            dl->AddRect(p1Min, p1Max, colGreenBorder, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(p1Min.x, p1Min.y), ImVec2(p1Min.x + 3.f, p1Max.y), colGreenBar, 2.f);
            dl->AddText(ImVec2(p1Min.x + padX + 3.f, p1Min.y + padY), IM_COL32(221,221,221,255), durationStr.c_str());
            if (p1Hovered)
                ImGui::SetTooltip("%s", Localization::GetText("session_duration_tooltip"));

            // --- Pill 2: Next Reset ---
            float p2X = p1Max.x + spacing;
            ImVec2 p2Min = ImVec2(p2X, y);
            ImVec2 p2Max = ImVec2(p2X + textSize2.x + padX * 2.0f, y + pillH);

            if (g_Settings.automaticResetMode != 1)
            {
                ImGui::SetCursorScreenPos(p2Min);
                ImGui::InvisibleButton("##pill_lastreset", ImVec2(p2Max.x - p2Min.x, pillH));
                bool p2Hovered = ImGui::IsItemHovered();
                ImU32 colBlueTop    = IM_COL32(40,  100, 180, 255);
                ImU32 colBlueBot    = IM_COL32(20,  50,  90,  255);
                ImU32 colBlueTopHov = IM_COL32(60,  120, 200, 255);
                ImU32 colBlueBotHov = IM_COL32(30,  70,  130, 255);
                ImU32 colBlueBorder = IM_COL32(30,  80,  150, 255);
                ImU32 colBlueBar    = IM_COL32(40,  100, 180, 255);
                dl->AddRectFilledMultiColor(p2Min, p2Max,
                    p2Hovered ? colBlueTopHov : colBlueTop, p2Hovered ? colBlueTopHov : colBlueTop,
                    p2Hovered ? colBlueBotHov : colBlueBot, p2Hovered ? colBlueBotHov : colBlueBot);
                dl->AddRect(p2Min, p2Max, colBlueBorder, 4.f, 0, 0.5f);
                dl->AddRectFilled(ImVec2(p2Min.x, p2Min.y), ImVec2(p2Min.x + 3.f, p2Max.y), colBlueBar, 2.f);
                dl->AddText(ImVec2(p2Min.x + padX + 3.f, p2Min.y + padY), IM_COL32(255,255,255,255), lastResetLabel.c_str());
                if (p2Hovered)
                    ImGui::SetTooltip("%s", Localization::GetText("next_reset_tooltip"));
            }

            // --- Pill 3: Reset Button ---
            x = (g_Settings.automaticResetMode != 1) ? p2Max.x + spacing : p1Max.x + spacing;
            ImVec2 p3Min = ImVec2(x, y);
            ImVec2 p3Max = ImVec2(x + textSize3.x + padX * 2.0f, y + pillH);
            ImGui::SetCursorScreenPos(p3Min);
            ImGui::InvisibleButton("##pill_reset", ImVec2(p3Max.x - p3Min.x, pillH));
            bool hovered = ImGui::IsItemHovered();
            bool clicked = ImGui::IsItemClicked();
            ImU32 colRedTop    = IM_COL32(192, 48,  48,  255);
            ImU32 colRedBot    = IM_COL32(80,  10,  10,  255);
            ImU32 colRedTopHov = IM_COL32(220, 70,  70,  255);
            ImU32 colRedBotHov = IM_COL32(110, 20,  20,  255);
            ImU32 colRedBorder = IM_COL32(122, 26,  26,  255);
            ImU32 colRedBar    = IM_COL32(192, 48,  48,  255);
            dl->AddRectFilledMultiColor(p3Min, p3Max,
                hovered ? colRedTopHov : colRedTop, hovered ? colRedTopHov : colRedTop,
                hovered ? colRedBotHov : colRedBot, hovered ? colRedBotHov : colRedBot);
            dl->AddRect(p3Min, p3Max, colRedBorder, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(p3Min.x, p3Min.y), ImVec2(p3Min.x + 3.f, p3Max.y), colRedBar, 2.f);
            dl->AddText(ImVec2(p3Min.x + padX + 3.f, p3Min.y + padY), IM_COL32(255,220,220,255), Localization::GetText("reset_button"));
            if (hovered)
                ImGui::SetTooltip("%s", Localization::GetText("reset_tooltip"));
            if (clicked)
            {
                s_LastManualResetTime = std::chrono::steady_clock::now();
                SafeReset();
                AutoReset::OnManualReset();
            }

            // Account Dropdown
            if (!g_Settings.accounts.empty())
            {
                ImGui::SameLine(0, 16.f);
                std::vector<const char*> accountNames;
                for (const auto& acc : g_Settings.accounts)
                    accountNames.push_back(acc.name.c_str());

                int currentAccountIndex = g_Settings.currentAccountIndex;
                if (currentAccountIndex < 0 || currentAccountIndex >= g_Settings.accounts.size())
                    currentAccountIndex = 0;

                ImGui::SetNextItemWidth(120.f);
                if (ImGui::Combo("##AccountSelectMain", &currentAccountIndex, accountNames.data(), static_cast<int>(accountNames.size())))
                {
                    if (currentAccountIndex != g_Settings.currentAccountIndex)
                    {
                        {
                            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                            g_Settings.currentAccountIndex = currentAccountIndex;
                            if (!g_Settings.accounts.empty() && g_Settings.currentAccountIndex >= 0 && g_Settings.currentAccountIndex < g_Settings.accounts.size())
                            {
                                g_Settings.drfToken = g_Settings.accounts[g_Settings.currentAccountIndex].drfToken;
                                g_Settings.gw2ApiKey = g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey;
                            }
                        }
                        SettingsManager::Save();
                        DrfClient::Connect(g_Settings.drfToken);
                        Gw2Fetcher::UpdateApiKey();
                    }
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + pillH + 4.0f));
        }
        ImGui::Spacing();

        // --- Tab content height ---
        float statusBarHeight   = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
        float windowHeight      = ImGui::GetWindowHeight();
        float windowPaddingY    = ImGui::GetStyle().WindowPadding.y;

        // --- Pill Tab Bar ---
        std::vector<std::string> order = g_Settings.mainTabOrder;
        EnsureMainTabOrderValidRuntime(order);
        RestoreActiveTabKey();
        RenderPillTabBar(order);
        ImGui::Spacing();

        float tabContentHeight = windowHeight - statusBarHeight - windowPaddingY * 2.f
                               - ImGui::GetCursorPosY();

        // --- Tab Content ---
        const std::string& key = s_ActiveTabKey;
        const char* childId = nullptr;
        if      (key == "dashboard")       childId = "DashboardScroll";
        else if (key == "drops")           childId = "DropsScroll";
        else if (key == "loot_filter")     childId = "LootFilterScroll";
        else if (key == "session_history") childId = "SessionHistoryScroll";
        else if (key == "timeline")        childId = "TimelineScroll";
        else if (key == "custom_profit")   childId = "CustomProfitScroll";
        else if (key == "loot_log")        childId = "LootLogScroll";
        else if (key == "debug")           childId = "DebugScroll";

        if (childId && ImGui::BeginChild(childId, ImVec2(0, tabContentHeight), false))
        {
            if      (key == "dashboard")       UIProfit::RenderProfitTab();
            else if (key == "drops")           UIDrops::RenderDropsTab();
            else if (key == "loot_filter")     UILootFilter::RenderLootFilterTab();
            else if (key == "session_history")
            {
                if (g_Settings.enableSessionHistory)
                    UISessionHistory::RenderSessionHistoryTab();
                else
                    ImGui::TextDisabled("%s", Localization::GetText("enable_session_history"));
            }
            else if (key == "timeline")        UITimeline::RenderTimelineTab();
            else if (key == "custom_profit")   UICustomProfit::RenderCustomProfitTab();
            else if (key == "loot_log")        UILootLog::RenderLootLogTab();
            else if (key == "debug")           UIDebug::RenderDebugTab();
            ImGui::EndChild();
        }

        // --- Status Bar ---
        ImGui::SetCursorPosY(windowHeight - statusBarHeight - windowPaddingY);
        ImGui::Separator();
        
        ImGui::BeginGroup();
        
        // DRF Status
        DrfStatus drfStatus = DrfClient::GetStatus();
        ImGui::TextColored(UICommon::StatusColor(drfStatus), "DRF: %s", UICommon::StatusText(drfStatus));
        
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // GW2 Status
        const char* gw2StatusText = Localization::GetText("status_unknown");
        ImVec4 gw2StatusColor = ImVec4(1.f, 1.f, 1.f, 1.f);
        switch (Gw2Fetcher::GetStatus())
        {
            case Gw2Fetcher::Gw2Status::Disconnected: gw2StatusText = Localization::GetText("status_disconnected"); gw2StatusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.f); break;
            case Gw2Fetcher::Gw2Status::Connecting: gw2StatusText = Localization::GetText("status_connecting"); gw2StatusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.f); break;
            case Gw2Fetcher::Gw2Status::Connected: gw2StatusText = Localization::GetText("status_connected"); gw2StatusColor = ImVec4(0.1f, 0.8f, 0.1f, 1.f); break;
            case Gw2Fetcher::Gw2Status::Error: gw2StatusText = Localization::GetText("status_error"); gw2StatusColor = ImVec4(0.8f, 0.1f, 0.1f, 1.f); break;
        }
        ImGui::TextColored(gw2StatusColor, "GW2: %s", gw2StatusText);

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // Info Button — opens the About window
        if (UICommon::OrangeGradientButton(Localization::GetText("info_button"), "##info_btn"))
        {
            s_ShowInfoWindow = !s_ShowInfoWindow;
        }

        ImGui::EndGroup();

        PopAccentColor();
        ImGui::End();
    }
}

static void RenderShortcut()
{
    ImGui::Checkbox(Localization::GetText("show_main_window"), &g_Settings.showMainWindow);
    ImGui::Checkbox(Localization::GetText("show_mini_window"), &g_Settings.showMiniWindow);
}

static void ProcessKeybind(const char* aIdentifier, bool aIsRelease)
{
    if (!aIsRelease && strcmp(aIdentifier, "FT_TOGGLE_MAIN") == 0)
    {
        g_Settings.showMainWindow = !g_Settings.showMainWindow;
        SettingsManager::Save();
    }
    if (!aIsRelease && strcmp(aIdentifier, "FT_TOGGLE_MINI") == 0)
    {
        g_Settings.showMiniWindow = !g_Settings.showMiniWindow;
        SettingsManager::Save();
    }
    if (!aIsRelease && strcmp(aIdentifier, "FT_RESET") == 0)
    {
        SafeReset();
        AutoReset::OnManualReset();
    }
}

void UI::Init()
{
    if (!APIDefs) return;

    std::vector<unsigned char> iconBytes = GetResourceBytes(IDB_ICON_FARMINGTRACKER);

    if (!iconBytes.empty()) {
        APIDefs->Textures_GetOrCreateFromMemory("ICON_FT", iconBytes.data(), iconBytes.size());
        APIDefs->Textures_GetOrCreateFromMemory("ICON_FT_HOVER", iconBytes.data(), iconBytes.size());
    }

    APIDefs->GUI_Register(RT_Render, RenderAutoResetTick);
    APIDefs->GUI_Register(RT_Render, RenderMainWindow);
    APIDefs->GUI_Register(RT_Render, UIMiniWindow::RenderMiniWindow);
    APIDefs->GUI_Register(RT_Render, UINotifications::Render);
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
    APIDefs->InputBinds_RegisterWithString("FT_RESET", ProcessKeybind, g_Settings.resetHotkey.c_str());
}

void UI::Shutdown()
{
    if (!APIDefs) return;

    APIDefs->GUI_Deregister(RenderAutoResetTick);
    APIDefs->GUI_Deregister(RenderMainWindow);
    APIDefs->GUI_Deregister(UIMiniWindow::RenderMiniWindow);
    APIDefs->GUI_Deregister(UINotifications::Render);
    APIDefs->GUI_Deregister(UISettings::RenderOptions);
    APIDefs->QuickAccess_Remove("QA_FT");
    APIDefs->QuickAccess_RemoveContextMenu("QAS_FT");
    APIDefs->InputBinds_Deregister("FT_TOGGLE_MAIN");
    APIDefs->InputBinds_Deregister("FT_TOGGLE_MINI");
}
