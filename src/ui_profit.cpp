#include "ui_profit.h"
#include "settings.h"
#include "item_tracker.h"
#include "localization.h"

namespace UIProfit
{
void RenderProfitTab()
{
    long long tpSell = ItemTracker::CalcTotalTpSellProfit();
    long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
    long long custom = ItemTracker::CalcTotalCustomProfit();

    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("profits_label"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("profits_tooltip"));
    ImGui::Separator();

    ImGui::Text("%s", Localization::GetText("approx_profits_label"));
    ImGui::SameLine();
    ImVec4 customColor = custom > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (custom < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(customColor, "%s", UICommon::FormatCoin(custom).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("approx_profits_tooltip"));

    ImGui::Spacing();
    ImGui::Text("%s", Localization::GetText("approx_gold_per_hour_label"));
    ImGui::SameLine();
    auto duration = ItemTracker::GetSessionDuration();
    long long seconds = duration.count();
    long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
    ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(profitPerHour).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("approx_gold_per_hour_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("trading_profits_label"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("trading_profits_tooltip"));
    ImGui::Separator();

    ImGui::Text("%s", Localization::GetText("approx_trading_profits_listings_label"));
    ImGui::SameLine();
    ImVec4 tpSellColor = tpSell > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpSell < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(tpSellColor, "%s", UICommon::FormatCoin(tpSell).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("approx_trading_profits_listings_tooltip"));

    ImGui::Spacing();
    ImGui::Text("%s", Localization::GetText("approx_trading_profits_instant_label"));
    ImGui::SameLine();
    ImVec4 tpInstantColor = tpInstant > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpInstant < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(tpInstantColor, "%s", UICommon::FormatCoin(tpInstant).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("approx_trading_profits_instant_tooltip"));

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("trading_details_label"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("trading_details_tooltip"));

    ImGui::Spacing();
    ImGui::Text("%s", Localization::GetText("lost_profit_vs_tp_sell_label"));
    ImGui::SameLine();
    long long opportunityCost = ItemTracker::GetOpportunityCostProfit();
    ImVec4 ocColor = opportunityCost > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (opportunityCost < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(ocColor, "%s", UICommon::FormatCoin(opportunityCost).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("lost_profit_vs_tp_sell_tooltip"));

    ImGui::Spacing();
    ImGui::Text("%s", Localization::GetText("lost_profit_per_hour_vs_tp_sell_label"));
    ImGui::SameLine();
    long long opportunityCostPerHour = ItemTracker::GetOpportunityCostProfitPerHour(duration);
    ImVec4 ocPerHourColor = opportunityCostPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (opportunityCostPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(ocPerHourColor, "%s", UICommon::FormatCoin(opportunityCostPerHour).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("lost_profit_per_hour_vs_tp_sell_tooltip"));

    if (seconds > 0)
    {
        ImGui::Spacing();
        ImGui::Separator();
        char sessionDurationLabel[256];
        snprintf(sessionDurationLabel, sizeof(sessionDurationLabel), Localization::GetText("session_duration_debug_label"), UICommon::FormatDuration(seconds).c_str());
        ImGui::Text("%s", sessionDurationLabel);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("session_duration_debug_tooltip"));
    }
}
}
