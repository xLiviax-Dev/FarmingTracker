#include "ui_drops.h"
#include "ui_overview.h"
#include "ui_items.h"
#include "ui_currencies.h"
#include "ui_tab_icons.h"
#include "settings.h"
#include "localization.h"
#include "shared.h"
#include <string>
#include <windows.h>
#include <shellapi.h>
#include <process.h>

namespace UIDrops
{
static int s_SubTab = 0; // 0 = Items, 1 = Currencies, 2 = Settings

// Toggle switch helper
static bool Toggle(const char* id, bool* value)
{
    const float  w      = 36.f;
    const float  h      = 18.f;
    const float  r      = h * 0.5f;
    const float  knobR  = r - 2.f;
    const ImVec4 colOn  = ImVec4(0.165f, 0.604f, 0.165f, 1.f); // Green when active
    const ImVec4 colOff = ImVec4(0.333f, 0.333f, 0.333f, 1.f);

    ImVec2 pos  = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton(id, ImVec2(w, h));
    bool changed = false;
    if (ImGui::IsItemClicked())
    {
        *value  = !(*value);
        changed = true;
    }

    ImVec4 trackCol = *value ? colOn : colOff;
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
        ImGui::ColorConvertFloat4ToU32(trackCol), r);

    float knobX = *value ? (pos.x + w - r) : (pos.x + r);
    dl->AddCircleFilled(ImVec2(knobX, pos.y + r), knobR,
        IM_COL32(255, 255, 255, 255));

    return changed;
}

// Render toggle without click handling (for use with InvisibleButton)
static void RenderToggleOnly(const char* id, bool* value)
{
    const float  w      = 36.f;
    const float  h      = 18.f;
    const float  r      = h * 0.5f;
    const float  knobR  = r - 2.f;
    const ImVec4 colOn  = ImVec4(0.165f, 0.604f, 0.165f, 1.f); // Green when active
    const ImVec4 colOff = ImVec4(0.333f, 0.333f, 0.333f, 1.f);

    ImVec2 pos  = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec4 trackCol = *value ? colOn : colOff;
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
        ImGui::ColorConvertFloat4ToU32(trackCol), r);

    float knobX = *value ? (pos.x + w - r) : (pos.x + r);
    dl->AddCircleFilled(ImVec2(knobX, pos.y + r), knobR,
        IM_COL32(255, 255, 255, 255));
}

static bool SettingsToggleRow(const char* label, bool* value, float colWidth, bool indent = false)
{
    if (indent)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            p, ImVec2(p.x + 2.f, p.y + 20.f),
            IM_COL32(80, 80, 80, 255));
        ImGui::SetCursorScreenPos(ImVec2(p.x + 8.f, p.y));
    }

    float toggleW = 36.f;
    float labelW  = colWidth - toggleW - 12.f;

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(labelW);
    ImGui::TextUnformatted(label);
    ImGui::EndGroup();

    ImGui::SameLine(colWidth - toggleW - (indent ? 8.f : 0.f));

    char toggleId[64];
    snprintf(toggleId, sizeof(toggleId), "##tog_%s", label);
    bool changed = Toggle(toggleId, value);

    ImGui::Separator();
    return changed;
}

static void ColHeader(const char* label)
{
    ImGui::TextUnformatted(label);
    ImGui::Separator();
    ImGui::Spacing();
}

static void PathRow(const char* sublabel, const std::string& effectivePath, float panelW)
{
    float btnW   = 100.f;
    float gap    = 4.f;
    float inputW = panelW - btnW - gap;

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextUnformatted(sublabel);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    ImGui::SetNextItemWidth(inputW);
    char buf[1024];
    strncpy_s(buf, effectivePath.c_str(), sizeof(buf) - 1);
    ImGui::InputText(("##path_" + std::string(sublabel)).c_str(),
        buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();

    ImGui::SameLine(0, gap);
    if (ImGui::Button(("Open folder##" + std::string(sublabel)).c_str(), { btnW, 0 }))
    {
        std::string cmd = "explorer.exe \"" + effectivePath + "\"";
        system(cmd.c_str());
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("open_folder_tooltip"));

    ImGui::Spacing();
}

// Draws an icon from the tab icon system at current cursor position inline
static void InlineIcon(const char* key, float sz = 14.f)
{
    void* tex = UITabIcons::GetIcon(key);
    if (!tex) return;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float lineH = ImGui::GetTextLineHeight();
    float offY  = (lineH - sz) * 0.5f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Glow effect - draw icon multiple times with offset and alpha
    const float glowOffset = 2.0f;
    const float glowAlpha = 0.3f;
    dl->AddImage(
        (ImTextureID)tex,
        ImVec2(pos.x - glowOffset, pos.y + offY - glowOffset),
        ImVec2(pos.x + sz + glowOffset, pos.y + offY + sz + glowOffset),
        ImVec2(0,0), ImVec2(1,1),
        IM_COL32(255, 255, 255, (int)(255 * glowAlpha)));
    dl->AddImage(
        (ImTextureID)tex,
        ImVec2(pos.x - glowOffset * 0.5f, pos.y + offY - glowOffset * 0.5f),
        ImVec2(pos.x + sz + glowOffset * 0.5f, pos.y + offY + sz + glowOffset * 0.5f),
        ImVec2(0,0), ImVec2(1,1),
        IM_COL32(255, 255, 255, (int)(255 * glowAlpha * 0.6f)));

    // Main icon - white
    dl->AddImage(
        (ImTextureID)tex,
        ImVec2(pos.x, pos.y + offY),
        ImVec2(pos.x + sz, pos.y + offY + sz),
        ImVec2(0,0), ImVec2(1,1),
        IM_COL32(255, 255, 255, 255));
    ImGui::Dummy(ImVec2(sz + 5.f, lineH));
    ImGui::SameLine(0, 0);
}

// Card header with white icon and accent-colored background (matching active main tab design)
static void CardHeader(const char* iconKey, const char* label)
{
    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 region = ImGui::GetContentRegionAvail();
    float headerHeight = 32.f;

    // Background gradient (matching active main tab design)
    ImU32 bgColorTop = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
    ImU32 bgColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 headerMin = cursor;
    ImVec2 headerMax = ImVec2(cursor.x + region.x, cursor.y + headerHeight);

    dl->AddRectFilledMultiColor(headerMin, headerMax, bgColorTop, bgColorTop, bgColorBottom, bgColorBottom);

    // Border (matching active tab border)
    ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));
    dl->AddRect(headerMin, headerMax, borderColor, 4.0f, 0, 0.5f);

    // Icon - white
    void* tex = UITabIcons::GetIcon(iconKey);
    float sz  = 14.f;
    float lineH = ImGui::GetTextLineHeight();

    if (tex)
    {
        float offY = (lineH - sz) * 0.5f;
        dl->AddImage(
            (ImTextureID)tex,
            ImVec2(headerMin.x + 8.f, headerMin.y + (headerHeight - sz) * 0.5f),
            ImVec2(headerMin.x + 8.f + sz, headerMin.y + (headerHeight - sz) * 0.5f + sz),
            ImVec2(0,0), ImVec2(1,1),
            IM_COL32(255, 255, 255, 255));
    }

    // Label - white
    dl->AddText(ImVec2(headerMin.x + 26.f, headerMin.y + (headerHeight - lineH) * 0.5f),
                IM_COL32(255, 255, 255, 255), label);

    ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y + 4.f));
}

// Toggle row with a small icon, label, optional indent, and toggle on the right
static bool IconToggleRow(const char* iconKey, const char* label, bool* value,
                          float colWidth, bool indent = false)
{
    const float iconSz   = 14.f;
    const float iconGap  = 5.f;
    const float toggleW  = 36.f;
    const float toggleH  = 18.f;
    const float indentW  = indent ? 16.f : 0.f;
    const float padX     = 8.0f;
    const float padY     = 4.0f;

    float curX = ImGui::GetCursorPosX();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float rowH = ImGui::GetTextLineHeight() + padY * 2.0f;

    // Draw gradient background with border and left accent bar (matching Filter tab design)
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    // Gradient background colors (matching Filter tab design, 70% active, 16% inactive)
    ImU32 bgTopOn    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.7f));
    ImU32 bgBotOn    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.25f, acG * 0.25f, acB * 0.25f, 0.7f));
    ImU32 bgTopOff   = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.16f));
    ImU32 bgBotOff   = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.25f, acG * 0.25f, acB * 0.25f, 0.16f));
    ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.75f, acG * 0.75f, acB * 0.75f, 0.5f));
    ImU32 accentBar  = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.8f));

    float rounding = 4.f;

    // Gradient background
    if (*value)
    {
        dl->AddRectFilledMultiColor(cursorPos, ImVec2(cursorPos.x + colWidth, cursorPos.y + rowH),
            bgTopOn, bgTopOn, bgBotOn, bgBotOn);
    }
    else
    {
        dl->AddRectFilledMultiColor(cursorPos, ImVec2(cursorPos.x + colWidth, cursorPos.y + rowH),
            bgTopOff, bgTopOff, bgBotOff, bgBotOff);
    }

    // Border
    dl->AddRect(cursorPos, ImVec2(cursorPos.x + colWidth, cursorPos.y + rowH), borderColor, rounding, 0, 0.5f);

    // Left accent bar
    dl->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y), ImVec2(cursorPos.x + 3.f, cursorPos.y + rowH), accentBar, 2.f);

    // Invisible button for click detection
    ImGui::SetCursorScreenPos(cursorPos);
    ImGui::InvisibleButton(("##itog_btn_" + std::string(label)).c_str(), ImVec2(colWidth, rowH));
    bool changed = false;
    if (ImGui::IsItemClicked())
    {
        *value = !(*value);
        changed = true;
    }

    // Move cursor back to start of row for rendering content
    ImGui::SetCursorScreenPos(cursorPos);

    // Left: icon + label
    ImGui::SetCursorPosX(curX + indentW + padX + 3.f);
    InlineIcon(iconKey, iconSz);

    const ImVec4 lblCol = indent
        ? ImVec4(0.67f, 0.67f, 0.67f, 1.f)
        : ImVec4(1.f, 1.f, 1.f, 1.f);
    ImGui::SameLine(0, iconGap);
    ImGui::TextColored(lblCol, "%s", label);

    // Right: toggle (directly next to text, render only, no click handling)
    ImGui::SameLine(0, 8.f);

    char id[64];
    snprintf(id, sizeof(id), "##itog_%s", label);
    // Render toggle without click handling (click is handled by InvisibleButton)
    RenderToggleOnly(id, value);

    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + rowH + 4.f));
    return changed;
}

// Redesigned path row with Save + Copy + Open buttons
static void PathRowNew(const char* iconKey, const char* sublabel,
                       const std::string& effectivePath)
{
    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    float panelW = ImGui::GetContentRegionAvail().x;
    const float btnW = 60.f;
    const float ieBtnW = 130.f; // Wider button for Import/Export (30px wider)
    const float gap  = 6.f;
    float pathW = (panelW - (btnW + gap) * 2.f - (ieBtnW + gap) - 4.f) * 0.5f; // 50% width

    // Label row
    InlineIcon(iconKey, 13.f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", sublabel);
    ImGui::Spacing();

    // Path input
    ImGui::PushStyleColor(ImGuiCol_FrameBg,    ImVec4(0.10f, 0.10f, 0.10f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Border,     ImVec4(0.25f, 0.25f, 0.25f, 1.f));
    ImGui::SetNextItemWidth(pathW);
    char buf[1024];
    strncpy_s(buf, effectivePath.c_str(), sizeof(buf) - 1);
    ImGui::InputText(("##pr_" + std::string(sublabel)).c_str(),
        buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::Spacing();
}

static void RenderSettingsSubTab()
{
    float panelW = ImGui::GetContentRegionAvail().x;
    float colW   = (panelW - 10.f) * 0.5f;

    // ── Two-column card layout for Items and Currencies ──────────────────────────────────────────
    ImGui::BeginGroup();

    // Left card: Items
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::BeginChild("##col_items", ImVec2(colW, 270.f), true);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();

    CardHeader("items", Localization::GetText("tab_items"));

    if (IconToggleRow("layout-grid",    "Enable Grid View",     &g_Settings.itemsEnableGridView,     colW))        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_grid_view_tooltip"));

    if (IconToggleRow("layout-grid",    "Favorites as Grid",    &g_Settings.itemsFavoritesAsGrid,    colW))        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the Favorites section as a grid of icons instead of a table.");

    if (IconToggleRow("favorites",      "Favorites First",      &g_Settings.itemsFavoritesFirst,     colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("favorites_first_tooltip"));

    if (IconToggleRow("color-swatch",   "Group by Rarity",      &g_Settings.itemsGroupByRarity,      colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("group_by_rarity_tooltip"));

    if (g_Settings.itemsGroupByRarity)
    {
        if (IconToggleRow("layout-columns", "Rarity as Tabs",    &g_Settings.itemsShowRarityAsTabs,  colW, true))
            SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_rarity_as_tabs_tooltip"));
    }

    if (IconToggleRow("groupcategory", "Group by Category",    &g_Settings.itemsGroupByCategory,    colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("group_by_category_tooltip"));

    if (g_Settings.itemsGroupByCategory)
    {
        if (IconToggleRow("layout-columns", "Category as Tabs",  &g_Settings.itemsShowGroupAsTabs,   colW, true))
            SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_group_as_tabs_tooltip"));
    }

    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::SameLine(0, 10.f);

    // Right card: Currencies
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::BeginChild("##col_currencies", ImVec2(colW, 270.f), true);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();

    CardHeader("currencies", Localization::GetText("tab_currencies"));

    if (IconToggleRow("layout-grid",    "Enable Grid View",     &g_Settings.currenciesEnableGridView,  colW))        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_grid_view_tooltip"));

    if (IconToggleRow("layout-grid",    "Favorites as Grid",    &g_Settings.currenciesFavoritesAsGrid, colW))        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the Favorites section as a grid of icons instead of a table.");

    if (IconToggleRow("favorites",      "Favorites First",      &g_Settings.currenciesFavoritesFirst,  colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("favorites_first_tooltip"));

    if (IconToggleRow("groupcategory", "Group by Category",    &g_Settings.currenciesGroupByCategory,  colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("group_by_category_tooltip"));

    if (g_Settings.currenciesGroupByCategory)
    {
        if (IconToggleRow("layout-columns", "Category as Tabs",  &g_Settings.currenciesShowGroupAsTabs,  colW, true))
            SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_group_as_tabs_tooltip"));
    }

    ImGui::EndChild();
    ImGui::EndGroup();

    // ── Overview card below ───────────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::BeginChild("##col_overview", ImVec2(colW, 270.f), true);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();

    CardHeader("summaries", Localization::GetText("tab_overview"));

    if (IconToggleRow("layout-grid",    "Enable Grid View",     &g_Settings.overviewEnableGridView,     colW))        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_grid_view_tooltip"));

    if (IconToggleRow("favorites",      "Favorites as Grid",      &g_Settings.overviewFavoritesAsGrid,     colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show favorites section as grid in overview tab");

    if (IconToggleRow("currencies",      "Favorite currencies first",      &g_Settings.overviewCurrenciesFirst,     colW))
        SettingsManager::Save();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show currencies before items in favorites section");



    ImGui::EndChild();
    ImGui::EndGroup();

    // ── Path rows ───────────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
}

void RenderDropsTab()
{
    // Profit and Profit Per Hour at top
    long long totalProfit = ItemTracker::CalcTotalCustomProfit();
    auto duration = ItemTracker::GetSessionDuration();
    long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);

    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("total_profit_label_simple"));
    ImGui::SameLine();
    ImVec4 profitColor = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(totalProfit).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("total_profit_tooltip"));

    ImGui::Text("%s", Localization::GetText("profit_per_hour_label_simple"));
    ImGui::SameLine();
    ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(profitPerHour).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("profit_per_hour_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const char* subTabLabels[] = {
        Localization::GetText("tab_overview"),
        Localization::GetText("tab_items"),
        Localization::GetText("tab_currencies"),
        Localization::GetText("settings_tab")
    };

    UITabIcons::RenderSubPillTabBar({
        { "summaries",  subTabLabels[0] },
        { "items",      subTabLabels[1] },
        { "currencies", subTabLabels[2] },
        { "general",    subTabLabels[3] }
    }, s_SubTab);

    switch (s_SubTab)
    {
        case 0: UIOverview::RenderOverviewTab();    break;
        case 1: UIItems::RenderItemsTab();          break;
        case 2: UICurrencies::RenderCurrenciesTab(); break;
        case 3: RenderSettingsSubTab();              break;
    }
}
}
