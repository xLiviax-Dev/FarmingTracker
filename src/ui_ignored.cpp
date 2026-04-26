#include "ui_ignored.h"
#include "settings.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"

namespace UIIgnored
{
void RenderIgnoredTab()
{
    static int ignoredSubTab = 0; // 0 = Items, 1 = Currencies

    ImGui::BeginTabBar("IgnoredSubTabs");
    if (ImGui::BeginTabItem(Localization::GetText("tab_items")))
    {
        ignoredSubTab = 0;
        ImGui::Text("%s", Localization::GetText("manage_ignored_items"));

        auto ignoredItems = IgnoredItemsManager::GetIgnoredItems();

        size_t ignoredItemsCount = ignoredItems.size();
        ImVec4 ignoredItemsColor = ignoredItemsCount > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (ignoredItemsCount < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextColored(ignoredItemsColor, "%s %zu", Localization::GetText("ignored_items_label"), ignoredItemsCount);

        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("clear_all_ignored_items")))
        {
            for (auto& id : ignoredItems)
                IgnoredItemsManager::UnignoreItem(id);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Display ignored items in a table
        if (ImGui::BeginTable("##IgnoredItemsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f;
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 110.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 180.0f);
            ImGui::TableHeadersRow();

            for (auto& id : ignoredItems)
            {
                auto items = ItemTracker::GetItemsCopy();
                if (items.find(id) != items.end())
                {
                    auto& st = items[id];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");

                    ImGui::TableSetColumnIndex(1);
                    std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
                    ImGui::Text("%s", name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    bool isIgnored = true;
                    if (ImGui::Checkbox(("##unign_" + std::to_string(id)).c_str(), &isIgnored))
                    {
                        if (!isIgnored)
                            IgnoredItemsManager::UnignoreItem(id);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", Localization::GetText("unignore_item"));
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(Localization::GetText("tab_currencies")))
    {
        ignoredSubTab = 1;
        ImGui::Text("%s", Localization::GetText("manage_ignored_currencies"));

        auto ignoredCurrencies = IgnoredItemsManager::GetIgnoredCurrencies();

        size_t ignoredCurrenciesCount = ignoredCurrencies.size();
        ImVec4 ignoredCurrenciesColor = ignoredCurrenciesCount > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (ignoredCurrenciesCount < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextColored(ignoredCurrenciesColor, "%s %zu", Localization::GetText("ignored_currencies_label"), ignoredCurrenciesCount);

        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("clear_all_ignored_currencies")))
        {
            for (auto& id : ignoredCurrencies)
                IgnoredItemsManager::UnignoreCurrency(id);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Display ignored currencies in a table
        if (ImGui::BeginTable("##IgnoredCurrenciesTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f;
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
            ImGui::TableSetupColumn(Localization::GetText("column_currency"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 100.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 180.0f);
            ImGui::TableHeadersRow();

            for (auto& id : ignoredCurrencies)
            {
                auto currencies = ItemTracker::GetCurrenciesCopy();
                if (currencies.find(id) != currencies.end())
                {
                    auto& st = currencies[id];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty())
                        iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");

                    ImGui::TableSetColumnIndex(1);
                    std::string name = st.details.loaded ? st.details.name : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"));
                    ImGui::Text("%s", name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    bool isIgnored = true;
                    if (ImGui::Checkbox(("##unign_cur_" + std::to_string(id)).c_str(), &isIgnored))
                    {
                        if (!isIgnored)
                            IgnoredItemsManager::UnignoreCurrency(id);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", Localization::GetText("unignore_currency"));
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
}
