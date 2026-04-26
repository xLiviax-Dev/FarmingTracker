#include "ui_items.h"
#include "settings.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"
#include <algorithm>

namespace UIItems
{
void RenderItemsTab()
{
    const char* sortLabels[] = {
        Localization::GetText("sort_price_down"),
        Localization::GetText("sort_price_up"),
        Localization::GetText("sort_count_high"),
        Localization::GetText("sort_count_low"),
        Localization::GetText("sort_name_az")
    };
    if (ImGui::Combo("##SortItems", &g_Settings.itemSortMode, sortLabels, 5))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("sort_tooltip"));

    ImGui::SameLine();

    const char* rarityLabels[] = {
        Localization::GetText("rarity_all"),
        Localization::GetText("rarity_basic"),
        Localization::GetText("rarity_fine"),
        Localization::GetText("rarity_masterwork"),
        Localization::GetText("rarity_rare"),
        Localization::GetText("rarity_exotic"),
        Localization::GetText("rarity_ascended"),
        Localization::GetText("rarity_legendary")
    };
    int rarityCombo = std::clamp(g_Settings.itemRarityFilterMin, 0, 7);
    if (ImGui::Combo("##RarityF", &rarityCombo, rarityLabels, 8))
    {
        g_Settings.itemRarityFilterMin = rarityCombo;
        SettingsManager::Save();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("rarity_tooltip"));

    ImGui::Spacing();

    // Search bar
    if (ImGui::InputTextWithHint("##Search", Localization::GetText("search_items_hint"), UICommon::s_SearchBuf, sizeof(UICommon::s_SearchBuf)))
    {
        g_Settings.searchTerm = UICommon::s_SearchBuf;
        SettingsManager::Save();
    }
    ImGui::Spacing();

    auto sortedItems = ItemTracker::GetSortedItems(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));

    if (g_Settings.enableGridViewItems)
    {
        // Grid View
        float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f; // icon + padding
        ImVec2 windowSize = ImGui::GetWindowSize();
        int columns = std::clamp(static_cast<int>(windowSize.x / cellSize), 2, 50);

        if (ImGui::BeginChild("##ItemsGrid", ImVec2(0, 0), true))
        {
            int col = 0;
            for (auto& [id, st] : sortedItems)
            {
                if (col > 0)
                    ImGui::SameLine();

                ImGui::PushID(id);
                if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                {
                    // Icon
                    ImVec2 cursor = ImGui::GetCursorPos();
                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");

                    // Tooltip for icon
                    if (ImGui::IsItemHovered() && st.details.loaded)
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", st.details.name.c_str());
                        ImGui::Separator();
                        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(countColor, "%s %lld", Localization::GetText("count_label"), st.count);
                        long long profit = ItemTracker::GetStatProfit(st);
                        if (profit > 0)
                            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s %s", Localization::GetText("profit_label"), UICommon::FormatCoin(profit).c_str());
                        else if (profit < 0)
                            ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s %s", Localization::GetText("profit_label"), UICommon::FormatCoin(profit).c_str());
                        else
                            ImGui::TextUnformatted(Localization::GetText("no_profit"));
                        ImGui::Separator();
                        char rarityLabel[256];
                        snprintf(rarityLabel, sizeof(rarityLabel), Localization::GetText("rarity_label"), st.details.rarity.c_str());
                        ImGui::Text("%s", rarityLabel);
                        char typeLabel[256];
                        snprintf(typeLabel, sizeof(typeLabel), Localization::GetText("type_label"), static_cast<int>(st.details.itemType));
                        ImGui::Text("%s", typeLabel);
                        ImVec4 vendorColor = st.details.vendorValue > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.details.vendorValue < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(vendorColor, "%s %s", Localization::GetText("vendor_value_label"), UICommon::FormatCoin(st.details.vendorValue).c_str());
                        ImVec4 tpSellGrossColor = st.details.tpSellPrice > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.details.tpSellPrice < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(tpSellGrossColor, "%s %s", Localization::GetText("tp_sell_gross_label"), UICommon::FormatCoin(st.details.tpSellPrice).c_str());
                        long long tpSellNet = static_cast<long long>(st.details.tpSellPrice * 85.0 / 100.0);
                        ImVec4 tpSellNetColor = tpSellNet > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpSellNet < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(tpSellNetColor, "%s %s", Localization::GetText("tp_sell_net_label"), UICommon::FormatCoin(tpSellNet).c_str());
                        ImVec4 tpBuyGrossColor = st.details.tpBuyPrice > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.details.tpBuyPrice < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(tpBuyGrossColor, "%s %s", Localization::GetText("tp_buy_gross_label"), UICommon::FormatCoin(st.details.tpBuyPrice).c_str());
                        long long tpBuyNet = static_cast<long long>(st.details.tpBuyPrice * 85.0 / 100.0);
                        ImVec4 tpBuyNetColor = tpBuyNet > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpBuyNet < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(tpBuyNetColor, "%s %s", Localization::GetText("tp_buy_net_label"), UICommon::FormatCoin(tpBuyNet).c_str());
                        char accountBoundLabel[256];
                        snprintf(accountBoundLabel, sizeof(accountBoundLabel), Localization::GetText("account_bound_label"), st.details.accountBound ? Localization::GetText("yes_label") : Localization::GetText("no_label"));
                        ImGui::Text("%s", accountBoundLabel);
                        char noSellLabel[256];
                        snprintf(noSellLabel, sizeof(noSellLabel), Localization::GetText("nosell_label"), st.details.noSell ? Localization::GetText("yes_label") : Localization::GetText("no_label"));
                        ImGui::Text("%s", noSellLabel);
                        ImGui::EndTooltip();
                    }

                    // Count text (bottom right, proportional to icon size)
                    ImVec2 iconSize = ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize));
                    float offsetX = iconSize.x * 0.15f; // 15% offset from right
                    float offsetY = iconSize.y * 0.15f; // 15% offset from bottom
                    ImVec2 countPos = ImVec2(cursor.x + iconSize.x - ImGui::CalcTextSize("999").x - offsetX, cursor.y + iconSize.y - ImGui::CalcTextSize("999").y - offsetY);
                    ImGui::SetCursorPos(countPos);

                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    float fontSize = static_cast<float>(g_Settings.gridIconSize) * 0.4f; // Increased from 0.3f to 0.4f
                    ImGui::PushFont(ImGui::GetFont());
                    ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());
                    ImGui::TextColored(countColor, "%lld", st.count);
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::PopFont();
                }
                ImGui::EndChild();
                ImGui::PopID();

                col++;
                if (col >= columns)
                {
                    col = 0;
                }
            }
        }
        ImGui::EndChild();
    }
    else
    {
        // Items Table with enhanced features
        int itemTableColumnCount = 6;
        if (ImGui::BeginTable("##ItemsTable", itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f; // iconSize + padding
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 110.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 120.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_favorite"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 180.0f);
            ImGui::TableHeadersRow();

            for (auto& [id, st] : sortedItems)
            {
                // Skip ignored items if showIgnoredItems is disabled
                if (!g_Settings.showIgnoredItems && st.isIgnored)
                    continue;

                ImGui::TableNextRow();

                // Apply favorite row background color if enabled
                if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                }

                ImGui::TableSetColumnIndex(0);
                UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");

                ImGui::TableSetColumnIndex(1);
                std::string name = st.details.loaded ? st.details.name : "Loading...";

                // Add star icon for favorites
                if (st.isFavorite)
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* ");
                    ImGui::SameLine();
                }

                ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                if (st.details.loaded && !st.details.rarity.empty())
                {
                    // Rarity color (enhanced) - according to GW2 Wiki
                    if (st.details.rarity == "Junk") col = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                    else if (st.details.rarity == "Basic") col = ImVec4(1.f, 1.f, 1.f, 1.f);
                    else if (st.details.rarity == "Fine") col = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                    else if (st.details.rarity == "Masterwork") col = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                    else if (st.details.rarity == "Rare") col = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                    else if (st.details.rarity == "Exotic") col = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                    else if (st.details.rarity == "Ascended") col = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                    else if (st.details.rarity == "Legendary") col = ImVec4(1.0f, 0.5f, 0.8f, 1.f);
                }

                // Apply favorite text color if enabled
                if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                    col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);

                UICommon::TextWithTooltip(name.c_str(), 200.0f, col);
                if (ImGui::IsItemHovered() && st.details.loaded)
                {
                    ImGui::BeginTooltip();
                    char rarityLabel[256];
                    snprintf(rarityLabel, sizeof(rarityLabel), Localization::GetText("rarity_label"), st.details.rarity.c_str());
                    ImGui::Text("%s", rarityLabel);
                    char typeLabel[256];
                    snprintf(typeLabel, sizeof(typeLabel), Localization::GetText("type_label"), static_cast<int>(st.details.itemType));
                    ImGui::Text("%s", typeLabel);
                    ImVec4 vendorColor = st.details.vendorValue > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.details.vendorValue < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(vendorColor, Localization::GetText("vendor_value_format"), UICommon::FormatCoin(st.details.vendorValue).c_str());
                    ImVec4 tpSellGrossColor = st.details.tpSellPrice > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.details.tpSellPrice < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(tpSellGrossColor, Localization::GetText("tp_sell_gross_format"), UICommon::FormatCoin(st.details.tpSellPrice).c_str());
                    long long tpSellNet = static_cast<long long>(st.details.tpSellPrice * 85.0 / 100.0);
                    ImVec4 tpSellNetColor = tpSellNet > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpSellNet < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(tpSellNetColor, Localization::GetText("tp_sell_net_format"), UICommon::FormatCoin(tpSellNet).c_str());
                    ImVec4 tpBuyGrossColor = st.details.tpBuyPrice > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.details.tpBuyPrice < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(tpBuyGrossColor, Localization::GetText("tp_buy_gross_format"), UICommon::FormatCoin(st.details.tpBuyPrice).c_str());
                    long long tpBuyNet = static_cast<long long>(st.details.tpBuyPrice * 85.0 / 100.0);
                    ImVec4 tpBuyNetColor = tpBuyNet > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpBuyNet < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(tpBuyNetColor, Localization::GetText("tp_buy_net_format"), UICommon::FormatCoin(tpBuyNet).c_str());
                    char accountBoundLabel[256];
                    snprintf(accountBoundLabel, sizeof(accountBoundLabel), Localization::GetText("account_bound_label"), st.details.accountBound ? Localization::GetText("yes_label") : Localization::GetText("no_label"));
                    ImGui::Text("%s", accountBoundLabel);
                    char noSellLabel[256];
                    snprintf(noSellLabel, sizeof(noSellLabel), Localization::GetText("nosell_label"), st.details.noSell ? Localization::GetText("yes_label") : Localization::GetText("no_label"));
                    ImGui::Text("%s", noSellLabel);
                    ImGui::EndTooltip();
                }

                ImGui::TableSetColumnIndex(2);
                ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                ImGui::TextColored(countColor, "%lld", st.count);

                ImGui::TableSetColumnIndex(3);
                long long profit = ItemTracker::GetStatProfit(st);
                if (profit > 0)
                    ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                else if (profit < 0)
                    ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                else
                    ImGui::TextUnformatted(Localization::GetText("no_profit"));

                ImGui::TableSetColumnIndex(4);
                bool isFavorite = st.isFavorite;
                if (ImGui::Checkbox(("##fav_" + std::to_string(id)).c_str(), &isFavorite))
                {
                    ItemTracker::SetFavorite(id, isFavorite);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite"));

                ImGui::TableSetColumnIndex(5);
                bool isIgnored = st.isIgnored;
                if (ImGui::Checkbox(("##ign_" + std::to_string(id)).c_str(), &isIgnored))
                {
                    if (isIgnored)
                        IgnoredItemsManager::IgnoreItem(id);
                    else
                        IgnoredItemsManager::UnignoreItem(id);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("toggle_ignore"));
            }

            ImGui::EndTable();
        }
    }
}
}
