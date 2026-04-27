#include "ui_mini_window.h"
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

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (g_Settings.miniWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Farming Tracker Mini##FT_Mini", &g_Settings.showMiniWindow, flags))
    {
        // Save position
        ImVec2 pos = ImGui::GetWindowPos();
        g_Settings.miniWindowPosX = pos.x;
        g_Settings.miniWindowPosY = pos.y;

        // Save size
        ImVec2 size = ImGui::GetWindowSize();
        g_Settings.miniWindowWidth = size.x;
        g_Settings.miniWindowHeight = size.y;

        // Display selected metrics
        if (g_Settings.miniWindowShowProfit)
        {
            long long totalProfit = ItemTracker::CalcTotalCustomProfit();
            ImVec4 profitColor = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Text("%s: ", Localization::GetText("profit"));
            ImGui::SameLine();
            ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(totalProfit).c_str());
        }

        if (g_Settings.miniWindowShowProfitPerHour)
        {
            auto duration = ItemTracker::GetSessionDuration();
            long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
            ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Text("%s: ", Localization::GetText("profit_per_hour"));
            ImGui::SameLine();
            ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(profitPerHour).c_str());
        }

        if (g_Settings.miniWindowShowTradingProfitSell)
        {
            long long tpSell = ItemTracker::CalcTotalTpSellProfit();
            ImVec4 tpSellColor = tpSell > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpSell < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Text("%s: ", Localization::GetText("tp_sell"));
            ImGui::SameLine();
            ImGui::TextColored(tpSellColor, "%s", UICommon::FormatCoin(tpSell).c_str());
        }

        if (g_Settings.miniWindowShowTradingProfitInstant)
        {
            long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
            ImVec4 tpInstantColor = tpInstant > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpInstant < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Text("%s: ", Localization::GetText("tp_instant"));
            ImGui::SameLine();
            ImGui::TextColored(tpInstantColor, "%s", UICommon::FormatCoin(tpInstant).c_str());
        }

        if (g_Settings.miniWindowShowTotalItems)
        {
            auto items = ItemTracker::GetItemsCopy();
            size_t totalItems = items.size();
            ImVec4 totalItemsColor = totalItems > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalItems < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Text("%s: ", Localization::GetText("total_items"));
            ImGui::SameLine();
            ImGui::TextColored(totalItemsColor, "%zu", totalItems);
        }

        if (g_Settings.miniWindowShowSessionDuration)
        {
            auto duration = ItemTracker::GetSessionDuration();
            ImGui::Text("%s: %s", Localization::GetText("session"), UICommon::FormatDuration(duration.count()).c_str());
        }
    }

    ImGui::End();
}
}
