#include "ui_currencies.h"
#include "settings.h"
#include "item_tracker.h"
#include "custom_profit.h"
#include "ui_common.h"
#include "ignored_items.h"
#include "localization.h"
#include "ui_context_menu.h"
#include "ui_tooltips.h"
#include "ui_tab_icons.h"
#include "shared.h"
#include <algorithm>
#include <cstring>

namespace UICurrencies
{

static void DrawGridCurrencyCount(int id, long long count, float iconSz)
{
    const float fontSize = iconSz * 0.45f;
    std::string cs = (id == 1) ? UICommon::FormatCoin(count) : UICommon::FormatCompact(count);
    const char* countStr = cs.c_str();
    ImGui::PushFont(ImGui::GetFont());
    ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());
    ImVec2 textSize = ImGui::CalcTextSize(countStr);
    ImVec2 origin   = ImGui::GetItemRectMin();
    ImVec2 pos = ImVec2(origin.x + iconSz - textSize.x - 2.f,
                        origin.y + iconSz - textSize.y - 2.f);
    ImVec4 col = count > 0 ? ImVec4(1.f,0.84f,0.f,1.f)
               : count < 0 ? ImVec4(0.9f,0.2f,0.2f,1.f)
               :              ImVec4(1.f,1.f,1.f,1.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    static const ImVec2 kOff[8] = {{-1,-1},{1,-1},{-1,1},{1,1},{0,-1},{0,1},{-1,0},{1,0}};
    for (int k = 0; k < 8; k++)
        dl->AddText(ImVec2(pos.x+kOff[k].x, pos.y+kOff[k].y), IM_COL32(0,0,0,255), countStr);
    dl->AddText(pos, ImGui::ColorConvertFloat4ToU32(col), countStr);
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopFont();

    // Tooltip for non-coin currencies with compact display
    if (id != 1)
    {
        ImGui::SetCursorScreenPos(origin);
        ImGui::InvisibleButton(("##currency_count_tooltip_" + std::to_string(id)).c_str(), ImVec2(iconSz, iconSz));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%lld", count);
        }
    }
}
void RenderCurrenciesTab()
{
    int pendingContextId = 0;
    std::string pendingContextName;

    // Favorite Currencies Section (Collapsible)
    static bool favoritesExpanded = true;
    
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    float headerHeight = 35.0f;
    
    // Draw custom header
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Background gradient (matching active main tab design)
    ImVec2 headerMin = cursor;
    ImVec2 headerMax = ImVec2(cursor.x + regionAvail.x, cursor.y + headerHeight);

    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    // Active tab gradient colors
    ImU32 bgColorTop = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
    ImU32 bgColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));

    drawList->AddRectFilledMultiColor(headerMin, headerMax, bgColorTop, bgColorTop, bgColorBottom, bgColorBottom);

    // Border (matching active tab border)
    ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));
    drawList->AddRect(headerMin, headerMax, borderColor, 4.0f, 0, 0.5f);
    
    // Active state indicator (left bar) - full accent color
    if (favoritesExpanded)
    {
        ImU32 activeColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.0f));
        ImVec2 barMin = ImVec2(headerMin.x, headerMin.y);
        ImVec2 barMax = ImVec2(headerMin.x + 3.0f, headerMax.y);
        drawList->AddRectFilled(barMin, barMax, activeColor, 2.0f);
    }
    
    // Icon
    float iconSize = 16.0f;
    float iconX = cursor.x + 12.0f;
    float iconY = cursor.y + (headerHeight - iconSize) * 0.5f;
    void* iconTex = UITabIcons::GetIcon("favorites");
    if (iconTex)
    {
        drawList->AddImage((ImTextureID)iconTex,
                         ImVec2(iconX, iconY),
                         ImVec2(iconX + iconSize, iconY + iconSize),
                         ImVec2(0,0), ImVec2(1,1),
                         ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)));
    }
    
    // Header text
    float textX = iconX + iconSize + 8.0f;
    float textY = cursor.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f;
    
    // Draw text directly for left alignment
    drawList->AddText(ImVec2(textX, textY), 
                     ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)),
                     "Favorite Currencies");
    
    // Invisible button for click detection
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    
    if (ImGui::Button("##FavoritesHeaderButton", ImVec2(regionAvail.x, headerHeight)))
    {
        favoritesExpanded = !favoritesExpanded;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("toggle_favorites_tooltip"));

    ImGui::PopStyleColor(3);
    
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + headerHeight + 8.0f));
    
    if (favoritesExpanded)
    {
        ImGui::Spacing();

        auto currencies = ItemTracker::GetFilteredCurrencies();
        std::vector<std::pair<int, Stat>> favoriteCurrencies;
        for (auto& [id, st] : currencies)
        {
            if (st.isFavorite)
                favoriteCurrencies.push_back({id, st});
        }

        if (g_Settings.enableGridViewSummary)
        {
            // Grid View for Favorite Currencies
            float cellSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) + 10.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;
            int columns = std::max(1, static_cast<int>((ImGui::GetContentRegionAvail().x - scrollbarWidth + spacing) / (cellSize + spacing)));

            int count = 0;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
            for (auto& [id, st] : favoriteCurrencies)
            {
                if (st.count <= 0) continue;

                if (count > 0)
                {
                    if (count % columns == 0)
                        ImGui::NewLine();
                    else
                        ImGui::SameLine(0, 4.0f);
                }

                ImGui::PushID(id);
                ImGui::BeginGroup();
                ImVec2 cursor = ImGui::GetCursorScreenPos();
                
                // Draw favorite row background if enabled
                if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                {
                    ImVec2 iconSize = ImVec2(static_cast<float>(g_Settings.gridIconSizeCurrencies), static_cast<float>(g_Settings.gridIconSizeCurrencies));
                    ImVec2 bgPos = cursor;
                    ImVec2 bgEnd = ImVec2(bgPos.x + iconSize.x, bgPos.y + iconSize.y);
                    ImGui::GetWindowDrawList()->AddRectFilled(bgPos, bgEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                }
                
                std::string iconUrl = st.details.iconUrl;
                if (id == 1 && iconUrl.empty())
                    iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");
                
                // Draw favorite star
                if (st.isFavorite)
                {
                    ImVec2 starPos = ImVec2(cursor.x + 2.0f, cursor.y + 2.0f);
                    ImGui::GetWindowDrawList()->AddText(starPos, IM_COL32(255, 215, 0, 255), "*");
                }

                if (ImGui::IsItemHovered())
                {
                    UITooltips::CurrencyTooltipOptions opt;
                    opt.showCount = true;
                    opt.count = st.count;
                    opt.showId = true;
                    if (st.details.loaded)
                        UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                    else
                        UITooltips::RenderCurrencyTooltipFallback(Localization::GetText("loading"), "", id, opt);
                }

                // Draw count number on icon
                ImVec2 iconSize = ImVec2(static_cast<float>(g_Settings.gridIconSizeCurrencies), static_cast<float>(g_Settings.gridIconSizeCurrencies));
                char countStr[32];
                snprintf(countStr, sizeof(countStr), "%lld", st.count);

                float fontSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) * 0.45f;
                ImGui::PushFont(ImGui::GetFont());
                ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());

                ImVec2 textSize = ImGui::CalcTextSize(countStr);
                ImVec2 countPos = ImVec2(cursor.x + iconSize.x - textSize.x - 2.0f, cursor.y + iconSize.y - textSize.y - 2.0f);

                ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));

                // Draw shadow/outline for better readability
                static const ImVec2 kOff[8] = {{-1,-1},{1,-1},{-1,1},{1,1},{0,-1},{0,1},{-1,0},{1,0}};
                ImDrawList* dl = ImGui::GetWindowDrawList();
                for (int k = 0; k < 8; k++)
                    dl->AddText(ImVec2(countPos.x+kOff[k].x, countPos.y+kOff[k].y), IM_COL32(0,0,0,255), countStr);
                dl->AddText(countPos, ImGui::ColorConvertFloat4ToU32(countColor), countStr);

                ImGui::SetWindowFontScale(1.f);
                ImGui::PopFont();

                ImGui::EndGroup();
                ImGui::PopID();
                count++;
            }
            ImGui::PopStyleVar();
        }
        else
        {
            // List View for Favorite Currencies
            if (ImGui::BeginTable("##FavoriteCurrenciesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 70.f);
                ImGui::TableSetupColumn(Localization::GetText("item"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 350.f);
                ImGui::TableSetupColumn(Localization::GetText("count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.f);
                ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.f);
                ImGui::TableSetupColumn(Localization::GetText("value"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : favoriteCurrencies)
                {
                    if (st.count <= 0) continue;

                    long long profit = ItemTracker::GetStatProfit(st);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty())
                        iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, 32.f, st.details.loaded ? st.details.rarity : "");
                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::CurrencyTooltipOptions opt;
                        opt.showCount = true;
                        opt.count = st.count;
                        opt.showProfit = (profit != 0);
                        opt.profit = profit;
                        opt.showId = true;
                        if (st.details.loaded)
                            UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                        else
                            UITooltips::RenderCurrencyTooltipFallback(Localization::GetText("loading"), "", id, opt);
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading")));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    if (id == 1)
                    {
                        ImGui::Text("%s", UICommon::FormatCoin(st.count).c_str());
                    }
                    else
                    {
                        long long customProfit = CustomProfitManager::GetCustomProfit(id);
                        if (customProfit != 0)
                            ImGui::Text("%s", UICommon::FormatCoin(customProfit * st.count).c_str());
                        else
                            ImGui::Text("-");
                    }

                    ImGui::TableSetColumnIndex(4);
                    if (id == 1) 
                        ImGui::Text("%s", UICommon::FormatCoin(st.count).c_str());
                    else if (profit != 0)
                        ImGui::Text("%s", UICommon::FormatCoin(profit).c_str());
                    else
                        ImGui::Text("%s", UICommon::FormatCompact(st.count).c_str());
                }
                ImGui::EndTable();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
    }

    ImGui::Spacing();

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
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_search_tooltip"));
    ImGui::Spacing();

    if (ImGui::BeginPopup("CurrenciesLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = ItemTracker::ExportToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\currencies_export.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(json.data(), 1, json.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("export_csv")))
        {
            std::string csv = ItemTracker::ExportToCsv();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\currencies_export.csv";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(csv.data(), 1, csv.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_csv_tooltip"));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Import section
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("import_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import_currencies_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\currencies_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0) {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    try { ItemTracker::ImportFromJson(nlohmann::json::parse(buf)); } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_currencies_json_tooltip"));
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    auto sortedCurrencies = ItemTracker::GetSortedCurrencies(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));

    if (g_Settings.currenciesEnableGridView)
    {
        // Grid View for Currencies
        float cellSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) + 20.0f; // Increased padding for longer currency strings
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float scrollbarWidth = 20.0f; // Safer buffer for scrollbar
        
        auto getColumns = [&](float width) {
            return std::max(1, static_cast<int>((width - scrollbarWidth + spacing) / (cellSize + spacing)));
        };

        if (ImGui::BeginChild("##CurrenciesGrid", ImVec2(0, 0), true))
        {
            if (g_Settings.currenciesFavoritesFirst || g_Settings.currenciesGroupByRarity || g_Settings.currenciesGroupByCategory)
            {
                // Group by Category logic
                std::vector<std::string> categories = {
                    Localization::GetText("currency_cat_common"),
                    Localization::GetText("currency_cat_fractal"),
                    Localization::GetText("currency_cat_raid_strike"),
                    Localization::GetText("currency_cat_wvw"),
                    Localization::GetText("currency_cat_pvp"),
                    Localization::GetText("currency_cat_map"),
                    Localization::GetText("currency_cat_janthir"),
                    Localization::GetText("currency_cat_other")
                };

                if (g_Settings.currenciesShowGroupAsTabs || g_Settings.currenciesShowRarityAsTabs)
                {
                    if (ImGui::BeginTabBar("##CurrencyCategoryTabsGrid"))
                    {
                        for (const auto& cat : categories)
                        {
                            // Check if category has any currencies with count > 0
                            bool hasItems = false;
                            for (auto& [id, st] : sortedCurrencies)
                            {
                                if (ItemTracker::GetCurrencyCategory(id) == cat && st.count > 0)
                                {
                                    hasItems = true;
                                    break;
                                }
                            }
                            if (!hasItems) continue;

                            if (ImGui::BeginTabItem(cat.c_str()))
                            {
                                int columns = getColumns(ImGui::GetContentRegionAvail().x);
                                int col = 0;
                                for (auto& [id, st] : sortedCurrencies)
                                {
                                    if (ItemTracker::GetCurrencyCategory(id) != cat) continue;
                                    if (st.count == 0) continue;

                                    if (col > 0) ImGui::SameLine();
                                    
                                    ImGui::PushID(id);
                                    // ... Render Currency Cell (using a helper or just repeating for now)
                                    if (ImGui::BeginChild("##CurrencyCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                                    {
                                        ImVec2 cursor = ImGui::GetCursorPos();
                                        std::string iconUrl = st.details.iconUrl;
                                        if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                                        UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");
                                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                        {
                                            pendingContextId = id;
                                            pendingContextName = st.details.loaded ? st.details.name : "";
                                        }
                                        if (ImGui::IsItemHovered())
                                        {
                                            UITooltips::CurrencyTooltipOptions opt;
                                            opt.showCount = true;
                                            opt.count = st.count;
                                            opt.showRarity = true;
                                            opt.showId = true;
                                            if (st.details.loaded)
                                                UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                                            else
                                                UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                                        }
                                        DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
                                    }
                                    ImGui::EndChild();
                                    ImGui::PopID();

                                    col++;
                                    if (col >= columns) col = 0;
                                }
                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                }
                else
                {
                    // Collapsible sections
                    for (const auto& cat : categories)
                    {
                        bool hasItems = false;
                        for (const auto& [id, st] : sortedCurrencies) { if (ItemTracker::GetCurrencyCategory(id) == cat && st.count > 0) { hasItems = true; break; } }
                        if (!hasItems) continue;

                        if (ImGui::CollapsingHeader(cat.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            int columns = getColumns(ImGui::GetContentRegionAvail().x);
                            int col = 0;
                            for (auto& [id, st] : sortedCurrencies)
                            {
                                if (ItemTracker::GetCurrencyCategory(id) != cat) continue;
                                if (st.count == 0) continue;
                                if (col > 0) ImGui::SameLine();
                                
                                ImGui::PushID(id);
                                if (ImGui::BeginChild("##CurrencyCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                                {
                                    ImVec2 cursor = ImGui::GetCursorPos();
                                    std::string iconUrl = st.details.iconUrl;
                                    if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                                    UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                    {
                                        pendingContextId = id;
                                        pendingContextName = st.details.loaded ? st.details.name : "";
                                    }
                                    if (ImGui::IsItemHovered())
                                    {
                                        UITooltips::CurrencyTooltipOptions opt;
                                        opt.showCount = true;
                                        opt.count = st.count;
                                        opt.showRarity = true;
                                        opt.showId = true;
                                        if (st.details.loaded)
                                            UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                                        else
                                            UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                                    }
                                    DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
                                }
                                ImGui::EndChild();
                                ImGui::PopID();

                                col++;
                                if (col >= columns) col = 0;
                            }
                        }
                    }
                }
            }
            else
            {
                // No grouping
                int columns = getColumns(ImGui::GetContentRegionAvail().x);
                int col = 0;
                for (auto& [id, st] : sortedCurrencies)
                {
                    if (st.count == 0) continue;
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

                        // Right-click context menu
                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                        {
                            pendingContextId = id;
                            pendingContextName = st.details.loaded ? st.details.name : "";
                        }

                        if (ImGui::IsItemHovered())
                        {
                            UITooltips::CurrencyTooltipOptions opt;
                            opt.showCount = true;
                            opt.count = st.count;
                            opt.showRarity = true;
                            opt.showId = true;
                            if (st.details.loaded)
                                UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                            else
                                UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                        }

                        DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
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

            // Context menu popup (rendered once outside the loop)
            if (pendingContextId != 0)
                UIContextMenu::OpenContextMenu("CurrencyContextMenu", pendingContextId, pendingContextName);
            UIContextMenu::RenderCurrencyContextMenu("CurrencyContextMenu", UIContextMenu::ContextMenuType::General);
        }
        ImGui::EndChild();
    }
    else
    {
        // Table View for Currencies
        auto renderCurrencyRow = [&](int id, const Stat& st) {
            float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.iconSize));
            ImGui::TableNextRow(0, rowH);
            if (st.isFavorite && g_Settings.enableFavoriteRowColor) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
            ImGui::TableSetColumnIndex(0);
            UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.iconSize));
            std::string iconUrl = st.details.iconUrl;
            if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
            UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
            {
                pendingContextId = id;
                pendingContextName = st.details.loaded ? st.details.name : "";
            }
            if (ImGui::IsItemHovered())
            {
                UITooltips::CurrencyTooltipOptions opt;
                opt.showCount = true;
                opt.count = st.count;
                opt.showRarity = true;
                opt.showId = true;
                if (st.details.loaded)
                    UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                else
                    UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
            }
            ImGui::TableSetColumnIndex(1);
            UICommon::AlignTableCellText(rowH);
            std::string name = st.details.loaded ? st.details.name : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"));
            if (st.isFavorite) { ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "*"); ImGui::SameLine(); }
            
            // Default color is white
            ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            
            // Apply gold color for favorites if setting is enabled, otherwise use default gold for favorites
            if (st.isFavorite) {
                col = ImVec4(1.0f, 0.84f, 0.0f, 1.0f); // Default favorite gold
                if (g_Settings.enableFavoriteTextColor)
                    col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);
            }
            
            ImGui::TextColored(col, "%s", name.c_str());
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
            {
                pendingContextId = id;
                pendingContextName = st.details.loaded ? st.details.name : "";
            }
            if (ImGui::IsItemHovered())
            {
                UITooltips::CurrencyTooltipOptions opt;
                opt.showCount = true;
                opt.count = st.count;
                opt.showRarity = true;
                opt.showId = true;
                if (st.details.loaded)
                    UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                else
                    UITooltips::RenderCurrencyTooltipFallback(name, "", id, opt);
            }
            ImGui::TableSetColumnIndex(2);
            UICommon::AlignTableCellText(rowH);
            ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            std::string countStr = std::to_string(st.count);
            ImGui::TextColored(countColor, "%s", countStr.c_str());
            if (ImGui::IsItemHovered())
            {
                if (id == 1)
                    ImGui::SetTooltip("%s", UICommon::FormatCoin(st.count).c_str());
                else
                    ImGui::SetTooltip("%lld", st.count);
            }

            ImGui::TableSetColumnIndex(3);
            UICommon::AlignTableCellText(rowH);
            if (id == 1)
            {
                ImGui::Text("%s", UICommon::FormatCoin(st.count).c_str());
            }
            else
            {
                long long customProfit = CustomProfitManager::GetCustomProfit(id);
                if (customProfit != 0)
                    ImGui::Text("%s", UICommon::FormatCoin(customProfit * st.count).c_str());
                else
                    ImGui::Text("-");
            }
        };

        if (g_Settings.currencyGroupByCategory)
        {
            std::vector<std::string> categories = {
                Localization::GetText("currency_cat_common"),
                Localization::GetText("currency_cat_fractal"),
                Localization::GetText("currency_cat_raid_strike"),
                Localization::GetText("currency_cat_wvw"),
                Localization::GetText("currency_cat_pvp"),
                Localization::GetText("currency_cat_map"),
                Localization::GetText("currency_cat_janthir"),
                Localization::GetText("currency_cat_other")
            };

            if (g_Settings.currencyShowAsTabs)
            {
                if (ImGui::BeginTabBar("##CurrencyCategoryTabsList"))
                {
                    for (const auto& cat : categories)
                    {
                        if (ImGui::BeginTabItem(cat.c_str()))
                        {
                            if (ImGui::BeginTable("##CurrenciesTable_v3_Tabs", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                            {
                                float iconColumnWidth = (static_cast<float>(g_Settings.iconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.iconSize) + 10.0f) : 70.0f;
                                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                                ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthStretch);
                                ImGui::TableHeadersRow();

                                for (auto& [id, st] : sortedCurrencies)
                                {
                                    if (ItemTracker::GetCurrencyCategory(id) == cat)
                                    {
                                        if (st.count == 0) continue;
                                        renderCurrencyRow(id, st);
                                    }
                                }
                                ImGui::EndTable();
                            }
                            ImGui::EndTabItem();
                        }
                    }
                    ImGui::EndTabBar();
                }
            }
            else
            {
                for (const auto& cat : categories)
                {
                    bool hasItems = false;
                    for (const auto& [id, st] : sortedCurrencies) { if (ItemTracker::GetCurrencyCategory(id) == cat) { hasItems = true; break; } }
                    if (!hasItems) continue;

                    if (ImGui::CollapsingHeader(cat.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (ImGui::BeginTable(("##CurrenciesTable_v3_" + cat).c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                        {
                            float iconColumnWidth = (static_cast<float>(g_Settings.iconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.iconSize) + 10.0f) : 70.0f;
                            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                            ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableHeadersRow();

                            for (auto& [id, st] : sortedCurrencies)
                            {
                                if (ItemTracker::GetCurrencyCategory(id) == cat)
                                {
                                    if (st.count == 0) continue;
                                    renderCurrencyRow(id, st);
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                }
            }
        }
        else
        {
            // Table View for Currencies
            int currencyTableColumnCount = 4;
            if (ImGui::BeginTable("##CurrenciesTable_v3", currencyTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                float iconColumnWidth = (static_cast<float>(g_Settings.iconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.iconSize) + 10.0f) : 70.0f;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedCurrencies)
                {
                    if (st.count == 0) continue;
                    renderCurrencyRow(id, st);
                }
                ImGui::EndTable();
            }
        }
        if (pendingContextId != 0)
            UIContextMenu::OpenContextMenu("CurrencyContextMenu", pendingContextId, pendingContextName);
        UIContextMenu::RenderCurrencyContextMenu("CurrencyContextMenu", UIContextMenu::ContextMenuType::General);
    }
}
}
