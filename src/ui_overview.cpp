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

    static bool CollapsingHeaderWithIcon(const char* label, const char* iconKey, ImGuiTreeNodeFlags flags = 0)
    {
        InlineIcon(iconKey);
        return ImGui::CollapsingHeader(label, flags);
    }

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
            float iconSize = isCurrency ? static_cast<float>(g_Settings.overviewFavoritesIconSize) : static_cast<float>(g_Settings.gridIconSize);
            float cellSize = isCurrency ? (iconSize + 65.0f) : (iconSize + 10.0f);
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
                
                if (st.isIgnored || st.count == 0) continue;

                bool isCoin = isCurrency && id == 1;

                if (count > 0)
                {
                    if (count % columns == 0)
                    {
                        ImGui::NewLine();
                    }
                    else
                    {
                        ImGui::SameLine();
                    }
                }
                else
                {
                    // Erste Zeile
                    if (isCoin)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f); // +20px mehr
                }

                ImGui::PushID(id);
                
                if (isCurrency)
                {
                    // Keep the old group approach for currencies
                    ImGui::BeginGroup(); // Gesamte Zelle (inkl. Abstände) in eine Gruppe

                    // Abstand vor Coin links (nur wenn es NICHT das erste Element ist)
                    if (isCoin && count > 0)
                    {
                        ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                        ImGui::SameLine(0, 0.0f);
                    }

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
                        ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                    }

                    ImGui::EndGroup();
                }
                else
                {
                    // Use the same child approach as regular items for favorite items
                    if (ImGui::BeginChild("##FavItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
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
            if (ImGui::BeginTable(tableId.c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 70.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 300.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 150.0f);
                ImGui::TableHeadersRow();

                for (int id : ids)
                {
                    Stat st;
                    if (isCurrency)
                        st = ItemTracker::GetCurrencyStat(id);
                    else
                        st = ItemTracker::GetItemStat(id);

                    if (st.isIgnored || st.count == 0) continue;

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

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lld", st.count);

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

        // Render subsections in order (Currencies first, then Items)
        RenderFavoritesSubsection(favoriteCurrencies, true, Localization::GetText("favorite_currencies"), "favorites",
                                  s_CurGridPendingId, s_CurGridPendingName, s_CurGridPendingIsCurrency,
                                  "OvFavCurGridMenu", "OvFavCurTableMenu");

        ImGui::Spacing();
        ImGui::Spacing();

        RenderFavoritesSubsection(favoriteItems, false, Localization::GetText("favorite_items"), "favorites",
                                  s_ItemGridPendingId, s_ItemGridPendingName, s_ItemGridPendingIsCurrency,
                                  "OvFavItemGridMenu", "OvFavItemTableMenu");
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

        if (g_Settings.overviewEnableGridView)
        {
            // Grid view for Currencies
            float iconSize = static_cast<float>(g_Settings.gridIconSizeCurrencies);
            float cellSize = iconSize + 20.0f; // Same padding as in Currencies tab
            float gridSpacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;

            auto getColumns = [&](float width) {
                return std::max(1, static_cast<int>((width - scrollbarWidth + gridSpacing) / (cellSize + gridSpacing)));
            };

            int columns = getColumns(ImGui::GetContentRegionAvail().x);
            int count = 0;

            // FIX Bug 1 (Currencies-Teil): Pending-System
            static int s_CurGridPendingId = -1;
            static std::string s_CurGridPendingName;

            for (auto& [id, st] : currencies)
            {
                if (st.isIgnored || st.count == 0) continue;

                bool isCoin = id == 1;

                if (count > 0)
                {
                    if (count % columns == 0)
                    {
                        ImGui::NewLine();
                    }
                    else
                    {
                        ImGui::SameLine();
                    }
                }
                else
                {
                    // Erste Zeile
                    if (isCoin)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f); // +20px mehr
                }

                ImGui::PushID(id);
                ImGui::BeginGroup(); // Gesamte Zelle (inkl. Abstände) in eine Gruppe

                // Abstand vor Coin links (nur wenn es NICHT das erste Element ist)
                if (isCoin && count > 0)
                {
                    ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                    ImGui::SameLine(0, 0.0f);
                }

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
                    ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                }

                ImGui::EndGroup();
                ImGui::PopID();

                count++;
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
        auto items = ItemTracker::GetItemsCopy();

        if (items.empty())
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

            if (ImGui::BeginChild("##OverviewItemsGrid", ImVec2(0, 300), true))
            {
                int columns = getColumns(ImGui::GetContentRegionAvail().x);
                int col = 0;

                for (auto& [id, st] : items)
                {
                    if (st.isIgnored || st.count == 0) continue;

                    if (col > 0) ImGui::SameLine();

                    ImGui::PushID(id);
                    if (ImGui::BeginChild("##ItemCell", ImVec2(cellSize, cellSize), true, ImGuiWindowFlags_NoScrollbar))
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
                    ImGui::PopID();

                    col++;
                    if (col >= columns) col = 0;
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

                for (auto& [id, st] : items)
                {
                    if (st.isIgnored || st.count == 0) continue;

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
