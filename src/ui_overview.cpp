#include "ui_overview.h"
#include "settings.h"
#include "item_tracker.h"
#include "localization.h"
#include "ui_common.h"
#include "ui_tab_icons.h"
#include "ignored_items.h"
#include "ui_favorites.h"
#include "custom_profit.h"
#include "ui_tooltips.h"
#include "ui_context_menu.h"

namespace UIOverview
{
    static void InlineIcon(const char* key, float sz = 14.f)
    {
        void* tex = UITabIcons::GetIcon(key);
        if (!tex) return;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float lineH = ImGui::GetTextLineHeight();
        float offY  = (lineH - sz) * 0.5f;
        ImDrawList* dl = ImGui::GetWindowDrawList();

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

    static std::map<std::string, ImVec4> GetRarityHeaderColors()
    {
        std::map<std::string, ImVec4> rc;
        rc[Localization::GetText("rarity_name_legendary")]  = ImVec4(1.f,0.5f,0.8f,1.f);
        rc[Localization::GetText("rarity_name_ascended")]   = ImVec4(0.9f,0.3f,0.9f,1.f);
        rc[Localization::GetText("rarity_name_exotic")]     = ImVec4(1.f,0.6f,0.f,1.f);
        rc[Localization::GetText("rarity_name_rare")]       = ImVec4(1.f,0.9f,0.f,1.f);
        rc[Localization::GetText("rarity_name_masterwork")] = ImVec4(0.2f,0.8f,0.2f,1.f);
        rc[Localization::GetText("rarity_name_fine")]       = ImVec4(0.f,0.5f,1.f,1.f);
        rc[Localization::GetText("rarity_name_basic")]      = ImVec4(1.f,1.f,1.f,1.f);
        rc[Localization::GetText("rarity_name_junk")]       = ImVec4(0.7f,0.7f,0.7f,1.f);
        rc[Localization::GetText("rarity_name_unknown")]    = ImVec4(0.5f,0.5f,0.5f,1.f);
        return rc;
    }

    static const ImVec4 kCategoryHeaderColor = ImVec4(0.85f,0.70f,0.40f,1.f);

    // Card header with white icon and accent-colored background (matching active main tab design)
    static void CardHeader(const char* iconKey, const char* label)
    {
        const float acR = g_Settings.accentColorR;
        const float acG = g_Settings.accentColorG;
        const float acB = g_Settings.accentColorB;

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 region = ImGui::GetContentRegionAvail();
        float headerHeight = 35.0f;

        ImU32 bgColorTop = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
        ImU32 bgColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec2 headerMin = cursor;
        ImVec2 headerMax = ImVec2(cursor.x + region.x, cursor.y + headerHeight);

        dl->AddRectFilledMultiColor(headerMin, headerMax, bgColorTop, bgColorTop, bgColorBottom, bgColorBottom);

        ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));
        dl->AddRect(headerMin, headerMax, borderColor, 4.0f, 0, 0.5f);

        void* tex = UITabIcons::GetIcon(iconKey);
        float sz  = 16.0f;
        float lineH = ImGui::GetTextLineHeight();

        if (tex)
        {
            ImVec2 iconPos = ImVec2(cursor.x + 12.f, cursor.y + (headerHeight - sz) * 0.5f);
            const float glowOffset = 2.0f;
            const float glowAlpha = 0.3f;
            
            dl->AddImage(
                (ImTextureID)tex,
                ImVec2(iconPos.x - glowOffset, iconPos.y - glowOffset),
                ImVec2(iconPos.x + sz + glowOffset, iconPos.y + sz + glowOffset),
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32(255, 255, 255, (int)(255 * glowAlpha)));
                
            dl->AddImage(
                (ImTextureID)tex,
                ImVec2(iconPos.x - glowOffset * 0.5f, iconPos.y - glowOffset * 0.5f),
                ImVec2(iconPos.x + sz + glowOffset * 0.5f, iconPos.y + sz + glowOffset * 0.5f),
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32(255, 255, 255, (int)(255 * glowAlpha * 0.6f)));

            dl->AddImage(
                (ImTextureID)tex,
                iconPos,
                ImVec2(iconPos.x + sz, iconPos.y + sz),
                ImVec2(0, 0), ImVec2(1, 1),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)));
        }

        ImVec2 textPos = ImVec2(cursor.x + 12.f + sz + 8.f, cursor.y + (headerHeight - lineH) * 0.5f);
        dl->AddText(textPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)), label);

        ImGui::Dummy(ImVec2(0, headerHeight));
    }

    static void DrawGridCount(long long count, float iconSz, bool isCurrency = false, int currencyId = 0)
    {
        bool isCoin = (isCurrency && currencyId == 1);
        
        std::string cs;
        if (isCoin) {
            cs = UICommon::FormatCoin(count);
        } else {
            cs = UICommon::FormatCompact(count);
        }
        const char* countStr = cs.c_str();
        
        // Origin holen
        ImVec2 origin   = ImGui::GetItemRectMin();
        
        // Basis-Schriftgröße berechnen - 10% kleiner!
        const float desiredPixelSize = isCoin ? iconSz * 0.495f : iconSz * 0.54f;
        ImFont* font = ImGui::GetFont();
        
        // Text Größe berechnen MIT der gewünschten Schriftgröße
        ImVec2 textSize = font->CalcTextSizeA(desiredPixelSize, FLT_MAX, 0.0f, countStr);
        
        // Position: nur bei Coin weiter nach rechts!
        const float rightPush = (isCurrency && currencyId == 1) ? 35.0f : 0.0f;
        ImVec2 pos = ImVec2(origin.x + iconSz - textSize.x + rightPush,
                          origin.y + iconSz - textSize.y - 2.0f);
        
        // Farben
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

    static void DrawGridItemCount(long long count, float iconSz)
    {
        DrawGridCount(count, iconSz, false, 0);
    }

    static void RenderFavoritesSubsection(const std::vector<int>& ids, bool isCurrency, const char* headerLabel, const char* iconKey,
                                        int& pendingId, std::string& pendingName, bool& pendingIsCurrency,
                                        const char* gridContextMenuId, const char* tableContextMenuId)
    {
        CardHeader(iconKey, headerLabel);

        if (ids.empty())
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", Localization::GetText("no_favorites_yet"));
            ImGui::Spacing();
            return;
        }

        if (g_Settings.overviewFavoritesAsGrid)
        {
            // Grid view for this subsection
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
            float iconSize = isCurrency ? static_cast<float>(g_Settings.overviewFavoritesIconSize) : static_cast<float>(g_Settings.gridIconSize);
            float cellSize = isCurrency ? (iconSize + 20.0f) : (iconSize + 10.0f); // Same cell size as regular currencies
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;

            auto getColumns = [&](float width) {
                return std::max(1, static_cast<int>((width - scrollbarWidth + spacing) / (cellSize + spacing)));
            };

            int columns = getColumns(ImGui::GetContentRegionAvail().x);
            int count = 0;

            for (int id : ids)
            {
                Stat st;
                if (isCurrency)
                    st = ItemTracker::GetCurrencyStat(id);
                else
                    st = ItemTracker::GetItemStat(id);
                
                if (st.isIgnored) continue;
                // Only show if count > 0 for both items and currencies
                if (st.count == 0) continue;

                bool isCoin = isCurrency && id == 1;

                if (count > 0)
                {
                    if (count % columns == 0)
                    {
                        ImGui::NewLine();
                    }
                    else
                    {
                        if (!isCoin)
                            ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x + 15.0f);
                        else
                            ImGui::SameLine();
                    }
                }
                else
                {
                    // Erste Zeile (Coin ist hier immer das erste Element)
                    if (isCurrency && isCoin)
                    {
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f); // 20px
                    }
                }

                ImGui::PushID(id);
                
                if (isCurrency)
                {
                    // Keep the old group approach for currencies
                    ImGui::BeginGroup(); // Gesamte Zelle (inkl. Abstände) in eine Gruppe

                    // Eigentliche Icon-Gruppe
                    ImGui::BeginGroup();
                    ImVec2 cur = ImGui::GetCursorScreenPos();

                    if (st.isFavorite && isCurrency && g_Settings.enableFavoriteRowColor)
                    {
                        ImVec2 bgEnd = ImVec2(cur.x + iconSize, cur.y + iconSize);
                        ImGui::GetWindowDrawList()->AddRectFilled(cur, bgEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                    }

                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, iconSize, st.details.loaded ? st.details.rarity : "");

                    if (st.isFavorite)
                        ImGui::GetWindowDrawList()->AddText(ImVec2(cur.x + 2.f, cur.y + 2.f), IM_COL32(255, 215, 0, 255), "*");

                    // Tooltip
                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::CurrencyTooltipOptions opt;
                        opt.showCount = true;
                        opt.count = st.count;
                        opt.showProfit = true;
                        opt.profit = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                        opt.showRarity = false;
                        opt.showId = true;
                        if (st.details.loaded)
                            UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                        else
                            UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                    }

                    // Right-click: nur pending setzen
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        pendingId = id;
                        pendingName = st.details.loaded ? st.details.name : "";
                        pendingIsCurrency = isCurrency;
                    }

                    DrawGridCount(st.count, iconSize, isCurrency, id);
                    ImGui::EndGroup();

                    // Abstand nach Coin rechts
                    if (isCoin)
                    {
                        ImGui::SameLine(0, 0.0f);
                        ImGui::Dummy(ImVec2(55.0f, 0.0f)); // +20px mehr + 15px extra
                    }

                    ImGui::EndGroup();
                }
                else
                {
                    // Use the same child approach as regular items for favorite items
                    if (ImGui::BeginChild("##FavItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                    {
                        UICommon::DrawItemIconCell(id, st.details.iconUrl, iconSize, st.details.loaded ? st.details.rarity : "");

                        if (st.isFavorite)
                        {
                            ImVec2 cur = ImGui::GetCursorScreenPos();
                            ImGui::GetWindowDrawList()->AddText(ImVec2(cur.x + 2.f, cur.y + 2.f), IM_COL32(255, 215, 0, 255), "*");
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

                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                        {
                            pendingId = id;
                            pendingName = st.details.loaded ? st.details.name : "";
                            pendingIsCurrency = isCurrency;
                        }

                        DrawGridItemCount(st.count, iconSize);
                    }
                    ImGui::EndChild();
                }

                ImGui::PopID();

                count++;
            }

            // Render context menu for this subsection
            if (pendingId != -1)
            {
                UIContextMenu::OpenContextMenu(gridContextMenuId, pendingId, pendingName);
                pendingId = -1;
            }
            if (isCurrency)
                UIContextMenu::RenderCurrencyContextMenu(gridContextMenuId, UIContextMenu::ContextMenuType::Favorites);
            else
                UIContextMenu::RenderItemContextMenu(gridContextMenuId, UIContextMenu::ContextMenuType::Favorites);
        }
        else
        {
            // Table view for this subsection
            std::string tableId = isCurrency ? "##OverviewFavCurrenciesTable" : "##OverviewFavItemsTable";
            auto setupTable = [&](const char* id, int cols) -> bool {
                float icw = std::max(32.0f + 10.f, 70.f);
                if (!ImGui::BeginTable(id, cols, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable|ImGuiTableFlags_NoSavedSettings)) return false;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, icw);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 430.f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 100.f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                return true;
            };
            if (setupTable(tableId.c_str(), 4))
            {

                for (int id : ids)
                {
                    Stat st;
                    if (isCurrency)
                        st = ItemTracker::GetCurrencyStat(id);
                    else
                        st = ItemTracker::GetItemStat(id);

                    if (st.isIgnored) continue;
                // Only show if count > 0 for both items and currencies
                if (st.count == 0) continue;

                    float rowH = UICommon::CalcTableRowHeight(32.0f);
                    ImGui::TableNextRow(0, rowH);

                    ImGui::TableSetColumnIndex(0);
                    UICommon::AlignTableCellIcon(rowH, 32.0f);
                    std::string iconUrl = st.details.iconUrl;
                    if (isCurrency && id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, 32.0f, st.details.loaded ? st.details.rarity : "");

                    if (ImGui::IsItemHovered())
                    {
                        if (isCurrency)
                        {
                            UITooltips::CurrencyTooltipOptions opt;
                            opt.showCount = true;
                            opt.count = st.count;
                            opt.showProfit = true;
                            opt.profit = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                            opt.showRarity = false;
                            opt.showId = true;
                            if (st.details.loaded)
                                UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                            else
                                UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                        }
                        else
                        {
                            UITooltips::ItemTooltipOptions opt;
                            opt.showCount = true; opt.count = st.count;
                            opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                            opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                            else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        pendingId = id;
                        pendingName = st.details.loaded ? st.details.name : "";
                        pendingIsCurrency = isCurrency;
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : Localization::GetText("loading"));
                    if (ImGui::IsItemHovered())
                    {
                        if (isCurrency)
                        {
                            UITooltips::CurrencyTooltipOptions opt;
                            opt.showCount = true;
                            opt.count = st.count;
                            opt.showProfit = true;
                            opt.profit = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                            opt.showRarity = false;
                            opt.showId = true;
                            if (st.details.loaded)
                                UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                            else
                                UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                        }
                        else
                        {
                            UITooltips::ItemTooltipOptions opt;
                            opt.showCount = true; opt.count = st.count;
                            opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                            opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                            else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        pendingId = id;
                        pendingName = st.details.loaded ? st.details.name : "";
                        pendingIsCurrency = isCurrency;
                    }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lld", st.count);
                    if (ImGui::IsItemHovered())
                    {
                        if (isCurrency)
                        {
                            UITooltips::CurrencyTooltipOptions opt;
                            opt.showCount = true;
                            opt.count = st.count;
                            opt.showProfit = true;
                            opt.profit = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                            opt.showRarity = false;
                            opt.showId = true;
                            if (st.details.loaded)
                                UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                            else
                                UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                        }
                        else
                        {
                            UITooltips::ItemTooltipOptions opt;
                            opt.showCount = true; opt.count = st.count;
                            opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                            opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                            else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        pendingId = id;
                        pendingName = st.details.loaded ? st.details.name : "";
                        pendingIsCurrency = isCurrency;
                    }

                    ImGui::TableSetColumnIndex(3);
                    if (isCurrency)
                    {
                        if (id == 1)
                        {
                            ImGui::Text("%s", UICommon::FormatCoin(st.count).c_str());
                        }
                        else
                        {
                            long long customProfit = CustomProfitManager::GetCustomProfit(id);
                            if (customProfit != 0)
                            {
                                long long totalProfit = customProfit * st.count;
                                ImVec4 pc = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f)
                                          : totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f)
                                          : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                                ImGui::TextColored(pc, "%s", UICommon::FormatCoin(totalProfit).c_str());
                            }
                            else
                                ImGui::Text("-");
                        }
                    }
                    else
                    {
                        long long profit = ItemTracker::GetStatProfit(st);
                        ImVec4 pc = profit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f)
                                  : profit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f)
                                  : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                        ImGui::TextColored(pc, "%s", UICommon::FormatCoin(profit).c_str());
                    }
                    if (ImGui::IsItemHovered())
                    {
                        if (isCurrency)
                        {
                            UITooltips::CurrencyTooltipOptions opt;
                            opt.showCount = true;
                            opt.count = st.count;
                            opt.showProfit = true;
                            opt.profit = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                            opt.showRarity = false;
                            opt.showId = true;
                            if (st.details.loaded)
                                UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                            else
                                UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                        }
                        else
                        {
                            UITooltips::ItemTooltipOptions opt;
                            opt.showCount = true; opt.count = st.count;
                            opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                            opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                            else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        pendingId = id;
                        pendingName = st.details.loaded ? st.details.name : "";
                        pendingIsCurrency = isCurrency;
                    }
                }

                ImGui::EndTable();

                // Render context menu for this subsection
                if (pendingId != -1)
                {
                    UIContextMenu::OpenContextMenu(tableContextMenuId, pendingId, pendingName);
                    pendingId = -1;
                }
                if (isCurrency)
                    UIContextMenu::RenderCurrencyContextMenu(tableContextMenuId, UIContextMenu::ContextMenuType::Favorites);
                else
                    UIContextMenu::RenderItemContextMenu(tableContextMenuId, UIContextMenu::ContextMenuType::Favorites);
            }
        }
    }

    static void RenderFavoritesSection()
    {
        // Holen wir uns alle Favoriten-IDs, UNABHÄNGIG von der aktuellen Count!
        auto favoriteItemsMap = ItemTracker::GetFavoriteItems();
        auto favoriteCurrenciesMap = ItemTracker::GetFavoriteCurrencies();

        std::vector<int> favoriteItems;
        for (auto& [id, st] : favoriteItemsMap)
            favoriteItems.push_back(id);

        std::vector<int> favoriteCurrencies;
        for (auto& [id, st] : favoriteCurrenciesMap)
            favoriteCurrencies.push_back(id);

        // Pending variables for context menus
        static int s_CurGridPendingId = -1;
        static std::string s_CurGridPendingName;
        static bool s_CurGridPendingIsCurrency = false;

        static int s_ItemGridPendingId = -1;
        static std::string s_ItemGridPendingName;
        static bool s_ItemGridPendingIsCurrency = false;

        static int s_CurTablePendingId = -1;
        static std::string s_CurTablePendingName;
        static bool s_CurTablePendingIsCurrency = false;

        static int s_ItemTablePendingId = -1;
        static std::string s_ItemTablePendingName;
        static bool s_ItemTablePendingIsCurrency = false;

        // Render subsections in fixed order
        RenderFavoritesSubsection(favoriteItems, false, Localization::GetText("favorite_items"), "favorites",
                                  s_ItemGridPendingId, s_ItemGridPendingName, s_ItemGridPendingIsCurrency,
                                  "OvFavItemGridMenu", "OvFavItemTableMenu");

        ImGui::Spacing();
        ImGui::Spacing();

        RenderFavoritesSubsection(favoriteCurrencies, true, Localization::GetText("favorite_currencies"), "favorites",
                                  s_CurGridPendingId, s_CurGridPendingName, s_CurGridPendingIsCurrency,
                                  "OvFavCurGridMenu", "OvFavCurTableMenu");
    }

    static void RenderCurrenciesSection()
    {
        auto currencies = ItemTracker::GetCurrenciesCopy();

        if (currencies.empty())
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", Localization::GetText("no_currencies"));
            ImGui::Spacing();
            return;
        }

        // Split currencies into two groups: favorites and non-favorites
        // Coin (id 1) wird separat behandelt und immer an den Anfang der zuerst
        // gerenderten Gruppe gesetzt, damit es unabhängig vom Favoriten-Status
        // immer als erstes Element angezeigt wird.
        std::vector<std::pair<int, Stat>> favoriteCurrencies;
        std::vector<std::pair<int, Stat>> otherCurrencies;
        bool hasCoin = false;
        std::pair<int, Stat> coin;
        for (auto& [id, st] : currencies)
        {
            if (st.isIgnored || st.count == 0) continue;
            if (id == 1)
            {
                hasCoin = true;
                coin = {id, st};
                continue;
            }
            if (st.isFavorite)
                favoriteCurrencies.emplace_back(id, st);
            else
                otherCurrencies.emplace_back(id, st);
        }
        if (hasCoin)
        {
            if (!favoriteCurrencies.empty() || otherCurrencies.empty())
                favoriteCurrencies.insert(favoriteCurrencies.begin(), coin);
            else
                otherCurrencies.insert(otherCurrencies.begin(), coin);
        }

        if (g_Settings.overviewEnableGridView)
        {
            // Grid view for Currencies
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f); // Same as favorite currencies
            float iconSize = static_cast<float>(g_Settings.gridIconSizeCurrencies);
            float cellSize = iconSize + 20.0f; // Same padding as in Currencies tab
            float gridSpacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;

            auto getColumns = [&](float width, float extraSpacing = 0.0f) {
                return std::max(1, static_cast<int>((width - scrollbarWidth + gridSpacing) / (cellSize + gridSpacing + extraSpacing)));
            };

            int columns = getColumns(ImGui::GetContentRegionAvail().x, -1.0f);
            int count = 0;

            // FIX Bug 1 (Currencies-Teil): Pending-System
            static int s_CurGridPendingId = -1;
            static std::string s_CurGridPendingName;

            // Helper function to render a list of currencies in grid view
            auto renderCurrencyGridGroup = [&](const std::vector<std::pair<int, Stat>>& group) {
                for (auto& [id, st] : group)
                {
                    bool isCoin = id == 1;

                    if (count > 0)
                    {
                        if (count % columns == 0)
                        {
                            ImGui::NewLine();
                        }
                        else
                        {
                            if (!isCoin)
                                ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x + 15.0f);
                            else
                                ImGui::SameLine();
                        }
                    }
                    else
                    {
                        // Erste Zeile (Coin ist hier immer das erste Element)
                        if (isCoin)
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f); // 20px
                    }

                    ImGui::PushID(id);
                    ImGui::BeginGroup(); // Gesamte Zelle (inkl. Abstände) in eine Gruppe

                    // Eigentliche Icon-Gruppe
                    ImGui::BeginGroup();
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, iconSize, st.details.loaded ? st.details.rarity : "");

                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::CurrencyTooltipOptions opt;
                        opt.showCount  = true;
                        opt.count      = st.count;
                        opt.showProfit = true;
                        opt.profit     = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                        opt.showRarity = false;
                        opt.showId     = true;
                        if (st.details.loaded)
                            UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                        else
                            UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                    }

                    // Right-click: nur pending setzen
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        s_CurGridPendingId   = id;
                        s_CurGridPendingName = st.details.loaded ? st.details.name : "";
                    }

                    DrawGridCount(st.count, iconSize, true, id);
                    ImGui::EndGroup();

                    // Abstand nach Coin rechts
                    if (isCoin)
                    {
                        ImGui::SameLine(0, 0.0f);
                        ImGui::Dummy(ImVec2(55.0f, 0.0f)); // +20px mehr + 15px extra
                    }

                    ImGui::EndGroup();
                    ImGui::PopID();

                    count++;
                }
            };

            // Render groups in correct order
            if (g_Settings.overviewCurrenciesFirst)
            {
                if (!favoriteCurrencies.empty())
                {
                    renderCurrencyGridGroup(favoriteCurrencies);
                }
                if (!otherCurrencies.empty())
                {
                    // Nur eine sichtbare Lücke einfügen, wenn die Favoriten-Zeile(n) exakt voll waren -
                    // sonst würde ein Umbruch erzwungen, obwohl in der aktuellen Zeile noch Platz ist.
                    if (!favoriteCurrencies.empty() && count % columns == 0)
                        ImGui::Spacing();
                    renderCurrencyGridGroup(otherCurrencies);
                }
            }
            else
            {
                // Original order, just render all together
                std::vector<std::pair<int, Stat>> allCurrencies;
                allCurrencies.reserve(favoriteCurrencies.size() + otherCurrencies.size());
                allCurrencies.insert(allCurrencies.end(), favoriteCurrencies.begin(), favoriteCurrencies.end());
                allCurrencies.insert(allCurrencies.end(), otherCurrencies.begin(), otherCurrencies.end());
                renderCurrencyGridGroup(allCurrencies);
            }


            // FIX Bug 1: Context-Menu außerhalb der Schleife
            if (s_CurGridPendingId != -1)
            {
                UIContextMenu::OpenContextMenu("OvCurGridMenu", s_CurGridPendingId, s_CurGridPendingName);
                s_CurGridPendingId = -1;
            }
            UIContextMenu::RenderCurrencyContextMenu("OvCurGridMenu", UIContextMenu::ContextMenuType::General);
        }
        else
        {
            // Table view for currencies
            static int s_CurTblPendingId = -1;
            static std::string s_CurTblPendingName;

            if (ImGui::BeginTable("##OverviewCurrenciesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 70.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 300.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : currencies)
                {
                    if (st.isIgnored || st.count == 0) continue;

                    float rowH = UICommon::CalcTableRowHeight(32.0f);
                    ImGui::TableNextRow(0, rowH);

                    ImGui::TableSetColumnIndex(0);
                    UICommon::AlignTableCellIcon(rowH, 32.0f);
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, 32.0f, st.details.loaded ? st.details.rarity : "");

                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::CurrencyTooltipOptions opt;
                        opt.showCount = true;
                        opt.count = st.count;
                        opt.showProfit = true;
                        opt.profit = (id == 1) ? st.count : CustomProfitManager::GetCustomProfit(id) * st.count;
                        opt.showRarity = false;
                        opt.showId = true;
                        if (st.details.loaded)
                            UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                        else
                            UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        s_CurTblPendingId = id;
                        s_CurTblPendingName = st.details.loaded ? st.details.name : "";
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : Localization::GetText("loading"));

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
                        {
                            long long totalProfit = customProfit * st.count;
                            ImVec4 pc = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f)
                                      : totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f)
                                      : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                            ImGui::TextColored(pc, "%s", UICommon::FormatCoin(totalProfit).c_str());
                        }
                        else
                            ImGui::Text("-");
                    }
                }

                ImGui::EndTable();

                if (s_CurTblPendingId != -1)
                {
                    UIContextMenu::OpenContextMenu("OvCurTblMenu", s_CurTblPendingId, s_CurTblPendingName);
                    s_CurTblPendingId = -1;
                }
                UIContextMenu::RenderCurrencyContextMenu("OvCurTblMenu", UIContextMenu::ContextMenuType::General);
            }
        }
    }

    static void RenderItemsSection()
    {
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
        if (ImGui::Combo("##OvSortItems", &g_Settings.overviewItemSortMode, sortLabels, 10))
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
        int rarityCombo = std::clamp(g_Settings.overviewItemRarityFilterMin, 0, 7);
        if (ImGui::Combo("##OvRarityF", &rarityCombo, rarityLabels, 8))
        {
            g_Settings.overviewItemRarityFilterMin = rarityCombo;
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("rarity_tooltip"));

        ImGui::Spacing();

        if (ImGui::Button(Localization::GetText("mass_actions_label"), ImVec2(130, 0)))
            ImGui::OpenPopup("OvMassActionsPopup");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("mass_actions_tooltip"));

        if (ImGui::BeginPopup("OvMassActionsPopup"))
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

        auto sortedItems = ItemTracker::GetSortedItems(static_cast<ItemTracker::SortMode>(g_Settings.overviewItemSortMode));
        // Filter out ignored items and apply rarity filter
        std::vector<std::pair<int, Stat>> filteredSortedItems;
        for (auto& [id, st] : sortedItems)
        {
            if (!st.isIgnored && st.count != 0 && !st.IsCurrency())
            {
                if (g_Settings.overviewItemRarityFilterMin > 0)
                {
                    // Check rarity
                    std::string rarity = st.details.loaded ? st.details.rarity : "";
                    int rarityRank = 0;
                    if (rarity == "Junk") rarityRank = 1;
                    else if (rarity == "Basic") rarityRank = 2;
                    else if (rarity == "Fine") rarityRank = 3;
                    else if (rarity == "Masterwork") rarityRank = 4;
                    else if (rarity == "Rare") rarityRank = 5;
                    else if (rarity == "Exotic") rarityRank = 6;
                    else if (rarity == "Ascended") rarityRank = 7;
                    else if (rarity == "Legendary") rarityRank = 8;
                    if (rarityRank < g_Settings.overviewItemRarityFilterMin) continue;
                }
                filteredSortedItems.push_back({id, st});
            }
        }
        sortedItems.swap(filteredSortedItems);

        if (sortedItems.empty())
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", "No items");
            ImGui::Spacing();
            return;
        }

        if (g_Settings.overviewEnableGridView)
        {
            float cellSize = static_cast<float>(g_Settings.gridIconSize) + 10.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;

            auto getColumns = [&](float width) {
                return std::max(1, static_cast<int>((width - scrollbarWidth + spacing) / (cellSize + spacing)));
            };

            static int s_ItemGridPendingId = -1;
            static std::string s_ItemGridPendingName;

            if (ImGui::BeginChild("##OverviewItemsGrid", ImVec2(0, 0), true))
            {
                auto renderCell = [&](int id, const Stat& st) {
                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                    {
                        UICommon::DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.gridIconSize), st.details.loaded ? st.details.rarity : "");

                        if (ImGui::IsItemHovered())
                        {
                            UITooltips::ItemTooltipOptions opt;
                            opt.showCount = true; opt.count = st.count;
                            opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                            opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                            else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                        }

                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                        {
                            s_ItemGridPendingId   = id;
                            s_ItemGridPendingName = st.details.loaded ? st.details.name : "";
                        }

                        DrawGridItemCount(st.count, static_cast<float>(g_Settings.gridIconSize));
                    }
                    ImGui::EndChild();
                };

                if (g_Settings.overviewGroupByRarity)
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

                    static const auto s_rarityHeaderColorsGrid = GetRarityHeaderColors();

                    if (g_Settings.overviewShowRarityAsTabs)
                    {
                        if (ImGui::BeginTabBar("##OvRarityTabs"))
                        {
                            for (const auto& rn : rarityOrder)
                            {
                                auto it = rarityGroups.find(rn);
                                if (it == rarityGroups.end() || it->second.empty()) continue;
                                char tl[256]; snprintf(tl, sizeof(tl), "%s (%zu)", rn.c_str(), it->second.size());
                                ImVec4 tabColor = s_rarityHeaderColorsGrid.count(rn) ? s_rarityHeaderColorsGrid.at(rn) : ImVec4(1.f,1.f,1.f,1.f);
                                ImGui::PushStyleColor(ImGuiCol_Text, tabColor);
                                bool tabOpen = ImGui::BeginTabItem(tl);
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
                            char hl[256]; snprintf(hl, sizeof(hl), "%s (%zu)", rn.c_str(), it->second.size());
                            ImGui::Spacing();
                            ImVec4 headerColor = s_rarityHeaderColorsGrid.count(rn) ? s_rarityHeaderColorsGrid.at(rn) : ImVec4(1.f,1.f,1.f,1.f);
                            if (CollapsingHeaderWithIcon(hl, "items", headerColor, ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                int cols = getColumns(ImGui::GetContentRegionAvail().x), col = 0;
                                for (auto& [id, st] : it->second) { if (col > 0) ImGui::SameLine(); ImGui::PushID(id); renderCell(id, st); ImGui::PopID(); col++; if (col >= cols) col = 0; }
                            }
                        }
                    }
                }
                else if (g_Settings.overviewGroupByCategory)
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

                    if (g_Settings.overviewShowGroupAsTabs)
                    {
                        if (ImGui::BeginTabBar("##OvTypeTabs"))
                        {
                            for (auto type : typeOrder)
                            {
                                auto it = typeGroups.find(type);
                                if (it == typeGroups.end() || it->second.empty()) continue;
                                char tl[256]; snprintf(tl, sizeof(tl), "%s (%zu)", getTypeName(type).c_str(), it->second.size());
                                if (ImGui::BeginTabItem(tl))
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
                            char hl[256]; snprintf(hl, sizeof(hl), "%s (%zu)", getTypeName(type).c_str(), it->second.size());
                            ImGui::Spacing();
                            const char* iconKey = "items";
                            if (type == ItemType::Weapon) iconKey = "sword";
                            if (CollapsingHeaderWithIcon(hl, iconKey, kCategoryHeaderColor, ImGuiTreeNodeFlags_DefaultOpen))
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
                        renderCell(id, st);
                        ImGui::PopID();
                        col++; if (col >= columns) col = 0;
                    }
                }
            }
            ImGui::EndChild();

            if (s_ItemGridPendingId != -1)
            {
                UIContextMenu::OpenContextMenu("OvItemGridMenu", s_ItemGridPendingId, s_ItemGridPendingName);
                s_ItemGridPendingId = -1;
            }
            UIContextMenu::RenderItemContextMenu("OvItemGridMenu", UIContextMenu::ContextMenuType::General);
        }
        else
        {
            // Table view for items
            static int s_ItemTblPendingId = -1;
            static std::string s_ItemTblPendingName;

            if (ImGui::BeginTable("##OverviewItemsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 70.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 300.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_magic_find"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedItems)
                {
                    float rowH = UICommon::CalcTableRowHeight(32.0f);
                    ImGui::TableNextRow(0, rowH);

                    ImGui::TableSetColumnIndex(0);
                    UICommon::AlignTableCellIcon(rowH, 32.0f);
                    UICommon::DrawItemIconCell(id, st.details.iconUrl, 32.0f, st.details.loaded ? st.details.rarity : "");

                    if (ImGui::IsItemHovered())
                    {
                        UITooltips::ItemTooltipOptions opt;
                        opt.showCount = true; opt.count = st.count;
                        opt.showProfit = true; opt.profit = ItemTracker::GetStatProfit(st);
                        opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
                        if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                        else UITooltips::RenderItemTooltipFallback(Localization::GetText("loading"), "", id, opt);
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    {
                        s_ItemTblPendingId   = id;
                        s_ItemTblPendingName = st.details.loaded ? st.details.name : "";
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : Localization::GetText("loading"));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    long long profit = ItemTracker::GetStatProfit(st);
                    ImVec4 pc = profit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f)
                              : profit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f)
                              : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                    ImGui::TextColored(pc, "%s", UICommon::FormatCoin(profit).c_str());

                    ImGui::TableSetColumnIndex(4);
                    if (st.lastMagicFind >= 0)
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d%%", st.lastMagicFind);
                    else
                        ImGui::TextDisabled("N/A");
                }

                ImGui::EndTable();

                if (s_ItemTblPendingId != -1)
                {
                    UIContextMenu::OpenContextMenu("OvItemTblMenu", s_ItemTblPendingId, s_ItemTblPendingName);
                    s_ItemTblPendingId = -1;
                }
                UIContextMenu::RenderItemContextMenu("OvItemTblMenu", UIContextMenu::ContextMenuType::General);
            }
        }
    }

    void RenderOverviewTab()
{
    RenderFavoritesSection();

    ImGui::Spacing();
    ImGui::Spacing();

    CardHeader("currencies", Localization::GetText("tab_currencies"));
    RenderCurrenciesSection();

    ImGui::Spacing();
    ImGui::Spacing();

    CardHeader("items", Localization::GetText("tab_items"));
    RenderItemsSection();
}
}
