#include "ui_currencies.h"
#include "settings.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"
#include <algorithm>
#include <cstring>

namespace UICurrencies
{
void RenderCurrenciesTab()
{
    // Search bar
    if (ImGui::InputTextWithHint("##SearchCurrencies", Localization::GetText("search_currencies_hint"), UICommon::s_SearchBuf, sizeof(UICommon::s_SearchBuf)))
    {
        g_Settings.searchTerm = UICommon::s_SearchBuf;
        SettingsManager::Save();
    }
    ImGui::SameLine();
    char clearSearchCurrencies[256];
    snprintf(clearSearchCurrencies, sizeof(clearSearchCurrencies), "%s##ClearSearchCurrencies", Localization::GetText("clear_search"));
    if (ImGui::Button(clearSearchCurrencies))
    {
        memset(UICommon::s_SearchBuf, 0, sizeof(UICommon::s_SearchBuf));
        g_Settings.searchTerm = "";
        SettingsManager::Save();
    }
    ImGui::Spacing();

    auto sortedCurrencies = ItemTracker::GetSortedCurrencies(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));

    if (g_Settings.enableGridViewCurrencies)
    {
        // Grid View for Currencies
        float cellSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) + 10.0f; // icon + padding
        ImVec2 windowSize = ImGui::GetWindowSize();
        int columns = std::clamp(static_cast<int>(windowSize.x / cellSize), 2, 50);

        if (ImGui::BeginChild("##CurrenciesGrid", ImVec2(0, 0), true))
        {
            int col = 0;
            for (auto& [id, st] : sortedCurrencies)
            {
                // Skip ignored currencies if showIgnoredItems is disabled
                if (!g_Settings.showIgnoredItems && IgnoredItemsManager::IsCurrencyIgnored(id))
                    continue;

                if (col > 0)
                    ImGui::SameLine();

                ImGui::PushID(id);
                if (ImGui::BeginChild("##CurrencyCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
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
                        ImGui::TextColored(countColor, "Count: %lld", st.count);
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
        // Table View for Currencies
        int currencyTableColumnCount = 5;
        if (ImGui::BeginTable("##CurrenciesTable", currencyTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f;
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 110.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_favorite"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
            ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 180.0f);
            ImGui::TableHeadersRow();

            for (auto& [id, st] : sortedCurrencies)
            {
                // Skip ignored currencies if showIgnoredItems is disabled
                if (!g_Settings.showIgnoredItems && IgnoredItemsManager::IsCurrencyIgnored(id))
                    continue;

                ImGui::TableNextRow();

                // Apply favorite row background color if enabled
                if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                }

                ImGui::TableSetColumnIndex(0);
                // Fallback for Coin (ID 1) - use copper coin icon from GW2 Wiki
                std::string iconUrl = st.details.iconUrl;
                if (id == 1 && iconUrl.empty())
                    iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png"; // Copper coin icon
                UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");
                if (ImGui::IsItemHovered() && st.details.loaded)
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", st.details.name.c_str());
                    ImGui::EndTooltip();
                }

                ImGui::TableSetColumnIndex(1);
                std::string name = st.details.loaded ? st.details.name : (id == 1 ? "Coin" : "Loading...");

                // Add star icon for favorites
                if (st.isFavorite)
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "★ ");
                    ImGui::SameLine();
                }

                // Karma (ID 2) in pinker Farbe anzeigen
                if (id == 2)
                {
                    ImVec4 karmaColor = ImVec4(1.0f, 0.41f, 0.71f, 1.0f);
                    // Apply favorite text color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                        karmaColor = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);
                    ImGui::TextColored(karmaColor, "%s", name.c_str());
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(name.c_str());
                }
                else
                {
                    ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                    // Apply favorite text color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                        col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);
                    UICommon::TextWithTooltip(name.c_str(), 200.0f, col);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    if (st.details.loaded)
                    {
                        ImGui::Text("Name: %s", st.details.name.c_str());
                        if (!st.details.description.empty())
                            ImGui::Text("In-game description: %s", st.details.description.c_str());
                        ImGui::Text("API ID: %d", id);
                        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(countColor, "Count: %lld", st.count);
                    }
                    else
                    {
                        ImGui::Text("API ID: %d", id);
                        ImGui::Text("Loading...");
                        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                        ImGui::TextColored(countColor, "Count: %lld", st.count);
                    }
                    ImGui::EndTooltip();
                }

                ImGui::TableSetColumnIndex(2);
                // Special display for Coin (ID 1) - show Gold/Silver/Copper with colored icons
                if (id == 1)
                {
                    UICommon::DrawCoinDisplay(st.count);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Gold: %lld", st.count);
                }
                else
                {
                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(countColor, "%lld", st.count);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Count: %lld", st.count);
                }

                ImGui::TableSetColumnIndex(3);
                bool isFavorite = st.isFavorite;
                if (ImGui::Checkbox(("##fav_cur_" + std::to_string(id)).c_str(), &isFavorite))
                {
                    ItemTracker::SetFavorite(id, isFavorite);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite_tooltip"));

                ImGui::TableSetColumnIndex(4);
                bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(id);
                if (ImGui::Checkbox(("##ign_cur_" + std::to_string(id)).c_str(), &isIgnored))
                {
                    if (isIgnored)
                        IgnoredItemsManager::IgnoreCurrency(id);
                    else
                        IgnoredItemsManager::UnignoreCurrency(id);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("toggle_ignore_tooltip"));
            }

            ImGui::EndTable();
        }
    }
}
}
