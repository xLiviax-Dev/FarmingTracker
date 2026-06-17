#include "ui_mini_window.h"
#include "ui.h"
#include "ui_common.h"
#include "settings.h"
#include "item_tracker.h"
#include "localization.h"

namespace UIMiniWindow
{
void RenderMiniWindow()
{
    if (!g_Settings.showMiniWindow) return;

    ImGui::SetNextWindowPos(ImVec2(g_Settings.miniWindowPosX, g_Settings.miniWindowPosY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(g_Settings.miniWindowWidth, g_Settings.miniWindowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(g_Settings.miniWindowOpacity);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (g_Settings.miniWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (g_Settings.miniWindowHideTitleBar)
        flags |= ImGuiWindowFlags_NoDecoration;

    if (g_Settings.miniWindowLocked)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("Farming Tracker Mini##FT_Mini", &g_Settings.showMiniWindow, flags))
    {
        PushAccentColor();

        // Apply font scale
        ImGui::SetWindowFontScale(g_Settings.miniWindowFontSize / 14.0f);

        // Convert text color from int to ImVec4 (RGB format)
        ImVec4 textColor = ImVec4(
            ((g_Settings.miniWindowTextColor >> 16) & 0xFF) / 255.0f,
            ((g_Settings.miniWindowTextColor >> 8) & 0xFF) / 255.0f,
            (g_Settings.miniWindowTextColor & 0xFF) / 255.0f,
            1.0f
        );

        // Save position
        ImVec2 pos = ImGui::GetWindowPos();
        g_Settings.miniWindowPosX = pos.x;
        g_Settings.miniWindowPosY = pos.y;

        // Save size
        ImVec2 size = ImGui::GetWindowSize();
        g_Settings.miniWindowWidth = size.x;
        g_Settings.miniWindowHeight = size.y;

        // Display selected metrics in custom order
        for (const auto& element : g_Settings.miniWindowElementOrder)
        {
            if (element == "Profit" && g_Settings.miniWindowShowProfit)
            {
                long long totalProfit = ItemTracker::CalcTotalCustomProfit();
                ImVec4 profitColor = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    // Render text 8 times with slight offset for outline effect
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                    ImGui::SetCursorPos(cursor);
                }
                
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("profit"));
                ImGui::SameLine();
                ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(totalProfit).c_str());
            }
            else if (element == "Profit/Hour" && g_Settings.miniWindowShowProfitPerHour)
            {
                auto duration = ItemTracker::GetSessionDuration();
                long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
                ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                    ImGui::SetCursorPos(cursor);
                }
                
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("profit_per_hour"));
                ImGui::SameLine();
                ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(profitPerHour).c_str());
            }
            else if (element == "TP Sell" && g_Settings.miniWindowShowTradingProfitSell)
            {
                long long tpSell = ItemTracker::CalcTotalTpSellProfit();
                ImVec4 tpSellColor = tpSell > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpSell < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                    ImGui::SetCursorPos(cursor);
                }
                
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("tp_sell"));
                ImGui::SameLine();
                ImGui::TextColored(tpSellColor, "%s", UICommon::FormatCoin(tpSell).c_str());
            }
            else if (element == "TP Instant" && g_Settings.miniWindowShowTradingProfitInstant)
            {
                long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
                ImVec4 tpInstantColor = tpInstant > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpInstant < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                    ImGui::SetCursorPos(cursor);
                }
                
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("tp_instant"));
                ImGui::SameLine();
                ImGui::TextColored(tpInstantColor, "%s", UICommon::FormatCoin(tpInstant).c_str());
            }
            else if (element == "Total Items" && g_Settings.miniWindowShowTotalItems)
            {
                auto items = ItemTracker::GetFilteredItems();
                size_t totalItems = items.size();
                ImVec4 totalItemsColor = totalItems > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalItems < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                    ImGui::SetCursorPos(cursor);
                }
                
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("total_items"));
                ImGui::SameLine();
                ImGui::TextColored(totalItemsColor, "%zu", totalItems);
            }
            else if (element == "Session Duration" && g_Settings.miniWindowShowSessionDuration)
            {
                auto duration = ItemTracker::GetSessionDuration();
                
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
                    ImGui::SetCursorPos(cursor);
                }
                
                ImGui::TextColored(textColor, "%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
            }
        }

        bool showSingle = false;
        if (g_Settings.miniWindowShowBestDropSingle)
        {
            auto bestDrop = ItemTracker::GetBestDrop();
            if (bestDrop.first != 0 && bestDrop.second.count > 0)
            {
                ImGui::Separator();
                
                // Best Drop Label
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));
                    ImGui::SetCursorPos(cursor);
                }
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("best_drop_single"));
                ImGui::SameLine();
                
                // Show icon if enabled
                if (g_Settings.miniWindowShowBestDropIcons)
                {
                    // Draw the icon
                    float iconSize = static_cast<float>(g_Settings.miniWindowBestDropIconSize);
                    UICommon::DrawItemIconCell(bestDrop.first, bestDrop.second.details.iconUrl, iconSize, bestDrop.second.details.rarity, true);
                    // Move cursor to the right of the icon
                    ImGui::SameLine(0.0f, 4.0f);
                }
                
                // For single drop, we show the unit value in the mini window to differentiate
                long long totalProfit = ItemTracker::GetStatProfit(bestDrop.second);
                long long unitProfit = totalProfit / bestDrop.second.count;
                
                ImVec4 bestDropColor = unitProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (unitProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                // Best Drop Name
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(cursor);
                }
                ImGui::TextColored(bestDropColor, "%s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : Localization::GetText("loading"));
                
                // Unit Value Label
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                    ImGui::SetCursorPos(cursor);
                }
                ImGui::TextColored(textColor, "%s: %s", Localization::GetText("unit_value"), UICommon::FormatCoin(unitProfit).c_str());
                
                showSingle = true;
            }
        }

        if (g_Settings.miniWindowShowBestDropTotalValue)
        {
            auto bestTotal = ItemTracker::GetBestDropTotalValue();
            if (bestTotal.first != 0 && bestTotal.second.count > 0)
            {
                if (!showSingle) ImGui::Separator();
                
                // Best Drop Total Label
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));
                    ImGui::SetCursorPos(cursor);
                }
                ImGui::TextColored(textColor, "%s: ", Localization::GetText("best_drop_total"));
                ImGui::SameLine();
                
                // Show icon if enabled
                if (g_Settings.miniWindowShowBestDropIcons)
                {
                    // Draw the icon
                    float iconSize = static_cast<float>(g_Settings.miniWindowBestDropIconSize);
                    UICommon::DrawItemIconCell(bestTotal.first, bestTotal.second.details.iconUrl, iconSize, bestTotal.second.details.rarity, true);
                    // Move cursor to the right of the icon
                    ImGui::SameLine(0.0f, 4.0f);
                }
                
                long long bestTotalProfit = ItemTracker::GetStatProfit(bestTotal.second);
                ImVec4 bestTotalColor = bestTotalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (bestTotalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                // Best Drop Total Name
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                    ImGui::SetCursorPos(cursor);
                }
                ImGui::TextColored(bestTotalColor, "%s", bestTotal.second.details.loaded ? bestTotal.second.details.name.c_str() : Localization::GetText("loading"));
                
                // Total Value Label
                if (g_Settings.miniWindowEnableTextShadow)
                {
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y - 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x - 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(ImVec2(cursor.x + 1, cursor.y + 1));
                    ImGui::TextColored(ImVec4(0, 0, 0, 1.0f), "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
                    ImGui::SetCursorPos(cursor);
                }
                ImGui::TextColored(textColor, "%s (%zu): %s", Localization::GetText("total_value"), bestTotal.second.count, UICommon::FormatCoin(bestTotalProfit).c_str());
            }
        }
    }

    PopAccentColor();
    ImGui::End();
}
}
