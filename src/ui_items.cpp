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

static void InlineIcon(const char* key, float sz = 14.f)
{
    void* tex = UITabIcons::GetIcon(key);
    if (!tex) return;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float lineH = ImGui::GetTextLineHeight();
    float offY  = (lineH - sz) * 0.5f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Glow effect - draw icon multiple times with offset and alpha
    const float glowOffset = 2.0f;
    const float glowAlpha = 0.3f;
    dl->AddImage(
        (ImTextureID)tex,
        ImVec2(pos.x - glowOffset, pos.y + offY - glowOffset),
        ImVec2(pos.x + sz + glowOffset, pos.y + offY + sz + glowOffset),
        ImVec2(0,0), ImVec2(1,1),
        IM_COL32(255, 255, 255, (int)(255 * glowAlpha)));
    dl->AddImage(
        (ImTextureID)tex,
        ImVec2(pos.x - glowOffset * 0.5f, pos.y + offY - glowOffset * 0.5f),
        ImVec2(pos.x + sz + glowOffset * 0.5f, pos.y + offY + sz + glowOffset * 0.5f),
        ImVec2(0,0), ImVec2(1,1),
        IM_COL32(255, 255, 255, (int)(255 * glowAlpha * 0.6f)));

    // Main icon - white
    dl->AddImage(
        (ImTextureID)tex,
        ImVec2(pos.x, pos.y + offY),
        ImVec2(pos.x + sz, pos.y + offY + sz),
        ImVec2(0,0), ImVec2(1,1),
        IM_COL32(255, 255, 255, 255));
    ImGui::Dummy(ImVec2(sz + 5.f, lineH));
    ImGui::SameLine(0, 0);
}

// Static (non-collapsible) section header. Always renders its content underneath - clicking
// does nothing, there is no arrow/toggle. The flags parameter is kept for call-site compatibility
// but is no longer used.
static bool CollapsingHeaderWithIcon(const char* label, const char* iconKey, ImVec4 headerColor, ImGuiTreeNodeFlags flags = 0)
{
    (void)flags;

    ImDrawList* dl     = ImGui::GetWindowDrawList();
    ImVec2      pos    = ImGui::GetCursorScreenPos();
    float       w      = ImGui::GetContentRegionAvail().x;
    float       lineH  = ImGui::GetTextLineHeight();
    float       h      = lineH + 10.f;
    float       iconSz = 14.f;

    // Draw full-width background, tinted with the header color for a subtle colored background
    ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x*0.18f + 0.02f, headerColor.y*0.18f + 0.02f, headerColor.z*0.18f + 0.02f, 0.85f));
    dl->AddRectFilled({pos.x + 4.f, pos.y}, {pos.x + w, pos.y + h}, bgCol, 3.f);
    dl->AddRect({pos.x + 4.f, pos.y}, {pos.x + w, pos.y + h},
                ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x*0.6f, headerColor.y*0.6f, headerColor.z*0.6f, 0.8f)), 3.f, 0, 0.5f);
    dl->AddRectFilled(pos, {pos.x + 4.f, pos.y + h},
                      ImGui::ColorConvertFloat4ToU32(headerColor), 0.f);

    void* tex = UITabIcons::GetIcon(iconKey);
    float iconX = pos.x + 8.f;
    float iconY = pos.y + (h - iconSz) * 0.5f;
    if (tex)
        dl->AddImage((ImTextureID)tex, {iconX, iconY}, {iconX + iconSz, iconY + iconSz},
                     {0,0}, {1,1}, ImGui::ColorConvertFloat4ToU32(headerColor));

    float textX = iconX + iconSz + 5.f;
    float textY = pos.y + (h - lineH) * 0.5f;
    dl->AddText({textX, textY}, ImGui::ColorConvertFloat4ToU32(headerColor), label);

    // Advance the cursor by the header's height (no interactive widget, so nothing is clickable)
    ImGui::Dummy(ImVec2(w, h));

    return true;
}

static void DrawGridItemCount(long long count, float iconSz)
{
    std::string cs = UICommon::FormatCompact(count);
    const char* countStr = cs.c_str();

    // Origin holen
    ImVec2 origin   = ImGui::GetItemRectMin();

    // Basis-Schriftgröße berechnen - 10% kleiner!
    const float desiredPixelSize = iconSz * 0.54f;
    ImFont* font = ImGui::GetFont();

    // Text Größe berechnen MIT der gewünschten Schriftgröße
    ImVec2 textSize = font->CalcTextSizeA(desiredPixelSize, FLT_MAX, 0.0f, countStr);

    // Position unten rechts
    ImVec2 pos = ImVec2(origin.x + iconSz - textSize.x,
                      origin.y + iconSz - textSize.y - 2.0f);

    // Same colors as overview
    ImVec4 col = count < 0 ? ImVec4(0.9f,0.3f,0.3f,1.f)
                          : ImVec4(0.95f,0.7f,0.1f,1.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // 1. Sehr dünner weißer Outline (außen 5 Pixel)
    static const ImVec2 kOffWhiteOuter[] = {
        {-5,-5}, {-4,-5}, {-3,-5}, {-2,-5}, {-1,-5}, {0,-5}, {1,-5}, {2,-5}, {3,-5}, {4,-5}, {5,-5},
        {-5,-4}, {5,-4},
        {-5,-3}, {5,-3},
        {-5,-2}, {5,-2},
        {-5,-1}, {5,-1},
        {-5,0}, {5,0},
        {-5,1}, {5,1},
        {-5,2}, {5,2},
        {-5,3}, {5,3},
        {-5,4}, {5,4},
        {-5,5}, {-4,5}, {-3,5}, {-2,5}, {-1,5}, {0,5}, {1,5}, {2,5}, {3,5}, {4,5}, {5,5}
    };
    for (const auto& off : kOffWhiteOuter) {
        dl->AddText(font, desiredPixelSize, ImVec2(pos.x + off.x, pos.y + off.y), IM_COL32(255,255,255,255), countStr);
    }

    // 2. Dickerer schwarzer Outline (4 Pixel)
    static const ImVec2 kOffBlack[] = {
        {-4,-4}, {-3,-4}, {-2,-4}, {-1,-4}, {0,-4}, {1,-4}, {2,-4}, {3,-4}, {4,-4},
        {-4,-3}, {4,-3},
        {-4,-2}, {4,-2},
        {-4,-1}, {4,-1},
        {-4,0}, {4,0},
        {-4,1}, {4,1},
        {-4,2}, {4,2},
        {-4,3}, {4,3},
        {-4,4}, {-3,4}, {-2,4}, {-1,4}, {0,4}, {1,4}, {2,4}, {3,4}, {4,4}
    };
    for (const auto& off : kOffBlack) {
        dl->AddText(font, desiredPixelSize, ImVec2(pos.x + off.x, pos.y + off.y), IM_COL32(0,0,0,255), countStr);
    }

    // 3. Eigentlicher Text
    dl->AddText(font, desiredPixelSize, pos, ImGui::ColorConvertFloat4ToU32(col), countStr);
}

void RenderItemsTab()
{
    // Favorite Items Section (immer sichtbar, nicht mehr einklappbar)
    const bool favoritesExpanded = true;

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    float headerHeight = 35.0f;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 headerMin = cursor;
    ImVec2 headerMax = ImVec2(cursor.x + regionAvail.x, cursor.y + headerHeight);

    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    ImU32 bgColorTop    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
    ImU32 bgColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));
    drawList->AddRectFilledMultiColor(headerMin, headerMax, bgColorTop, bgColorTop, bgColorBottom, bgColorBottom);

    ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));
    drawList->AddRect(headerMin, headerMax, borderColor, 4.0f, 0, 0.5f);

    if (favoritesExpanded)
    {
        ImU32 activeColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.0f));
        drawList->AddRectFilled(ImVec2(headerMin.x, headerMin.y), ImVec2(headerMin.x + 3.0f, headerMax.y), activeColor, 2.0f);
    }

    float iconSize = 16.0f;
    float iconX = cursor.x + 12.0f;
    float iconY = cursor.y + (headerHeight - iconSize) * 0.5f;
    void* iconTex = UITabIcons::GetIcon("favorites");
    if (iconTex)
    {
        const float glowOffset = 2.0f;
        const float glowAlpha = 0.3f;
        
        drawList->AddImage(
            (ImTextureID)iconTex,
            ImVec2(iconX - glowOffset, iconY - glowOffset),
            ImVec2(iconX + iconSize + glowOffset, iconY + iconSize + glowOffset),
            ImVec2(0,0), ImVec2(1,1),
            IM_COL32(255, 255, 255, (int)(255 * glowAlpha)));
            
        drawList->AddImage(
            (ImTextureID)iconTex,
            ImVec2(iconX - glowOffset * 0.5f, iconY - glowOffset * 0.5f),
            ImVec2(iconX + iconSize + glowOffset * 0.5f, iconY + iconSize + glowOffset * 0.5f),
            ImVec2(0,0), ImVec2(1,1),
            IM_COL32(255, 255, 255, (int)(255 * glowAlpha * 0.6f)));

        drawList->AddImage((ImTextureID)iconTex,
                         ImVec2(iconX, iconY),
                         ImVec2(iconX + iconSize, iconY + iconSize),
                         ImVec2(0,0), ImVec2(1,1),
                         ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)));
    }

    float textX = iconX + iconSize + 8.0f;
    float textY = cursor.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f;
    drawList->AddText(ImVec2(textX, textY),
                     ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)),
                     Localization::GetText("favorite_items_header"));

    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + headerHeight + 8.0f));

    // FIX Bug 2: Pending-State für Favorites-Grid Rechtsklick (static, außerhalb des if-Blocks)
    static int s_FavGridPendingId = -1;
    static std::string s_FavGridPendingName;

    if (favoritesExpanded)
    {
        ImGui::Spacing();

        auto items = ItemTracker::GetFilteredItems();
        std::vector<std::pair<int, Stat>> favoriteItems;
        for (auto& [id, st] : items)
        {
            if (st.isFavorite && !st.IsCurrency())
                favoriteItems.push_back({id, st});
        }

        if (g_Settings.itemsFavoritesAsGrid)
        {
            // Grid View for Favorite Items
            float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;
            int columns = std::max(1, static_cast<int>((ImGui::GetContentRegionAvail().x - 40.0f - scrollbarWidth + spacing) / (cellSize + spacing)));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);

            int count = 0;
            for (auto& [id, st] : favoriteItems)
            {
                if (st.isIgnored || st.count == 0 || st.IsCurrency()) continue;

                if (count > 0)
                {
                    if (count % columns == 0)
                        ImGui::NewLine();
                    else
                        ImGui::SameLine();
                }

                ImGui::PushID(id);
                if (ImGui::BeginChild("##FavItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImVec2 cur = ImGui::GetCursorScreenPos();
                    
                    // Draw favorite row background if enabled
                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                    {
                        ImVec2 bgEnd = ImVec2(cur.x + g_Settings.gridIconSize, cur.y + g_Settings.gridIconSize);
                        ImGui::GetWindowDrawList()->AddRectFilled(cur, bgEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                    }

                    UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");

                    // Draw favorite star
                    if (st.isFavorite)
                        ImGui::GetWindowDrawList()->AddText(ImVec2(cur.x + 2.0f, cur.y + 2.0f), IM_COL32(255, 215, 0, 255), "*");

                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::ItemTooltipOptions opt;
                        opt.showCount = true; opt.count = st.count;
                        opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                        opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                        if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                        else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                    }

                    // Rechtsklick → pending setzen
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        s_FavGridPendingId   = id;
                        s_FavGridPendingName = st.details.loaded ? st.details.name : "";
                    }

                    DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
                }
                ImGui::EndChild();
                ImGui::PopID();
                count++;
            }

            // FIX Bug 2: Context-Menu außerhalb der Schleife rendern
            if (s_FavGridPendingId != -1)
            {
                UIContextMenu::OpenContextMenu("FavItemsGridMenu", s_FavGridPendingId, s_FavGridPendingName);
                s_FavGridPendingId = -1;
            }
            UIContextMenu::RenderItemContextMenu("FavItemsGridMenu", UIContextMenu::ContextMenuType::Favorites);
        }
        else
        {
            // List View for Favorite Items
            auto setupTable = [&](const char* id, int cols) -> bool {
                float icw = std::max(32.0f + 10.f, 70.f);
                if (!ImGui::BeginTable(id, cols, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable|ImGuiTableFlags_NoSavedSettings)) return false;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, icw);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 430.f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 150.f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                return true;
            };
            if (setupTable("##FavoriteItemsTable", 4))
            {

                for (auto& [id, st] : favoriteItems)
                {
                    if (st.isIgnored || st.count == 0 || st.IsCurrency()) continue;

                    float rowH = UICommon::CalcTableRowHeight(32.f);
                    ImGui::TableNextRow(0, rowH);

                    if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));

                    ImGui::TableSetColumnIndex(0);
                    UICommon::DrawItemIconCell(id, st.details.iconUrl, 32.f, st.details.loaded ? st.details.rarity : "");
                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::ItemTooltipOptions opt;
                        opt.showCount = true; opt.count = st.count;
                        opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                        opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                        if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                        else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                    }

                    ImGui::TableSetColumnIndex(1);
                    if (st.isFavorite) { ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "* "); ImGui::SameLine(); }

                    ImVec4 nameCol = ImVec4(1.f, 1.f, 1.f, 1.f);
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
                    if (st.details.rarity == rarityName)
                        IgnoredItemsManager::IgnoreItem(id);
                SettingsManager::Save();
            }
        };
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.f)); renderMenuItem("mass_actions_ignore_junk", "Junk"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f)); renderMenuItem("mass_actions_ignore_basic", "Basic"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.f, 1.f)); renderMenuItem("mass_actions_ignore_fine", "Fine"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.f)); renderMenuItem("mass_actions_ignore_masterwork", "Masterwork"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.9f, 0.2f, 1.f)); renderMenuItem("mass_actions_ignore_rare", "Rare"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.6f, 0.2f, 1.f)); renderMenuItem("mass_actions_ignore_exotic", "Exotic"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.20f, 0.50f, 1.f)); renderMenuItem("mass_actions_ignore_ascended", "Ascended"); ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.20f, 0.90f, 1.f)); renderMenuItem("mass_actions_ignore_legendary", "Legendary"); ImGui::PopStyleColor();
        ImGui::Separator();
        if (ImGui::MenuItem(Localization::GetText("mass_actions_clear_ignore"))) { IgnoredItemsManager::ClearAll(); SettingsManager::Save(); }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    if (ImGui::BeginPopup("ItemsLoadSavePopup"))
    {
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
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("export_json_tooltip"));
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
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("export_csv_tooltip"));
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("import_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import_items_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\items_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f) { fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET); if (sz > 0) { std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f); try { ItemTracker::ImportFromJson(nlohmann::json::parse(buf)); } catch (...) {} } else fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("import_items_json_tooltip"));
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    auto sortedItems = ItemTracker::GetSortedItems(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));
    // Filter out ignored items
    std::vector<std::pair<int, Stat>> filteredSortedItems;
    for (auto& [id, st] : sortedItems)
    {
        if (!st.isIgnored && st.count != 0 && !st.IsCurrency())
            filteredSortedItems.push_back({id, st});
    }
    sortedItems.swap(filteredSortedItems);

    if (g_Settings.itemsEnableGridView)
    {
        float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float scrollbarWidth = 20.0f;

        auto getColumns = [&](float width) {
            return std::max(1, static_cast<int>((width - scrollbarWidth + spacing) / (cellSize + spacing)));
        };

        ImGui::Spacing();

        static int s_GridPendingId = -1;
        static std::string s_GridPendingName;

        if (ImGui::BeginChild("##ItemsGrid", ImVec2(0, 0), true))
        {
            if (g_Settings.itemsGroupByRarity)
            {
                std::vector<std::string> rarityOrder = {
                    Localization::GetText("rarity_name_legendary"), Localization::GetText("rarity_name_ascended"),
                    Localization::GetText("rarity_name_exotic"), Localization::GetText("rarity_name_rare"),
                    Localization::GetText("rarity_name_masterwork"), Localization::GetText("rarity_name_fine"),
                    Localization::GetText("rarity_name_basic"), Localization::GetText("rarity_name_junk"),
                    Localization::GetText("rarity_name_unknown")
                };
                std::map<std::string, std::vector<std::pair<int, Stat>>> rarityGroups;
                for (auto& [id, st] : sortedItems)
                {
                    std::string ar = st.details.loaded ? st.details.rarity : "Unknown";
                    std::string lr;
                    if (ar == "Legendary") lr = Localization::GetText("rarity_name_legendary");
                    else if (ar == "Ascended") lr = Localization::GetText("rarity_name_ascended");
                    else if (ar == "Exotic") lr = Localization::GetText("rarity_name_exotic");
                    else if (ar == "Rare") lr = Localization::GetText("rarity_name_rare");
                    else if (ar == "Masterwork") lr = Localization::GetText("rarity_name_masterwork");
                    else if (ar == "Fine") lr = Localization::GetText("rarity_name_fine");
                    else if (ar == "Basic") lr = Localization::GetText("rarity_name_basic");
                    else if (ar == "Junk") lr = Localization::GetText("rarity_name_junk");
                    else lr = Localization::GetText("rarity_name_unknown");
                    rarityGroups[lr].push_back({id, st});
                }

                auto renderCell = [&](int id, const Stat& st) {
                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                    {
                        ImVec2 cur = ImGui::GetCursorPos();
                        char btnId[64]; snprintf(btnId, sizeof(btnId), "##IB_%d", id);
                        if (ImGui::InvisibleButton(btnId, ImVec2((float)g_Settings.gridIconSize, (float)g_Settings.gridIconSize))) {}
                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { s_GridPendingId = id; s_GridPendingName = st.details.loaded ? st.details.name : ""; }
                        ImGui::SetCursorPos(cur);
                        UICommon::DrawItemIconCell(id, st.details.iconUrl, (float)g_Settings.gridIconSize, st.details.loaded ? st.details.rarity : "");
                        if (ImGui::IsItemHovered()) {
                            UITooltips::ItemTooltipOptions opt; opt.showCount=true; opt.count=st.count; opt.showProfit=true; opt.profit=ItemTracker::GetStatProfit(st); opt.showTrading=true; opt.showAccountFlags=true; opt.showId=true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt); else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                        DrawGridItemCount(st.count, (float)g_Settings.gridIconSize);
                    }
                    ImGui::EndChild();
                };

                std::map<std::string,ImVec4> rcGrid;
                rcGrid[Localization::GetText("rarity_name_legendary")]=ImVec4(1.f,0.5f,0.8f,1.f);
                rcGrid[Localization::GetText("rarity_name_ascended")]=ImVec4(0.9f,0.3f,0.9f,1.f);
                rcGrid[Localization::GetText("rarity_name_exotic")]=ImVec4(1.f,0.6f,0.f,1.f);
                rcGrid[Localization::GetText("rarity_name_rare")]=ImVec4(1.f,0.9f,0.f,1.f);
                rcGrid[Localization::GetText("rarity_name_masterwork")]=ImVec4(0.2f,0.8f,0.2f,1.f);
                rcGrid[Localization::GetText("rarity_name_fine")]=ImVec4(0.f,0.5f,1.f,1.f);
                rcGrid[Localization::GetText("rarity_name_basic")]=ImVec4(1.f,1.f,1.f,1.f);
                rcGrid[Localization::GetText("rarity_name_junk")]=ImVec4(0.7f,0.7f,0.7f,1.f);
                rcGrid[Localization::GetText("rarity_name_unknown")]=ImVec4(0.5f,0.5f,0.5f,1.f);

                if (g_Settings.itemsShowRarityAsTabs)
                {
                    static int selRarTab = 0;
                    if (ImGui::BeginTabBar("##RarityTabs"))
                    {
                        for (const auto& rn : rarityOrder)
                        {
                            auto it = rarityGroups.find(rn);
                            if (it == rarityGroups.end() || it->second.empty()) continue;
                            ImVec4 tabColor = rcGrid.count(rn) ? rcGrid[rn] : ImVec4(1.f,1.f,1.f,1.f);
                            ImGui::PushStyleColor(ImGuiCol_Text, tabColor);
                            bool tabOpen = ImGui::BeginTabItem(rn.c_str());
                            ImGui::PopStyleColor();
                            if (tabOpen)
                            {
                                int cols = getColumns(ImGui::GetContentRegionAvail().x), col = 0;
                                for (auto& [id, st] : it->second) { if (col > 0) ImGui::SameLine(); ImGui::PushID(id); renderCell(id, st); ImGui::PopID(); col++; if (col >= cols) col = 0; }
                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                }
                else
                {
                    for (const auto& rn : rarityOrder)
                    {
                        auto it = rarityGroups.find(rn);
                        if (it == rarityGroups.end() || it->second.empty()) continue;
                        ImVec4 hcGrid = rcGrid.count(rn) ? rcGrid[rn] : ImVec4(1.f,1.f,1.f,1.f);
                        ImGui::Spacing();
                        if (CollapsingHeaderWithIcon(rn.c_str(), "items", hcGrid, ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            int cols = getColumns(ImGui::GetContentRegionAvail().x), col = 0;
                            for (auto& [id, st] : it->second) { if (col > 0) ImGui::SameLine(); ImGui::PushID(id); renderCell(id, st); ImGui::PopID(); col++; if (col >= cols) col = 0; }
                        }
                    }
                }
            }
            else if (g_Settings.itemsGroupByCategory)
            {
                std::vector<ItemType> typeOrder = {
                    ItemType::Weapon, ItemType::Armor, ItemType::Trinket, ItemType::Backpack,
                    ItemType::CraftingMaterial, ItemType::Consumable, ItemType::Container, ItemType::Bag,
                    ItemType::UpgradeComponent, ItemType::Trophy, ItemType::Gizmo, ItemType::Tool,
                    ItemType::GatheringTool, ItemType::MiniPet, ItemType::Unlock, ItemType::Unknown
                };
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
                std::map<ItemType, std::vector<std::pair<int, Stat>>> typeGroups;
                for (auto& [id, st] : sortedItems) typeGroups[st.details.itemType].push_back({id, st});

                auto renderCell = [&](int id, const Stat& st) {
                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                    {
                        ImVec2 cur = ImGui::GetCursorPos();
                        char btnId[64]; snprintf(btnId, sizeof(btnId), "##IB_%d", id);
                        if (ImGui::InvisibleButton(btnId, ImVec2((float)g_Settings.gridIconSize, (float)g_Settings.gridIconSize))) {}
                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { s_GridPendingId = id; s_GridPendingName = st.details.loaded ? st.details.name : ""; }
                        ImGui::SetCursorPos(cur);
                        UICommon::DrawItemIconCell(id, st.details.iconUrl, (float)g_Settings.gridIconSize, st.details.loaded ? st.details.rarity : "");
                        if (ImGui::IsItemHovered()) {
                            UITooltips::ItemTooltipOptions opt; opt.showCount=true; opt.count=st.count; opt.showProfit=true; opt.profit=ItemTracker::GetStatProfit(st); opt.showTrading=true; opt.showAccountFlags=true; opt.showId=true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt); else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                        DrawGridItemCount(st.count, (float)g_Settings.gridIconSize);
                    }
                    ImGui::EndChild();
                };

                if (g_Settings.itemsShowGroupAsTabs)
                {
                    static int selTypeTab = 0;
                    if (ImGui::BeginTabBar("##TypeTabs"))
                    {
                        for (auto type : typeOrder)
                        {
                            auto it = typeGroups.find(type);
                            if (it == typeGroups.end() || it->second.empty()) continue;
                            std::string tn = getTypeName(type);
                            if (ImGui::BeginTabItem(tn.c_str()))
                            {
                                int cols = getColumns(ImGui::GetContentRegionAvail().x), col = 0;
                                for (auto& [id, st] : it->second) { if (col > 0) ImGui::SameLine(); ImGui::PushID(id); renderCell(id, st); ImGui::PopID(); col++; if (col >= cols) col = 0; }
                                ImGui::EndTabItem();
                            }
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
                        const char* iconKey = "items";
                        if (type == ItemType::Weapon) iconKey = "sword";
                        if (CollapsingHeaderWithIcon(getTypeName(type).c_str(), iconKey, ImVec4(0.85f,0.70f,0.40f,1.f), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            int cols = getColumns(ImGui::GetContentRegionAvail().x), col = 0;
                            for (auto& [id, st] : it->second) { if (col > 0) ImGui::SameLine(); ImGui::PushID(id); renderCell(id, st); ImGui::PopID(); col++; if (col >= cols) col = 0; }
                        }
                    }
                }
            }
            else
            {
                int columns = getColumns(ImGui::GetContentRegionAvail().x), col = 0;
                for (auto& [id, st] : sortedItems)
                {
                    if (col > 0) ImGui::SameLine();
                    ImGui::PushID(id);
                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                    {
                        ImVec2 cur = ImGui::GetCursorPos();
                        char btnId[64]; snprintf(btnId, sizeof(btnId), "##IB_%d", id);
                        if (ImGui::InvisibleButton(btnId, ImVec2((float)g_Settings.gridIconSize, (float)g_Settings.gridIconSize))) {}
                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { s_GridPendingId = id; s_GridPendingName = st.details.loaded ? st.details.name : ""; }
                        ImGui::SetCursorPos(cur);
                        UICommon::DrawItemIconCell(id, st.details.iconUrl, (float)g_Settings.gridIconSize, st.details.loaded ? st.details.rarity : "");
                        if (ImGui::IsItemHovered()) {
                            UITooltips::ItemTooltipOptions opt; opt.showCount=true; opt.count=st.count; opt.showProfit=true; opt.profit=ItemTracker::GetStatProfit(st); opt.showTrading=true; opt.showAccountFlags=true; opt.showId=true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt); else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                        DrawGridItemCount(st.count, (float)g_Settings.gridIconSize);
                    }
                    ImGui::EndChild();
                    ImGui::PopID();
                    col++; if (col >= columns) col = 0;
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
        int itemTableColumnCount = 5;
        auto bestDrop = ItemTracker::GetBestDrop();
        int bestDropId = bestDrop.first;
        if (bestDropId != 0 && !ItemTracker::PassesFilter(bestDrop.second)) bestDropId = 0;

        auto renderItemRow = [&](int id, const Stat& st, int colCount) {
            float rowH = UICommon::CalcTableRowHeight((float)g_Settings.itemsIconSize);
            ImGui::TableNextRow(0, rowH);
            if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
            if (g_Settings.enableBestDropHighlight && id == bestDropId && st.count > 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.bestDropHighlightColor[0], g_Settings.bestDropHighlightColor[1], g_Settings.bestDropHighlightColor[2], g_Settings.bestDropHighlightColor[3])));

            ImGui::TableSetColumnIndex(0);
            UICommon::AlignTableCellIcon(rowH, (float)g_Settings.itemsIconSize);
            UICommon::DrawItemIconCell(id, st.details.iconUrl, (float)g_Settings.itemsIconSize, st.details.loaded ? st.details.rarity : "");
            auto renderTT = [&]() {
                UITooltips::ItemTooltipOptions opt; opt.showCount=true; opt.count=st.count; opt.showProfit=true; opt.profit=ItemTracker::GetStatProfit(st); opt.showTrading=st.details.loaded; opt.showAccountFlags=st.details.loaded; opt.showId=true;
                if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt); else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
            };
            if (ImGui::IsItemHovered()) renderTT();
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");

            ImGui::TableSetColumnIndex(1);
            UICommon::AlignTableCellText(rowH);
            std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
            if (st.isFavorite) { ImGui::TextColored(ImVec4(1.0f,0.8f,0.4f,1.0f), "* "); ImGui::SameLine(); }
            ImVec4 col = ImVec4(1.f,1.f,1.f,1.f);
            if (st.details.loaded && !st.details.rarity.empty()) {
                if (st.details.rarity=="Junk") col=ImVec4(0.7f,0.7f,0.7f,1.f);
                else if (st.details.rarity=="Basic") col=ImVec4(1.f,1.f,1.f,1.f);
                else if (st.details.rarity=="Fine") col=ImVec4(0.f,0.5f,1.f,1.f);
                else if (st.details.rarity=="Masterwork") col=ImVec4(0.2f,0.8f,0.2f,1.f);
                else if (st.details.rarity=="Rare") col=ImVec4(1.f,0.9f,0.f,1.f);
                else if (st.details.rarity=="Exotic") col=ImVec4(1.f,0.6f,0.f,1.f);
                else if (st.details.rarity=="Ascended") col=ImVec4(0.9f,0.3f,0.9f,1.f);
                else if (st.details.rarity=="Legendary") col=ImVec4(0.55f,0.25f,0.85f,1.f);
            }
            if (st.isFavorite && g_Settings.enableFavoriteTextColor) col=ImVec4(g_Settings.favoriteTextColor[0],g_Settings.favoriteTextColor[1],g_Settings.favoriteTextColor[2],g_Settings.favoriteTextColor[3]);
            ImGui::TextColored(col, "%s", name.c_str());
            if (ImGui::IsItemHovered()) renderTT();
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) UIContextMenu::OpenContextMenu("ItemContextMenu", id, st.details.loaded ? st.details.name : "");

            ImGui::TableSetColumnIndex(2);
            UICommon::AlignTableCellText(rowH);
            ImVec4 cc = st.count>0?ImVec4(1.f,0.84f,0.f,1.f):(st.count<0?ImVec4(0.9f,0.2f,0.2f,1.f):ImVec4(1.f,1.f,1.f,1.f));
            ImGui::TextColored(cc, "%lld", st.count);

            ImGui::TableSetColumnIndex(3);
            UICommon::AlignTableCellText(rowH);
            long long profit = ItemTracker::GetStatProfit(st);
            if (profit>0) ImGui::TextColored(ImVec4(1.f,0.84f,0.f,1.f),"%s",UICommon::FormatCoin(profit).c_str());
            else if (profit<0) ImGui::TextColored(ImVec4(0.9f,0.2f,0.2f,1.f),"%s",UICommon::FormatCoin(profit).c_str());
            else ImGui::TextUnformatted(Localization::GetText("no_profit"));

            ImGui::TableSetColumnIndex(4);
            UICommon::AlignTableCellText(rowH);
            if (st.lastMagicFind>=0) ImGui::TextColored(ImVec4(0.4f,0.8f,1.0f,1.0f),"%d%%",st.lastMagicFind);
            else ImGui::TextDisabled("N/A");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",Localization::GetText("magic_find_tooltip"));
        };

        auto setupTable = [&](const char* id, int cols) -> bool {
            float icw = std::max((float)g_Settings.itemsIconSize+10.f, 70.f);
            if (!ImGui::BeginTable(id, cols, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable|ImGuiTableFlags_NoSavedSettings)) return false;
            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, icw);
            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 430.f);
            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 150.f);
            ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 150.f);
            ImGui::TableSetupColumn(Localization::GetText("magic_find"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            return true;
        };

        if (!g_Settings.itemsGroupByRarity && !g_Settings.itemsGroupByCategory)
        {
            if (setupTable("##ItemsTable_v3", itemTableColumnCount))
            {
                for (auto& [id, st] : sortedItems) renderItemRow(id, st, itemTableColumnCount);
                UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::General);
                ImGui::EndTable();
            }
        }
        else if (g_Settings.itemsGroupByRarity)
        {
            std::vector<std::string> rarityOrder = {
                Localization::GetText("rarity_name_junk"), Localization::GetText("rarity_name_basic"),
                Localization::GetText("rarity_name_fine"), Localization::GetText("rarity_name_masterwork"),
                Localization::GetText("rarity_name_rare"), Localization::GetText("rarity_name_exotic"),
                Localization::GetText("rarity_name_ascended"), Localization::GetText("rarity_name_legendary"),
                Localization::GetText("rarity_name_unknown")
            };
            std::map<std::string, std::vector<std::pair<int,Stat>>> rarityGroups;
            for (auto& [id, st] : sortedItems) {
                std::string ar = st.details.loaded ? st.details.rarity : "Unknown", lr;
                if (ar=="Legendary") lr=Localization::GetText("rarity_name_legendary");
                else if (ar=="Ascended") lr=Localization::GetText("rarity_name_ascended");
                else if (ar=="Exotic") lr=Localization::GetText("rarity_name_exotic");
                else if (ar=="Rare") lr=Localization::GetText("rarity_name_rare");
                else if (ar=="Masterwork") lr=Localization::GetText("rarity_name_masterwork");
                else if (ar=="Fine") lr=Localization::GetText("rarity_name_fine");
                else if (ar=="Basic") lr=Localization::GetText("rarity_name_basic");
                else if (ar=="Junk") lr=Localization::GetText("rarity_name_junk");
                else lr=Localization::GetText("rarity_name_unknown");
                rarityGroups[lr].push_back({id,st});
            }
            std::map<std::string,ImVec4> rc;
            rc[Localization::GetText("rarity_name_legendary")]=ImVec4(1.f,0.5f,0.8f,1.f);
            rc[Localization::GetText("rarity_name_ascended")]=ImVec4(0.9f,0.3f,0.9f,1.f);
            rc[Localization::GetText("rarity_name_exotic")]=ImVec4(1.f,0.6f,0.f,1.f);
            rc[Localization::GetText("rarity_name_rare")]=ImVec4(1.f,0.9f,0.f,1.f);
            rc[Localization::GetText("rarity_name_masterwork")]=ImVec4(0.2f,0.8f,0.2f,1.f);
            rc[Localization::GetText("rarity_name_fine")]=ImVec4(0.f,0.5f,1.f,1.f);
            rc[Localization::GetText("rarity_name_basic")]=ImVec4(1.f,1.f,1.f,1.f);
            rc[Localization::GetText("rarity_name_junk")]=ImVec4(0.7f,0.7f,0.7f,1.f);
            rc[Localization::GetText("rarity_name_unknown")]=ImVec4(0.5f,0.5f,0.5f,1.f);

            if (!g_Settings.itemsShowRarityAsTabs)
            {
                for (const auto& r : rarityOrder) {
                    if (rarityGroups.find(r)==rarityGroups.end()||rarityGroups[r].empty()) continue;
                    char hl[256]; snprintf(hl,sizeof(hl),"%s (%zu)",r.c_str(),rarityGroups[r].size());
                    ImVec4 hc = rc.count(r) ? rc[r] : ImVec4(1.f,1.f,1.f,1.f);
                    if (CollapsingHeaderWithIcon(hl, "items", hc, ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (setupTable(("##RarityTable_v3_"+r).c_str(), itemTableColumnCount)) {
                            for (auto& [id,st]:rarityGroups[r]) renderItemRow(id,st,itemTableColumnCount);
                            UIContextMenu::RenderItemContextMenu("ItemContextMenu",UIContextMenu::ContextMenuType::General);
                            ImGui::EndTable();
                        }
                    }
                }
            }
            else
            {
                if (ImGui::BeginTabBar("##RarityTabs")) {
                    for (const auto& r : rarityOrder) {
                        if (rarityGroups.find(r)==rarityGroups.end()||rarityGroups[r].empty()) continue;
                        char tl[256]; snprintf(tl,sizeof(tl),"%s (%zu)",r.c_str(),rarityGroups[r].size());
                        ImVec4 tc=rc.count(r)?rc[r]:ImVec4(1.f,1.f,1.f,1.f);
                        ImGui::PushStyleColor(ImGuiCol_Text,tc);
                        if (ImGui::BeginTabItem(tl)) {
                            ImGui::PopStyleColor();
                            if (setupTable(("##RarityTable_"+r).c_str(),itemTableColumnCount)) {
                                for (auto& [id,st]:rarityGroups[r]) renderItemRow(id,st,itemTableColumnCount);
                                UIContextMenu::RenderItemContextMenu("ItemContextMenu",UIContextMenu::ContextMenuType::CopyOnly);
                                ImGui::EndTable();
                            }
                            ImGui::EndTabItem();
                        } else ImGui::PopStyleColor();
                    }
                    ImGui::EndTabBar();
                }
            }
        }
        else if (g_Settings.itemsGroupByCategory)
        {
            std::vector<ItemType> typeOrder = {
                ItemType::Weapon, ItemType::Armor, ItemType::Trinket, ItemType::Backpack,
                ItemType::CraftingMaterial, ItemType::Consumable, ItemType::Container, ItemType::Bag,
                ItemType::UpgradeComponent, ItemType::Trophy, ItemType::Gizmo, ItemType::Tool,
                ItemType::GatheringTool, ItemType::MiniPet, ItemType::Unlock, ItemType::Unknown
            };
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
            std::map<ItemType, std::vector<std::pair<int,Stat>>> typeGroups;
            for (auto& [id,st]:sortedItems) typeGroups[st.details.itemType].push_back({id,st});

            if (!g_Settings.itemsShowGroupAsTabs)
            {
                for (auto type : typeOrder) {
                    auto it=typeGroups.find(type); if(it==typeGroups.end()||it->second.empty()) continue;
                    ImGui::Spacing();
                    char hl[256]; snprintf(hl,sizeof(hl),"%s (%zu)",getTypeName(type).c_str(),it->second.size());
                    const char* iconKey = "items";
                    if (type == ItemType::Weapon) iconKey = "sword";
                    if (CollapsingHeaderWithIcon(hl, iconKey, ImVec4(0.85f,0.70f,0.40f,1.f), ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (setupTable(("##TypeTable_"+std::to_string((int)type)).c_str(), itemTableColumnCount)) {
                            for (auto& [id,st]:it->second) renderItemRow(id,st,itemTableColumnCount);
                            UIContextMenu::RenderItemContextMenu("ItemContextMenu",UIContextMenu::ContextMenuType::General);
                            ImGui::EndTable();
                        }
                    }
                }
            }
            else // itemsShowGroupAsTabs
            {
                if (ImGui::BeginTabBar("##TypeTabs")) {
                    for (auto type : typeOrder) {
                        auto it=typeGroups.find(type); if(it==typeGroups.end()||it->second.empty()) continue;
                        char tl[256]; snprintf(tl,sizeof(tl),"%s (%zu)",getTypeName(type).c_str(),it->second.size());
                        if (ImGui::BeginTabItem(tl)) {
                            if (setupTable(("##TypeTable_"+std::to_string((int)type)).c_str(), itemTableColumnCount)) {
                                for (auto& [id,st]:it->second) renderItemRow(id,st,itemTableColumnCount);
                                UIContextMenu::RenderItemContextMenu("ItemContextMenu",UIContextMenu::ContextMenuType::General);
                                ImGui::EndTable();
                            }
                            ImGui::EndTabItem();
                        }
                    }
                    ImGui::EndTabBar();
                }
            }
        } // end else if itemsGroupByCategory
    }

    UIContextMenu::RenderItemContextMenu("ItemContextMenu", UIContextMenu::ContextMenuType::CopyOnly);
}
} // namespace UIItems
