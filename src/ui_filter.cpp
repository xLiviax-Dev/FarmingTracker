#include "ui_filter.h"
#include "settings.h"
#include "localization.h"
#include "shared.h"
#include "../include/nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <cstdio>
#include <sstream>
#include <atomic>
#include <thread>
#include <windows.h>
#include <shlobj.h>

namespace UIFilter
{

// ─────────────────────────────────────────────────────────────────────────────
// State
// ─────────────────────────────────────────────────────────────────────────────
static bool s_OpenSell  = false;
static bool s_OpenType  = false;
static bool s_OpenAttr  = false;
static bool s_OpenRange = false;
static bool s_OpenCur   = false;
static char s_Search[128] = "";

// ─────────────────────────────────────────────────────────────────────────────
// Thread-safe folder browser
// SHBrowseForFolder MUST NOT run on the render thread — it blocks DirectX.
// We spin up a worker thread and poll the result each frame.
// ─────────────────────────────────────────────────────────────────────────────
static std::atomic<bool> s_FolderDialogOpen { false };
static std::atomic<bool> s_FolderDialogDone { false };
static std::string       s_FolderDialogResult;
static std::string       s_InitialPathForDialog;

// Callback to set initial directory
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        if (!s_InitialPathForDialog.empty())
        {
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)s_InitialPathForDialog.c_str());
        }
    }
    return 0;
}

static void OpenFolderDialogAsync(const std::string& initialPath)
{
    if (s_FolderDialogOpen.load()) return; // already open
    s_FolderDialogOpen.store(true);
    s_FolderDialogDone.store(false);
    s_FolderDialogResult.clear();
    s_InitialPathForDialog = initialPath;

    std::thread([]()
    {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        BROWSEINFOA bi = {};
        bi.hwndOwner    = nullptr; // no owner — avoids blocking GW2 window
        bi.lpszTitle    = Localization::GetText("select_log_folder");
        bi.ulFlags      = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.lpfn         = BrowseCallbackProc;

        LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
        std::string result;
        if (pidl)
        {
            char path[MAX_PATH];
            if (SHGetPathFromIDListA(pidl, path))
                result = path;
            CoTaskMemFree(pidl);
        }

        CoUninitialize();
        s_FolderDialogResult = result;
        s_FolderDialogDone.store(true);
        s_FolderDialogOpen.store(false);
    }).detach();
}

// ─────────────────────────────────────────────────────────────────────────────
// Pill checkbox — Variante A
//
// Draws a rounded pill button that acts as a checkbox.
// Active (val=true):  filled blue pill  + small filled circle left + label white
// Inactive (val=false): muted outline pill + empty circle          + label gray
//
// Returns true if the value changed.
// ─────────────────────────────────────────────────────────────────────────────
static bool PillCheckbox(const char* label, bool& val)
{
    // Search filter
    if (s_Search[0] != '\0')
    {
        std::string lbl(label), q(s_Search);
        std::transform(lbl.begin(), lbl.end(), lbl.begin(), ::tolower);
        std::transform(q.begin(),   q.end(),   q.begin(),   ::tolower);
        if (lbl.find(q) == std::string::npos) return false;
    }

    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImGuiStyle& sty = ImGui::GetStyle();

    const float textH  = ImGui::GetTextLineHeight();
    const float padX   = 8.0f;
    const float padY   = 4.0f;
    const float toggleW = 36.0f; // width of toggle switch
    const float toggleH = 18.0f; // height of toggle switch
    const float knobR   = 7.0f;  // radius of the knob
    const float avail  = ImGui::GetContentRegionAvail().x;

    ImVec2 textSz = ImGui::CalcTextSize(label);
    float  pillW  = avail;              // stretch to column width
    float  pillH  = textH + padY * 2.0f;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    // Use Dummy + manual mouse check instead of InvisibleButton
    // so ImGui never registers a "focused item" and draws no rect around it.
    ImGui::Dummy(ImVec2(pillW, pillH));
    bool hovered = ImGui::IsItemHovered();
    bool changed = false;
    if (hovered && ImGui::IsMouseClicked(0)) { val = !val; BackgroundJobs::EnqueueDebouncedSettingsSave(); changed = true; }
    if (hovered)
        ImGui::SetTooltip("%s: %s", label, val ? Localization::GetText("filter_active") : Localization::GetText("filter_inactive"));

    // Accent colors from settings
    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    // Gradient background colors (matching new design, 30% inactive, 70% active)
    ImU32 bgTopOn    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.7f));
    ImU32 bgBotOn    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.25f, acG * 0.25f, acB * 0.25f, 0.7f));
    ImU32 bgTopOff   = ImGui::ColorConvertFloat4ToU32(ImVec4(80, 80, 80, 0.3f));
    ImU32 bgBotOff   = ImGui::ColorConvertFloat4ToU32(ImVec4(50, 50, 50, 0.3f));
    ImU32 bgTopHov   = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.25f, acG * 0.25f, acB * 0.25f, 0.2f));
    ImU32 bgBotHov   = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.15f, acG * 0.15f, acB * 0.15f, 0.2f));
    ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.75f, acG * 0.75f, acB * 0.75f, 0.5f));
    ImU32 accentBar  = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.8f));
    ImU32 colKnobOn  = IM_COL32(255, 255, 255, 255);
    ImU32 colKnobOff = IM_COL32(180, 180, 180, 255);
    ImU32 colTextOn  = IM_COL32(255, 255, 255, 255);
    ImU32 colTextOff = IM_COL32(160, 160, 160, 255);

    float rounding = 4.f;     // rounded corners (not fully rounded)

    // The table-cell clip rect cuts off rounded corners — expand it just enough
    // for the antialiased edges to render correctly (2px on each side).
    const float clipPad = 2.0f;
    ImVec2 clipMin = ImGui::GetWindowDrawList()->GetClipRectMin();
    ImVec2 clipMax = ImGui::GetWindowDrawList()->GetClipRectMax();
    dl->PushClipRect(
        ImVec2(clipMin.x - clipPad, clipMin.y - clipPad),
        ImVec2(clipMax.x + clipPad, clipMax.y + clipPad),
        false);

    // Gradient background
    if (val)
    {
        dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + pillW, pos.y + pillH),
            bgTopOn, bgTopOn, bgBotOn, bgBotOn);
    }
    else
    {
        ImU32 bgTop = hovered ? bgTopHov : bgTopOff;
        ImU32 bgBot = hovered ? bgBotHov : bgBotOff;
        dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + pillW, pos.y + pillH),
            bgTop, bgTop, bgBot, bgBot);
    }

    // Border
    dl->AddRect(pos, ImVec2(pos.x + pillW, pos.y + pillH), borderColor, rounding, 0, 0.5f);

    // Left accent bar
    dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 3.f, pos.y + pillH), accentBar, 2.f);

    // Toggle switch (left side)
    float toggleX = pos.x + padX + 3.f;
    float toggleY = pos.y + (pillH - toggleH) * 0.5f;
    ImVec2 toggleMin = ImVec2(toggleX, toggleY);
    ImVec2 toggleMax = ImVec2(toggleX + toggleW, toggleY + toggleH);

    // Toggle background (green when active)
    ImU32 toggleBgOn = IM_COL32(42, 154, 42, 255);
    ImU32 toggleBgOff = IM_COL32(60, 60, 60, 255);
    dl->AddRectFilled(toggleMin, toggleMax, val ? toggleBgOn : toggleBgOff, toggleH * 0.5f);

    // Toggle knob (slides left/right)
    float knobX = val ? toggleMax.x - knobR - 2.f : toggleMin.x + knobR + 2.f;
    float knobY = toggleMin.y + toggleH * 0.5f;
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR, val ? colKnobOn : colKnobOff);

    // Label
    float textX = toggleX + toggleW + 8.f;
    float textY = pos.y + (pillH - textH) * 0.5f;
    dl->AddText(ImVec2(textX, textY), val ? colTextOn : colTextOff, label);

    dl->PopClipRect(); // restore cell clip rect

    return changed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pill grid — lays items in an N-column wrapping table of pill checkboxes.
// Default columns = 2 (same as before).
// ─────────────────────────────────────────────────────────────────────────────
static void FilterGrid(std::initializer_list<std::pair<const char*, bool*>> items,
                       int columns = 2)
{
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3.0f, 3.0f));
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight,  ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_TableRowBg,        ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,     ImVec4(0,0,0,0));
    if (ImGui::BeginTable("##fg_outer", columns,
            ImGuiTableFlags_None,
            ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
    {
        for (auto& [lbl, ptr] : items)
        {
            ImGui::TableNextColumn();
            PillCheckbox(lbl, *ptr);
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}

// ─────────────────────────────────────────────────────────────────────────────
// Half-width pill grid — items fill left column only (Sell method section).
// ─────────────────────────────────────────────────────────────────────────────
static void FilterGridHalfWidth(std::initializer_list<std::pair<const char*, bool*>> items)
{
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3.0f, 3.0f));
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight,  ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_TableRowBg,        ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,     ImVec4(0,0,0,0));
    if (ImGui::BeginTable("##fghalf_outer", 2,
            ImGuiTableFlags_None,
            ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
    {
        for (auto& [lbl, ptr] : items)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            PillCheckbox(lbl, *ptr);
            // col 1 stays empty
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}

// ─────────────────────────────────────────────────────────────────────────────
// Muted sub-label (matching active main tab design)
// ─────────────────────────────────────────────────────────────────────────────
static void SubLabel(const char* text)
{
    ImGui::Spacing();

    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 region = ImGui::GetContentRegionAvail();
    float headerHeight = 28.f;

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

    // Text
    float textX = headerMin.x + 8.0f;
    float textY = headerMin.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f;
    dl->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), text);

    ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y + 4.f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Collapsible section header with optional All/None buttons + active count
// ─────────────────────────────────────────────────────────────────────────────
static bool SectionHeader(const char* id, const char* label, bool& open,
                           bool** ptrs = nullptr, int n = 0, int active = 0)
{
    ImGui::PushID(id);

    float totalW  = ImGui::GetContentRegionAvail().x;
    float btnW    = 90.0f; // Always 90.0f for consistent width
    float badgeW  = 28.0f; // Always 28.0f for consistent width (space for active count)
    // Only reserve space for buttons if they will be shown
    float reservedSpace = (ptrs && n > 0) ? btnW + badgeW + 24.0f : 24.0f;
    float selW    = totalW - reservedSpace;

    std::string hdrLabel = (open ? "v  " : ">  ") + std::string(label) + "##hdr_" + id;
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 itemSize = ImVec2(selW, ImGui::GetFrameHeight());

    // Draw gradient background with border and left accent bar (matching new design)
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    // Gradient background
    ImU32 bgColorTop = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
    ImU32 bgColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));
    dl->AddRectFilledMultiColor(cursorPos, ImVec2(cursorPos.x + itemSize.x, cursorPos.y + itemSize.y),
        bgColorTop, bgColorTop, bgColorBottom, bgColorBottom);

    // Border
    ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));
    dl->AddRect(cursorPos, ImVec2(cursorPos.x + itemSize.x, cursorPos.y + itemSize.y), borderColor, 4.f, 0, 0.5f);

    // Left accent bar
    ImU32 accentBarColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.0f));
    dl->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y), ImVec2(cursorPos.x + 3.f, cursorPos.y + itemSize.y), accentBarColor, 2.f);

    // Invisible button for click detection
    ImGui::SetCursorScreenPos(cursorPos);
    ImGui::InvisibleButton(("##hdr_btn_" + std::string(id)).c_str(), itemSize);
    if (ImGui::IsItemClicked())
        open = !open;

    // Label text
    float textX = cursorPos.x + 8.0f + 3.f;
    float textY = cursorPos.y + (itemSize.y - ImGui::GetTextLineHeight()) * 0.5f;
    std::string labelText = (open ? "v  " : ">  ") + std::string(label);
    dl->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), labelText.c_str());

    // Active count badge
    if (active > 0)
    {
        float badgeX = cursorPos.x + selW - badgeW;
        float badgeY = cursorPos.y + (itemSize.y - ImGui::GetTextLineHeight()) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.9f, 0.4f, 1.0f)); // Green for better visibility
        dl->AddText(ImVec2(badgeX, badgeY), IM_COL32(0.2f, 0.9f, 0.4f, 1.0f), std::to_string(active).c_str());
        ImGui::PopStyleColor();
    }

    // All / None buttons (orange gradient)
    if (ptrs && n > 0)
    {
        float btnStartX = totalW - btnW + ImGui::GetStyle().WindowPadding.x - 20.0f; // 20px Padding rechts
        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + btnStartX, cursorPos.y));

        if (UICommon::OrangeGradientButton(Localization::GetText("filter_all"), ("##all_" + std::string(id)).c_str()))
        { for (int i=0;i<n;i++) *ptrs[i]=true;  BackgroundJobs::EnqueueDebouncedSettingsSave(); }
        ImGui::SameLine(0.0f, 4.0f);
        if (UICommon::OrangeGradientButton(Localization::GetText("filter_none"), ("##none_" + std::string(id)).c_str()))
        { for (int i=0;i<n;i++) *ptrs[i]=false; BackgroundJobs::EnqueueDebouncedSettingsSave(); }
    }

    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + itemSize.y + 4.f));
    ImGui::Separator();
    ImGui::PopID();
    return open;
}

// ─────────────────────────────────────────────────────────────────────────────
// Reset to defaults
// ─────────────────────────────────────────────────────────────────────────────
static void ResetAllToDefaults()
{
    g_Settings.filterSellableToVendor     = true;
    g_Settings.filterSellableOnTp         = true;
    g_Settings.filterCustomProfit         = true;
    g_Settings.filterKnownByApi           = true;
    g_Settings.filterUnknownByApi         = true;
    g_Settings.filterAccountBound         = true;
    g_Settings.filterNotAccountBound      = true;
    g_Settings.filterNoSell               = true;
    g_Settings.filterNotNoSell            = true;
    g_Settings.filterFavorite             = true;
    g_Settings.filterNotFavorite          = true;
    g_Settings.filterIgnored              = false;
    g_Settings.filterNotIgnored           = true;
    g_Settings.filterTypeArmor            = true;
    g_Settings.filterTypeWeapon           = true;
    g_Settings.filterTypeTrinket          = true;
    g_Settings.filterTypeGizmo            = true;
    g_Settings.filterTypeCraftingMaterial = true;
    g_Settings.filterTypeConsumable       = true;
    g_Settings.filterTypeGatheringTool    = true;
    g_Settings.filterTypeBag              = true;
    g_Settings.filterTypeContainer        = true;
    g_Settings.filterTypeMiniPet          = true;
    g_Settings.filterTypeGizmoContainer   = true;
    g_Settings.filterTypeBackpack         = true;
    g_Settings.filterTypeUpgradeComponent = true;
    g_Settings.filterTypeTool             = true;
    g_Settings.filterTypeTrophy           = true;
    g_Settings.filterTypeUnlock           = true;
    g_Settings.filterKarma                = true;
    g_Settings.filterLaurel               = true;
    g_Settings.filterGem                  = true;
    g_Settings.filterFractalRelic         = true;
    g_Settings.filterBadgeOfHonor         = true;
    g_Settings.filterGuildCommendation    = true;
    g_Settings.filterTransmutationCharge  = true;
    g_Settings.filterSpiritShards         = true;
    g_Settings.filterUnboundMagic         = true;
    g_Settings.filterVolatileMagic        = true;
    g_Settings.filterAirshipParts         = true;
    g_Settings.filterGeode                = true;
    g_Settings.filterLeyLineCrystals      = true;
    g_Settings.filterTradeContracts       = true;
    g_Settings.filterElegyMosaic          = true;
    g_Settings.filterUncommonCoins        = true;
    g_Settings.filterAstralAcclaim        = true;
    g_Settings.filterPristineFractalRelics   = true;
    g_Settings.filterUnstableFractalEssence  = true;
    g_Settings.filterMagnetiteShards      = true;
    g_Settings.filterGaetingCrystals      = true;
    g_Settings.filterProphetShards        = true;
    g_Settings.filterGreenProphetShards   = true;
    g_Settings.filterWvWSkirmishTickets   = true;
    g_Settings.filterProofsOfHeroics      = true;
    g_Settings.filterPvpLeagueTickets     = true;
    g_Settings.filterAscendedShardsOfGlory  = true;
    g_Settings.filterResearchNotes        = true;
    g_Settings.filterTyrianDefenseSeal    = true;
    g_Settings.filterTestimonyOfDesertHeroics    = true;
    g_Settings.filterTestimonyOfJadeHeroics      = true;
    g_Settings.filterTestimonyOfCastoranHeroics  = true;
    g_Settings.filterLegendaryInsight     = true;
    g_Settings.filterTalesOfDungeonDelving= true;
    g_Settings.filterImperialFavor        = true;
    g_Settings.filterCanachCoins          = true;
    g_Settings.filterAncientCoin          = true;
    g_Settings.filterUnusualCoin          = true;
    g_Settings.filterJadeSliver           = true;
    g_Settings.filterStaticCharge         = true;
    g_Settings.filterPinchOfStardust      = true;
    g_Settings.filterCalcifiedGasp        = true;
    g_Settings.filterUrsusOblige          = true;
    g_Settings.filterGaetingCrystalJanthir= true;
    g_Settings.filterAntiquatedDucat      = true;
    g_Settings.filterAetherRichSap        = true;
    g_Settings.filterMinPriceGold         = 0;
    g_Settings.filterMinPriceSilver       = 0;
    g_Settings.filterMinPriceCopper       = 0;
    g_Settings.filterMaxPriceGold         = 0;
    g_Settings.filterMaxPriceSilver       = 0;
    g_Settings.filterMaxPriceCopper       = 0;
    g_Settings.filterMinQuantity          = 0;
    g_Settings.filterMaxQuantity          = 0;
    BackgroundJobs::EnqueueDebouncedSettingsSave();
}

// ─────────────────────────────────────────────────────────────────────────────
// Count active restrictions
// ─────────────────────────────────────────────────────────────────────────────
static int CountActive()
{
    int n = 0;
    // Count all filters that are ON.
    // filterIgnored uses inverse logic and is handled separately.
    auto f = [&](bool v){ if (v) n++; };
    f(g_Settings.filterSellableToVendor); f(g_Settings.filterSellableOnTp);
    f(g_Settings.filterCustomProfit);     f(g_Settings.filterKnownByApi);
    f(g_Settings.filterUnknownByApi);     f(g_Settings.filterAccountBound);
    f(g_Settings.filterNotAccountBound);  f(g_Settings.filterNoSell);
    f(g_Settings.filterNotNoSell);        f(g_Settings.filterFavorite);
    f(g_Settings.filterNotFavorite);      f(g_Settings.filterNotIgnored);
    f(g_Settings.filterTypeArmor);        f(g_Settings.filterTypeWeapon);
    f(g_Settings.filterTypeTrinket);      f(g_Settings.filterTypeGizmo);
    f(g_Settings.filterTypeCraftingMaterial); f(g_Settings.filterTypeConsumable);
    f(g_Settings.filterTypeGatheringTool);f(g_Settings.filterTypeBag);
    f(g_Settings.filterTypeContainer);   f(g_Settings.filterTypeMiniPet);
    f(g_Settings.filterTypeGizmoContainer);f(g_Settings.filterTypeBackpack);
    f(g_Settings.filterTypeUpgradeComponent);f(g_Settings.filterTypeTool);
    f(g_Settings.filterTypeTrophy);       f(g_Settings.filterTypeUnlock);
    f(g_Settings.filterKarma);            f(g_Settings.filterLaurel);
    f(g_Settings.filterGem);              f(g_Settings.filterFractalRelic);
    f(g_Settings.filterBadgeOfHonor);     f(g_Settings.filterGuildCommendation);
    f(g_Settings.filterTransmutationCharge);f(g_Settings.filterSpiritShards);
    f(g_Settings.filterUnboundMagic);     f(g_Settings.filterVolatileMagic);
    f(g_Settings.filterAirshipParts);     f(g_Settings.filterGeode);
    f(g_Settings.filterLeyLineCrystals);  f(g_Settings.filterTradeContracts);
    f(g_Settings.filterElegyMosaic);      f(g_Settings.filterUncommonCoins);
    f(g_Settings.filterAstralAcclaim);    f(g_Settings.filterPristineFractalRelics);
    f(g_Settings.filterUnstableFractalEssence); f(g_Settings.filterMagnetiteShards);
    f(g_Settings.filterGaetingCrystals);  f(g_Settings.filterProphetShards);
    f(g_Settings.filterGreenProphetShards);f(g_Settings.filterWvWSkirmishTickets);
    f(g_Settings.filterProofsOfHeroics);  f(g_Settings.filterPvpLeagueTickets);
    f(g_Settings.filterAscendedShardsOfGlory);f(g_Settings.filterResearchNotes);
    f(g_Settings.filterTyrianDefenseSeal);
    f(g_Settings.filterTestimonyOfDesertHeroics);
    f(g_Settings.filterTestimonyOfJadeHeroics);
    f(g_Settings.filterTestimonyOfCastoranHeroics);
    f(g_Settings.filterLegendaryInsight); f(g_Settings.filterTalesOfDungeonDelving);
    f(g_Settings.filterImperialFavor);    f(g_Settings.filterCanachCoins);
    f(g_Settings.filterAncientCoin);      f(g_Settings.filterUnusualCoin);
    f(g_Settings.filterJadeSliver);       f(g_Settings.filterStaticCharge);
    f(g_Settings.filterPinchOfStardust);  f(g_Settings.filterCalcifiedGasp);
    f(g_Settings.filterUrsusOblige);      f(g_Settings.filterGaetingCrystalJanthir);
    f(g_Settings.filterAntiquatedDucat);  f(g_Settings.filterAetherRichSap);
    // filterIgnored is already counted via the f() loop above — no extra count needed
    f(g_Settings.filterIgnored);
    return n;
}

// ═════════════════════════════════════════════════════════════════════════════
// RenderFilterTab
// ═════════════════════════════════════════════════════════════════════════════
void RenderFilterTab()
{
    const int active = CountActive();

    // ── Top bar ───────────────────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::Indent(4.0f);

    if (active > 0)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.9f, 0.4f, 1.0f)); // Green for better visibility
        char badge[48];
        snprintf(badge, sizeof(badge),
            Localization::GetText("filter_active_count"), active);
        ImGui::TextUnformatted(badge);
        ImGui::PopStyleColor();
        ImGui::SameLine();
    }

    const char* resetLbl = Localization::GetText("filter_reset_all");
    float resetW = ImGui::CalcTextSize(resetLbl).x + 14.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - resetW - 10.0f);
    if (UICommon::RedGradientButton(resetLbl, "##filter_reset_all"))
        ResetAllToDefaults();
    ImGui::Unindent(4.0f);
    ImGui::Spacing();

    // ── Search ────────────────────────────────────────────────────────────────
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##filsearch", s_Search, sizeof(s_Search));
    if (s_Search[0] == '\0')
    {
        ImVec2 p = ImGui::GetItemRectMin();
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(p.x + 10.0f, p.y + 4.0f),
            IM_COL32(255,255,255,200),
            Localization::GetText("filter_search_hint"));
    }
    ImGui::PopStyleVar();
    ImGui::Spacing();

    // ══════════════════════════════════════════════════════════════════════════
    // SEKTION: Verkaufsmethode
    // ══════════════════════════════════════════════════════════════════════════
    {
        bool* ptrs[] = {
            &g_Settings.filterSellableToVendor,
            &g_Settings.filterSellableOnTp,
            &g_Settings.filterCustomProfit
        };
        int cnt = 0; for (auto p : ptrs) if (*p) cnt++;
        bool show = SectionHeader("sell",
            Localization::GetText("sell_method_filters_label"),
            s_OpenSell, ptrs, 3, cnt) || s_Search[0];
        if (show)
        {
            ImGui::Indent(8.0f);
            FilterGridHalfWidth({
                { Localization::GetText("filter_sellable_to_vendor"), &g_Settings.filterSellableToVendor },
                { Localization::GetText("filter_sellable_on_tp"),     &g_Settings.filterSellableOnTp     },
                { Localization::GetText("filter_custom_profit"),      &g_Settings.filterCustomProfit     }
            });
            ImGui::Unindent(8.0f);
            ImGui::Spacing();
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SEKTION: Gegenstandstypen
    // ══════════════════════════════════════════════════════════════════════════
    {
        bool* ptrs[] = {
            &g_Settings.filterTypeArmor,           &g_Settings.filterTypeWeapon,
            &g_Settings.filterTypeTrinket,         &g_Settings.filterTypeCraftingMaterial,
            &g_Settings.filterTypeConsumable,      &g_Settings.filterTypeContainer,
            &g_Settings.filterTypeBag,             &g_Settings.filterTypeMiniPet,
            &g_Settings.filterTypeGizmo,           &g_Settings.filterTypeGizmoContainer,
            &g_Settings.filterTypeBackpack,        &g_Settings.filterTypeUpgradeComponent,
            &g_Settings.filterTypeTool,            &g_Settings.filterTypeTrophy,
            &g_Settings.filterTypeUnlock,          &g_Settings.filterTypeGatheringTool
        };
        int cnt = 0; for (auto p : ptrs) if (*p) cnt++;
        bool show = SectionHeader("type",
            Localization::GetText("item_type_filters_label"),
            s_OpenType, ptrs, 16, cnt) || s_Search[0];
        if (show)
        {
            ImGui::Indent(8.0f);
            FilterGrid({
                { Localization::GetText("filter_type_armor"),             &g_Settings.filterTypeArmor             },
                { Localization::GetText("filter_type_weapon"),            &g_Settings.filterTypeWeapon            },
                { Localization::GetText("filter_type_trinket"),           &g_Settings.filterTypeTrinket           },
                { Localization::GetText("filter_type_crafting_material"), &g_Settings.filterTypeCraftingMaterial  },
                { Localization::GetText("filter_type_consumable"),        &g_Settings.filterTypeConsumable        },
                { Localization::GetText("filter_type_container"),         &g_Settings.filterTypeContainer         },
                { Localization::GetText("filter_type_bag"),               &g_Settings.filterTypeBag               },
                { Localization::GetText("filter_type_mini_pet"),          &g_Settings.filterTypeMiniPet           },
                { Localization::GetText("filter_type_gizmo"),             &g_Settings.filterTypeGizmo             },
                { Localization::GetText("filter_type_gizmo_container"),   &g_Settings.filterTypeGizmoContainer    },
                { Localization::GetText("filter_type_backpack"),          &g_Settings.filterTypeBackpack          },
                { Localization::GetText("filter_type_upgrade_component"), &g_Settings.filterTypeUpgradeComponent  },
                { Localization::GetText("filter_type_tool"),              &g_Settings.filterTypeTool              },
                { Localization::GetText("filter_type_trophy"),            &g_Settings.filterTypeTrophy            },
                { Localization::GetText("filter_type_unlock"),            &g_Settings.filterTypeUnlock            },
                { Localization::GetText("filter_type_gathering_tool"),    &g_Settings.filterTypeGatheringTool     }
            });
            ImGui::Unindent(8.0f);
            ImGui::Spacing();
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SEKTION: Eigenschaften — alle als Toggles
    // ══════════════════════════════════════════════════════════════════════════
    {
        bool* ptrs[] = {
            &g_Settings.filterAccountBound,    &g_Settings.filterNotAccountBound,
            &g_Settings.filterNoSell,          &g_Settings.filterNotNoSell,
            &g_Settings.filterFavorite,        &g_Settings.filterNotFavorite,
            &g_Settings.filterIgnored,         &g_Settings.filterNotIgnored,
            &g_Settings.filterKnownByApi,      &g_Settings.filterUnknownByApi
        };
        int cnt = 0;
        for (auto p : ptrs) if (*p) cnt++;

        bool show = SectionHeader("attr",
            Localization::GetText("additional_filters_label"),
            s_OpenAttr, ptrs, 10, cnt) || s_Search[0];
        if (show)
        {
            ImGui::Indent(8.0f);

            SubLabel(Localization::GetText("filter_account_bound"));
            FilterGrid({
                { Localization::GetText("filter_account_bound"),     &g_Settings.filterAccountBound     },
                { Localization::GetText("filter_not_account_bound"), &g_Settings.filterNotAccountBound  }
            });

            SubLabel(Localization::GetText("filter_nosell"));
            FilterGrid({
                { Localization::GetText("filter_nosell"),     &g_Settings.filterNoSell     },
                { Localization::GetText("filter_not_nosell"), &g_Settings.filterNotNoSell  }
            });

            SubLabel(Localization::GetText("filter_favorite"));
            FilterGrid({
                { Localization::GetText("filter_favorite"),     &g_Settings.filterFavorite     },
                { Localization::GetText("filter_not_favorite"), &g_Settings.filterNotFavorite  }
            });

            SubLabel(Localization::GetText("filter_ignored"));
            FilterGrid({
                { Localization::GetText("filter_ignored"),     &g_Settings.filterIgnored     },
                { Localization::GetText("filter_not_ignored"), &g_Settings.filterNotIgnored  }
            });

            SubLabel(Localization::GetText("api_knowledge_filters_label"));
            FilterGrid({
                { Localization::GetText("filter_known_by_api"),   &g_Settings.filterKnownByApi   },
                { Localization::GetText("filter_unknown_by_api"), &g_Settings.filterUnknownByApi }
            });

            ImGui::Unindent(8.0f);
            ImGui::Spacing();
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SECTION: Currencies
    // ══════════════════════════════════════════════════════════════════════════
    {
        bool* ptrs[] = {
            &g_Settings.filterKarma,                &g_Settings.filterLaurel,
            &g_Settings.filterGem,                  &g_Settings.filterSpiritShards,
            &g_Settings.filterTransmutationCharge,  &g_Settings.filterResearchNotes,
            &g_Settings.filterAstralAcclaim,       &g_Settings.filterBadgeOfHonor,
            &g_Settings.filterWvWSkirmishTickets,  &g_Settings.filterPvpLeagueTickets,
            &g_Settings.filterProofsOfHeroics,     &g_Settings.filterAscendedShardsOfGlory,
            &g_Settings.filterTestimonyOfDesertHeroics, &g_Settings.filterTestimonyOfJadeHeroics,
            &g_Settings.filterTestimonyOfCastoranHeroics, &g_Settings.filterFractalRelic,
            &g_Settings.filterPristineFractalRelics,    &g_Settings.filterUnstableFractalEssence,
            &g_Settings.filterMagnetiteShards,      &g_Settings.filterGaetingCrystals,
            &g_Settings.filterLegendaryInsight,     &g_Settings.filterTalesOfDungeonDelving,
            &g_Settings.filterAirshipParts,         &g_Settings.filterAetherRichSap,
            &g_Settings.filterAntiquatedDucat,      &g_Settings.filterCalcifiedGasp,
            &g_Settings.filterElegyMosaic,          &g_Settings.filterGaetingCrystalJanthir,
            &g_Settings.filterGeode,                &g_Settings.filterGuildCommendation,
            &g_Settings.filterGreenProphetShards,   &g_Settings.filterImperialFavor,
            &g_Settings.filterJadeSliver,           &g_Settings.filterLeyLineCrystals,
            &g_Settings.filterPinchOfStardust,      &g_Settings.filterProphetShards,
            &g_Settings.filterStaticCharge,         &g_Settings.filterTradeContracts,
            &g_Settings.filterUnboundMagic,         &g_Settings.filterUncommonCoins,
            &g_Settings.filterUrsusOblige,         &g_Settings.filterVolatileMagic,
            &g_Settings.filterCanachCoins,          &g_Settings.filterAncientCoin,
            &g_Settings.filterUnusualCoin,          &g_Settings.filterTyrianDefenseSeal
        };
        int cnt = 0; for (auto p : ptrs) if (*p) cnt++;
        bool show = SectionHeader("cur",
            Localization::GetText("currency_filters_label"),
            s_OpenCur, ptrs, 38, cnt) || s_Search[0];
        if (show)
        {
            ImGui::Indent(8.0f);

            // Allgemein
            SubLabel(Localization::GetText("currency_general"));
            FilterGrid({
                { Localization::GetText("filter_karma"),                &g_Settings.filterKarma               },
                { Localization::GetText("filter_laurel"),               &g_Settings.filterLaurel              },
                { Localization::GetText("filter_gem"),                  &g_Settings.filterGem                 },
                { Localization::GetText("filter_spirit_shards"),        &g_Settings.filterSpiritShards        },
                { Localization::GetText("filter_transmutation_charge"), &g_Settings.filterTransmutationCharge },
                { Localization::GetText("filter_research_notes"),       &g_Settings.filterResearchNotes       },
                { Localization::GetText("filter_astral_acclaim"),       &g_Settings.filterAstralAcclaim       }
            });

            // WvW / PvP
            SubLabel(Localization::GetText("currency_wvw_pvp"));
            FilterGrid({
                { Localization::GetText("filter_badge_of_honor"),                &g_Settings.filterBadgeOfHonor               },
                { Localization::GetText("filter_wvw_skirmish_tickets"),          &g_Settings.filterWvWSkirmishTickets          },
                { Localization::GetText("filter_pvp_league_tickets"),            &g_Settings.filterPvpLeagueTickets            },
                { Localization::GetText("filter_proofs_of_heroics"),             &g_Settings.filterProofsOfHeroics             },
                { Localization::GetText("filter_ascended_shards_of_glory"),      &g_Settings.filterAscendedShardsOfGlory      },
                { Localization::GetText("filter_testimony_of_desert_heroics"),   &g_Settings.filterTestimonyOfDesertHeroics   },
                { Localization::GetText("filter_testimony_of_jade_heroics"),     &g_Settings.filterTestimonyOfJadeHeroics     },
                { Localization::GetText("filter_testimony_of_castoran_heroics"), &g_Settings.filterTestimonyOfCastoranHeroics }
            });

            // Fractals & Raids
            SubLabel(Localization::GetText("currency_fractal"));
            FilterGrid({
                { Localization::GetText("filter_fractal_relic"),            &g_Settings.filterFractalRelic            },
                { Localization::GetText("filter_pristine_fractal_relics"),  &g_Settings.filterPristineFractalRelics   },
                { Localization::GetText("filter_unstable_fractal_essence"), &g_Settings.filterUnstableFractalEssence  },
                { Localization::GetText("filter_magnetite_shards"),         &g_Settings.filterMagnetiteShards         },
                { Localization::GetText("filter_gaeting_crystals"),         &g_Settings.filterGaetingCrystals         },
                { Localization::GetText("filter_legendary_insight"),        &g_Settings.filterLegendaryInsight        },
                { Localization::GetText("filter_tales_of_dungeon_delving"), &g_Settings.filterTalesOfDungeonDelving   }
            });

            // Map Currencies
            SubLabel(Localization::GetText("currency_map"));
            FilterGrid({
                { Localization::GetText("filter_airship_parts"),           &g_Settings.filterAirshipParts           },
                { Localization::GetText("filter_aether_rich_sap"),         &g_Settings.filterAetherRichSap          },
                { Localization::GetText("filter_antiquated_ducat"),        &g_Settings.filterAntiquatedDucat        },
                { Localization::GetText("filter_calcified_gasp"),          &g_Settings.filterCalcifiedGasp          },
                { Localization::GetText("filter_elegy_mosaic"),            &g_Settings.filterElegyMosaic            },
                { Localization::GetText("filter_gaeting_crystal_janthir"), &g_Settings.filterGaetingCrystalJanthir  },
                { Localization::GetText("filter_geode"),                   &g_Settings.filterGeode                  },
                { Localization::GetText("filter_guild_commendation"),      &g_Settings.filterGuildCommendation      },
                { Localization::GetText("filter_green_prophet_shards"),    &g_Settings.filterGreenProphetShards     },
                { Localization::GetText("filter_imperial_favor"),          &g_Settings.filterImperialFavor          },
                { Localization::GetText("filter_jade_sliver"),             &g_Settings.filterJadeSliver             },
                { Localization::GetText("filter_ley_line_crystals"),       &g_Settings.filterLeyLineCrystals        },
                { Localization::GetText("filter_pinch_of_stardust"),       &g_Settings.filterPinchOfStardust        },
                { Localization::GetText("filter_prophet_shards"),          &g_Settings.filterProphetShards          },
                { Localization::GetText("filter_static_charge"),           &g_Settings.filterStaticCharge           },
                { Localization::GetText("filter_trade_contracts"),         &g_Settings.filterTradeContracts         },
                { Localization::GetText("filter_unbound_magic"),           &g_Settings.filterUnboundMagic           },
                { Localization::GetText("filter_uncommon_coins"),          &g_Settings.filterUncommonCoins          },
                { Localization::GetText("filter_ursus_oblige"),            &g_Settings.filterUrsusOblige            },
                { Localization::GetText("filter_volatile_magic"),          &g_Settings.filterVolatileMagic          },
                { Localization::GetText("filter_canach_coins"),            &g_Settings.filterCanachCoins            },
                { Localization::GetText("filter_ancient_coin"),            &g_Settings.filterAncientCoin            },
                { Localization::GetText("filter_unusual_coin"),            &g_Settings.filterUnusualCoin            },
                { Localization::GetText("filter_tyrian_defense_seal"),     &g_Settings.filterTyrianDefenseSeal      }
            });

            ImGui::Unindent(8.0f);
            ImGui::Spacing();
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SEKTION: Preis & Menge
    // ══════════════════════════════════════════════════════════════════════════
    {
        bool show = SectionHeader("range",
            Localization::GetText("range_filters"),
            s_OpenRange, nullptr, 0, 0) || s_Search[0];
        if (show)
        {
            ImGui::Spacing();
            ImGui::Indent(8.0f);

            SubLabel(Localization::GetText("price_range"));
            float colW = (ImGui::GetContentRegionAvail().x - 80.0f) / 13.2f; // 50% kleiner (war / 6.6f)
            float labelW = ImGui::CalcTextSize(Localization::GetText("filter_max_price")).x;

            ImGui::Text("%s", Localization::GetText("filter_min_price"));
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + labelW - ImGui::CalcTextSize(Localization::GetText("filter_min_price")).x);
            ImGui::SetNextItemWidth(colW);
            if (ImGui::InputInt("G##MinG", &g_Settings.filterMinPriceGold,   0)) BackgroundJobs::EnqueueDebouncedSettingsSave();
            ImGui::SameLine(0.0f,4.0f); ImGui::SetNextItemWidth(colW);
            if (ImGui::InputInt("S##MinS", &g_Settings.filterMinPriceSilver, 0)) BackgroundJobs::EnqueueDebouncedSettingsSave();
            ImGui::SameLine(0.0f,4.0f); ImGui::SetNextItemWidth(colW);
            if (ImGui::InputInt("C##MinC", &g_Settings.filterMinPriceCopper, 0)) BackgroundJobs::EnqueueDebouncedSettingsSave();

            ImGui::Text("%s", Localization::GetText("filter_max_price"));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(colW);
            if (ImGui::InputInt("G##MaxG", &g_Settings.filterMaxPriceGold,   0)) BackgroundJobs::EnqueueDebouncedSettingsSave();
            ImGui::SameLine(0.0f,4.0f); ImGui::SetNextItemWidth(colW);
            if (ImGui::InputInt("S##MaxS", &g_Settings.filterMaxPriceSilver, 0)) BackgroundJobs::EnqueueDebouncedSettingsSave();
            ImGui::SameLine(0.0f,4.0f); ImGui::SetNextItemWidth(colW);
            if (ImGui::InputInt("C##MaxC", &g_Settings.filterMaxPriceCopper, 0)) BackgroundJobs::EnqueueDebouncedSettingsSave();

            ImGui::Spacing();
            SubLabel(Localization::GetText("quantity_range"));
            float halfW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.20f; // 20% kleiner (war * 0.25f)
            ImGui::SetNextItemWidth(halfW);
            if (ImGui::InputInt(Localization::GetText("filter_min_quantity"),
                    &g_Settings.filterMinQuantity, 0)) BackgroundJobs::EnqueueDebouncedSettingsSave();
            ImGui::SameLine(0.0f,8.0f);
            ImGui::SetNextItemWidth(halfW);
            if (ImGui::InputInt(Localization::GetText("filter_max_quantity"),
                    &g_Settings.filterMaxQuantity, 0)) BackgroundJobs::EnqueueDebouncedSettingsSave();

            ImGui::Unindent(8.0f);
            ImGui::Spacing();
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // Export/Import Pfad
    // ══════════════════════════════════════════════════════════════════════════
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f,1.0f,1.0f,1.0f));
    ImGui::TextUnformatted(Localization::GetText("current_export_path"));
    ImGui::PopStyleColor();
    ImGui::Spacing();

    const char* addonDir   = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
    std::string effectivePath = g_Settings.liveLogCustomPath.empty()
        ? (std::string(addonDir ? addonDir : "") + "\\")
        : g_Settings.liveLogCustomPath;

    float totalW = ImGui::GetContentRegionAvail().x;
    const float btnW = 116.0f;
    float inputW = (totalW - btnW * 2.0f - 12.0f) * 0.5f;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,
        ImVec4(g_Settings.accentColorR * 0.6f, g_Settings.accentColorG * 0.6f, g_Settings.accentColorB * 0.6f, 0.95f));
    ImGui::SetNextItemWidth(inputW);
    ImGui::InputText("##logpath_disp",
        const_cast<char*>(effectivePath.c_str()),
        effectivePath.size() + 1,
        ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked())
        OpenFolderDialogAsync(effectivePath);

    // Poll result from folder dialog thread each frame
    if (s_FolderDialogDone.load())
    {
        s_FolderDialogDone.store(false);
        if (!s_FolderDialogResult.empty())
        {
            g_Settings.liveLogCustomPath = s_FolderDialogResult;
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        }
    }

    ImGui::SameLine(0.0f, 4.0f);
    if (UICommon::OrangeGradientButton(Localization::GetText("open_folder_button"), "##filter_open"))
    {
        std::string cmd = "explorer.exe \"" + effectivePath + "\"";
        system(cmd.c_str());
    }
    ImGui::SameLine(0.0f, 4.0f);
    if (UICommon::OrangeGradientButton("Import/Export", "##filter_ie"))
        ImGui::OpenPopup("FilterLoadSavePopup");

    // Import/Export popup
    if (ImGui::BeginPopup("FilterLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            nlohmann::json j = SettingsManager::ExportFilterSettings();
            std::string json = j.dump();
            std::string path = effectivePath + "filter_export.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(json.data(), 1, json.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("export_csv")))
        {
            std::string path = effectivePath + "filter_export.csv";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f)
            {
                nlohmann::json j = SettingsManager::ExportFilterSettings();
                std::string csv = "Filter,Value\n";
                for (auto& [key, val] : j.items())
                {
                    if (val.is_boolean())
                        csv += key + "," + (val.get<bool>() ? "true" : "false") + "\n";
                }
                fwrite(csv.data(), 1, csv.size(), f);
                fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_csv_tooltip"));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Import section
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Localization::GetText("import_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import_json")))
        {
            std::string path = effectivePath + "filter_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0) {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    try { SettingsManager::ImportFilterSettings(nlohmann::json::parse(buf)); } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("import_csv")))
        {
            std::string path = effectivePath + "filter_import.csv";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0) {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    // Parse CSV and import settings
                    try {
                        nlohmann::json j;
                        std::istringstream iss(buf);
                        std::string line;
                        // Skip header line
                        std::getline(iss, line);
                        while (std::getline(iss, line))
                        {
                            size_t comma = line.find(',');
                            if (comma != std::string::npos)
                            {
                                std::string key = line.substr(0, comma);
                                std::string value = line.substr(comma + 1);
                                // Trim whitespace
                                key.erase(0, key.find_first_not_of(" \t\r\n"));
                                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                                value.erase(0, value.find_first_not_of(" \t\r\n"));
                                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                                if (value == "true")
                                    j[key] = true;
                                else if (value == "false")
                                    j[key] = false;
                            }
                        }
                        SettingsManager::ImportFilterSettings(j);
                    } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_csv_tooltip"));
        ImGui::EndPopup();
    }
}

} // namespace UIFilter
