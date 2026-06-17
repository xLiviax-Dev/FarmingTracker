#pragma once

#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "localization.h"
#include "../include/imgui/imgui.h"
#include <string>
#include <vector>
#include <utility>

namespace UICommon
{
    // Shared UI state variables
    extern char s_SearchBuf[256];
    extern bool s_ShowMainWindow;
    extern char s_AccountNameBuf[128];
    extern char s_AccountDrfBuf[512];
    extern char s_AccountGw2Buf[512];
    extern char s_NewProfileNameBuf[128];
    
    // Thread safety
    extern std::mutex s_AccountNameMutex;

    // Icon disk-cache
    void InitIconCache(const char* addonDir);
    void EnforceIconCacheLimit();

    void EnsureItemIconTexture(int itemId, const std::string& url);
    void DrawItemIconCell(int itemId, const std::string& url, float sz, const std::string& rarity = "", bool forceShow = false);
    const char* StatusText(DrfStatus s);
    ImVec4 StatusColor(DrfStatus s);
    ImVec4 ValueColor(long long value);
    std::string FormatCoin(long long copper);
    std::string FormatCompact(long long value); // e.g. 999 -> "999", 1500 -> "1.5K", 1200000 -> "1.2M"
    // Draws a red gradient button (matching the HTML plugin button design) via DrawList.
    // Returns true if clicked. id must be unique (e.g. "##clear_hist").
    bool RedGradientButton(const char* label, const char* id, bool hovered_override = false);
    // Draws a green gradient button (matching the HTML plugin button design) via DrawList.
    // Returns true if clicked. id must be unique (e.g. "##save_acc").
    bool GreenGradientButton(const char* label, const char* id, bool hovered_override = false);
    // Draws an orange gradient button (matching the HTML plugin button design) via DrawList.
    // Returns true if clicked. id must be unique (e.g. "##info").
    bool OrangeGradientButton(const char* label, const char* id, bool hovered_override = false);
    void DrawCoinDisplay(long long copper);
    std::string FormatDuration(long long seconds);
    void TextWithTooltip(const char* text, float maxWidth = 200.0f, const ImVec4& color = ImVec4(1.f, 1.f, 1.f, 1.f));
    bool PassesRarityFilter(const Stat& st);
    void SortVisible(std::vector<std::pair<int, Stat>>& v);
    float CalcTableRowHeight(float contentHeight);
    void AlignTableCell(float rowHeight, float itemHeight);
    void AlignTableCellText(float rowHeight);
    void AlignTableCellFrame(float rowHeight);
    void AlignTableCellIcon(float rowHeight, float iconSize);
    bool ShouldShowTooltip();
    void SetTooltipIfEnabled(const char* tooltip);
}
