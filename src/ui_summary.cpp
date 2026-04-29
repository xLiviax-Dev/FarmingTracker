#include "ui_summary.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "shared.h"
#include "localization.h"
#include "ignored_items.h"
#include "ui_context_menu.h"
#include <chrono>

namespace UISummary
{
static void RenderProfitSparkline()
{
    if (!g_Settings.showProfitSparkline) return;

    auto history = ItemTracker::GetProfitHistory();
    if (history.size() < 2) return;

    // Extract profit values for plotting
    std::vector<float> values;
    for (const auto& entry : history)
    {
        values.push_back(static_cast<float>(entry.second));
    }

    if (values.empty()) return;

    // Calculate min/max for scaling
    float minVal = *std::min_element(values.begin(), values.end());
    float maxVal = *std::max_element(values.begin(), values.end());

    // Draw sparkline
    ImGui::SameLine();
    float sparklineWidth = 100.0f;
    float sparklineHeight = 20.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 p_min = cursor;
    ImVec2 p_max = ImVec2(cursor.x + sparklineWidth, cursor.y + sparklineHeight);

    // Draw background
    drawList->AddRectFilled(p_min, p_max, IM_COL32(50, 50, 50, 100));

    // Draw line
    if (values.size() > 1)
    {
        for (size_t i = 0; i < values.size() - 1; i++)
        {
            float x1 = p_min.x + (static_cast<float>(i) / (values.size() - 1)) * sparklineWidth;
            float x2 = p_min.x + (static_cast<float>(i + 1) / (values.size() - 1)) * sparklineWidth;
            
            float y1, y2;
            if (maxVal != minVal)
            {
                y1 = p_max.y - ((values[i] - minVal) / (maxVal - minVal)) * sparklineHeight;
                y2 = p_max.y - ((values[i + 1] - minVal) / (maxVal - minVal)) * sparklineHeight;
            }
            else
            {
                y1 = p_min.y + sparklineHeight / 2;
                y2 = p_min.y + sparklineHeight / 2;
            }

            ImVec4 color = values[i] > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (values[i] < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(color), 2.0f);
        }
    }

    // Hover tooltip
    ImGui::SetCursorScreenPos(p_min);
    ImGui::InvisibleButton("Sparkline", ImVec2(sparklineWidth, sparklineHeight));
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", Localization::GetText("profit_per_hour_label_simple"));
        for (size_t i = 0; i < history.size(); i++)
        {
            auto timeSince = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - history[i].first);
            ImGui::Text("%lld ago: %s", timeSince.count(), UICommon::FormatCoin(history[i].second).c_str());
        }
        ImGui::EndTooltip();
    }

    ImGui::SetCursorScreenPos(ImVec2(p_max.x, cursor.y));
}

void RenderSummaryTab()
{
    // DRF Status Warning
    DrfStatus status = DrfClient::GetStatus();
    if (status == DrfStatus::Disconnected)
    {
        ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "%s", Localization::GetText("warning_drf_not_connected"));
        ImGui::TextDisabled("%s", Localization::GetText("warning_drf_not_connected_desc"));
        ImGui::TextDisabled("%s", Localization::GetText("warning_drf_install"));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }
    else if (status == DrfStatus::AuthFailed)
    {
        ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "%s", Localization::GetText("warning_drf_token_invalid"));
        ImGui::TextDisabled("%s", Localization::GetText("warning_drf_token_invalid_desc"));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    // API Keys Warning
    if (g_Settings.gw2ApiKey.empty())
    {
        ImGui::TextColored(ImVec4(1.f, 0.6f, 0.0f, 1.f), "%s", Localization::GetText("warning_gw2_api_key_not_set"));
        ImGui::TextDisabled("%s", Localization::GetText("warning_gw2_api_key_not_set_desc"));
        ImGui::Spacing();
        ImGui::Separator();
    }

    // Overview
    ImGui::Text("%s", Localization::GetText("overview_label"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("overview_tooltip"));
    ImGui::Separator();

    if (ImGui::BeginTable("OverviewStatsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn(Localization::GetText("column_label"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 200.0f);
        ImGui::TableSetupColumn(Localization::GetText("column_value"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        long long totalProfit = ItemTracker::CalcTotalCustomProfit();
        auto duration = ItemTracker::GetSessionDuration();
        long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
        auto items = ItemTracker::GetItemsCopy();
        auto currencies = ItemTracker::GetCurrenciesCopy();

        // Total Profit
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", Localization::GetText("total_profit_label_simple"));
        ImGui::TableSetColumnIndex(1);
        ImVec4 profitColor = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(totalProfit).c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("total_profit_tooltip"));

        // Total Items
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", Localization::GetText("total_items_label_simple"));
        ImGui::TableSetColumnIndex(1);
        size_t totalItems = items.size();
        ImVec4 totalItemsColor = totalItems > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalItems < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextColored(totalItemsColor, "%zu", totalItems);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("total_items_tooltip"));

        // Total Currencies
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", Localization::GetText("total_currencies_label_simple"));
        ImGui::TableSetColumnIndex(1);
        size_t totalCurrencies = currencies.size();
        ImVec4 totalCurrenciesColor = totalCurrencies > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalCurrencies < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextColored(totalCurrenciesColor, "%zu", totalCurrencies);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("total_currencies_tooltip"));

        // Profit Per Hour
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", Localization::GetText("profit_per_hour_label_simple"));
        ImGui::TableSetColumnIndex(1);
        ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(profitPerHour).c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("profit_per_hour_tooltip"));
        RenderProfitSparkline();

        // Session Duration
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", Localization::GetText("session_duration_label_simple"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", UICommon::FormatDuration(duration.count()).c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("session_duration_tooltip"));

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Top Items by Profit (Table)
    char topItemsProfitHeader[256];
    snprintf(topItemsProfitHeader, sizeof(topItemsProfitHeader), "%s##TopItemsProfitHeader", Localization::GetText("top_items_profit_header"));
    if (ImGui::CollapsingHeader(topItemsProfitHeader, g_Settings.showTopItems ? ImGuiTreeNodeFlags_DefaultOpen : 0))
    {
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("top_items_profit_tooltip"));

        if (ImGui::BeginTable("TopItemsProfitTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn(Localization::GetText("column_item"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 130.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 120.0f);
            ImGui::TableHeadersRow();

            auto sortedByProfit = ItemTracker::GetSortedItems(ItemTracker::SortMode::ProfitDesc);
            int count = 0;
            for (auto& [id, st] : sortedByProfit)
            {
                if (count >= 10) break;
                if (st.count == 0) continue;
                long long profit = ItemTracker::GetStatProfit(st);
                if (profit <= 0) continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
                ImGui::Text("%s", name.c_str());

                // Right-click context menu for top items
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                {
                    UIContextMenu::OpenContextMenu("TopItemContextMenu", id, st.details.loaded ? st.details.name : "");
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lld", st.count);
                ImGui::TableSetColumnIndex(2);
                ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                count++;
            }

            // Context menu popup (rendered once outside the loop)
            UIContextMenu::RenderItemContextMenu("TopItemContextMenu", UIContextMenu::ContextMenuType::General);

            ImGui::EndTable();
        }
    }
    if (ImGui::IsItemClicked())
        SettingsManager::Save();

    ImGui::Spacing();
    char topItemsCountHeader[256];
    snprintf(topItemsCountHeader, sizeof(topItemsCountHeader), "%s##TopItemsHeader", Localization::GetText("top_items_count_header"));
    if (ImGui::CollapsingHeader(topItemsCountHeader, g_Settings.showTopItems ? ImGuiTreeNodeFlags_DefaultOpen : 0))
    {
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("top_items_count_tooltip"));

        if (ImGui::BeginTable("TopItemsCountTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn(Localization::GetText("column_item"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 130.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableHeadersRow();

            auto sortedByCount = ItemTracker::GetSortedItems(ItemTracker::SortMode::CountDesc);
            int count = 0;
            for (auto& [id, st] : sortedByCount)
            {
                if (count >= 10) break;
                if (st.count == 0) continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
                ImGui::Text("%s", name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                ImGui::TextColored(countColor, "%lld", st.count);
                count++;
            }
            ImGui::EndTable();
        }
    }
    if (ImGui::IsItemClicked())
        SettingsManager::Save();

    ImGui::Spacing();
    char topCurrenciesCountHeader[256];
    snprintf(topCurrenciesCountHeader, sizeof(topCurrenciesCountHeader), "%s##TopCurrenciesHeader", Localization::GetText("top_currencies_count_header"));
    if (ImGui::CollapsingHeader(topCurrenciesCountHeader, g_Settings.showTopCurrencies ? ImGuiTreeNodeFlags_DefaultOpen : 0))
    {
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("top_currencies_count_tooltip"));

        if (ImGui::BeginTable("TopCurrenciesCountTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn(Localization::GetText("column_currency"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 100.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableHeadersRow();

            auto sortedCurrencies = ItemTracker::GetSortedCurrencies(ItemTracker::SortMode::CountDesc);
            int count = 0;
            for (auto& [id, st] : sortedCurrencies)
            {
                if (count >= 10) break;
                if (st.count == 0) continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                std::string name = st.details.loaded ? st.details.name : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"));
                ImGui::Text("%s", name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                ImGui::TextColored(countColor, "%lld", st.count);
                count++;
            }
            ImGui::EndTable();
        }
    }
    if (ImGui::IsItemClicked())
        SettingsManager::Save();

    ImGui::Spacing();
    ImGui::Spacing();

    // No Data Warning
    auto items = ItemTracker::GetItemsCopy();
    auto currencies = ItemTracker::GetCurrenciesCopy();
    if (items.empty() && currencies.empty())
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.f, 0.6f, 0.0f, 1.f), "%s", Localization::GetText("warning_no_data"));
        ImGui::TextDisabled("%s", Localization::GetText("warning_no_data_desc"));
    }

    // Export Buttons
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("export_label"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("export_tooltip"));
    char exportJsonLabel[256];
    snprintf(exportJsonLabel, sizeof(exportJsonLabel), "%s", Localization::GetText("export_json"));
    if (ImGui::Button(exportJsonLabel))
    {
        std::string jsonData = ItemTracker::ExportToJson();
        const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
        std::string filename = std::string(addonDir ? addonDir : "") + "\\FarmingTracker_Export_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json";

        // Use standard file operations to save
        FILE* file = nullptr;
        errno_t err = fopen_s(&file, filename.c_str(), "w");
        if (err == 0 && file)
        {
            if (fprintf(file, "%s", jsonData.c_str()) < 0)
            {
                // Handle write error
            }
            fclose(file);
        }
    }
    ImGui::SameLine();
    char exportCsvLabel[256];
    snprintf(exportCsvLabel, sizeof(exportCsvLabel), "%s", Localization::GetText("export_csv"));
    if (ImGui::Button(exportCsvLabel))
    {
        std::string csvData = ItemTracker::ExportToCsv();
        const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
        std::string filename = std::string(addonDir ? addonDir : "") + "\\FarmingTracker_Export_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".csv";

        // Use standard file operations to save
        FILE* file = nullptr;
        errno_t err = fopen_s(&file, filename.c_str(), "w");
        if (err == 0 && file)
        {
            if (fprintf(file, "%s", csvData.c_str()) < 0)
            {
                // Handle write error
            }
            fclose(file);
        }
    }
}
}
