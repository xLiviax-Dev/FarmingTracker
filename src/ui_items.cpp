#include "ui_items.h"
#include "settings.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"
#include "ui_context_menu.h"
#include "ui_tooltips.h"
#include "ui_tab_icons.h"
#include "shared.h"
#include <algorithm>
#include <cstring>

namespace UIItems
{

static void DrawGridItemCount(long long count, float iconSz)
{
    const float fontSize = iconSz * 0.45f;
    std::string cs = UICommon::FormatCompact(count);
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
}
void RenderItemsTab()
{
    // Favorite Items Section (Collapsible)
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
                     Localization::GetText("favorite_items_header"));
    
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

        auto items = ItemTracker::GetFilteredItems();
        std::vector<std::pair<int, Stat>> favoriteItems;
        for (auto& [id, st] : items)
        {
            if (st.isFavorite)
                favoriteItems.push_back({id, st});
        }

        if (g_Settings.enableGridViewSummary)
        {
            // Grid View for Favorite Items
            float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;
            int columns = std::max(1, static_cast<int>((ImGui::GetContentRegionAvail().x - scrollbarWidth + spacing) / (cellSize + spacing)));

            int count = 0;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
            for (auto& [id, st] : favoriteItems)
            {
                if (st.count == 0) continue;

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
                    ImVec2 iconSize = ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize));
                    ImVec2 bgPos = cursor;
                    ImVec2 bgEnd = ImVec2(bgPos.x + iconSize.x, bgPos.y + iconSize.y);
                    ImGui::GetWindowDrawList()->AddRectFilled(bgPos, bgEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                }
                
                UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");
                
                // Draw favorite star
                if (st.isFavorite)
                {
                    ImVec2 starPos = ImVec2(cursor.x + 2.0f, cursor.y + 2.0f);
                    ImGui::GetWindowDrawList()->AddText(starPos, IM_COL32(255, 215, 0, 255), "*");
                }

                if (ImGui::IsItemHovered())
                {
                    UITooltips::ItemTooltipOptions opt;
                    opt.showCount = true;
                    opt.count = st.count;
                    opt.showProfit = true;
                    opt.profit = ItemTracker::GetStatProfit(st);
                    opt.showTrading = true;
                    opt.showAccountFlags = true;
                    opt.showId = true;
                    if (st.details.loaded)
                        UITooltips::RenderItemTooltip(st.details, id, opt);
                    else
                        UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                }

                // Draw count number on icon
                ImVec2 iconSize = ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize));
                char countStr[32];
                snprintf(countStr, sizeof(countStr), "%lld", st.count);

                float fontSize = static_cast<float>(g_Settings.gridIconSize) * 0.45f;
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
            // List View for Favorite Items
            if (ImGui::BeginTable("##FavoriteItemsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 70.f);
                ImGui::TableSetupColumn(Localization::GetText("item"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 350.f);
                ImGui::TableSetupColumn(Localization::GetText("count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.f);
                ImGui::TableSetupColumn(Localization::GetText("value"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : favoriteItems)
                {
                    if (st.count == 0) continue;

                    float rowH = UICommon::CalcTableRowHeight(32.f);
                    ImGui::TableNextRow(0, rowH);

                    // Apply favorite row background color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                    }

                    ImGui::TableSetColumnIndex(0);
                    UICommon::DrawItemIconCell(id, st.details.iconUrl, 32.f, st.details.loaded ? st.details.rarity : "");
                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::ItemTooltipOptions opt;
                        opt.showCount = true;
                        opt.count = st.count;
                        opt.showProfit = true;
                        opt.profit = ItemTracker::GetStatProfit(st);
                        opt.showTrading = true;
                        opt.showAccountFlags = true;
                        opt.showId = true;
                        if (st.details.loaded)
                            UITooltips::RenderItemTooltip(st.details, id, opt);
                        else
                            UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                    }

                    ImGui::TableSetColumnIndex(1);
                    
                    // Add star icon for favorites
                    if (st.isFavorite)
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* ");
                        ImGui::SameLine();
                    }

                    ImVec4 nameCol = ImVec4(1.f, 1.f, 1.f, 1.f);
                    // Apply favorite text color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                        nameCol = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);
                    ImGui::TextColored(nameCol, "%s", st.details.loaded ? st.details.name.c_str() : Localization::GetText("loading"));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    long long profit = ItemTracker::GetStatProfit(st);
                    ImVec4 profitColor = profit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(profit).c_str());
                }
                ImGui::EndTable();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
    }
    
    ImGui::Spacing();

    ImGui::Spacing();

    const char* sortLabels[] = {
        Localization::GetText("sort_price_down"),
        Localization::GetText("sort_price_up"),
        Localization::GetText("sort_count_high"),
        Localization::GetText("sort_count_low"),
        Localization::GetText("sort_name_az"),
        Localization::GetText("sort_name_za"),
        Localization::GetText("sort_profit_high"),
        Localization::GetText("sort_profit_low"),
        Localization::GetText("sort_rarity_high"),
        Localization::GetText("sort_rarity_low")
    };
    if (ImGui::Combo("##SortItems", &g_Settings.itemSortMode, sortLabels, 10))
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

    // Search bar and Mass Actions
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 140.0f);
    if (ImGui::InputTextWithHint("##Search", Localization::GetText("search_items_hint"), UICommon::s_SearchBuf, sizeof(UICommon::s_SearchBuf)))
    {
        g_Settings.searchTerm = UICommon::s_SearchBuf;
        SettingsManager::Save();
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button(Localization::GetText("mass_actions_label"), ImVec2(130, 0)))
        ImGui::OpenPopup("MassActionsPopup");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("mass_actions_tooltip"));

    if (ImGui::BeginPopup("MassActionsPopup"))
    {
        auto items = ItemTracker::GetItemsCopy();
        
        auto renderMenuItem = [&](const char* labelKey, const std::string& rarityName) {
            if (ImGui::MenuItem(Localization::GetText(labelKey)))
            {
                for (const auto& [id, st] : items)
                {
                    if (st.details.rarity == rarityName)
                    {
                        IgnoredItemsManager::IgnoreItem(id);
                    }
                }
                SettingsManager::Save();
            }
        };

        // Rarity options ascending (Junk to Legendary)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.f));
        renderMenuItem("mass_actions_ignore_junk", "Junk");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        renderMenuItem("mass_actions_ignore_basic", "Basic");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.f, 1.f));
        renderMenuItem("mass_actions_ignore_fine", "Fine");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.f));
        renderMenuItem("mass_actions_ignore_masterwork", "Masterwork");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.9f, 0.2f, 1.f));
        renderMenuItem("mass_actions_ignore_rare", "Rare");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.6f, 0.2f, 1.f));
        renderMenuItem("mass_actions_ignore_exotic", "Exotic");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.20f, 0.50f, 1.f));
        renderMenuItem("mass_actions_ignore_ascended", "Ascended");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.20f, 0.90f, 1.f));
        renderMenuItem("mass_actions_ignore_legendary", "Legendary");
        ImGui::PopStyleColor();

        ImGui::Separator();

        if (ImGui::MenuItem(Localization::GetText("mass_actions_clear_ignore")))
        {
            IgnoredItemsManager::ClearAll();
            SettingsManager::Save();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    if (ImGui::BeginPopup("ItemsLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = ItemTracker::ExportToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\items_export.json";
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
            std::string path = std::string(dir ? dir : "") + "\\items_export.csv";
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
        if (ImGui::Button(Localization::GetText("import_items_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\items_import.json";
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
            ImGui::SetTooltip("%s", Localization::GetText("import_items_json_tooltip"));
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    auto sortedItems = ItemTracker::GetSortedItems(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));

    if (g_Settings.itemsEnableGridView)
    {
        // Grid View
        float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f; // icon + padding
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float scrollbarWidth = 20.0f; // Safer buffer for scrollbar
        
        auto getColumns = [&](float width) {
            return std::max(1, static_cast<int>((width - scrollbarWidth + spacing) / (cellSize + spacing)));
        };

        ImGui::Spacing();

        static int s_GridPendingId = -1;
        static std::string s_GridPendingName;

        if (ImGui::BeginChild("##ItemsGrid", ImVec2(0, 0), true))
        {
            if (g_Settings.itemsFavoritesFirst || g_Settings.itemsGroupByRarity)
            {
                // Rarity order (highest to lowest)
                std::vector<std::string> rarityOrder = {
                    Localization::GetText("rarity_name_legendary"),
                    Localization::GetText("rarity_name_ascended"),
                    Localization::GetText("rarity_name_exotic"),
                    Localization::GetText("rarity_name_rare"),
                    Localization::GetText("rarity_name_masterwork"),
                    Localization::GetText("rarity_name_fine"),
                    Localization::GetText("rarity_name_basic"),
                    Localization::GetText("rarity_name_junk"),
                    Localization::GetText("rarity_name_unknown")
                };

                // Group items by localized rarity name
                std::map<std::string, std::vector<std::pair<int, Stat>>> rarityGroups;
                for (auto& [id, st] : sortedItems)
                {
                    std::string apiRarity = st.details.loaded ? st.details.rarity : "Unknown";
                    std::string localizedRarity;
                    if (apiRarity == "Legendary") localizedRarity = Localization::GetText("rarity_name_legendary");
                    else if (apiRarity == "Ascended") localizedRarity = Localization::GetText("rarity_name_ascended");
                    else if (apiRarity == "Exotic") localizedRarity = Localization::GetText("rarity_name_exotic");
                    else if (apiRarity == "Rare") localizedRarity = Localization::GetText("rarity_name_rare");
                    else if (apiRarity == "Masterwork") localizedRarity = Localization::GetText("rarity_name_masterwork");
                    else if (apiRarity == "Fine") localizedRarity = Localization::GetText("rarity_name_fine");
                    else if (apiRarity == "Basic") localizedRarity = Localization::GetText("rarity_name_basic");
                    else if (apiRarity == "Junk") localizedRarity = Localization::GetText("rarity_name_junk");
                    else localizedRarity = Localization::GetText("rarity_name_unknown");

                    rarityGroups[localizedRarity].push_back({id, st});
                }

                if (g_Settings.itemsShowRarityAsTabs)
                {
                    static int selectedRarityTab = 0;
                    if (ImGui::BeginTabBar("##RarityTabs"))
                    {
                        int tabIndex = 0;
                        for (const auto& rarityName : rarityOrder)
                        {
                            auto it = rarityGroups.find(rarityName);
                            if (it == rarityGroups.end() || it->second.empty()) continue;

                            bool isSelected = (selectedRarityTab == tabIndex);
                            if (ImGui::BeginTabItem(rarityName.c_str(), &isSelected))
                            {
                                selectedRarityTab = tabIndex;
                                int columns = getColumns(ImGui::GetContentRegionAvail().x);
                                int col = 0;
                                for (auto& [id, st] : it->second)
                                {
                                    if (col > 0) ImGui::SameLine();
                                    ImGui::PushID(id);
                                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                                    {
                                        ImVec2 cursor = ImGui::GetCursorPos();
                                        char iconButtonId[64];
                                        snprintf(iconButtonId, sizeof(iconButtonId), "##IconBtn_%d", id);
                                        if (ImGui::InvisibleButton(iconButtonId, ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize)))) {}
                                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                                            s_GridPendingId = id;
                                            s_GridPendingName = st.details.loaded ? st.details.name : "";
                                        }
                                        ImGui::SetCursorPos(cursor);
                                        UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");
                                        if (ImGui::IsItemHovered())
                                        {
                                            UITooltips::ItemTooltipOptions opt;
                                            opt.showCount = true;
                                            opt.count = st.count;
                                            opt.showProfit = true;
                                            opt.profit = ItemTracker::GetStatProfit(st);
                                            opt.showTrading = true;
                                            opt.showAccountFlags = true;
                                            opt.showId = true;
                                            if (st.details.loaded)
                                                UITooltips::RenderItemTooltip(st.details, id, opt);
                                            else
                                                UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                                        }
                                        DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
                                    }
                                    ImGui::EndChild();
                                    ImGui::PopID();
                                    col++;
                                    if (col >= columns) col = 0;
                                }
                                ImGui::EndTabItem();
                            }
                            tabIndex++;
                        }
                        ImGui::EndTabBar();
                    }
                }
                else
                {
                    // Rarity order (highest to lowest)
                    std::vector<std::string> rarityOrder = {
                        Localization::GetText("rarity_name_legendary"),
                        Localization::GetText("rarity_name_ascended"),
                        Localization::GetText("rarity_name_exotic"),
                        Localization::GetText("rarity_name_rare"),
                        Localization::GetText("rarity_name_masterwork"),
                        Localization::GetText("rarity_name_fine"),
                        Localization::GetText("rarity_name_basic"),
                        Localization::GetText("rarity_name_junk"),
                        Localization::GetText("rarity_name_unknown")
                    };

                    // Group items by localized rarity name
                    std::map<std::string, std::vector<std::pair<int, Stat>>> rarityGroups;
                    for (auto& [id, st] : sortedItems)
                    {
                        std::string apiRarity = st.details.loaded ? st.details.rarity : "Unknown";
                        std::string localizedRarity;
                        if (apiRarity == "Legendary") localizedRarity = Localization::GetText("rarity_name_legendary");
                        else if (apiRarity == "Ascended") localizedRarity = Localization::GetText("rarity_name_ascended");
                        else if (apiRarity == "Exotic") localizedRarity = Localization::GetText("rarity_name_exotic");
                        else if (apiRarity == "Rare") localizedRarity = Localization::GetText("rarity_name_rare");
                        else if (apiRarity == "Masterwork") localizedRarity = Localization::GetText("rarity_name_masterwork");
                        else if (apiRarity == "Fine") localizedRarity = Localization::GetText("rarity_name_fine");
                        else if (apiRarity == "Basic") localizedRarity = Localization::GetText("rarity_name_basic");
                        else if (apiRarity == "Junk") localizedRarity = Localization::GetText("rarity_name_junk");
                        else localizedRarity = Localization::GetText("rarity_name_unknown");

                        rarityGroups[localizedRarity].push_back({id, st});
                    }

                    for (const auto& rarityName : rarityOrder)
                    {
                        auto it = rarityGroups.find(rarityName);
                        if (it == rarityGroups.end() || it->second.empty()) continue;
                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                        if (ImGui::CollapsingHeader(rarityName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::PopStyleColor(2);
                            int columns = getColumns(ImGui::GetContentRegionAvail().x);
                            int col = 0;
                            for (auto& [id, st] : it->second)
                            {
                                if (col > 0) ImGui::SameLine();
                                ImGui::PushID(id);
                                if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                                {
                                    ImVec2 cursor = ImGui::GetCursorPos();
                                    char iconButtonId[64];
                                    snprintf(iconButtonId, sizeof(iconButtonId), "##IconBtn_%d", id);
                                    if (ImGui::InvisibleButton(iconButtonId, ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize)))) {}
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                                        s_GridPendingId = id;
                                        s_GridPendingName = st.details.loaded ? st.details.name : "";
                                    }
                                    ImGui::SetCursorPos(cursor);
                                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");
                                    if (ImGui::IsItemHovered())
                                    {
                                        UITooltips::ItemTooltipOptions opt;
                                        opt.showCount = true;
                                        opt.count = st.count;
                                        opt.showProfit = true;
                                        opt.profit = ItemTracker::GetStatProfit(st);
                                        opt.showTrading = true;
                                        opt.showAccountFlags = true;
                                        opt.showId = true;
                                        if (st.details.loaded)
                                            UITooltips::RenderItemTooltip(st.details, id, opt);
                                        else
                                            UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                                    }
                                    DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
                                }
                                ImGui::EndChild();
                                ImGui::PopID();
                                col++;
                                if (col >= columns) col = 0;
                            }
                        }
                        else { ImGui::PopStyleColor(2); }
                    }
                }
            }
            else if (g_Settings.itemsGroupByCategory)
            {
                // Define ItemType order (logical sorting)
                std::vector<ItemType> typeOrder = {
                    ItemType::Weapon, ItemType::Armor, ItemType::Trinket, ItemType::Backpack,
                    ItemType::CraftingMaterial, ItemType::Consumable, ItemType::Container, ItemType::Bag,
                    ItemType::UpgradeComponent, ItemType::Trophy, ItemType::Gizmo, ItemType::Tool,
                    ItemType::GatheringTool, ItemType::MiniPet, ItemType::Unlock, ItemType::Unknown
                };

                // Helper to get localized type name
                auto getTypeName = [](ItemType type) -> std::string {
                    switch (type) {
                        case ItemType::Armor: return Localization::GetText("type_armor");
                        case ItemType::Weapon: return Localization::GetText("type_weapon");
                        case ItemType::Trinket: return Localization::GetText("type_trinket");
                        case ItemType::Gizmo: return Localization::GetText("type_gizmo");
                        case ItemType::CraftingMaterial: return Localization::GetText("type_crafting_material");
                        case ItemType::Consumable: return Localization::GetText("type_consumable");
                        case ItemType::GatheringTool: return Localization::GetText("type_gathering_tool");
                        case ItemType::Bag: return Localization::GetText("type_bag");
                        case ItemType::Container: return Localization::GetText("type_container");
                        case ItemType::MiniPet: return Localization::GetText("type_mini_pet");
                        case ItemType::GizmoContainer: return Localization::GetText("type_gizmo_container");
                        case ItemType::Backpack: return Localization::GetText("type_backpack");
                        case ItemType::UpgradeComponent: return Localization::GetText("type_upgrade_component");
                        case ItemType::Tool: return Localization::GetText("type_tool");
                        case ItemType::Trophy: return Localization::GetText("type_trophy");
                        case ItemType::Unlock: return Localization::GetText("type_unlock");
                        default: return Localization::GetText("rarity_name_unknown");
                    }
                };

                // Group items by type
                std::map<ItemType, std::vector<std::pair<int, Stat>>> typeGroups;
                for (auto& [id, st] : sortedItems) {
                    typeGroups[st.details.itemType].push_back({id, st});
                }

                if (g_Settings.itemsShowGroupAsTabs)
                {
                    static int selectedTypeTab = 0;
                    if (ImGui::BeginTabBar("##TypeTabs"))
                    {
                        int tabIndex = 0;
                        for (auto type : typeOrder)
                        {
                            auto it = typeGroups.find(type);
                            if (it == typeGroups.end() || it->second.empty()) continue;

                            std::string typeName = getTypeName(type);
                            bool isSelected = (selectedTypeTab == tabIndex);
                            if (ImGui::BeginTabItem(typeName.c_str(), &isSelected))
                            {
                                selectedTypeTab = tabIndex;
                                int columns = getColumns(ImGui::GetContentRegionAvail().x);
                                int col = 0;
                                for (auto& [id, st] : it->second)
                                {
                                    if (col > 0) ImGui::SameLine();
                                    ImGui::PushID(id);
                                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                                    {
                                        ImVec2 cursor = ImGui::GetCursorPos();
                                        char iconButtonId[64];
                                        snprintf(iconButtonId, sizeof(iconButtonId), "##IconBtn_%d", id);
                                        if (ImGui::InvisibleButton(iconButtonId, ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize)))) {}
                                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                                            s_GridPendingId = id;
                                            s_GridPendingName = st.details.loaded ? st.details.name : "";
                                        }
                                        ImGui::SetCursorPos(cursor);
                                        UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");
                                        if (ImGui::IsItemHovered()) {
                                            UITooltips::ItemTooltipOptions opt;
                                            opt.showCount = true;
                                            opt.count = st.count;
                                            opt.showProfit = true;
                                            opt.profit = ItemTracker::GetStatProfit(st);
                                            opt.showTrading = true;
                                            opt.showAccountFlags = true;
                                            opt.showId = true;
                                            if (st.details.loaded)
                                                UITooltips::RenderItemTooltip(st.details, id, opt);
                                            else
                                                UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                                        }
                                        DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
                                    }
                                    ImGui::EndChild();
                                    ImGui::PopID();
                                    col++;
                                    if (col >= columns) col = 0;
                                }
                                ImGui::EndTabItem();
                            }
                            tabIndex++;
                        }
                        ImGui::EndTabBar();
                    }
                }
                else
                {
                    for (auto type : typeOrder)
                    {
                        auto it = typeGroups.find(type);
                        if (it == typeGroups.end() || it->second.empty()) continue;
                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                        if (ImGui::CollapsingHeader(getTypeName(type).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::PopStyleColor(2);
                            int columns = getColumns(ImGui::GetContentRegionAvail().x);
                            int col = 0;
                            for (auto& [id, st] : it->second)
                            {
                                if (col > 0) ImGui::SameLine();
                                ImGui::PushID(id);
                                if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                                {
                                    ImVec2 cursor = ImGui::GetCursorPos();
                                    char iconButtonId[64];
                                    snprintf(iconButtonId, sizeof(iconButtonId), "##IconBtn_%d", id);
                                    if (ImGui::InvisibleButton(iconButtonId, ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize)))) {}
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                                        s_GridPendingId = id;
                                        s_GridPendingName = st.details.loaded ? st.details.name : "";
                                    }
                                    ImGui::SetCursorPos(cursor);
                                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");
                                    if (ImGui::IsItemHovered()) {
                                        UITooltips::ItemTooltipOptions opt;
                                        opt.showCount = true;
                                        opt.count = st.count;
                                        opt.showProfit = true;
                                        opt.profit = ItemTracker::GetStatProfit(st);
                                        opt.showTrading = true;
                                        opt.showAccountFlags = true;
                                        opt.showId = true;
                                        if (st.details.loaded)
                                            UITooltips::RenderItemTooltip(st.details, id, opt);
                                        else
                                            UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                                    }
                                    DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
                                }
                                ImGui::EndChild();
                                ImGui::PopID();
                                col++;
                                if (col >= columns) col = 0;
                            }
                        }
                        else { ImGui::PopStyleColor(2); }
                    }
                }
            }
            else
            {
                // Normal grid view without grouping
                int columns = getColumns(ImGui::GetContentRegionAvail().x);
                int col = 0;
                for (auto& [id, st] : sortedItems)
                {
                    if (col > 0)
                        ImGui::SameLine();

                    ImGui::PushID(id);
                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
                    {
                        // Icon as button for right-click
                        ImVec2 cursor = ImGui::GetCursorPos();
                        char iconButtonId[64];
                        snprintf(iconButtonId, sizeof(iconButtonId), "##IconBtn_%d", id);
                        if (ImGui::InvisibleButton(iconButtonId, ImVec2(static_cast<float>(g_Settings.gridIconSize), static_cast<float>(g_Settings.gridIconSize))))
                        {
                            // Left click - could add functionality here
                        }
                        
                        // Right-click context menu
                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                        {
                            s_GridPendingId = id;
                            s_GridPendingName = st.details.loaded ? st.details.name : "";
                        }
                        
                        // Draw icon on top of button
                        ImGui::SetCursorPos(cursor);
                        UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");

                        // Tooltip for icon
                        if (ImGui::IsItemHovered())
                        {
                            UITooltips::ItemTooltipOptions opt;
                            opt.showCount = true;
                            opt.count = st.count;
                            opt.showProfit = true;
                            opt.profit = ItemTracker::GetStatProfit(st);
                            opt.showTrading = true;
                            opt.showAccountFlags = true;
                            opt.showId = true;
                            if (st.details.loaded)
                                UITooltips::RenderItemTooltip(st.details, id, opt);
                            else
                                UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }

                        DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
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
        }
        ImGui::EndChild();

        if (s_GridPendingId != -1)
        {
            UIContextMenu::OpenContextMenu("ItemContextMenu", s_GridPendingId, s_GridPendingName);
            s_GridPendingId = -1;
        }
        UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::General);
    }
    else
    {
        // Group by Rarity options removed - now in Settings tab

        // Items Table with enhanced features
        int itemTableColumnCount = 5;

        if (!g_Settings.itemsGroupByRarity && !g_Settings.itemsGroupByCategory)
        {
            // Get best drop for highlighting
            auto bestDrop = ItemTracker::GetBestDrop();
            int bestDropId = bestDrop.first;
            if (bestDropId != 0 && !ItemTracker::PassesFilter(bestDrop.second))
                bestDropId = 0;
            if (bestDropId != 0 && !ItemTracker::PassesFilter(bestDrop.second))
                bestDropId = 0;

            // Normal view without grouping
            if (ImGui::BeginTable("##ItemsTable_v3", itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("magic_find"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedItems)
                {
                    float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.itemsIconSize));
                    ImGui::TableNextRow(0, rowH);

                    // Apply favorite row background color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                    }

                    // Apply best drop golden border if enabled
                    if (g_Settings.enableBestDropHighlight && id == bestDropId && st.count > 0)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.bestDropHighlightColor[0], g_Settings.bestDropHighlightColor[1], g_Settings.bestDropHighlightColor[2], g_Settings.bestDropHighlightColor[3])));
                    }

                    ImGui::TableSetColumnIndex(0);
                    UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.itemsIconSize));
                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.itemsIconSize), st.details.loaded ? st.details.rarity : "");

                    // Helper lambda for rendering the same tooltip
                    auto renderItemTooltip = [&]() {
                        UITooltips::ItemTooltipOptions opt;
                        opt.showCount = true;
                        opt.count = st.count;
                        opt.showProfit = true;
                        opt.profit = ItemTracker::GetStatProfit(st);
                        opt.showTrading = st.details.loaded;
                        opt.showAccountFlags = st.details.loaded;
                        opt.showId = true;
                        if (st.details.loaded)
                            UITooltips::RenderItemTooltip(st.details, id, opt);
                        else
                            UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                    };

                    // Show tooltip on icon
                    if (ImGui::IsItemHovered())
                    {
                        renderItemTooltip();
                    }

                    // Right-click context menu
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                    }

                    ImGui::TableSetColumnIndex(1);
                    UICommon::AlignTableCellText(rowH);
                    std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");

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
                        else if (st.details.rarity == "Legendary") col = ImVec4(0.55f, 0.25f, 0.85f, 1.f);
                    }

                    // Apply favorite text color if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                        col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);

                    ImGui::TextColored(col, "%s", name.c_str());
                    if (ImGui::IsItemHovered())
                    {
                        renderItemTooltip();
                    }

                    // Right-click context menu for name
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                    }

                    ImGui::TableSetColumnIndex(2);
                    UICommon::AlignTableCellText(rowH);
                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::TextColored(countColor, "%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    UICommon::AlignTableCellText(rowH);
                    long long profit = ItemTracker::GetStatProfit(st);
                    if (profit > 0)
                        ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                    else if (profit < 0)
                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                    else
                        ImGui::TextUnformatted(Localization::GetText("no_profit"));

                    ImGui::TableSetColumnIndex(4);
                    UICommon::AlignTableCellText(rowH);
                    if (st.lastMagicFind >= 0)
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d%%", st.lastMagicFind);
                    else
                        ImGui::TextDisabled("N/A");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", Localization::GetText("magic_find_tooltip"));

                }

                // Context menu popup (rendered once outside the loop)
                UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::General);

                ImGui::EndTable();
            }
        }
        else if (g_Settings.itemsGroupByRarity)
        {
            // Group by Rarity view
            // Get best drop for highlighting
            auto bestDrop = ItemTracker::GetBestDrop();
            int bestDropId = bestDrop.first;

            // Rarity order (lowest to highest)
            std::vector<std::string> rarityOrder = {
                Localization::GetText("rarity_name_junk"),
                Localization::GetText("rarity_name_basic"),
                Localization::GetText("rarity_name_fine"),
                Localization::GetText("rarity_name_masterwork"),
                Localization::GetText("rarity_name_rare"),
                Localization::GetText("rarity_name_exotic"),
                Localization::GetText("rarity_name_ascended"),
                Localization::GetText("rarity_name_legendary"),
                Localization::GetText("rarity_name_unknown")
            };

            // Group items by localized rarity name
            std::map<std::string, std::vector<std::pair<int, Stat>>> rarityGroups;
            for (auto& [id, st] : sortedItems)
            {
                std::string apiRarity = st.details.loaded ? st.details.rarity : "Unknown";
                std::string localizedRarity;
                if (apiRarity == "Legendary") localizedRarity = Localization::GetText("rarity_name_legendary");
                else if (apiRarity == "Ascended") localizedRarity = Localization::GetText("rarity_name_ascended");
                else if (apiRarity == "Exotic") localizedRarity = Localization::GetText("rarity_name_exotic");
                else if (apiRarity == "Rare") localizedRarity = Localization::GetText("rarity_name_rare");
                else if (apiRarity == "Masterwork") localizedRarity = Localization::GetText("rarity_name_masterwork");
                else if (apiRarity == "Fine") localizedRarity = Localization::GetText("rarity_name_fine");
                else if (apiRarity == "Basic") localizedRarity = Localization::GetText("rarity_name_basic");
                else if (apiRarity == "Junk") localizedRarity = Localization::GetText("rarity_name_junk");
                else localizedRarity = Localization::GetText("rarity_name_unknown");

                rarityGroups[localizedRarity].push_back({id, st});
            }

            // Rarity colors (mapped to localized names)
            std::map<std::string, ImVec4> rarityColors;
            rarityColors[Localization::GetText("rarity_name_legendary")] = ImVec4(1.0f, 0.5f, 0.8f, 1.f);
            rarityColors[Localization::GetText("rarity_name_ascended")] = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
            rarityColors[Localization::GetText("rarity_name_exotic")] = ImVec4(1.f, 0.6f, 0.0f, 1.f);
            rarityColors[Localization::GetText("rarity_name_rare")] = ImVec4(1.f, 0.9f, 0.0f, 1.f);
            rarityColors[Localization::GetText("rarity_name_masterwork")] = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
            rarityColors[Localization::GetText("rarity_name_fine")] = ImVec4(0.0f, 0.5f, 1.f, 1.f);
            rarityColors[Localization::GetText("rarity_name_basic")] = ImVec4(1.f, 1.f, 1.f, 1.f);
            rarityColors[Localization::GetText("rarity_name_junk")] = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
            rarityColors[Localization::GetText("rarity_name_unknown")] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);

            if (!g_Settings.itemsShowRarityAsTabs)
            {
                // Sections mode
                for (const auto& rarity : rarityOrder)
                {
                    if (rarityGroups.find(rarity) == rarityGroups.end() || rarityGroups[rarity].empty())
                        continue;

                    char headerLabel[256];
                    snprintf(headerLabel, sizeof(headerLabel), "%s (%zu)", rarity.c_str(), rarityGroups[rarity].size());

                    ImVec4 headerColor = rarityColors.count(rarity) ? rarityColors[rarity] : ImVec4(1.f, 1.f, 1.f, 1.f);
                    ImGui::PushStyleColor(ImGuiCol_Text, headerColor);
                    // Add dark semi-transparent background for better visibility on any accent color
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));

                    if (ImGui::CollapsingHeader(headerLabel, ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::PopStyleColor(4);

                        if (ImGui::BeginTable(("##RarityTable_v3_" + rarity).c_str(), itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                        {
                            float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                            ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                            ImGui::TableSetupColumn(Localization::GetText("magic_find"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableHeadersRow();

                            for (auto& [id, st] : rarityGroups[rarity])
                            {
                                float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.itemsIconSize));
                                ImGui::TableNextRow(0, rowH);

                                // Apply favorite row background color if enabled
                                if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                                {
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                                }

                                // Apply best drop golden border if enabled
                                if (g_Settings.enableBestDropHighlight && id == bestDropId && st.count > 0)
                                {
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.bestDropHighlightColor[0], g_Settings.bestDropHighlightColor[1], g_Settings.bestDropHighlightColor[2], g_Settings.bestDropHighlightColor[3])));
                                }

                                ImGui::TableSetColumnIndex(0);
                                UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.itemsIconSize));
                                UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.itemsIconSize), st.details.loaded ? st.details.rarity : "");

                                // Right-click context menu
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                {
                                    UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                }

                                ImGui::TableSetColumnIndex(1);
                                UICommon::AlignTableCellText(rowH);
                                std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");

                                // Add star icon for favorites
                                if (st.isFavorite)
                                {
                                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* ");
                                    ImGui::SameLine();
                                }

                                ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                if (st.details.loaded && !st.details.rarity.empty())
                                {
                                    if (st.details.rarity == "Junk") col = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                                    else if (st.details.rarity == "Basic") col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                    else if (st.details.rarity == "Fine") col = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                                    else if (st.details.rarity == "Masterwork") col = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                                    else if (st.details.rarity == "Rare") col = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                                    else if (st.details.rarity == "Exotic") col = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                                    else if (st.details.rarity == "Ascended") col = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                                    else if (st.details.rarity == "Legendary") col = ImVec4(0.55f, 0.25f, 0.85f, 1.f);
                                }

                                if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                                    col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);

                                ImGui::TextColored(col, "%s", name.c_str());
                                if (ImGui::IsItemHovered())
                                {
                                    UITooltips::ItemTooltipOptions opt;
                                    opt.showCount = true;
                                    opt.count = st.count;
                                    opt.showProfit = true;
                                    opt.profit = ItemTracker::GetStatProfit(st);
                                    opt.showTrading = st.details.loaded;
                                    opt.showAccountFlags = st.details.loaded;
                                    opt.showId = true;
                                    if (st.details.loaded)
                                        UITooltips::RenderItemTooltip(st.details, id, opt);
                                    else
                                        UITooltips::RenderItemTooltipFallback(name, "", id, opt);
                                }

                                // Right-click context menu for name
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                {
                                    UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                }

                                ImGui::TableSetColumnIndex(2);
                                UICommon::AlignTableCellText(rowH);
                                ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                                ImGui::TextColored(countColor, "%lld", st.count);

                                ImGui::TableSetColumnIndex(3);
                                UICommon::AlignTableCellText(rowH);
                                long long profit = ItemTracker::GetStatProfit(st);
                                if (profit > 0)
                                    ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                else if (profit < 0)
                                    ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                else
                                    ImGui::TextUnformatted(Localization::GetText("no_profit"));

                                ImGui::TableSetColumnIndex(4);
                                UICommon::AlignTableCellText(rowH);
                                if (st.lastMagicFind >= 0)
                                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d%%", st.lastMagicFind);
                                else
                                    ImGui::TextDisabled("N/A");
                                if (ImGui::IsItemHovered())
                                    ImGui::SetTooltip("%s", Localization::GetText("magic_find_tooltip"));

                            }

                            // Context menu popup (rendered once outside the loop)
                            UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::General);

                            ImGui::EndTable();
                        }
                    }
                    else
                    {
                        ImGui::PopStyleColor();
                    }
                }
            }
            else
            {
                // Tabs mode
                if (ImGui::BeginTabBar("##RarityTabs"))
                {
                    for (const auto& rarity : rarityOrder)
                    {
                        if (rarityGroups.find(rarity) == rarityGroups.end() || rarityGroups[rarity].empty())
                            continue;

                        char tabLabel[256];
                        snprintf(tabLabel, sizeof(tabLabel), "%s (%zu)", rarity.c_str(), rarityGroups[rarity].size());

                        ImVec4 tabColor = rarityColors.count(rarity) ? rarityColors[rarity] : ImVec4(1.f, 1.f, 1.f, 1.f);
                        ImGui::PushStyleColor(ImGuiCol_Text, tabColor);

                        if (ImGui::BeginTabItem(tabLabel))
                        {
                            ImGui::PopStyleColor();

                            if (ImGui::BeginTable(("##RarityTable_" + rarity).c_str(), itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
                            {
                                float iconColumnWidth = static_cast<float>(g_Settings.itemsIconSize) + 10.0f;
                                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 110.0f);
                                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 120.0f);
                                ImGui::TableSetupColumn(Localization::GetText("magic_find"), ImGuiTableColumnFlags_WidthStretch);
                                ImGui::TableHeadersRow();

                                for (auto& [id, st] : rarityGroups[rarity])
                                {
                                    float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.itemsIconSize));
                                    ImGui::TableNextRow(0, rowH);

                                    // Apply favorite row background color if enabled
                                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                                    }

                                    // Apply best drop golden border if enabled
                                    if (g_Settings.enableBestDropHighlight && id == bestDropId && st.count > 0)
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(1.0f, 0.84f, 0.0f, 0.15f))); // Gold with low alpha
                                    }

                                    ImGui::TableSetColumnIndex(0);
                                    UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.itemsIconSize));
                                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.itemsIconSize), st.details.loaded ? st.details.rarity : "");
                                    bool iconHovered = ImGui::IsItemHovered();

                                    // Right-click context menu
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                    {
                                        UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                    }

                                    ImGui::TableSetColumnIndex(1);
                                    UICommon::AlignTableCellText(rowH);
                                    std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");

                                    // Add star icon for favorites
                                    if (st.isFavorite)
                                    {
                                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* ");
                                        ImGui::SameLine();
                                    }

                                    ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                    if (st.details.loaded && !st.details.rarity.empty())
                                    {
                                        if (st.details.rarity == "Junk") col = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                                        else if (st.details.rarity == "Basic") col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                        else if (st.details.rarity == "Fine") col = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                                        else if (st.details.rarity == "Masterwork") col = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                                        else if (st.details.rarity == "Rare") col = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                                        else if (st.details.rarity == "Exotic") col = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                                        else if (st.details.rarity == "Ascended") col = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                                        else if (st.details.rarity == "Legendary") col = ImVec4(0.55f, 0.25f, 0.85f, 1.f);
                                    }

                                    if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                                        col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);

                                    ImGui::TextColored(col, "%s", name.c_str());
                                    bool nameHovered = ImGui::IsItemHovered();

                                    // Right-click context menu for name
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                    {
                                        UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                    }

                                    if (iconHovered || nameHovered)
                                    {
                                        UITooltips::ItemTooltipOptions opt;
                                        opt.showCount = true;
                                        opt.count = st.count;
                                        opt.showProfit = true;
                                        opt.profit = ItemTracker::GetStatProfit(st);
                                        opt.showTrading = true;
                                        opt.showAccountFlags = true;
                                        opt.showId = true;
                                        if (st.details.loaded)
                                            UITooltips::RenderItemTooltip(st.details, id, opt);
                                        else
                                            UITooltips::RenderItemTooltipFallback(name, "", id, opt);
                                    }

                                    ImGui::TableSetColumnIndex(2);
                                    UICommon::AlignTableCellText(rowH);
                                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                                    ImGui::TextColored(countColor, "%lld", st.count);

                                    ImGui::TableSetColumnIndex(3);
                                    UICommon::AlignTableCellText(rowH);
                                    long long profit = ItemTracker::GetStatProfit(st);
                                    if (profit > 0)
                                        ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                    else if (profit < 0)
                                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                    else
                                        ImGui::TextUnformatted(Localization::GetText("no_profit"));

                                    ImGui::TableSetColumnIndex(4);
                                    UICommon::AlignTableCellText(rowH);
                                    if (st.lastMagicFind >= 0)
                                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d%%", st.lastMagicFind);
                                    else
                                        ImGui::TextDisabled("N/A");
                                    if (ImGui::IsItemHovered())
                                        ImGui::SetTooltip("%s", Localization::GetText("magic_find_tooltip"));

                                    ImGui::TableSetColumnIndex(5);
                                    UICommon::AlignTableCellFrame(rowH);
                                    bool isFavorite = st.isFavorite;
                                    if (ImGui::Checkbox(("##fav_" + std::to_string(id)).c_str(), &isFavorite))
                                    {
                                        ItemTracker::SetFavorite(id, isFavorite);
                                    }
                                    if (ImGui::IsItemHovered())
                                        ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite"));

                                    ImGui::TableSetColumnIndex(6);
                                    UICommon::AlignTableCellFrame(rowH);
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

                                // Context menu popup (rendered once outside the loop)
                                UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::CopyOnly);

                                ImGui::EndTable();
                            }
                            ImGui::EndTabItem();
                        }
                        else
                        {
                            ImGui::PopStyleColor();
                        }
                    }
                    ImGui::EndTabBar();
                }
            }
        }
        else if (g_Settings.itemsGroupByCategory)
        {
            // Define ItemType order (logical sorting)
            std::vector<ItemType> typeOrder = {
                ItemType::Weapon, ItemType::Armor, ItemType::Trinket, ItemType::Backpack,
                ItemType::CraftingMaterial, ItemType::Consumable, ItemType::Container, ItemType::Bag,
                ItemType::UpgradeComponent, ItemType::Trophy, ItemType::Gizmo, ItemType::Tool,
                ItemType::GatheringTool, ItemType::MiniPet, ItemType::Unlock, ItemType::Unknown
            };

            // Helper to get localized type name
            auto getTypeName = [](ItemType type) -> std::string {
                switch (type) {
                    case ItemType::Armor: return Localization::GetText("type_armor");
                    case ItemType::Weapon: return Localization::GetText("type_weapon");
                    case ItemType::Trinket: return Localization::GetText("type_trinket");
                    case ItemType::Gizmo: return Localization::GetText("type_gizmo");
                    case ItemType::CraftingMaterial: return Localization::GetText("type_crafting_material");
                    case ItemType::Consumable: return Localization::GetText("type_consumable");
                    case ItemType::GatheringTool: return Localization::GetText("type_gathering_tool");
                    case ItemType::Bag: return Localization::GetText("type_bag");
                    case ItemType::Container: return Localization::GetText("type_container");
                    case ItemType::MiniPet: return Localization::GetText("type_mini_pet");
                    case ItemType::GizmoContainer: return Localization::GetText("type_gizmo_container");
                    case ItemType::Backpack: return Localization::GetText("type_backpack");
                    case ItemType::UpgradeComponent: return Localization::GetText("type_upgrade_component");
                    case ItemType::Tool: return Localization::GetText("type_tool");
                    case ItemType::Trophy: return Localization::GetText("type_trophy");
                    case ItemType::Unlock: return Localization::GetText("type_unlock");
                    default: return Localization::GetText("rarity_name_unknown");
                }
            };

            // Group items by type
            std::map<ItemType, std::vector<std::pair<int, Stat>>> typeGroups;
            for (auto& [id, st] : sortedItems) {
                typeGroups[st.details.itemType].push_back({id, st});
            }

            if (!g_Settings.showTypeAsTabs)
            {
                // Get best drop for highlighting
                auto bestDrop = ItemTracker::GetBestDrop();
                int bestDropId = bestDrop.first;
                if (bestDropId != 0 && !ItemTracker::PassesFilter(bestDrop.second))
                    bestDropId = 0;

                for (auto type : typeOrder)
                {
                    auto it = typeGroups.find(type);
                    if (it == typeGroups.end() || it->second.empty()) continue;

                    ImGui::Spacing();
                    // Add dark semi-transparent background for better visibility on any accent color
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));

                    char headerLabel[256];
                    snprintf(headerLabel, sizeof(headerLabel), "%s (%zu)", getTypeName(type).c_str(), it->second.size());

                    if (ImGui::CollapsingHeader(headerLabel, ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::PopStyleColor(3);
                        if (ImGui::BeginTable(("##TypeTable_" + std::to_string(static_cast<int>(type))).c_str(), itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                        {
                            float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                            ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                            ImGui::TableSetupColumn(Localization::GetText("magic_find"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableHeadersRow();

                            for (auto& [id, st] : it->second)
                            {
                                float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.itemsIconSize));
                                ImGui::TableNextRow(0, rowH);

                                // Apply favorite row background color if enabled
                                if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                                {
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                                }

                                // Apply best drop golden border if enabled
                                if (g_Settings.enableBestDropHighlight && id == bestDropId && st.count > 0)
                                {
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.bestDropHighlightColor[0], g_Settings.bestDropHighlightColor[1], g_Settings.bestDropHighlightColor[2], g_Settings.bestDropHighlightColor[3])));
                                }

                                ImGui::TableSetColumnIndex(0);
                                UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.itemsIconSize));
                                UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.itemsIconSize), st.details.loaded ? st.details.rarity : "");

                                // Right-click context menu
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                {
                                    UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                }

                                ImGui::TableSetColumnIndex(1);
                                UICommon::AlignTableCellText(rowH);
                                std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");

                                // Add star icon for favorites
                                if (st.isFavorite)
                                {
                                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* ");
                                    ImGui::SameLine();
                                }

                                ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                if (st.details.loaded && !st.details.rarity.empty())
                                {
                                    if (st.details.rarity == "Junk") col = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                                    else if (st.details.rarity == "Basic") col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                    else if (st.details.rarity == "Fine") col = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                                    else if (st.details.rarity == "Masterwork") col = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                                    else if (st.details.rarity == "Rare") col = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                                    else if (st.details.rarity == "Exotic") col = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                                    else if (st.details.rarity == "Ascended") col = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                                    else if (st.details.rarity == "Legendary") col = ImVec4(0.55f, 0.25f, 0.85f, 1.f);
                                }

                                if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                                    col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);

                                ImGui::TextColored(col, "%s", name.c_str());
                                if (ImGui::IsItemHovered())
                                {
                                    UITooltips::ItemTooltipOptions opt;
                                    opt.showCount = true;
                                    opt.count = st.count;
                                    opt.showProfit = true;
                                    opt.profit = ItemTracker::GetStatProfit(st);
                                    opt.showTrading = st.details.loaded;
                                    opt.showAccountFlags = st.details.loaded;
                                    opt.showId = true;
                                    if (st.details.loaded)
                                        UITooltips::RenderItemTooltip(st.details, id, opt);
                                    else
                                        UITooltips::RenderItemTooltipFallback(name, "", id, opt);
                                }

                                // Right-click context menu for name
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                {
                                    UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                }

                                ImGui::TableSetColumnIndex(2);
                                UICommon::AlignTableCellText(rowH);
                                ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                                ImGui::TextColored(countColor, "%lld", st.count);

                                ImGui::TableSetColumnIndex(3);
                                UICommon::AlignTableCellText(rowH);
                                long long profit = ItemTracker::GetStatProfit(st);
                                if (profit > 0)
                                    ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                else if (profit < 0)
                                    ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                else
                                    ImGui::TextUnformatted(Localization::GetText("no_profit"));

                                ImGui::TableSetColumnIndex(4);
                                UICommon::AlignTableCellText(rowH);
                                if (st.lastMagicFind >= 0)
                                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d%%", st.lastMagicFind);
                                else
                                    ImGui::TextDisabled("N/A");
                                if (ImGui::IsItemHovered())
                                    ImGui::SetTooltip("%s", Localization::GetText("magic_find_tooltip"));

                            }
                            ImGui::EndTable();
                        }
                    }
                    else { ImGui::PopStyleColor(3); }
                }
            }
            else
            {
                // Get best drop for highlighting
                auto bestDrop = ItemTracker::GetBestDrop();
                int bestDropId = bestDrop.first;
                if (bestDropId != 0 && !ItemTracker::PassesFilter(bestDrop.second))
                    bestDropId = 0;

                if (ImGui::BeginTabBar("##TypeTabs"))
                {
                    for (auto type : typeOrder)
                    {
                        auto it = typeGroups.find(type);
                        if (it == typeGroups.end() || it->second.empty()) continue;

                        char tabLabel[256];
                        snprintf(tabLabel, sizeof(tabLabel), "%s (%zu)", getTypeName(type).c_str(), it->second.size());

                        if (ImGui::BeginTabItem(tabLabel))
                        {
                            if (ImGui::BeginTable(("##TypeTable_" + std::to_string(static_cast<int>(type))).c_str(), itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                            {
                                float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                                ImGui::TableSetupColumn(Localization::GetText("magic_find"), ImGuiTableColumnFlags_WidthStretch);
                                ImGui::TableHeadersRow();

                                for (auto& [id, st] : it->second)
                                {
                                    float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.itemsIconSize));
                                    ImGui::TableNextRow(0, rowH);

                                    // Apply favorite row background color if enabled
                                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                                    }

                                    // Apply best drop golden border if enabled
                                    if (g_Settings.enableBestDropHighlight && id == bestDropId && st.count > 0)
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(1.0f, 0.84f, 0.0f, 0.15f))); // Gold with low alpha
                                    }

                                    ImGui::TableSetColumnIndex(0);
                                    UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.itemsIconSize));
                                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.itemsIconSize), st.details.loaded ? st.details.rarity : "");
                                    bool iconHovered = ImGui::IsItemHovered();

                                    // Right-click context menu
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                    {
                                        UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                    }

                                    ImGui::TableSetColumnIndex(1);
                                    UICommon::AlignTableCellText(rowH);
                                    std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");

                                    // Add star icon for favorites
                                    if (st.isFavorite)
                                    {
                                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* ");
                                        ImGui::SameLine();
                                    }

                                    ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                    if (st.details.loaded && !st.details.rarity.empty())
                                    {
                                        if (st.details.rarity == "Junk") col = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                                        else if (st.details.rarity == "Basic") col = ImVec4(1.f, 1.f, 1.f, 1.f);
                                        else if (st.details.rarity == "Fine") col = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                                        else if (st.details.rarity == "Masterwork") col = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                                        else if (st.details.rarity == "Rare") col = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                                        else if (st.details.rarity == "Exotic") col = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                                        else if (st.details.rarity == "Ascended") col = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                                        else if (st.details.rarity == "Legendary") col = ImVec4(0.55f, 0.25f, 0.85f, 1.f);
                                    }

                                    if (st.isFavorite && g_Settings.enableFavoriteTextColor)
                                        col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);

                                    ImGui::TextColored(col, "%s", name.c_str());
                                    bool nameHovered = ImGui::IsItemHovered();

                                    // Right-click context menu for name
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                                    {
                                        UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");
                                    }

                                    if (iconHovered || nameHovered)
                                    {
                                        UITooltips::ItemTooltipOptions opt;
                                        opt.showCount = true;
                                        opt.count = st.count;
                                        opt.showProfit = true;
                                        opt.profit = ItemTracker::GetStatProfit(st);
                                        opt.showTrading = true;
                                        opt.showAccountFlags = true;
                                        opt.showId = true;
                                        if (st.details.loaded)
                                            UITooltips::RenderItemTooltip(st.details, id, opt);
                                        else
                                            UITooltips::RenderItemTooltipFallback(name, "", id, opt);
                                    }

                                    ImGui::TableSetColumnIndex(2);
                                    UICommon::AlignTableCellText(rowH);
                                    ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                                    ImGui::TextColored(countColor, "%lld", st.count);

                                    ImGui::TableSetColumnIndex(3);
                                    UICommon::AlignTableCellText(rowH);
                                    long long profit = ItemTracker::GetStatProfit(st);
                                    if (profit > 0)
                                        ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                    else if (profit < 0)
                                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", UICommon::FormatCoin(profit).c_str());
                                    else
                                        ImGui::TextUnformatted(Localization::GetText("no_profit"));

                                    ImGui::TableSetColumnIndex(4);
                                    UICommon::AlignTableCellText(rowH);
                                    if (st.lastMagicFind >= 0)
                                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d%%", st.lastMagicFind);
                                    else
                                        ImGui::TextDisabled("N/A");
                                    if (ImGui::IsItemHovered())
                                        ImGui::SetTooltip("%s", Localization::GetText("magic_find_tooltip"));

                                    ImGui::TableSetColumnIndex(5);
                                    UICommon::AlignTableCellFrame(rowH);
                                    bool isFavorite = st.isFavorite;
                                    if (ImGui::Checkbox(("##fav_" + std::to_string(id)).c_str(), &isFavorite))
                                    {
                                        ItemTracker::SetFavorite(id, isFavorite);
                                    }
                                    if (ImGui::IsItemHovered())
                                        ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite"));

                                    ImGui::TableSetColumnIndex(6);
                                    UICommon::AlignTableCellFrame(rowH);
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
                            ImGui::EndTabItem();
                        }
                    }
                    ImGui::EndTabBar();
                }
            }
        }
    }

    // Context menu popup (rendered once outside the loop)
    UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::CopyOnly);
}
}
