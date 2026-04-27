#include "ui_custom_profit.h"
#include "settings.h"
#include "custom_profit.h"
#include "item_tracker.h"
#include "localization.h"
#include "ui_common.h"
#include <vector>

namespace UICustomProfit
{
void RenderCustomProfitTab()
{
    ImGui::Text("%s", Localization::GetText("custom_profit_system"));
    ImGui::Separator();

    ImGui::Spacing();

    // Clear All Button
    if (ImGui::Button(Localization::GetText("clear_all_custom_profits")))
    {
        CustomProfitManager::ClearAll();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_all_custom_profits_tooltip"));

    ImGui::Spacing();

    // Add Custom Profit for Item
    ImGui::Text("%s", Localization::GetText("add_custom_profit_item"));
    static int itemIdInput = 0;
    static long long profitValueInput = 0;
    ImGui::InputInt("Item ID##AddItem", &itemIdInput);
    ImGui::InputScalar("Profit (Copper)##AddItemProfit", ImGuiDataType_S64, &profitValueInput);
    ImGui::SameLine();
    if (ImGui::Button(Localization::GetText("custom_profit_set_profit")))
    {
        if (itemIdInput > 0)
        {
            CustomProfitManager::SetCustomProfit(itemIdInput, profitValueInput);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("custom_profit_set_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();

    // Add Custom Profit for Currency
    ImGui::Text("%s", Localization::GetText("add_custom_profit_currency"));
    static int currencyIdInput = 0;
    static long long currencyProfitValueInput = 0;
    ImGui::InputInt("Currency ID##AddCurrency", &currencyIdInput);
    ImGui::InputScalar("Profit (Copper)##AddCurrencyProfit", ImGuiDataType_S64, &currencyProfitValueInput);
    ImGui::SameLine();
    char setCurrencyProfitButtonLabel[256];
    snprintf(setCurrencyProfitButtonLabel, sizeof(setCurrencyProfitButtonLabel), "%s##Currency", Localization::GetText("custom_profit_set_profit"));
    if (ImGui::Button(setCurrencyProfitButtonLabel))
    {
        if (currencyIdInput > 0)
        {
            CustomProfitManager::SetCustomProfit(currencyIdInput, currencyProfitValueInput);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("custom_profit_set_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();

    // Items with Custom Profit
    ImGui::Text("%s", Localization::GetText("custom_profit_items_header"));
    ImGui::Separator();

    auto items = ItemTracker::GetItemsCopy();
    std::vector<std::pair<int, long long>> customProfitItems;
    for (auto& [id, st] : items)
    {
        if (CustomProfitManager::HasCustomProfit(id))
        {
            customProfitItems.push_back({id, CustomProfitManager::GetCustomProfit(id)});
        }
    }

    if (customProfitItems.empty())
    {
        ImGui::TextDisabled("%s", Localization::GetText("no_custom_profit_items"));
    }
    else
    {
        if (ImGui::BeginTable("CustomProfitItemsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 40.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("custom_profit_value"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("custom_profit_remove"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
            ImGui::TableHeadersRow();

            for (auto& [id, profit] : customProfitItems)
            {
                auto it = items.find(id);
                if (it == items.end()) continue;
                auto& st = it->second;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                UICommon::DrawItemIconCell(id, st.details.iconUrl, 32.0f, st.details.loaded ? st.details.rarity : "");

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : "Loading...");

                ImGui::TableSetColumnIndex(2);
                char profitLabel[256];
                snprintf(profitLabel, sizeof(profitLabel), "%lld", profit);
                ImGui::Text("%s", profitLabel);

                ImGui::TableSetColumnIndex(3);
                char removeButtonLabel[256];
                snprintf(removeButtonLabel, sizeof(removeButtonLabel), "Remove##%d", id);
                if (ImGui::Button(removeButtonLabel))
                {
                    CustomProfitManager::RemoveCustomProfit(id);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("custom_profit_remove_tooltip"));
            }
            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Currencies with Custom Profit
    ImGui::Text("%s", Localization::GetText("custom_profit_currencies_header"));
    ImGui::Separator();

    auto currencies = ItemTracker::GetCurrenciesCopy();
    std::vector<std::pair<int, long long>> customProfitCurrencies;
    for (auto& [id, st] : currencies)
    {
        if (CustomProfitManager::HasCustomProfit(id))
        {
            customProfitCurrencies.push_back({id, CustomProfitManager::GetCustomProfit(id)});
        }
    }

    if (customProfitCurrencies.empty())
    {
        ImGui::TextDisabled("%s", Localization::GetText("no_custom_profit_currencies"));
    }
    else
    {
        if (ImGui::BeginTable("CustomProfitCurrenciesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 40.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("custom_profit_value"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("custom_profit_remove"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
            ImGui::TableHeadersRow();

            for (auto& [id, profit] : customProfitCurrencies)
            {
                auto it = currencies.find(id);
                if (it == currencies.end()) continue;
                auto& st = it->second;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                UICommon::DrawItemIconCell(id, st.details.iconUrl, 32.0f, st.details.loaded ? st.details.rarity : "");

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : "Loading...");

                ImGui::TableSetColumnIndex(2);
                char profitLabel[256];
                snprintf(profitLabel, sizeof(profitLabel), "%lld", profit);
                ImGui::Text("%s", profitLabel);

                ImGui::TableSetColumnIndex(3);
                char removeButtonLabel[256];
                snprintf(removeButtonLabel, sizeof(removeButtonLabel), "Remove##%d", id);
                if (ImGui::Button(removeButtonLabel))
                {
                    CustomProfitManager::RemoveCustomProfit(id);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("custom_profit_remove_tooltip"));
            }
            ImGui::EndTable();
        }
    }
}
}
