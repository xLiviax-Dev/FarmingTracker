#include "ui_favorites.h"
#include "ui_tab_icons.h"
#include "settings.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"
#include "ui_context_menu.h"
#include "ui_tooltips.h"
#include "ui_common.h"
#include "shared.h"
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

namespace UIFavorites
{

// ────────────────────────────────────────────────────────────────────────────
// State
// ────────────────────────────────────────────────────────────────────────────
static int  s_SubTab             = 0; // 0 = Items, 1 = Currencies
static char s_SearchBuf[128]     = "";
static char s_SearchCurBuf[128]  = "";
static std::set<int> s_SelectedItems;
static std::set<int> s_SelectedCurrencies;
static int s_PendingItemId = -1;
static std::string s_PendingItemName;
static int s_PendingCurrencyId = -1;
static std::string s_PendingCurrencyName;

// ────────────────────────────────────────────────────────────────────────────
// Helpers (mirror ui_ignored.cpp style)
// ────────────────────────────────────────────────────────────────────────────
static ImVec4 RarityColor(const std::string& rarity)
{
    if (rarity == "Legendary")  return {1.00f, 0.50f, 0.80f, 1.f};
    if (rarity == "Ascended")   return {0.90f, 0.30f, 0.90f, 1.f};
    if (rarity == "Exotic")     return {1.00f, 0.60f, 0.00f, 1.f};
    if (rarity == "Rare")       return {1.00f, 0.90f, 0.00f, 1.f};
    if (rarity == "Masterwork") return {0.20f, 0.80f, 0.20f, 1.f};
    if (rarity == "Fine")       return {0.00f, 0.50f, 1.00f, 1.f};
    if (rarity == "Basic")      return {1.00f, 1.00f, 1.00f, 1.f};
    return {0.70f, 0.70f, 0.70f, 1.f}; // Junk
}

static const char* RarityLabel(const std::string& r)
{
    if (r == "Legendary")  return Localization::GetText("rarity_name_legendary");
    if (r == "Ascended")   return Localization::GetText("rarity_name_ascended");
    if (r == "Exotic")     return Localization::GetText("rarity_name_exotic");
    if (r == "Rare")       return Localization::GetText("rarity_name_rare");
    if (r == "Masterwork") return Localization::GetText("rarity_name_masterwork");
    if (r == "Fine")       return Localization::GetText("rarity_name_fine");
    if (r == "Basic")      return Localization::GetText("rarity_name_basic");
    return Localization::GetText("rarity_name_junk");
}

static void CountChip(int count, ImVec4 col)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", count);
    float px = 6.f, py = 1.f;
    float fsz = ImGui::GetFontSize() * 0.88f;
    float scale = fsz / ImGui::GetFontSize();
    ImVec2 tsz = ImGui::CalcTextSize(buf);
    ImVec2 bsz = {tsz.x * scale + px * 2.f, tsz.y * scale + py * 2.f};
    float lineH = ImGui::GetTextLineHeight();
    float offsetY = std::max(0.f, (lineH - bsz.y) * 0.5f);
    ImVec2 start = ImGui::GetCursorScreenPos();
    start.y += offsetY;
    ImVec4 bg = {col.x * 0.30f, col.y * 0.30f, col.z * 0.30f, 0.90f};
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(start, {start.x + bsz.x, start.y + bsz.y},
        ImGui::ColorConvertFloat4ToU32(bg), 4.f);
    dl->AddRect(start, {start.x + bsz.x, start.y + bsz.y},
        ImGui::ColorConvertFloat4ToU32({col.x, col.y, col.z, 0.8f}), 4.f, 0, 1.f);
    dl->AddText(ImGui::GetFont(), fsz, {start.x + px, start.y + py},
        IM_COL32(255, 255, 255, 255), buf);
    ImGui::Dummy({bsz.x, lineH});
}

static void RarityBadge(const std::string& rarity)
{
    if (rarity.empty()) return;
    ImVec4 col = RarityColor(rarity);
    ImVec4 bg  = {col.x * 0.18f, col.y * 0.18f, col.z * 0.18f, 0.85f};
    float px   = 8.f, py = 2.f;                   // same as ui_ignored
    float fsz  = ImGui::GetFontSize() * 0.88f;     // same as ui_ignored
    float scale = fsz / ImGui::GetFontSize();
    const char* lbl = RarityLabel(rarity);
    ImVec2 tsz = ImGui::CalcTextSize(lbl);
    ImVec2 bsz = {tsz.x * scale + px * 2.f, tsz.y * scale + py * 2.f};

    // Vertically center the badge within the current text line
    float lineH   = ImGui::GetTextLineHeight();
    float offsetY = std::max(0.f, (lineH - bsz.y) * 0.5f);

    ImVec2 start = ImGui::GetCursorScreenPos();
    start.y += offsetY;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(start, {start.x + bsz.x, start.y + bsz.y},
        ImGui::ColorConvertFloat4ToU32(bg), 4.f);
    dl->AddRect(start, {start.x + bsz.x, start.y + bsz.y},
        ImGui::ColorConvertFloat4ToU32({col.x, col.y, col.z, 0.6f}), 4.f, 0, 0.5f);
    dl->AddText(ImGui::GetFont(), fsz, {start.x + px, start.y + py},
        ImGui::ColorConvertFloat4ToU32(col), lbl);

    ImGui::Dummy({bsz.x, lineH});
}

static void RenderStatsBar(const std::map<std::string, int>& counts, int total,
                           const char* totalLabel)
{
    const float  kH    = 34.f;
    ImVec2       cur   = ImGui::GetCursorScreenPos();
    float        avail = ImGui::GetContentRegionAvail().x;
    ImDrawList*  dl    = ImGui::GetWindowDrawList();
    const float  acR=g_Settings.accentColorR, acG=g_Settings.accentColorG, acB=g_Settings.accentColorB;
    ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
    ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
    ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
    ImVec2 hMin=cur, hMax={cur.x+avail, cur.y+kH};
    dl->AddRectFilledMultiColor(hMin,hMax,top,top,bot,bot);
    dl->AddRect(hMin,hMax,border,4.f,0,0.5f);
    dl->AddRectFilled({hMin.x,hMin.y},{hMin.x+3.f,hMax.y},
        ImGui::ColorConvertFloat4ToU32(ImVec4(acR,acG,acB,1.f)),2.f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::BeginChild("##fav_stats",{avail,kH},false,ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY()+std::max(0.f,(kH-ImGui::GetTextLineHeight())*.5f));
    ImGui::SetCursorPosX(10.f);
    ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1.f),"%d",total);
    ImGui::SameLine(0,4);
    ImGui::TextColored(ImVec4(1,1,1,1),"%s",totalLabel);
    ImGui::SameLine(0,14);
    static const std::vector<std::string> rarityOrder = {
        "Legendary","Ascended","Exotic","Rare","Masterwork","Fine","Basic",""
    };
    for (const auto& r : rarityOrder)
    {
        auto it = counts.find(r);
        if (it == counts.end() || it->second == 0) continue;
        ImVec4 col = r.empty() ? ImVec4(0.7f,0.7f,0.7f,1.f) : RarityColor(r);
        CountChip(it->second, col);
        ImGui::SameLine(0,3);
        RarityBadge(r.empty() ? "Junk" : r);
        ImGui::SameLine(0,10);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// Bulk selection bar
static void RenderBulkBar(std::set<int>& selected, bool isItems)
{
    if (selected.empty()) return;
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.14f, 0.0f, 0.5f));
    ImGui::BeginChild("##fav_bulk", {0, 26}, false);
    char lbl[64];
    snprintf(lbl, sizeof(lbl), "%d %s",
        (int)selected.size(), Localization::GetText("sessions_selected"));
    ImGui::TextColored({1.f, 0.84f, 0.f, 1.f}, "  %s  ", lbl);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
    if (ImGui::SmallButton(Localization::GetText("unfavorite_selected")))
    {
        for (int id : selected) ItemTracker::SetFavorite(id, false);
        selected.clear();
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::SmallButton(Localization::GetText("clear"))) selected.clear();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static bool RenderConfirmPopup(const char* id, const char* msg, const char* warning)
{
    bool confirmed = false;
    if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(msg);
        if (warning && warning[0])
            ImGui::TextUnformatted(warning);
        ImGui::Spacing();
        if (UICommon::GreenGradientButton(Localization::GetText("yes_clear"), "##yes_clear_fav"))
        { confirmed = true; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.18f, 0.18f, 0.60f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.22f, 0.22f, 0.22f, 1.00f));
        if (ImGui::Button(Localization::GetText("no_cancel")))
            ImGui::CloseCurrentPopup();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("cancel_tooltip"));
        ImGui::PopStyleColor(3);
        ImGui::EndPopup();
    }
    return confirmed;
}

// ────────────────────────────────────────────────────────────────────────────
// Items sub-tab
// ────────────────────────────────────────────────────────────────────────────
static bool RenderItemsSubTab()
{
    auto favoriteMap = ItemTracker::GetFavoriteItems();
    std::vector<int> favIds;
    for (auto& [id, st] : favoriteMap) favIds.push_back(id);

    // Stats bar
    std::map<std::string, int> rarCounts;
    for (int id : favIds)
    {
        Stat st = ItemTracker::GetItemStat(id);
        rarCounts[st.details.loaded ? st.details.rarity : ""]++;
    }
    RenderStatsBar(rarCounts, (int)favIds.size(),
        Localization::GetText("favorite_items_label"));

    // ── Toolbar ──────────────────────────────────────────────────────────
    ImGui::SetNextItemWidth(200.f);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::InputTextWithHint("##fav_search", "Search...",
        s_SearchBuf, sizeof(s_SearchBuf));
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 8);

    // Import/Export button (orange gradient)
    if (UICommon::OrangeGradientButton("Import/Export", "##favitems_ie"))
        ImGui::OpenPopup("FavoritesLoadSavePopup");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("load_save_tooltip"));

    if (ImGui::BeginPopup("FavoritesLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = ItemTracker::ExportFavoritesToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\favorites_export.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(json.data(), 1, json.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("export_csv")))
        {
            // CSV export for favorites
            std::string csv = ItemTracker::ExportToCsv();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\favorites_export.csv";
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
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Localization::GetText("import_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import_favorites_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\favorites_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0) {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    try { ItemTracker::ImportFavoritesFromJson(nlohmann::json::parse(buf)); } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_favorites_json_tooltip"));
        ImGui::SameLine();
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

    ImGui::SameLine(0, 4);

    // Clear button — red gradient design
    if (UICommon::RedGradientButton(Localization::GetText("clear_all_favorite_items"), "##clear_fav_items"))
        ImGui::OpenPopup("ClearFavItemsConfirm");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_all_favorites_tooltip"));

    // Bulk bar
    RenderBulkBar(s_SelectedItems, true);

    ImGui::Spacing();

    // Confirm popup
    if (RenderConfirmPopup("ClearFavItemsConfirm",
        Localization::GetText("reset_confirm"),
        Localization::GetText("clear_all_favorites_warning")))
    {
        for (int id : favIds) ItemTracker::SetFavorite(id, false);
        s_SelectedItems.clear();
    }

    if (favIds.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("no_favorites_yet"));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("right_click_to_add"));
        return false;
    }

    // Filter
    std::string search(s_SearchBuf);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::vector<int> filtered;
    for (int id : favIds)
    {
        if (search.empty()) { filtered.push_back(id); continue; }
        Stat st = ItemTracker::GetItemStat(id);
        std::string name = st.details.loaded ? st.details.name : "";
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::string rar = st.details.loaded ? st.details.rarity : "";
        std::transform(rar.begin(), rar.end(), rar.begin(), ::tolower);
        if (name.find(search) != std::string::npos ||
            rar.find(search)  != std::string::npos ||
            std::to_string(id).find(search) != std::string::npos)
            filtered.push_back(id);
    }

    // Table
    float iconSz   = static_cast<float>(g_Settings.itemsIconSize);
    float iconColW = std::max(iconSz + 10.f, 36.f);
    float rowH     = UICommon::CalcTableRowHeight(iconSz);

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH
                          | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY
                          | ImGuiTableFlags_NoSavedSettings;
    float tableH = ImGui::GetContentRegionAvail().y - 4.f;
    // Header: normal = accent, hover = gray
    ImVec4 favAccentDim = ImVec4(g_Settings.accentColorR * 0.7f, g_Settings.accentColorG * 0.7f, g_Settings.accentColorB * 0.7f, 1.0f);
    ImVec4 favGray      = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, favAccentDim);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, favGray);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  favGray);

    if (!ImGui::BeginTable("##FavItemsTable", 6, flags, {0, tableH}))
    {
        ImGui::PopStyleColor(3);
        return false;
    }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##sel",    ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 30.f);
    ImGui::TableSetupColumn("Icon",   ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, iconColW);
    ImGui::TableSetupColumn(Localization::GetText("column_name"),   ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("ID",                                   ImGuiTableColumnFlags_WidthFixed, 65.f);
    ImGui::TableSetupColumn(Localization::GetText("column_profit"), ImGuiTableColumnFlags_WidthFixed, 110.f);
    ImGui::TableSetupColumn(Localization::GetText("column_favorite"), ImGuiTableColumnFlags_WidthFixed, 90.f);

    // Header with select-all
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableSetColumnIndex(0);
    bool allSel = !filtered.empty() && std::all_of(filtered.begin(), filtered.end(),
        [](int id){ return s_SelectedItems.count(id) > 0; });
    if (ImGui::Checkbox("##selall", &allSel))
    {
        if (allSel) for (int id : filtered) s_SelectedItems.insert(id);
        else        for (int id : filtered) s_SelectedItems.erase(id);
    }
    for (int col = 1; col < 6; col++)
    { ImGui::TableSetColumnIndex(col); ImGui::TableHeader(ImGui::TableGetColumnName(col)); }

    for (int id : filtered)
    {
        Stat st = ItemTracker::GetItemStat(id);
        bool sel = s_SelectedItems.count(id) > 0;

        // Apply favorite row color if enabled
        if (g_Settings.enableFavoriteRowColor)
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                ImGui::ColorConvertFloat4ToU32(ImVec4(
                    g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1],
                    g_Settings.favoriteRowColor[2], 0.25f)));

        ImGui::PushID(id);
        ImGui::TableNextRow(0, rowH);

        // Col 0: checkbox
        ImGui::TableSetColumnIndex(0);
        UICommon::AlignTableCellFrame(rowH);
        if (ImGui::Checkbox("##sel", &sel))
        { if (sel) s_SelectedItems.insert(id); else s_SelectedItems.erase(id); }

        // Col 1: icon
        ImGui::TableSetColumnIndex(1);
        UICommon::AlignTableCellIcon(rowH, iconSz);
        UICommon::EnsureItemIconTexture(id, st.details.iconUrl);
        UICommon::DrawItemIconCell(id, st.details.iconUrl, iconSz,
            st.details.loaded ? st.details.rarity : "");
        bool iconHov = ImGui::IsItemHovered();
        if (iconHov && ImGui::IsMouseClicked(1))
        {
            s_PendingItemId = id;
            s_PendingItemName = st.details.loaded ? st.details.name : "";
        }

        // Col 2: name + rarity badge
        ImGui::TableSetColumnIndex(2);
        UICommon::AlignTableCellText(rowH);
        std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");

        // Star prefix + optional text color
        ImVec4 nameCol = ImVec4(1.f, 0.84f, 0.f, 1.f);
        if (g_Settings.enableFavoriteTextColor)
            nameCol = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1],
                             g_Settings.favoriteTextColor[2], 1.f);
        ImGui::TextColored({1.f, 0.78f, 0.2f, 1.f}, "*");
        ImGui::SameLine(0, 3);
        ImGui::TextColored(nameCol, "%s", name.c_str());
        bool nameHov = ImGui::IsItemHovered();
        if (nameHov && ImGui::IsMouseClicked(1))
        {
            s_PendingItemId = id;
            s_PendingItemName = name;
        }

        // Rarity badge inline
        if (st.details.loaded && !st.details.rarity.empty())
        { ImGui::SameLine(0, 6); RarityBadge(st.details.rarity); }

        // Tooltip
        if (iconHov || nameHov)
        {
            UITooltips::ItemTooltipOptions opt;
            opt.showCount = true; opt.count = st.count;
            opt.showProfit = true;
            opt.showTrading = true; opt.showAccountFlags = true; opt.showId = true;
            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
            else UITooltips::RenderItemTooltipFallback(name, "", id, opt);
        }

        // Col 3: ID
        ImGui::TableSetColumnIndex(3);
        UICommon::AlignTableCellText(rowH);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", id);

        // Col 4: profit
        ImGui::TableSetColumnIndex(4);
        UICommon::AlignTableCellText(rowH);
        long long profit = ItemTracker::GetStatProfit(st);
        ImVec4 pc = profit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f)
                  : profit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f)
                  : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
        ImGui::TextColored(pc, "%s", UICommon::FormatCoin(profit).c_str());

        // Col 5: unfavorite button
        ImGui::TableSetColumnIndex(5);
        UICommon::AlignTableCellFrame(rowH);
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.9f, 0.3f, 0.3f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.9f, 0.3f, 0.3f, 0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 0.18f));
        if (ImGui::SmallButton(Localization::GetText("unfavorite_item")))
        {
            ItemTracker::SetFavorite(id, false);
            ImGui::PopStyleColor(3);
            ImGui::PopID();
            UIContextMenu::RenderItemContextMenu("FavItemContextMenu",
                UIContextMenu::ContextMenuType::Favorites);
            ImGui::EndTable();
            ImGui::PopStyleColor(3);
            return true; // signal: stay on items tab
        }
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite_tooltip"));

        ImGui::PopID();
    }

    ImGui::EndTable();
    ImGui::PopStyleColor(3);

    // Render context menus outside table scope
    if (s_PendingItemId != -1)
    {
        UIContextMenu::OpenContextMenu("FavItemContextMenu", s_PendingItemId, s_PendingItemName);
        s_PendingItemId = -1;
    }
    UIContextMenu::RenderItemContextMenu("FavItemContextMenu",
        UIContextMenu::ContextMenuType::Favorites);
    return false;
}

// ────────────────────────────────────────────────────────────────────────────
// Currencies sub-tab
// ────────────────────────────────────────────────────────────────────────────
static bool RenderCurrenciesSubTab()
{
    auto favoriteMap = ItemTracker::GetFavoriteCurrencies();
    std::vector<int> favIds;
    for (auto& [id, st] : favoriteMap) favIds.push_back(id);

    // Stats bar (currencies have no rarity, just a total)
    {
        const float kH=34.f;
        ImVec2 cur=ImGui::GetCursorScreenPos(); float avail=ImGui::GetContentRegionAvail().x;
        ImDrawList* dl=ImGui::GetWindowDrawList();
        const float acR=g_Settings.accentColorR,acG=g_Settings.accentColorG,acB=g_Settings.accentColorB;
        ImU32 top=ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
        ImU32 bot=ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
        ImU32 border=ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
        ImVec2 hMin=cur,hMax={cur.x+avail,cur.y+kH};
        dl->AddRectFilledMultiColor(hMin,hMax,top,top,bot,bot);
        dl->AddRect(hMin,hMax,border,4.f,0,0.5f);
        dl->AddRectFilled({hMin.x,hMin.y},{hMin.x+3.f,hMax.y},ImGui::ColorConvertFloat4ToU32(ImVec4(acR,acG,acB,1.f)),2.f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg,ImVec4(0,0,0,0));
        ImGui::BeginChild("##favcur_stats",{avail,kH},false,ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+std::max(0.f,(kH-ImGui::GetTextLineHeight())*.5f));
        ImGui::SetCursorPosX(10.f);
        ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1.f),"%d",(int)favIds.size());
        ImGui::SameLine(0,4);
        ImGui::TextColored(ImVec4(1,1,1,1),"%s",Localization::GetText("favorite_currencies_label"));
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // Toolbar
    ImGui::SetNextItemWidth(200.f);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::InputTextWithHint("##favcur_search", "Search...",
        s_SearchCurBuf, sizeof(s_SearchCurBuf));
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 8);

    // Import/Export button (orange gradient)
    if (UICommon::OrangeGradientButton("Import/Export", "##favcur_ie"))
        ImGui::OpenPopup("FavoritesCurLoadSavePopup");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("load_save_tooltip"));

    if (ImGui::BeginPopup("FavoritesCurLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = ItemTracker::ExportFavoritesToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\favorites_currencies_export.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(json.data(), 1, json.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("export_csv")))
        {
            // CSV export for favorites
            std::string csv = ItemTracker::ExportToCsv();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\favorites_currencies_export.csv";
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
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Localization::GetText("import_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import_favorites_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\favorites_currencies_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0) {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    try { ItemTracker::ImportFavoritesFromJson(nlohmann::json::parse(buf)); } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_favorites_json_tooltip"));
        ImGui::SameLine();
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

    ImGui::SameLine(0, 4);

    // Clear button — red gradient design
    if (UICommon::RedGradientButton(Localization::GetText("clear_all_favorite_currencies"), "##clear_fav_cur"))
        ImGui::OpenPopup("ClearFavCurConfirm");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_all_favorites_tooltip"));

    // Bulk bar
    RenderBulkBar(s_SelectedCurrencies, false);

    ImGui::Spacing();

    if (RenderConfirmPopup("ClearFavCurConfirm",
        Localization::GetText("reset_confirm"),
        Localization::GetText("clear_all_favorites_warning")))
    {
        for (int id : favIds) ItemTracker::SetFavorite(id, false);
        s_SelectedCurrencies.clear();
    }

    if (favIds.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("no_favorites_yet"));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("right_click_to_add"));
        return false;
    }

    // Filter
    std::string search(s_SearchCurBuf);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::vector<int> filtered;
    for (int id : favIds)
    {
        if (search.empty()) { filtered.push_back(id); continue; }
        Stat st = ItemTracker::GetCurrencyStat(id);
        std::string name = st.details.loaded ? st.details.name : "";
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(search) != std::string::npos ||
            std::to_string(id).find(search) != std::string::npos)
            filtered.push_back(id);
    }

    // Table
    float iconSz   = static_cast<float>(g_Settings.itemsIconSize);
    float iconColW = std::max(iconSz + 10.f, 36.f);
    float rowH     = UICommon::CalcTableRowHeight(iconSz);

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH
                          | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY
                          | ImGuiTableFlags_NoSavedSettings;
    float tableH = ImGui::GetContentRegionAvail().y - 4.f;
    // Header: normal = accent, hover = gray
    ImVec4 favCurAccentDim = ImVec4(g_Settings.accentColorR * 0.7f, g_Settings.accentColorG * 0.7f, g_Settings.accentColorB * 0.7f, 1.0f);
    ImVec4 favCurGray      = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, favCurAccentDim);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, favCurGray);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  favCurGray);

    if (!ImGui::BeginTable("##FavCurTable", 4, flags, {0, tableH}))
    {
        ImGui::PopStyleColor(3);
        return false;
    }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##sel",  ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 30.f);
    ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, iconColW);
    ImGui::TableSetupColumn(Localization::GetText("currency_name"), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(Localization::GetText("column_favorite"), ImGuiTableColumnFlags_WidthFixed, 90.f);

    // Header with select-all
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableSetColumnIndex(0);
    bool allSel = !filtered.empty() && std::all_of(filtered.begin(), filtered.end(),
        [](int id){ return s_SelectedCurrencies.count(id) > 0; });
    if (ImGui::Checkbox("##selallcur", &allSel))
    {
        if (allSel) for (int id : filtered) s_SelectedCurrencies.insert(id);
        else        for (int id : filtered) s_SelectedCurrencies.erase(id);
    }
    for (int col = 1; col < 4; col++)
    { ImGui::TableSetColumnIndex(col); ImGui::TableHeader(ImGui::TableGetColumnName(col)); }

    for (int id : filtered)
    {
        Stat st = ItemTracker::GetCurrencyStat(id);
        std::string iconUrl = st.details.iconUrl;
        if (id == 1 && iconUrl.empty())
            iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
        std::string name = st.details.loaded ? st.details.name
            : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"));
        bool sel = s_SelectedCurrencies.count(id) > 0;

        if (g_Settings.enableFavoriteRowColor)
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                ImGui::ColorConvertFloat4ToU32(ImVec4(
                    g_Settings.favoriteRowColor[0], g_Settings.favoriteRowColor[1],
                    g_Settings.favoriteRowColor[2], 0.25f)));

        ImGui::PushID(id);
        ImGui::TableNextRow(0, rowH);

        // Col 0: checkbox
        ImGui::TableSetColumnIndex(0);
        UICommon::AlignTableCellFrame(rowH);
        if (ImGui::Checkbox("##selcur", &sel))
        { if (sel) s_SelectedCurrencies.insert(id); else s_SelectedCurrencies.erase(id); }

        // Col 1: icon
        ImGui::TableSetColumnIndex(1);
        UICommon::AlignTableCellIcon(rowH, iconSz);
        UICommon::EnsureItemIconTexture(id, iconUrl);
        UICommon::DrawItemIconCell(id, iconUrl, iconSz, "");
        bool iconHov = ImGui::IsItemHovered();
        if (iconHov && ImGui::IsMouseClicked(1))
        {
            s_PendingCurrencyId = id;
            s_PendingCurrencyName = name;
        }

        // Col 2: name
        ImGui::TableSetColumnIndex(2);
        UICommon::AlignTableCellText(rowH);
        ImVec4 nameCol = ImVec4(1.f, 0.84f, 0.f, 1.f);
        if (g_Settings.enableFavoriteTextColor)
            nameCol = ImVec4(g_Settings.favoriteTextColor[0], g_Settings.favoriteTextColor[1],
                             g_Settings.favoriteTextColor[2], 1.f);
        ImGui::TextColored({1.f, 0.78f, 0.2f, 1.f}, "*");
        ImGui::SameLine(0, 3);
        ImGui::TextColored(nameCol, "%s", name.c_str());
        bool nameHov = ImGui::IsItemHovered();
        if (nameHov && ImGui::IsMouseClicked(1))
        {
            s_PendingCurrencyId = id;
            s_PendingCurrencyName = name;
        }

        if (iconHov || nameHov)
        {
            UITooltips::CurrencyTooltipOptions opt;
            opt.showCount = true; opt.count = st.count;
            opt.showRarity = true; opt.showId = true;
            if (st.details.loaded) UITooltips::RenderCurrencyTooltip(st.details, id, opt);
            else UITooltips::RenderCurrencyTooltipFallback(name, "", id, opt);
        }

        // Col 3: unfavorite button
        ImGui::TableSetColumnIndex(3);
        UICommon::AlignTableCellFrame(rowH);
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.9f, 0.3f, 0.3f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.9f, 0.3f, 0.3f, 0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 0.18f));
        if (ImGui::SmallButton(Localization::GetText("unfavorite_item")))
        {
            ItemTracker::SetFavorite(id, false);
            ImGui::PopStyleColor(3);
            ImGui::PopID();
            UIContextMenu::RenderCurrencyContextMenu("FavCurContextMenu",
                UIContextMenu::ContextMenuType::Favorites);
            ImGui::EndTable();
            ImGui::PopStyleColor(3);
            return true; // signal: stay on currencies tab
        }
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("toggle_favorite_tooltip"));

        ImGui::PopID();
    }

    ImGui::EndTable();
    ImGui::PopStyleColor(3);

    // Render context menus outside table scope
    if (s_PendingCurrencyId != -1)
    {
        UIContextMenu::OpenContextMenu("FavCurContextMenu", s_PendingCurrencyId, s_PendingCurrencyName);
        s_PendingCurrencyId = -1;
    }
    UIContextMenu::RenderCurrencyContextMenu("FavCurContextMenu",
        UIContextMenu::ContextMenuType::Favorites);
    return false;
}

// ────────────────────────────────────────────────────────────────────────────
// Main entry point
// ────────────────────────────────────────────────────────────────────────────
void RenderFavoritesTab()
{
    static bool s_ForceItemsTab      = false;
    static bool s_ForceCurrenciesTab = false;

    int itemCount = (int)ItemTracker::GetFavoriteItemIds().size();
    int curCount  = (int)ItemTracker::GetFavoriteCurrencyIds().size();

    char lblItems[64], lblCurrencies[64];
    snprintf(lblItems,      sizeof(lblItems),      "%s (%d)",
        Localization::GetText("tab_items"),      itemCount);
    snprintf(lblCurrencies, sizeof(lblCurrencies), "%s (%d)",
        Localization::GetText("tab_currencies"), curCount);

    // Force tab switch if requested by sub-tab rendering
    if (s_ForceItemsTab)      { s_SubTab = 0; s_ForceItemsTab      = false; }
    if (s_ForceCurrenciesTab) { s_SubTab = 1; s_ForceCurrenciesTab = false; }

    UITabIcons::RenderSubPillTabBar({
        { "items",      lblItems      },
        { "currencies", lblCurrencies }
    }, s_SubTab);

    switch (s_SubTab)
    {
        case 0: if (RenderItemsSubTab())      s_ForceItemsTab      = true; break;
        case 1: if (RenderCurrenciesSubTab()) s_ForceCurrenciesTab = true; break;
    }
}

} // namespace UIFavorites
