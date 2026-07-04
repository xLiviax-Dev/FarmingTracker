#include "ui_ignored.h"
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

namespace UIIgnored
{

static int  s_SubTab             = 0;
static char s_SearchBuf[128]     = "";
static char s_SearchCurBuf[128]  = "";
static std::set<int> s_SelectedItems;
static std::set<int> s_SelectedCurrencies;
static int s_PendingItemId = -1;
static std::string s_PendingItemName;
static int s_PendingCurrencyId = -1;
static std::string s_PendingCurrencyName;

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
    float px = 8.f, py = 2.f;                    // slightly more padding = bigger badge
    float fsz = ImGui::GetFontSize() * 0.88f;     // slightly bigger font
    const char* lbl = RarityLabel(rarity);
    ImVec2 tsz   = ImGui::CalcTextSize(lbl);
    ImVec2 start = ImGui::GetCursorScreenPos();
    ImVec2 bsz   = {tsz.x * (fsz / ImGui::GetFontSize()) + px*2,
                    tsz.y * (fsz / ImGui::GetFontSize()) + py*2};
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(start, {start.x+bsz.x, start.y+bsz.y}, ImGui::ColorConvertFloat4ToU32(bg), 4.f);
    dl->AddRect      (start, {start.x+bsz.x, start.y+bsz.y}, ImGui::ColorConvertFloat4ToU32({col.x,col.y,col.z,0.6f}), 4.f, 0, 0.5f);
    dl->AddText(ImGui::GetFont(), fsz, {start.x+px, start.y+py}, ImGui::ColorConvertFloat4ToU32(col), lbl);
    ImGui::Dummy(bsz);
}

static void RenderStatsBar(const std::map<std::string, int>& counts, int total)
{
    const float  kH    = 34.f;
    ImVec2       cur   = ImGui::GetCursorScreenPos();
    float        avail = ImGui::GetContentRegionAvail().x;
    ImDrawList*  dl    = ImGui::GetWindowDrawList();
    const float  acR = g_Settings.accentColorR, acG = g_Settings.accentColorG, acB = g_Settings.accentColorB;
    ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f), std::min(1.f,acG*2.f), std::min(1.f,acB*2.f), 1.f));
    ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f, acG*.5f, acB*.5f, 1.f));
    ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f), std::min(1.f,acG*1.5f), std::min(1.f,acB*1.5f), 1.f));
    ImVec2 hMin  = cur, hMax = {cur.x + avail, cur.y + kH};
    dl->AddRectFilledMultiColor(hMin, hMax, top, top, bot, bot);
    dl->AddRect(hMin, hMax, border, 4.f, 0, 0.5f);
    dl->AddRectFilled({hMin.x, hMin.y}, {hMin.x + 3.f, hMax.y},
        ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.f)), 2.f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::BeginChild("##ignored_stats", {avail, kH}, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + std::max(0.f,(kH-ImGui::GetTextLineHeight())*.5f));
    ImGui::SetCursorPosX(10.f);
    ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1.f), "%d", total);
    ImGui::SameLine(0, 4);
    ImGui::TextColored(ImVec4(1,1,1,1), "%s", Localization::GetText("ignored_items_label"));
    ImGui::SameLine(0, 14);
    static const std::vector<std::string> order = {"","Basic","Fine","Masterwork","Rare","Exotic","Ascended","Legendary"};
    for (const auto& r : order)
    {
        auto it = counts.find(r);
        if (it == counts.end() || it->second == 0) continue;
        ImVec4 col = r.empty() ? ImVec4(0.7f,0.7f,0.7f,1.f) : RarityColor(r);
        CountChip(it->second, col);
        ImGui::SameLine(0, 3);
        RarityBadge(r.empty() ? "Junk" : r);
        ImGui::SameLine(0, 10);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void RenderMassIgnoreBar()
{
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("mass_actions_label"));
    ImGui::SameLine(0, 8);
    static const std::vector<std::string> rarities = {"Junk","Basic","Fine","Masterwork","Rare","Exotic","Ascended","Legendary"};
    
    // Map rarity names to settings
    static std::map<std::string, bool*> rarityToggleMap = {
        {"Junk", &g_Settings.ignoredRarityToggleJunk},
        {"Basic", &g_Settings.ignoredRarityToggleBasic},
        {"Fine", &g_Settings.ignoredRarityToggleFine},
        {"Masterwork", &g_Settings.ignoredRarityToggleMasterwork},
        {"Rare", &g_Settings.ignoredRarityToggleRare},
        {"Exotic", &g_Settings.ignoredRarityToggleExotic},
        {"Ascended", &g_Settings.ignoredRarityToggleAscended},
        {"Legendary", &g_Settings.ignoredRarityToggleLegendary}
    };
    
    for (const auto& rar : rarities)
    {
        ImVec4 col = RarityColor(rar);
        bool* toggleState = rarityToggleMap[rar];
        
        // Use gold color when toggled on
        if (*toggleState)
        {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(1.00f, 0.84f, 0.00f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.90f, 0.10f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.90f, 0.75f, 0.00f, 1.0f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button,        {col.x*0.5f, col.y*0.5f, col.z*0.5f, 0.8f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {col.x*0.7f, col.y*0.7f, col.z*0.7f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {col.x*0.6f, col.y*0.6f, col.z*0.6f, 0.9f});
        }
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        if (ImGui::SmallButton(RarityLabel(rar)))
        {
            *toggleState = !*toggleState;
            SettingsManager::Save();
            
            // When toggling on, ignore all existing items of this rarity
            if (*toggleState)
            {
                auto items = ItemTracker::GetSortedItems(ItemTracker::SortMode::NameAZ);
                for (const auto& [id, st] : items)
                    if (st.details.loaded && st.details.rarity == rar && !IgnoredItemsManager::IsItemIgnored(id))
                        IgnoredItemsManager::IgnoreItem(id);
            }
            // When toggling off, unignore all items of this rarity (optional - user might want to keep them ignored)
            // For now, we'll keep them ignored to avoid unexpected behavior
        }
        ImGui::PopStyleColor(4);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("mass_ignore_rarity_tooltip"));
        ImGui::SameLine(0, 4);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
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
        if (UICommon::GreenGradientButton(Localization::GetText("yes_clear"), "##yes_clear_ignored"))
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

static void RenderItemsSubTab()
{
    auto ignoredSet = IgnoredItemsManager::GetIgnoredItems();
    std::vector<int> ignored(ignoredSet.begin(), ignoredSet.end());

    std::map<std::string, int> rarCounts;
    for (int id : ignored)
    {
        Stat st = ItemTracker::GetItemStat(id);
        rarCounts[st.details.loaded ? st.details.rarity : ""]++;
    }
    RenderStatsBar(rarCounts, (int)ignored.size());

    ImGui::SetNextItemWidth(200.f);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::InputTextWithHint("##ign_search", Localization::GetText("search_items_hint"), s_SearchBuf, sizeof(s_SearchBuf));
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 8);

    // Group by Rarity checkbox
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::Checkbox("##ignored_group_by_rarity", &g_Settings.ignoredGroupByRarity))
        SettingsManager::Save();
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 4);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("group_by_rarity"));
    ImGui::SameLine(0, 12);

    // Show Rarity as Tabs — only available when Group by Rarity is active
    // BeginDisabled/EndDisabled not available in ImGui 1.80 — emulate with alpha push
    bool disableRarityTabs = !g_Settings.ignoredGroupByRarity;
    if (disableRarityTabs)
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.4f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (!disableRarityTabs && ImGui::Checkbox("##ignored_show_rarity_as_tabs", &g_Settings.ignoredShowRarityAsTabs))
        SettingsManager::Save();
    else if (disableRarityTabs)
    {
        bool dummy = g_Settings.ignoredShowRarityAsTabs;
        ImGui::Checkbox("##ignored_show_rarity_as_tabs_dis", &dummy);
    }
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 4);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("show_rarity_as_tabs"));
    if (disableRarityTabs)
        ImGui::PopStyleVar();
    ImGui::SameLine(0, 8);

    // Load & Save button (combined Import/Export with popup, orange gradient)
    if (UICommon::OrangeGradientButton("Import/Export", "##ignored_ie"))
        ImGui::OpenPopup("IgnoredLoadSavePopup");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("load_save_tooltip"));

    if (ImGui::BeginPopup("IgnoredLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = ItemTracker::ExportToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\ignored_export.json";
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
            std::string path = std::string(dir ? dir : "") + "\\ignored_export.csv";
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
        if (ImGui::Button(Localization::GetText("import_ignored_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\ignored_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0)
                {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    try { IgnoredItemsManager::ImportFromJson(nlohmann::json::parse(buf)); } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_ignored_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("import_items_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\items_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0)
                {
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
    if (UICommon::RedGradientButton(Localization::GetText("clear_all_ignored_items"), "##clear_ignored_items"))
        ImGui::OpenPopup("ClearIgnoredItemsConfirm");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_all_ignored_tooltip"));

    if (!s_SelectedItems.empty())
    {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f,0.18f,0.0f,0.5f));
        ImGui::BeginChild("##bulk_items", {0,26}, false);
        char lbl[64]; snprintf(lbl, sizeof(lbl), "%d %s", (int)s_SelectedItems.size(), Localization::GetText("sessions_selected"));
        ImGui::TextColored({1.f,0.78f,0.1f,1.f}, "  %s  ", lbl);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f,0.3f,0.3f,1.f));
        if (ImGui::SmallButton(Localization::GetText("unignore_item")))
        { for (int id : s_SelectedItems) IgnoredItemsManager::UnignoreItem(id); s_SelectedItems.clear(); }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::SmallButton(Localization::GetText("clear"))) s_SelectedItems.clear();
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    RenderMassIgnoreBar();

    if (RenderConfirmPopup("ClearIgnoredItemsConfirm",
        Localization::GetText("clear_history_confirm"),
        Localization::GetText("clear_history_warning")))
    { for (int id : ignored) IgnoredItemsManager::UnignoreItem(id); s_SelectedItems.clear(); }

    if (ignored.empty()) { ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("no_sessions_recorded")); return; }

    std::string search(s_SearchBuf);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::vector<int> filtered;
    for (int id : ignored)
    {
        Stat st = ItemTracker::GetItemStat(id);
        if (search.empty()) { filtered.push_back(id); continue; }
        auto lc = [](std::string x){ std::transform(x.begin(),x.end(),x.begin(),::tolower); return x; };
        std::string name = st.details.loaded ? st.details.name : "";
        std::string rarity = st.details.loaded ? st.details.rarity : "";
        if (lc(name).find(search)!=std::string::npos ||
            lc(rarity).find(search)!=std::string::npos ||
            std::to_string(id).find(search)!=std::string::npos)
            filtered.push_back(id);
    }

    float iconSz   = static_cast<float>(g_Settings.itemsIconSize);
    float iconColW = std::max(iconSz + 10.f, 36.f);
    float rowH     = UICommon::CalcTableRowHeight(iconSz);
    float tableH   = ImGui::GetContentRegionAvail().y - 4.f;

    // Header: normal = accent, hover = gray (push Header colors too for column hover)
    ImVec4 accentCol    = ImVec4(g_Settings.accentColorR,         g_Settings.accentColorG,         g_Settings.accentColorB,         1.0f);
    ImVec4 accentColDim = ImVec4(g_Settings.accentColorR * 0.7f,  g_Settings.accentColorG * 0.7f,  g_Settings.accentColorB * 0.7f,  1.0f);
    ImVec4 grayCol      = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,   accentColDim); // normal  = accent
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,   grayCol);      // hover   = gray
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,    grayCol);      // active  = gray

    // Group by Rarity logic
    if (g_Settings.ignoredGroupByRarity)
    {
        // Rarity order: Junk first (lowest) up to Legendary (highest)
        std::vector<std::string> rarityOrder = {"","Junk","Basic","Fine","Masterwork","Rare","Exotic","Ascended","Legendary"};
        
        // Group items by rarity
        std::map<std::string, std::vector<int>> rarityGroups;
        for (int id : filtered)
        {
            Stat st = ItemTracker::GetItemStat(id);
            std::string rarity = st.details.loaded ? st.details.rarity : "";
            rarityGroups[rarity].push_back(id);
        }

        // Rarity colors
        std::map<std::string, ImVec4> rarityColors;
        rarityColors["Legendary"] = ImVec4(0.70f, 0.17f, 0.89f, 1.f);
        rarityColors["Ascended"] = ImVec4(0.95f, 0.40f, 0.14f, 1.f);
        rarityColors["Exotic"] = ImVec4(1.00f, 0.65f, 0.00f, 1.f);
        rarityColors["Rare"] = ImVec4(1.00f, 0.90f, 0.10f, 1.f);
        rarityColors["Masterwork"] = ImVec4(0.20f, 0.80f, 0.20f, 1.f);
        rarityColors["Fine"] = ImVec4(0.00f, 0.50f, 1.f, 1.f);
        rarityColors["Basic"] = ImVec4(1.f, 1.f, 1.f, 1.f);
        rarityColors["Junk"] = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
        rarityColors[""] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);

        if (!g_Settings.ignoredShowRarityAsTabs)
        {
            // Sections mode
            for (const auto& rarity : rarityOrder)
            {
                if (rarityGroups.find(rarity) == rarityGroups.end() || rarityGroups[rarity].empty())
                    continue;

                char headerLabel[256];
                snprintf(headerLabel, sizeof(headerLabel), "%s (%zu)", rarity.empty() ? "Junk" : RarityLabel(rarity), rarityGroups[rarity].size());

                ImVec4 headerColor = rarityColors.count(rarity) ? rarityColors[rarity] : ImVec4(1.f, 1.f, 1.f, 1.f);
                ImGui::PushStyleColor(ImGuiCol_Text, headerColor);
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));

                if (ImGui::CollapsingHeader(headerLabel, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PopStyleColor(4);

                    // Calculate table height based on item count
                    float itemTableH = std::min(tableH, (float)rarityGroups[rarity].size() * rowH + 50.0f);
                    
                    if (ImGui::BeginTable(("##IgnoredRarityTable_" + rarity).c_str(), 5,
                        ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|
                        ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoSavedSettings, {0,itemTableH}))
                    {
                        ImGui::TableSetupScrollFreeze(0, 1);
                        ImGui::TableSetupColumn("##sel",  ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, 30.f);
                        ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, iconColW);
                        ImGui::TableSetupColumn(Localization::GetText("column_name"),   ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("ID",     ImGuiTableColumnFlags_WidthFixed, 65.f);
                        ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed, 100.f);

                        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                        ImGui::TableSetColumnIndex(0);
                        bool allSel = !rarityGroups[rarity].empty() && std::all_of(rarityGroups[rarity].begin(), rarityGroups[rarity].end(),
                            [](int id){ return s_SelectedItems.count(id)>0; });
                        if (ImGui::Checkbox(("##selall_" + rarity).c_str(), &allSel))
                        { if (allSel) for (int id : rarityGroups[rarity]) s_SelectedItems.insert(id); else for (int id : rarityGroups[rarity]) s_SelectedItems.erase(id); }
                        for (int col = 1; col < 5; col++) { ImGui::TableSetColumnIndex(col); ImGui::TableHeader(ImGui::TableGetColumnName(col)); }

                        for (int id : rarityGroups[rarity])
                        {
                            Stat st  = ItemTracker::GetItemStat(id);
                            bool sel = s_SelectedItems.count(id) > 0;
                            ImGui::PushID(id);
                            ImGui::TableNextRow(0, rowH);

                            ImGui::TableSetColumnIndex(0);
                            UICommon::AlignTableCellFrame(rowH);
                            if (ImGui::Checkbox("##sel", &sel)) { if (sel) s_SelectedItems.insert(id); else s_SelectedItems.erase(id); }

                            ImGui::TableSetColumnIndex(1);
                            UICommon::AlignTableCellIcon(rowH, iconSz);
                            UICommon::EnsureItemIconTexture(id, st.details.iconUrl);
                            UICommon::DrawItemIconCell(id, st.details.iconUrl, iconSz, st.details.loaded ? st.details.rarity : "");
                            bool iconHov = ImGui::IsItemHovered();
                            if (iconHov && ImGui::IsMouseClicked(1))
                            {
                                s_PendingItemId = id;
                                s_PendingItemName = st.details.loaded ? st.details.name : "";
                            }

                            ImGui::TableSetColumnIndex(2);
                            UICommon::AlignTableCellText(rowH);
                            std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
                            ImGui::Text("%s", name.c_str());
                            bool nameHov = ImGui::IsItemHovered();
                            if (nameHov && ImGui::IsMouseClicked(1))
                            {
                                s_PendingItemId = id;
                                s_PendingItemName = name;
                            }
                            if (st.details.loaded && !st.details.rarity.empty()) { ImGui::SameLine(0,6); RarityBadge(st.details.rarity); }

                            if (iconHov || nameHov)
                            {
                                UITooltips::ItemTooltipOptions opt;
                                opt.showCount=true; opt.count=st.count; opt.showProfit=false;
                                opt.showTrading=true; opt.showAccountFlags=true; opt.showId=true;
                                if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                                else UITooltips::RenderItemTooltipFallback(name, "", id, opt);
                            }

                            ImGui::TableSetColumnIndex(3);
                            UICommon::AlignTableCellText(rowH);
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", id);

                            ImGui::TableSetColumnIndex(4);
                            UICommon::AlignTableCellFrame(rowH);
                            ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.9f,0.3f,0.3f,1.f));
                            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.9f,0.3f,0.3f,0.08f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.3f,0.3f,0.18f));
                            if (ImGui::SmallButton(Localization::GetText("unignore_item"))) IgnoredItemsManager::UnignoreItem(id);
                            ImGui::PopStyleColor(3);

                            ImGui::PopID();
                        }

                        ImGui::EndTable();

                        // Render context menus outside table scope
                        if (s_PendingItemId != -1)
                        {
                            UIContextMenu::OpenContextMenu("IgnoredItemCtx", s_PendingItemId, s_PendingItemName);
                            s_PendingItemId = -1;
                        }
                        UIContextMenu::RenderItemContextMenu("IgnoredItemCtx", UIContextMenu::ContextMenuType::Ignored);
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
            // Tabs mode — one tab per rarity, colored text matching rarity
            static std::string s_ActiveRarityTab = "";
            static bool        s_ForceTabSelect  = false;

            if (ImGui::BeginTabBar("##IgnoredRarityTabs"))
            {
                for (const auto& rarity : rarityOrder)
                {
                    if (rarityGroups.find(rarity) == rarityGroups.end() || rarityGroups[rarity].empty())
                        continue;

                    char tabLabel[256];
                    snprintf(tabLabel, sizeof(tabLabel), "%s (%zu)",
                        rarity.empty() ? "Junk" : RarityLabel(rarity),
                        rarityGroups[rarity].size());

                    // Color the tab text in the rarity color
                    ImVec4 tabCol = rarityColors.count(rarity)
                        ? rarityColors.at(rarity)
                        : ImVec4(0.6f, 0.6f, 0.6f, 1.f);
                    ImGui::PushStyleColor(ImGuiCol_Text, tabCol);

                    // Keep the active tab selected after unignore
                    ImGuiTabItemFlags tabFlags = ImGuiTabItemFlags_None;
                    if (s_ForceTabSelect && s_ActiveRarityTab == rarity)
                    {
                        tabFlags = ImGuiTabItemFlags_SetSelected;
                        s_ForceTabSelect = false;
                    }

                    bool tabOpen = ImGui::BeginTabItem(tabLabel, nullptr, tabFlags);
                    ImGui::PopStyleColor(); // Text color

                    if (tabOpen)
                    {
                        s_ActiveRarityTab = rarity; // track current

                        // Restore normal text color inside the tab
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

                        float itemTableH = std::min(tableH, (float)rarityGroups[rarity].size() * rowH + 50.0f);

                        if (ImGui::BeginTable(("##IgnoredRarityTabTable_" + rarity).c_str(), 5,
                            ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|
                            ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoSavedSettings, {0,itemTableH}))
                        {
                            ImGui::TableSetupScrollFreeze(0, 1);
                            ImGui::TableSetupColumn("##sel",  ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, 30.f);
                            ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, iconColW);
                            ImGui::TableSetupColumn(Localization::GetText("column_name"),   ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("ID",     ImGuiTableColumnFlags_WidthFixed, 65.f);
                            ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed, 100.f);

                            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                            ImGui::TableSetColumnIndex(0);
                            bool allSel = !rarityGroups[rarity].empty() && std::all_of(rarityGroups[rarity].begin(), rarityGroups[rarity].end(),
                                [](int id){ return s_SelectedItems.count(id)>0; });
                            if (ImGui::Checkbox(("##selall_" + rarity).c_str(), &allSel))
                            { if (allSel) for (int id : rarityGroups[rarity]) s_SelectedItems.insert(id); else for (int id : rarityGroups[rarity]) s_SelectedItems.erase(id); }
                            for (int col = 1; col < 5; col++) { ImGui::TableSetColumnIndex(col); ImGui::TableHeader(ImGui::TableGetColumnName(col)); }

                            for (int id : rarityGroups[rarity])
                            {
                                Stat st  = ItemTracker::GetItemStat(id);
                                bool sel = s_SelectedItems.count(id) > 0;
                                ImGui::PushID(id);
                                ImGui::TableNextRow(0, rowH);

                                ImGui::TableSetColumnIndex(0);
                                UICommon::AlignTableCellFrame(rowH);
                                if (ImGui::Checkbox("##sel", &sel)) { if (sel) s_SelectedItems.insert(id); else s_SelectedItems.erase(id); }

                                ImGui::TableSetColumnIndex(1);
                                UICommon::AlignTableCellIcon(rowH, iconSz);
                                UICommon::EnsureItemIconTexture(id, st.details.iconUrl);
                                UICommon::DrawItemIconCell(id, st.details.iconUrl, iconSz, st.details.loaded ? st.details.rarity : "");
                                bool iconHov = ImGui::IsItemHovered();
                                if (iconHov && ImGui::IsMouseClicked(1))
                                {
                                    s_PendingItemId = id;
                                    s_PendingItemName = st.details.loaded ? st.details.name : "";
                                }

                                ImGui::TableSetColumnIndex(2);
                                UICommon::AlignTableCellText(rowH);
                                std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
                                ImGui::Text("%s", name.c_str());
                                bool nameHov = ImGui::IsItemHovered();
                                if (nameHov && ImGui::IsMouseClicked(1))
                                {
                                    s_PendingItemId = id;
                                    s_PendingItemName = name;
                                }
                                if (st.details.loaded && !st.details.rarity.empty()) { ImGui::SameLine(0,6); RarityBadge(st.details.rarity); }

                                if (iconHov || nameHov)
                                {
                                    UITooltips::ItemTooltipOptions opt;
                                    opt.showCount=true; opt.count=st.count; opt.showProfit=false;
                                    opt.showTrading=true; opt.showAccountFlags=true; opt.showId=true;
                                    if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                                    else UITooltips::RenderItemTooltipFallback(name, "", id, opt);
                                }

                                ImGui::TableSetColumnIndex(3);
                                UICommon::AlignTableCellText(rowH);
                                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", id);

                                ImGui::TableSetColumnIndex(4);
                                UICommon::AlignTableCellFrame(rowH);
                                ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.9f,0.3f,0.3f,1.f));
                                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.9f,0.3f,0.3f,0.08f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.3f,0.3f,0.18f));
                                if (ImGui::SmallButton(Localization::GetText("unignore_item")))
                                {
                                    IgnoredItemsManager::UnignoreItem(id);
                                    // Stay on current rarity tab even if group becomes empty momentarily
                                    s_ForceTabSelect = true;
                                }
                                ImGui::PopStyleColor(3);

                                ImGui::PopID();
                            }

                            ImGui::EndTable();

                            // Render context menus outside table scope
                            if (s_PendingItemId != -1)
                            {
                                UIContextMenu::OpenContextMenu("IgnoredItemCtx", s_PendingItemId, s_PendingItemName);
                                s_PendingItemId = -1;
                            }
                            UIContextMenu::RenderItemContextMenu("IgnoredItemCtx", UIContextMenu::ContextMenuType::Ignored);
                        }

                        ImGui::PopStyleColor(); // normal text
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
    }
    else
    {
        // Default view (no grouping)
        if (!ImGui::BeginTable("##IgnoredItemsTable", 5,
            ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|
            ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoSavedSettings, {0,tableH}))
            return;

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("##sel",  ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, 30.f);
        ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, iconColW);
        ImGui::TableSetupColumn(Localization::GetText("column_name"),   ImGuiTableColumnFlags_WidthStretch, 0.6f);
        ImGui::TableSetupColumn("ID",     ImGuiTableColumnFlags_WidthFixed, 65.f);
        ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed, 120.f);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableSetColumnIndex(0);
        bool allSel = !filtered.empty() && std::all_of(filtered.begin(), filtered.end(),
            [](int id){ return s_SelectedItems.count(id)>0; });
        if (ImGui::Checkbox("##selall", &allSel))
        { if (allSel) for (int id : filtered) s_SelectedItems.insert(id); else for (int id : filtered) s_SelectedItems.erase(id); }
        for (int col = 1; col < 5; col++) { ImGui::TableSetColumnIndex(col); ImGui::TableHeader(ImGui::TableGetColumnName(col)); }

        for (int id : filtered)
        {
            Stat st  = ItemTracker::GetItemStat(id);
            bool sel = s_SelectedItems.count(id) > 0;
            ImGui::PushID(id);
            ImGui::TableNextRow(0, rowH);

            ImGui::TableSetColumnIndex(0);
            UICommon::AlignTableCellFrame(rowH);
            if (ImGui::Checkbox("##sel", &sel)) { if (sel) s_SelectedItems.insert(id); else s_SelectedItems.erase(id); }

            ImGui::TableSetColumnIndex(1);
            UICommon::AlignTableCellIcon(rowH, iconSz);
            UICommon::EnsureItemIconTexture(id, st.details.iconUrl);
            UICommon::DrawItemIconCell(id, st.details.iconUrl, iconSz, st.details.loaded ? st.details.rarity : "");
            bool iconHov = ImGui::IsItemHovered();
            if (iconHov && ImGui::IsMouseClicked(1))
            {
                s_PendingItemId = id;
                s_PendingItemName = st.details.loaded ? st.details.name : "";
            }

            ImGui::TableSetColumnIndex(2);
            UICommon::AlignTableCellText(rowH);
            std::string name = st.details.loaded ? st.details.name : Localization::GetText("loading");
            ImGui::Text("%s", name.c_str());
            bool nameHov = ImGui::IsItemHovered();
            if (nameHov && ImGui::IsMouseClicked(1))
            {
                s_PendingItemId = id;
                s_PendingItemName = name;
            }
            if (st.details.loaded && !st.details.rarity.empty()) { ImGui::SameLine(0,6); RarityBadge(st.details.rarity); }

            if (iconHov || nameHov)
            {
                UITooltips::ItemTooltipOptions opt;
                opt.showCount=true; opt.count=st.count; opt.showProfit=false;
                opt.showTrading=true; opt.showAccountFlags=true; opt.showId=true;
                if (st.details.loaded) UITooltips::RenderItemTooltip(st.details, id, opt);
                else UITooltips::RenderItemTooltipFallback(name, "", id, opt);
            }

            ImGui::TableSetColumnIndex(3);
            UICommon::AlignTableCellText(rowH);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", id);

            ImGui::TableSetColumnIndex(4);
            UICommon::AlignTableCellFrame(rowH);
            ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.9f,0.3f,0.3f,1.f));
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.9f,0.3f,0.3f,0.08f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.3f,0.3f,0.18f));
            if (ImGui::SmallButton(Localization::GetText("unignore_item"))) IgnoredItemsManager::UnignoreItem(id);
            ImGui::PopStyleColor(3);

            ImGui::PopID();
        }

        ImGui::EndTable();

        // Render context menus outside table scope
        if (s_PendingItemId != -1)
        {
            UIContextMenu::OpenContextMenu("IgnoredItemCtx", s_PendingItemId, s_PendingItemName);
            s_PendingItemId = -1;
        }
        UIContextMenu::RenderItemContextMenu("IgnoredItemCtx", UIContextMenu::ContextMenuType::Ignored);
    }
    ImGui::PopStyleColor(3); // TableHeaderBg colors
}

static void RenderCurrenciesSubTab()
{
    auto ignoredSet = IgnoredItemsManager::GetIgnoredCurrencies();
    std::vector<int> ignored(ignoredSet.begin(), ignoredSet.end());

    {
        const float kH = 34.f;
        ImVec2 cur = ImGui::GetCursorScreenPos(); float avail = ImGui::GetContentRegionAvail().x;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float acR=g_Settings.accentColorR, acG=g_Settings.accentColorG, acB=g_Settings.accentColorB;
        ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
        ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
        ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
        ImVec2 hMin=cur, hMax={cur.x+avail, cur.y+kH};
        dl->AddRectFilledMultiColor(hMin,hMax,top,top,bot,bot);
        dl->AddRect(hMin,hMax,border,4.f,0,0.5f);
        dl->AddRectFilled({hMin.x,hMin.y},{hMin.x+3.f,hMax.y},ImGui::ColorConvertFloat4ToU32(ImVec4(acR,acG,acB,1.f)),2.f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
        ImGui::BeginChild("##ign_cur_stats",{avail,kH},false,ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+std::max(0.f,(kH-ImGui::GetTextLineHeight())*.5f));
        ImGui::SetCursorPosX(10.f);
        ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1.f),"%d",(int)ignored.size());
        ImGui::SameLine(0,4);
        ImGui::TextColored(ImVec4(1,1,1,1),"%s",Localization::GetText("ignored_currencies_label"));
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    ImGui::SetNextItemWidth(200.f);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::InputTextWithHint("##ign_cur_search", Localization::GetText("search_items_hint"), s_SearchCurBuf, sizeof(s_SearchCurBuf));
    ImGui::PopStyleColor();
    ImGui::SameLine(0,8);

    // Load & Save button (combined Import/Export with popup, orange gradient)
    if (UICommon::OrangeGradientButton("Import/Export", "##ignored_cur_ie"))
        ImGui::OpenPopup("IgnoredCurLoadSavePopup");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("load_save_tooltip"));

    if (ImGui::BeginPopup("IgnoredCurLoadSavePopup"))
    {
        // Export section
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = ItemTracker::ExportToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\ignored_currencies_export.json";
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
            std::string path = std::string(dir ? dir : "") + "\\ignored_currencies_export.csv";
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
        if (ImGui::Button(Localization::GetText("import_ignored_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\ignored_currencies_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0)
                {
                    std::string buf(sz, '\0'); fread(&buf[0], 1, sz, f); fclose(f);
                    try { IgnoredItemsManager::ImportFromJson(nlohmann::json::parse(buf)); } catch (...) {}
                } else fclose(f);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_ignored_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("import_currencies_json")))
        {
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\currencies_import.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
                if (sz > 0)
                {
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

    ImGui::SameLine(0,4);

    // Clear button — red gradient design
    if (UICommon::RedGradientButton(Localization::GetText("clear_all_ignored_currencies"), "##clear_ignored_cur"))
        ImGui::OpenPopup("ClearIgnoredCurrenciesConfirm");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_all_ignored_tooltip"));

    if (!s_SelectedCurrencies.empty())
    {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f,0.18f,0.0f,0.5f));
        ImGui::BeginChild("##bulk_cur", {0,26}, false);
        char lbl[64]; snprintf(lbl, sizeof(lbl), "%d %s", (int)s_SelectedCurrencies.size(), Localization::GetText("sessions_selected"));
        ImGui::TextColored({1.f,0.78f,0.1f,1.f}, "  %s  ", lbl);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f,0.3f,0.3f,1.f));
        if (ImGui::SmallButton(Localization::GetText("unignore_currency")))
        { for (int id : s_SelectedCurrencies) IgnoredItemsManager::UnignoreCurrency(id); s_SelectedCurrencies.clear(); }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::SmallButton(Localization::GetText("clear"))) s_SelectedCurrencies.clear();
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    if (RenderConfirmPopup("ClearIgnoredCurrenciesConfirm",
        Localization::GetText("clear_history_confirm"),
        Localization::GetText("clear_history_warning")))
    { for (int id : ignored) IgnoredItemsManager::UnignoreCurrency(id); s_SelectedCurrencies.clear(); }

    if (ignored.empty()) { ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", Localization::GetText("no_sessions_recorded")); return; }

    std::string search(s_SearchCurBuf);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::vector<int> filtered;
    for (int id : ignored)
    {
        if (search.empty()) { filtered.push_back(id); continue; }
        Stat st = ItemTracker::GetCurrencyStat(id);
        std::string name = st.details.loaded ? st.details.name : "";
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(search)!=std::string::npos || std::to_string(id).find(search)!=std::string::npos)
            filtered.push_back(id);
    }

    float iconSz   = static_cast<float>(g_Settings.itemsIconSize);
    float iconColW = std::max(iconSz + 10.f, 36.f);
    float rowH     = UICommon::CalcTableRowHeight(iconSz);
    float tableH   = ImGui::GetContentRegionAvail().y - 4.f;

    // Header: normal = accent, hover = gray
    ImVec4 accentColC    = ImVec4(g_Settings.accentColorR,        g_Settings.accentColorG,        g_Settings.accentColorB,        1.0f);
    ImVec4 accentColDimC = ImVec4(g_Settings.accentColorR * 0.7f, g_Settings.accentColorG * 0.7f, g_Settings.accentColorB * 0.7f, 1.0f);
    ImVec4 grayColC      = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, accentColDimC); // normal = accent
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, grayColC);      // hover  = gray
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  grayColC);      // active = gray

    if (!ImGui::BeginTable("##IgnoredCurTable", 4,
        ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|
        ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoSavedSettings, {0,tableH}))
        return;

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##sel",  ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, 30.f);
    ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, iconColW);
    ImGui::TableSetupColumn(Localization::GetText("currency_name"), ImGuiTableColumnFlags_WidthStretch, 0.6f);
    ImGui::TableSetupColumn(Localization::GetText("column_ignore"), ImGuiTableColumnFlags_WidthFixed, 120.f);

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableSetColumnIndex(0);
    bool allSel = !filtered.empty() && std::all_of(filtered.begin(), filtered.end(),
        [](int id){ return s_SelectedCurrencies.count(id)>0; });
    if (ImGui::Checkbox("##selallcur", &allSel))
    { if (allSel) for (int id : filtered) s_SelectedCurrencies.insert(id); else for (int id : filtered) s_SelectedCurrencies.erase(id); }
    for (int col = 1; col < 4; col++) { ImGui::TableSetColumnIndex(col); ImGui::TableHeader(ImGui::TableGetColumnName(col)); }

    for (int id : filtered)
    {
        Stat st = ItemTracker::GetCurrencyStat(id);
        std::string iconUrl = st.details.iconUrl;
        if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
        std::string name = st.details.loaded ? st.details.name
            : (id == 1 ? Localization::GetText("coin") : Localization::GetText("loading"));
        bool sel = s_SelectedCurrencies.count(id) > 0;

        ImGui::PushID(id);
        ImGui::TableNextRow(0, rowH);

        ImGui::TableSetColumnIndex(0);
        UICommon::AlignTableCellFrame(rowH);
        if (ImGui::Checkbox("##selcur", &sel)) { if (sel) s_SelectedCurrencies.insert(id); else s_SelectedCurrencies.erase(id); }

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

        ImGui::TableSetColumnIndex(2);
        UICommon::AlignTableCellText(rowH);
        ImGui::Text("%s", name.c_str());
        bool nameHov = ImGui::IsItemHovered();
        if (nameHov && ImGui::IsMouseClicked(1))
        {
            s_PendingCurrencyId = id;
            s_PendingCurrencyName = name;
        }

        if (iconHov || nameHov)
        {
            UITooltips::CurrencyTooltipOptions opt;
            opt.showCount=true; opt.count=st.count; opt.showRarity=true; opt.showId=true;
            if (st.details.loaded) UITooltips::RenderCurrencyTooltip(st.details, id, opt);
            else UITooltips::RenderCurrencyTooltipFallback(name, "", id, opt);
        }

        ImGui::TableSetColumnIndex(3);
        UICommon::AlignTableCellFrame(rowH);
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.9f,0.3f,0.3f,1.f));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.9f,0.3f,0.3f,0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.3f,0.3f,0.18f));
        if (ImGui::SmallButton(Localization::GetText("unignore_currency"))) IgnoredItemsManager::UnignoreCurrency(id);
        ImGui::PopStyleColor(3);

        ImGui::PopID();
    }

    ImGui::EndTable();
    ImGui::PopStyleColor(3); // TableHeaderBg colors

    // Render context menus outside table scope
    if (s_PendingCurrencyId != -1)
    {
        UIContextMenu::OpenContextMenu("IgnoredCurCtx", s_PendingCurrencyId, s_PendingCurrencyName);
        s_PendingCurrencyId = -1;
    }
    UIContextMenu::RenderCurrencyContextMenu("IgnoredCurCtx", UIContextMenu::ContextMenuType::Ignored);
}

void RenderIgnoredTab()
{
    // Labels with counts
    int itemCount = (int)IgnoredItemsManager::GetIgnoredItems().size();
    int curCount  = (int)IgnoredItemsManager::GetIgnoredCurrencies().size();

    char lblItems[64], lblCur[64];
    snprintf(lblItems, sizeof(lblItems), "%s (%d)", Localization::GetText("tab_items"),      itemCount);
    snprintf(lblCur,   sizeof(lblCur),   "%s (%d)", Localization::GetText("tab_currencies"), curCount);

    UITabIcons::RenderSubPillTabBar({
        { "items",      lblItems },
        { "currencies", lblCur   }
    }, s_SubTab);

    switch (s_SubTab)
    {
        case 0: RenderItemsSubTab();      break;
        case 1: RenderCurrenciesSubTab(); break;
    }
}

} // namespace UIIgnored
