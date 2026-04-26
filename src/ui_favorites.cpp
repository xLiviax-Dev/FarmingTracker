#include "ui_favorites.h"
#include "settings.h"
#include "item_tracker.h"
#include "localization.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace UIFavorites
{
void RenderFavoritesTab()
{
    static int favoritesSubTab = 0; // 0 = Items, 1 = Currencies

    ImGui::BeginTabBar("FavoritesSubTabs");
    if (ImGui::BeginTabItem(Localization::GetText("tab_items")))
    {
        favoritesSubTab = 0;
        // Search bar
        if (ImGui::InputTextWithHint("##SearchFavoritesItems", Localization::GetText("search_favorite_items_hint"), UICommon::s_SearchBuf, sizeof(UICommon::s_SearchBuf)))
        {
            g_Settings.searchTerm = UICommon::s_SearchBuf;
            SettingsManager::Save();
        }
        ImGui::SameLine();
        char clearSearchFavorites[256];
        snprintf(clearSearchFavorites, sizeof(clearSearchFavorites), "%s##ClearSearchFavoritesItems", Localization::GetText("clear_search_favorites"));
        if (ImGui::Button(clearSearchFavorites))
        {
            memset(UICommon::s_SearchBuf, 0, sizeof(UICommon::s_SearchBuf));
            g_Settings.searchTerm = "";
            SettingsManager::Save();
        }
        ImGui::Spacing();

        auto favoriteItems = ItemTracker::GetFavoriteItems();
        std::vector<std::pair<int, Stat>> sortedFavoriteItems(favoriteItems.begin(), favoriteItems.end());

        // Apply sort
        std::sort(sortedFavoriteItems.begin(), sortedFavoriteItems.end(), [](const auto& a, const auto& b) {
            return std::abs(a.second.count) > std::abs(b.second.count);
        });

        // Grid View for favorite items
        if (g_Settings.enableGridViewItems)
        {
            // Grid View
            float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f; // icon + padding
            ImVec2 windowSize = ImGui::GetWindowSize();
            int columns = std::clamp(static_cast<int>(windowSize.x / cellSize), 2, 50);

            if (ImGui::BeginChild("##FavoriteItemsGrid", ImVec2(0, 0), true))
            {
                int col = 0;
                for (auto& [id, st] : sortedFavoriteItems)
                {
                    if (col > 0)
                        ImGui::SameLine();

                    ImGui::PushID(id);
                    if (ImGui::BeginChild("##FavoriteItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
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
                            ImGui::EndTooltip();
                        }

                        // Count
                        ImVec2 countPos = ImGui::GetCursorPos();
                        float fontSize = 12.0f;
                        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
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
            // Table View for favorite items
            if (ImGui::BeginTable("##FavoriteItemsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
            {
                float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 110.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 120.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_favorite"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedFavoriteItems)
                {
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
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "★ ");
                    ImGui::SameLine();
                    ImGui::Text("%s", name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(countColor, "%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    long long profit = ItemTracker::GetStatProfit(st);
                    ImVec4 profitColor = profit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(profit).c_str());

                    ImGui::TableSetColumnIndex(4);
                    bool isFavorite = st.isFavorite;
                    if (ImGui::Checkbox(("##fav_fav_" + std::to_string(id)).c_str(), &isFavorite))
                    {
                        ItemTracker::SetFavorite(id, isFavorite);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite_tooltip"));
                }

                ImGui::EndTable();
            }
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(Localization::GetText("tab_currencies")))
    {
        favoritesSubTab = 1;
        // Search bar
        if (ImGui::InputTextWithHint("##SearchFavoritesCurrencies", Localization::GetText("search_favorite_currencies_hint"), UICommon::s_SearchBuf, sizeof(UICommon::s_SearchBuf)))
        {
            g_Settings.searchTerm = UICommon::s_SearchBuf;
            SettingsManager::Save();
        }
        ImGui::SameLine();
        char clearSearchFavoritesCurrencies[256];
        snprintf(clearSearchFavoritesCurrencies, sizeof(clearSearchFavoritesCurrencies), "%s##ClearSearchFavoritesCurrencies", Localization::GetText("clear_search_favorites"));
        if (ImGui::Button(clearSearchFavoritesCurrencies))
        {
            memset(UICommon::s_SearchBuf, 0, sizeof(UICommon::s_SearchBuf));
            g_Settings.searchTerm = "";
            SettingsManager::Save();
        }
        ImGui::Spacing();

        auto favoriteCurrencies = ItemTracker::GetFavoriteCurrencies();
        std::vector<std::pair<int, Stat>> sortedFavoriteCurrencies(favoriteCurrencies.begin(), favoriteCurrencies.end());

        // Apply sort
        std::sort(sortedFavoriteCurrencies.begin(), sortedFavoriteCurrencies.end(), [](const auto& a, const auto& b) {
            return std::abs(a.second.count) > std::abs(b.second.count);
        });

        // Grid View for favorite currencies
        if (g_Settings.enableGridViewCurrencies)
        {
            // Grid View
            float cellSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) + 10.0f; // icon + padding
            ImVec2 windowSize = ImGui::GetWindowSize();
            int columns = std::clamp(static_cast<int>(windowSize.x / cellSize), 2, 50);

            if (ImGui::BeginChild("##FavoriteCurrenciesGrid", ImVec2(0, 0), true))
            {
                int col = 0;
                for (auto& [id, st] : sortedFavoriteCurrencies)
                {
                    if (col > 0)
                        ImGui::SameLine();

                    ImGui::PushID(id);
                    if (ImGui::BeginChild("##FavoriteCurrencyCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                    {
                        // Icon
                        ImVec2 cursor = ImGui::GetCursorPos();
                        std::string iconUrl = st.details.iconUrl;
                        if (id == 1 && iconUrl.empty())
                            iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                        UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");

                        // Tooltip for icon
                        if (ImGui::IsItemHovered() && st.details.loaded)
                        {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", st.details.name.c_str());
                            ImGui::Separator();
                            ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                            ImGui::TextColored(countColor, "%s %lld", Localization::GetText("count_label"), st.count);
                            ImGui::EndTooltip();
                        }

                        // Count
                        ImVec2 countPos = ImGui::GetCursorPos();
                        float fontSize = 12.0f;
                        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
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
            // Table View for favorite currencies
            if (ImGui::BeginTable("##FavoriteCurrenciesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
            {
                float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 110.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_favorite"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedFavoriteCurrencies)
                {
                    ImGui::TableNextRow();

                    // Apply favorite row background color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                    }

                    ImGui::TableSetColumnIndex(0);
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty())
                        iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");

                    ImGui::TableSetColumnIndex(1);
                    std::string name = st.details.loaded ? st.details.name : (id == 1 ? "Coin" : "Loading...");

                    // Add star icon for favorites
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "★ ");
                    ImGui::SameLine();
                    ImGui::Text("%s", name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(countColor, "%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    bool isFavorite = st.isFavorite;
                    if (ImGui::Checkbox(("##fav_fav_cur_" + std::to_string(id)).c_str(), &isFavorite))
                    {
                        ItemTracker::SetFavorite(id, isFavorite);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite_tooltip"));
                }

                ImGui::EndTable();
            }
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
}
