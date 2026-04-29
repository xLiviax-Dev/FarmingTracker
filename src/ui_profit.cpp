#include "ui_profit.h"
#include "settings.h"
#include "item_tracker.h"
#include "localization.h"
#include "session_history.h"
#include "ui_common.h"

namespace UIProfit
{
static int s_SummaryPeriod = 0; // 0 = Today, 1 = This Week, 2 = This Month
void RenderProfitTab()
{
    long long tpSell = ItemTracker::CalcTotalTpSellProfit();
    long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
    long long custom = ItemTracker::CalcTotalCustomProfit();

    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("profits_label"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("profits_tooltip"));

    // Summaries Checkbox (only if Session History is enabled)
    if (g_Settings.enableSessionHistory)
    {
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("show_summaries"), &g_Settings.enableSummariesInProfitTab))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("show_summaries_tooltip"));
    }

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

    // Summaries Section
    if (g_Settings.enableSessionHistory && g_Settings.enableSummariesInProfitTab)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("%s", Localization::GetText("summaries_label"));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("summaries_tooltip"));
        ImGui::Separator();

        // Period Selection
        ImGui::Text("%s", Localization::GetText("summary_period"));
        ImGui::SameLine();
        if (ImGui::RadioButton(Localization::GetText("summary_today"), s_SummaryPeriod == 0))
            s_SummaryPeriod = 0;
        ImGui::SameLine();
        if (ImGui::RadioButton(Localization::GetText("summary_this_week"), s_SummaryPeriod == 1))
            s_SummaryPeriod = 1;
        ImGui::SameLine();
        if (ImGui::RadioButton(Localization::GetText("summary_this_month"), s_SummaryPeriod == 2))
            s_SummaryPeriod = 2;

        ImGui::Spacing();
        ImGui::Separator();

        // Get summary data
        SessionHistory::SummaryPeriod period;
        switch (s_SummaryPeriod)
        {
        case 0: period = SessionHistory::SummaryPeriod::Today; break;
        case 1: period = SessionHistory::SummaryPeriod::ThisWeek; break;
        case 2: period = SessionHistory::SummaryPeriod::ThisMonth; break;
        }
        auto summary = SessionHistory::GetSummary(period);

        // Display summary stats
        ImGui::Text("%s: %s", Localization::GetText("total_profit"), UICommon::FormatCoin(summary.totalProfit).c_str());
        ImGui::Text("%s: %s", Localization::GetText("profit_per_hour"), UICommon::FormatCoin(summary.profitPerHour).c_str());
        ImGui::Text("%s: %d", Localization::GetText("total_drops"), summary.totalDrops);
        ImGui::Text("%s: %d", Localization::GetText("session_count"), summary.sessionCount);
        ImGui::Text("%s: %s", Localization::GetText("total_duration"), UICommon::FormatDuration(summary.totalDurationSeconds).c_str());

        // Comparison with previous period
        if (summary.previousPeriodProfit != 0)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("%s", Localization::GetText("comparison_previous_period"));
            long long diff = summary.totalProfit - summary.previousPeriodProfit;
            float percentChange = summary.previousPeriodProfit != 0 ? ((float)diff / summary.previousPeriodProfit) * 100.0f : 0.0f;
            ImVec4 diffColor = diff > 0 ? ImVec4(0.f, 1.f, 0.f, 1.f) : (diff < 0 ? ImVec4(1.f, 0.f, 0.f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextColored(diffColor, "%s: %s (%.1f%%)", Localization::GetText("profit_change"), UICommon::FormatCoin(diff).c_str(), percentChange);
        }

        // Top Drops
        if (!summary.topDrops.empty())
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("%s", Localization::GetText("top_drops"));
            if (ImGui::BeginTable("ProfitTopDrops", 3, ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn(Localization::GetText("item"));
                ImGui::TableSetupColumn(Localization::GetText("count"));
                ImGui::TableSetupColumn(Localization::GetText("value"));
                ImGui::TableHeadersRow();

                for (const auto& drop : summary.topDrops)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", drop.itemName.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", drop.count);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", UICommon::FormatCoin(drop.totalValue).c_str());
                }
                ImGui::EndTable();
            }
        }
    }

    // Best Drop of Current Session
    auto bestDrop = ItemTracker::GetBestDrop();
    if (bestDrop.first != 0 && bestDrop.second.count > 0)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("%s", Localization::GetText("best_drop"));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("best_drop_tooltip"));
        ImGui::Separator();

        long long bestDropProfit = ItemTracker::GetStatProfit(bestDrop.second);
        ImGui::Text("%s: %s", bestDrop.second.details.loaded ? bestDrop.second.details.name.c_str() : "Loading...", UICommon::FormatCoin(bestDropProfit).c_str());
        ImGui::Text("%s: %lld", Localization::GetText("count_label"), bestDrop.second.count);
    }
}
}
