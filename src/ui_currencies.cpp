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

static bool CollapsingHeaderWithIcon(const char* label, const char* iconKey, ImVec4 headerColor, ImGuiTreeNodeFlags flags = 0)
{
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2      pos = ImGui::GetCursorScreenPos();
    float       w   = ImGui::GetContentRegionAvail().x;
    float       lineH  = ImGui::GetTextLineHeight();
    float       h      = lineH + 10.f;
    float       iconSz = 14.f;
    ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.08f,0.08f,0.08f,0.85f));

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

    ImGui::SetCursorScreenPos(pos);
    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1,1,1,0.05f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(1,1,1,0.10f));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0,0,0,0));
    bool open = ImGui::CollapsingHeader((std::string("##ch_")+label).c_str(), flags);
    ImGui::PopStyleColor(4);

    dl->AddRectFilled({pos.x + 4.f, pos.y}, {pos.x + w, pos.y + h}, bgCol, 3.f);
    dl->AddRect({pos.x + 4.f, pos.y}, {pos.x + w, pos.y + h},
                ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x*0.6f, headerColor.y*0.6f, headerColor.z*0.6f, 0.8f)), 3.f, 0, 0.5f);
    dl->AddRectFilled(pos, {pos.x + 4.f, pos.y + h},
                      ImGui::ColorConvertFloat4ToU32(headerColor), 0.f);
    if (tex)
        dl->AddImage((ImTextureID)tex, {iconX, iconY}, {iconX + iconSz, iconY + iconSz},
                     {0,0}, {1,1}, ImGui::ColorConvertFloat4ToU32(headerColor));
    dl->AddText({textX, textY}, ImGui::ColorConvertFloat4ToU32(headerColor), label);

    return open;
}

static void RenderCurrencyTooltip(int id, long long count, const Stat& st)
{
    UITooltips::CurrencyTooltipOptions opt;
    opt.showCount = true;
    opt.count = count;
    opt.showRarity = false;
    opt.showProfit = true; // Always enable, tooltip will decide what to show
    opt.profit = st.GetCustomProfit() * count;
    opt.showId = true;
    if (st.details.loaded)
        UITooltips::RenderCurrencyTooltip(st.details, id, opt);
    else
        UITooltips::RenderCurrencyTooltipFallback(id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"), "", id, opt);
}

static void DrawGridCurrencyCount(int id, long long count, float iconSz)
{
    std::string cs;
    if (id == 1) {
        cs = UICommon::FormatCoin(count);
    } else {
        cs = UICommon::FormatCompact(count);
    }
    const char* countStr = cs.c_str();

    // Get origin
    ImVec2 origin   = ImGui::GetItemRectMin();

    // Calculate base font size - 10% smaller!
    const float desiredPixelSize = id == 1 ? iconSz * 0.495f : iconSz * 0.54f;
    ImFont* font = ImGui::GetFont();

    // Calculate text size WITH the desired font size
    ImVec2 textSize = font->CalcTextSizeA(desiredPixelSize, FLT_MAX, 0.0f, countStr);
    
    const float rightPush = (id == 1) ? 35.0f : 0.0f;
    ImVec2 pos = ImVec2(origin.x + iconSz - textSize.x + rightPush,
                        origin.y + iconSz - textSize.y - 2.0f);
    ImVec4 col = count < 0 ? ImVec4(0.9f,0.3f,0.3f,1.f)
                           : ImVec4(0.95f,0.7f,0.1f,1.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // 1. Very thin white outline (outer 5 pixels)
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
void RenderCurrenciesTab()
{
    int pendingContextId = 0;
    std::string pendingContextName;

    // Favorite Currencies Section (immer sichtbar, nicht mehr einklappbar)
    const bool favoritesExpanded = true;
    
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
    
    // Header text
    float textX = iconX + iconSize + 8.0f;
    float textY = cursor.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f;
    
    // Draw text directly for left alignment
    drawList->AddText(ImVec2(textX, textY), 
                     ImGui::ColorConvertFloat4ToU32(ImVec4(0.82f, 0.796f, 0.757f, 1.0f)),
                     "Favorite Currencies");
    
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + headerHeight + 8.0f));
    
    if (favoritesExpanded)
    {
        ImGui::Spacing();

        const auto& currencies = ItemTracker::GetFavoriteCurrenciesView();
        std::vector<std::pair<int, Stat>> favoriteCurrencies;
        for (auto& [id, st] : currencies)
        {
            if (!st.IsCurrency()) continue;
            if (st.count == 0) continue;
            favoriteCurrencies.push_back({id, st});
        }

        if (g_Settings.currenciesFavoritesAsGrid)
        {
            // Grid View for Favorite Currencies
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
            float cellSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) + 20.0f; // Same as regular currencies grid
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float scrollbarWidth = 20.0f;
            int columns = std::max(1, static_cast<int>((ImGui::GetContentRegionAvail().x - scrollbarWidth + spacing) / (cellSize + spacing)));

            static int s_FavCurGridPendingId = -1;
            static std::string s_FavCurGridPendingName;

            int count = 0;
            for (auto& [id, st] : favoriteCurrencies)
            {

                bool isCoin = id == 1;

                if (count > 0)
                {
                    if (count % columns == 0)
                        ImGui::NewLine();
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
                ImVec2 cur = ImGui::GetCursorScreenPos();

                if (st.isFavorite && g_Settings.enableFavoriteRowColor)
                {
                    ImVec2 bgEnd = ImVec2(cur.x + g_Settings.gridIconSizeCurrencies, cur.y + g_Settings.gridIconSizeCurrencies);
                    ImGui::GetWindowDrawList()->AddRectFilled(cur, bgEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
                }

                std::string iconUrl = st.details.iconUrl;
                if (id == 1 && iconUrl.empty())
                    iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");

                if (st.isFavorite)
                    ImGui::GetWindowDrawList()->AddText(ImVec2(cur.x + 2.f, cur.y + 2.f), IM_COL32(255, 215, 0, 255), "*");

                if (ImGui::IsItemHovered())
                    RenderCurrencyTooltip(id, st.count, st);

                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                {
                    s_FavCurGridPendingId   = id;
                    s_FavCurGridPendingName = st.details.loaded ? st.details.name : "";
                }

                DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
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

            if (s_FavCurGridPendingId != -1)
            {
                UIContextMenu::OpenContextMenu("FavCurGridMenu", s_FavCurGridPendingId, s_FavCurGridPendingName);
                s_FavCurGridPendingId = -1;
            }
            UIContextMenu::RenderCurrencyContextMenu("FavCurGridMenu", UIContextMenu::ContextMenuType::Favorites);
        }
        else
        {
            // List View for Favorite Currencies
            auto setupTable = [&](const char* id, int cols) -> bool {
                float icw = std::max(32.0f + 10.f, 70.f);
                if (!ImGui::BeginTable(id, cols, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable|ImGuiTableFlags_NoSavedSettings)) return false;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, icw);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 430.f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 100.f);
                ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide, 100.f);
                ImGui::TableSetupColumn(Localization::GetText("value"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                return true;
            };
            if (setupTable("##FavoriteCurrenciesTable", 5))
            {

                for (auto& [id, st] : favoriteCurrencies)
                {

                    long long profit = ItemTracker::GetStatProfit(st);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    UICommon::AlignTableCellIcon(UICommon::CalcTableRowHeight(32.0f), 32.0f);
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty())
                        iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, 32.f, st.details.loaded ? st.details.rarity : "");
                    if (ImGui::IsItemHovered())
                {
                    UITooltips::CurrencyTooltipOptions opt;
                    opt.showCount = true;
                    opt.count = st.count;
                    opt.showProfit = true; // Always enable, tooltip will decide what to show
                    opt.profit = st.GetCustomProfit() * st.count;
                    opt.showId = true;
                    if (st.details.loaded)
                        UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                    else
                        UITooltips::RenderCurrencyTooltipFallback(Localization::GetText("loading"), "", id, opt);
                }

                    ImGui::TableSetColumnIndex(1);
                    UICommon::AlignTableCellText(UICommon::CalcTableRowHeight(32.0f));
                    ImGui::Text("%s", st.details.loaded ? st.details.name.c_str() : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading")));

                    ImGui::TableSetColumnIndex(2);
                    UICommon::AlignTableCellText(UICommon::CalcTableRowHeight(32.0f));
                    ImGui::Text("%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    UICommon::AlignTableCellText(UICommon::CalcTableRowHeight(32.0f));
                    if (id == 1)
                    {
                        ImGui::Text("%s", UICommon::FormatCoin(st.count));
                    }
                    else
                    {
                        long long customProfit = CustomProfitManager::GetCustomProfit(id);
                        if (customProfit != 0)
                            ImGui::Text("%s", UICommon::FormatCoin(customProfit * st.count));
                        else
                            ImGui::Text("-");
                    }

                    ImGui::TableSetColumnIndex(4);
                    UICommon::AlignTableCellText(UICommon::CalcTableRowHeight(32.0f));
                    if (id == 1) 
                        ImGui::Text("%s", UICommon::FormatCoin(st.count));
                    else if (profit != 0)
                        ImGui::Text("%s", UICommon::FormatCoin(profit));
                    else
                        ImGui::Text("%s", UICommon::FormatCompact(st.count));
                }
                ImGui::EndTable();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Search bar
    if (ImGui::InputTextWithHint("##SearchCurrencies", Localization::GetText("search_currencies_hint"), UICommon::s_CurrenciesSearchBuf, sizeof(UICommon::s_CurrenciesSearchBuf)))
    {
        BackgroundJobs::EnqueueDebouncedSettingsSave();
    }
    ImGui::SameLine();
    char clearSearchCurrencies[256];
    snprintf(clearSearchCurrencies, sizeof(clearSearchCurrencies), "%s##ClearSearchCurrencies", Localization::GetText("clear_search"));
    if (ImGui::Button(clearSearchCurrencies))
    {
        memset(UICommon::s_CurrenciesSearchBuf, 0, sizeof(UICommon::s_CurrenciesSearchBuf));
        BackgroundJobs::EnqueueDebouncedSettingsSave();
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

    // Use cached sorted view with filters
    ItemTracker::SortFilterOptions filter;
    filter.excludeIgnored = true;
    filter.excludeZeroCount = true;
    filter.excludeCurrencies = false; // We want currencies
    filter.searchTerm = UICommon::s_CurrenciesSearchBuf;

    const auto& sortedCurrencies = ItemTracker::GetSortedCurrenciesView(
        static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode),
        filter
    );
    
    // Separate coin (id == 1) from other currencies
    std::vector<std::pair<int, Stat>> filteredSortedCurrencies;
    std::optional<std::pair<int, Stat>> coin;
    for (auto& [id, st] : sortedCurrencies)
    {
        if (id == 1)
        {
            coin = {id, st};
        }
        else
        {
            filteredSortedCurrencies.push_back({id, st});
        }
    }
    if (coin.has_value())
    {
        filteredSortedCurrencies.insert(filteredSortedCurrencies.begin(), coin.value());
    }

    if (g_Settings.currenciesEnableGridView)
    {
        // Grid View for Currencies
        float cellSize = static_cast<float>(g_Settings.gridIconSizeCurrencies) + 20.0f; // Same padding as in Overview tab
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float scrollbarWidth = 20.0f; // Safer buffer for scrollbar
        
        auto getColumns = [&](float width, float extraSpacing = 0.0f) {
            return std::max(1, static_cast<int>((width - scrollbarWidth + spacing) / (cellSize + spacing + extraSpacing)));
        };

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::BeginChild("##CurrenciesGrid", ImVec2(0, 0), false))
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
                if (g_Settings.currenciesGroupByCategory)
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

                if (g_Settings.currenciesShowGroupAsTabs)
                {
                    if (ImGui::BeginTabBar("##CurrencyCategoryTabsGrid"))
                    {
                        for (const auto& cat : categories)
                        {
                            // Check if category has any currencies with count > 0
                            bool hasItems = false;
                            for (auto& [id, st] : filteredSortedCurrencies)
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
                                int columns = getColumns(ImGui::GetContentRegionAvail().x, 20.0f);
                                int col = 0;
                                for (auto& [id, st] : filteredSortedCurrencies)
                                {
                                    if (ItemTracker::GetCurrencyCategory(id) != cat) continue;
                                    if (st.count == 0) continue;

                                    bool isCoin = id == 1;

                                    if (col > 0)
                                    {
                                        if (col % columns == 0)
                                            ImGui::NewLine();
                                        else
                                        {
                                            if (!isCoin)
                            ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x + 35.0f);
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
                                    bool cellHovered = false;
                                    std::string iconUrl = st.details.iconUrl;
                                    if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                                    UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");
                                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { pendingContextId = id; pendingContextName = st.details.loaded ? st.details.name : ""; }
                                    cellHovered = ImGui::IsItemHovered();
                                    DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
                                    ImGui::EndGroup();

                                    // Abstand nach Coin rechts
                                    if (isCoin)
                                    {
                                        ImGui::SameLine(0, 0.0f);
                                        ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                                    }

                                    ImGui::EndGroup();
                                    if (cellHovered)
                                        RenderCurrencyTooltip(id, st.count, st);
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
                        for (const auto& [id, st] : filteredSortedCurrencies) { if (ItemTracker::GetCurrencyCategory(id) == cat && st.count > 0) { hasItems = true; break; } }
                        if (!hasItems) continue;

                        if (CollapsingHeaderWithIcon(cat.c_str(), "currencies", ImVec4(0.40f,0.75f,0.90f,1.f), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            int columns = getColumns(ImGui::GetContentRegionAvail().x, 10.0f);
                            int col = 0;
                            for (auto& [id, st] : filteredSortedCurrencies)
                            {
                                if (ItemTracker::GetCurrencyCategory(id) != cat) continue;
                                if (st.count == 0) continue;

                                bool isCoin = id == 1;

                                if (col > 0)
                                {
                                    if (col % columns == 0)
                                        ImGui::NewLine();
                                    else
                                    {
                                        if (!isCoin)
                                            ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x + 25.0f);
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
                                bool cellHovered = false;
                                std::string iconUrl = st.details.iconUrl;
                                if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                                UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { pendingContextId = id; pendingContextName = st.details.loaded ? st.details.name : ""; }
                                cellHovered = ImGui::IsItemHovered();
                                DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
                                ImGui::EndGroup();

                                // Abstand nach Coin rechts
                                if (isCoin)
                                {
                                    ImGui::SameLine(0, 0.0f);
                                    ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                                }

                                ImGui::EndGroup();
                                if (cellHovered)
                                    RenderCurrencyTooltip(id, st.count, st);
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
                int columns = getColumns(ImGui::GetContentRegionAvail().x, 10.0f);
                int col = 0;
                for (auto& [id, st] : filteredSortedCurrencies)
                {
                    if (st.count == 0) continue;

                    bool isCoin = id == 1;

                    if (col > 0)
                    {
                        if (col % columns == 0)
                            ImGui::NewLine();
                        else
                        {
                            if (!isCoin)
                                ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x + 25.0f);
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
                    bool cellHovered = false;
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty())
                        iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
                    UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.gridIconSizeCurrencies), st.details.loaded ? st.details.rarity : "");
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { pendingContextId = id; pendingContextName = st.details.loaded ? st.details.name : ""; }
                    cellHovered = ImGui::IsItemHovered();
                    DrawGridCurrencyCount(id, st.count, static_cast<float>(g_Settings.gridIconSizeCurrencies));
                    ImGui::EndGroup();

                    // Abstand nach Coin rechts
                    if (isCoin)
                    {
                        ImGui::SameLine(0, 0.0f);
                        ImGui::Dummy(ImVec2(40.0f, 0.0f)); // +20px mehr
                    }

                    ImGui::EndGroup();
                    if (cellHovered)
                        RenderCurrencyTooltip(id, st.count, st);
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
        ImGui::PopStyleVar();
    }
    else
    {
        // Table View for Currencies
        auto renderCurrencyRow = [&](int id, const Stat& st) {
            float rowH = UICommon::CalcTableRowHeight(static_cast<float>(g_Settings.itemsIconSize));
            ImGui::TableNextRow(0, rowH);
            if (st.isFavorite && g_Settings.enableFavoriteRowColor) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1], g_Settings.favoriteRowColor[2], g_Settings.favoriteRowColor[3])));
            ImGui::TableSetColumnIndex(0);
            UICommon::AlignTableCellIcon(rowH, static_cast<float>(g_Settings.itemsIconSize));
            std::string iconUrl = st.details.iconUrl;
            if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
            UICommon::DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.itemsIconSize), st.details.loaded ? st.details.rarity : "");
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
            {
                pendingContextId = id;
                pendingContextName = st.details.loaded ? st.details.name : "";
            }
            if (ImGui::IsItemHovered())
                RenderCurrencyTooltip(id, st.count, st);
            ImGui::TableSetColumnIndex(1);
            UICommon::AlignTableCellText(rowH);
            std::string name = st.details.loaded ? st.details.name : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"));
            if (st.isFavorite) { ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "*"); ImGui::SameLine(); }
            ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (st.isFavorite) {
                col = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                if (g_Settings.enableFavoriteTextColor)
                    col = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1], g_Settings.favoriteTextColor[2], g_Settings.favoriteTextColor[3]);
            }
            ImGui::TextColored(col, "%s", name.c_str());
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) { pendingContextId = id; pendingContextName = st.details.loaded ? st.details.name : ""; }
            if (ImGui::IsItemHovered())
                RenderCurrencyTooltip(id, st.count, st);
            ImGui::TableSetColumnIndex(2);
            UICommon::AlignTableCellText(rowH);
            ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            std::string countStr = std::to_string(st.count);
            ImGui::TextColored(countColor, "%s", countStr.c_str());
            if (ImGui::IsItemHovered())
            {
                if (id == 1)
                    ImGui::SetTooltip("%s", UICommon::FormatCoin(st.count));
                else
                    ImGui::SetTooltip("%lld", st.count);
            }

            ImGui::TableSetColumnIndex(3);
            UICommon::AlignTableCellText(rowH);
            if (id == 1)
            {
                ImGui::Text("%s", UICommon::FormatCoin(st.count));
            }
            else
            {
                long long customProfit = CustomProfitManager::GetCustomProfit(id);
                if (customProfit != 0)
                    ImGui::Text("%s", UICommon::FormatCoin(customProfit * st.count));
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
                                float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                                ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthStretch);
                                ImGui::TableHeadersRow();

                                for (auto& [id, st] : filteredSortedCurrencies)
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
                    for (const auto& [id, st] : filteredSortedCurrencies) { if (ItemTracker::GetCurrencyCategory(id) == cat) { hasItems = true; break; } }
                    if (!hasItems) continue;

                    if (CollapsingHeaderWithIcon(cat.c_str(), "currencies", ImVec4(0.40f,0.75f,0.90f,1.f), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (ImGui::BeginTable(("##CurrenciesTable_v3_" + cat).c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                        {
                            float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                            ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                            ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                            ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                            ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableHeadersRow();

                            for (auto& [id, st] : filteredSortedCurrencies)
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
                float iconColumnWidth = (static_cast<float>(g_Settings.itemsIconSize) + 10.0f > 70.0f) ? (static_cast<float>(g_Settings.itemsIconSize) + 10.0f) : 70.0f;
                ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn(Localization::GetText("column_name"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 430.0f);
                ImGui::TableSetupColumn(Localization::GetText("column_count"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : filteredSortedCurrencies)
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
