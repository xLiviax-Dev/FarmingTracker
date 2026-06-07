#include "ui_custom_profit.h"
#include "settings.h"
#include "custom_profit.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"
#include "ui_common.h"
#include "ui_context_menu.h"
#include "ui_tab_icons.h"
#include "shared.h"
#include "../include/nlohmann/json.hpp"
#include <vector>
#include <cstdio>
#include <string>

namespace UICustomProfit
{

// ─────────────────────────────────────────────────────────────────────────────
// Style helpers
// ─────────────────────────────────────────────────────────────────────────────

static void PushSegBtnStyle(bool active)
{
    if (active)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.45f, 0.80f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.52f, 0.88f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.38f, 0.70f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.14f, 0.14f, 0.14f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.60f, 0.60f, 0.60f, 1.00f));
    }
}

static void PushEditBtnStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.30f, 0.58f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.40f, 0.75f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.08f, 0.24f, 0.46f, 1.00f));
}

static void PushDeleteBtnStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.10f, 0.10f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.15f, 0.15f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.05f, 0.05f, 1.00f));
}

// ─────────────────────────────────────────────────────────────────────────────
// RenderCoinCompact: renders G S C on one line, compact
// ─────────────────────────────────────────────────────────────────────────────
static void RenderCoinCompact(long long copper)
{
    int g = static_cast<int>(copper / 10000);
    int s = static_cast<int>((copper % 10000) / 100);
    int c = static_cast<int>(copper % 100);

    bool any = false;
    if (g)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.86f, 0.57f, 0.12f, 1.0f));
        ImGui::Text("%dG", g);
        ImGui::PopStyleColor();
        any = true;
    }
    if (s)
    {
        if (any) ImGui::SameLine(0.0f, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Text("%dS", s);
        ImGui::PopStyleColor();
        any = true;
    }
    if (c || !any)
    {
        if (any) ImGui::SameLine(0.0f, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.32f, 0.18f, 1.0f));
        ImGui::Text("%dC", c);
        ImGui::PopStyleColor();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadImportFile: loads & parses JSON from addon dir
// ─────────────────────────────────────────────────────────────────────────────
static void LoadImportFile()
{
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
    std::string filename = std::string(addonDir ? addonDir : "") + "\\custom_profit_import.json";
    FILE* f = nullptr;
    fopen_s(&f, filename.c_str(), "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length <= 0 || length > 4 * 1024 * 1024) { fclose(f); return; }

    std::string buffer(static_cast<size_t>(length), '\0');
    fread(&buffer[0], 1, static_cast<size_t>(length), f);
    fclose(f);

    try
    {
        nlohmann::json jsonData = nlohmann::json::parse(buffer);
        CustomProfitManager::ImportFromJson(jsonData);
    }
    catch (...) {}
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadImportFileCsv: loads & parses CSV from addon dir
// ─────────────────────────────────────────────────────────────────────────────
static void LoadImportFileCsv()
{
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
    std::string filename = std::string(addonDir ? addonDir : "") + "\\custom_profit_import.csv";
    FILE* f = nullptr;
    fopen_s(&f, filename.c_str(), "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length <= 0 || length > 4 * 1024 * 1024) { fclose(f); return; }

    std::string buffer(static_cast<size_t>(length), '\0');
    fread(&buffer[0], 1, static_cast<size_t>(length), f);
    fclose(f);

    try
    {
        CustomProfitManager::ImportFromCsv(buffer);
    }
    catch (...) {}
}

// ─────────────────────────────────────────────────────────────────────────────
// RenderTable: shared table renderer for Items and Currencies
// ─────────────────────────────────────────────────────────────────────────────
static void RenderTable(
    const std::vector<std::pair<int, long long>>& entries,
    StatType                                       type,
    const char*                                    tableId,
    const char*                                    ctxMenuId,
    UIContextMenu::ContextMenuType                 ctxType,
    int&                                           editId,
    int&                                           editG,
    int&                                           editS,
    int&                                           editC)
{
    const bool isItems = (type == StatType::Item);

    if (entries.empty())
    {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
        ImGui::TextUnformatted(Localization::GetText(
            isItems ? "no_custom_profit_items" : "no_custom_profit_currencies"));
        ImGui::PopStyleColor();
        ImGui::Spacing();
        return;
    }

    // Coin column width: enough for "999G 99S 99C" in compact form
    const float coinW    = 130.0f;
    const float iconW    =  40.0f;
    const float actionsW =  60.0f;

    ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV
                          | ImGuiTableFlags_RowBg
                          | ImGuiTableFlags_ScrollY
                          | ImGuiTableFlags_SizingFixedFit;

    float availH = ImGui::GetContentRegionAvail().y - 4.0f;
    if (ImGui::BeginTable(tableId, 4, flags, ImVec2(0.0f, availH)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("##icon",                                ImGuiTableColumnFlags_WidthFixed,   iconW);
        ImGui::TableSetupColumn(Localization::GetText("column_name"),    ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn(Localization::GetText("custom_profit_value"), ImGuiTableColumnFlags_WidthFixed, coinW);
        ImGui::TableSetupColumn("##act",                                 ImGuiTableColumnFlags_WidthFixed,   actionsW);
        ImGui::TableHeadersRow();

        for (const auto& [id, profit] : entries)
        {
            Stat st = isItems
                ? ItemTracker::GetItemStat(id)
                : ItemTracker::GetCurrencyStat(id);

            float rowH = UICommon::CalcTableRowHeight(32.0f);
            ImGui::TableNextRow(0, rowH);

            // ── Icon ────────────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(0);
            UICommon::AlignTableCellIcon(rowH, 32.0f);
            std::string iconUrl = st.details.iconUrl;
            UICommon::DrawItemIconCell(id, iconUrl, 32.0f,
                st.details.loaded ? st.details.rarity : "");
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                UIContextMenu::OpenContextMenu(ctxMenuId, id,
                    st.details.loaded ? st.details.name : "");

            // ── Name (or inline edit fields) ─────────────────────────────────
            ImGui::TableSetColumnIndex(1);
            UICommon::AlignTableCellText(rowH);

            if (editId == id)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 2.0f));
                ImGui::SetNextItemWidth(40.0f);
                ImGui::InputInt("##eg", &editG, 0);
                ImGui::SameLine(0.0f, 2.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.86f, 0.57f, 0.12f, 1.0f));
                ImGui::TextUnformatted("G");
                ImGui::PopStyleColor();
                ImGui::SameLine(0.0f, 6.0f);
                ImGui::SetNextItemWidth(40.0f);
                ImGui::InputInt("##es", &editS, 0);
                ImGui::SameLine(0.0f, 2.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::TextUnformatted("S");
                ImGui::PopStyleColor();
                ImGui::SameLine(0.0f, 6.0f);
                ImGui::SetNextItemWidth(40.0f);
                ImGui::InputInt("##ec", &editC, 0);
                ImGui::SameLine(0.0f, 2.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.32f, 0.18f, 1.0f));
                ImGui::TextUnformatted("C");
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }
            else
            {
                const char* name = st.details.loaded
                    ? st.details.name.c_str()
                    : ((!isItems && id == 1) ? Localization::GetText("coin") : "...");
                ImGui::TextUnformatted(name);
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    UIContextMenu::OpenContextMenu(ctxMenuId, id,
                        st.details.loaded ? st.details.name : "");
            }

            // ── Coin value ───────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(2);
            UICommon::AlignTableCellText(rowH);
            if (editId == id)
            {
                // Show preview while editing
                long long prev = (long long)editG * 10000
                               + (long long)editS * 100
                               + editC;
                RenderCoinCompact(prev);
            }
            else
            {
                RenderCoinCompact(profit);
            }

            // ── Actions ──────────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(3);
            UICommon::AlignTableCellFrame(rowH);

            char btnEdit[32], btnDel[32];
            snprintf(btnEdit, sizeof(btnEdit), "##ed_%d", id);
            snprintf(btnDel,  sizeof(btnDel),  "##dl_%d", id);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            if (editId == id)
            {
                // Confirm button
                if (UICommon::GreenGradientButton("OK", (std::string("ok") + btnEdit).c_str()))
                {
                    long long val = (long long)editG * 10000
                                  + (long long)editS * 100
                                  + editC;
                    CustomProfitManager::SetCustomProfit(id, val, type);
                    editId = -1;
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("custom_profit_confirm_tooltip"));

                ImGui::SameLine(0.0f, 4.0f);

                // Cancel button
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.18f, 0.18f, 0.60f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.22f, 0.22f, 0.22f, 1.00f));
                if (ImGui::Button((std::string("x") + btnEdit).c_str(), ImVec2(22.0f, 0.0f)))
                    editId = -1;
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("cancel_tooltip"));
                ImGui::PopStyleColor(3);
            }
            else
            {
                // Edit button
                PushEditBtnStyle();
                if (ImGui::Button((std::string("~") + btnEdit).c_str(), ImVec2(26.0f, 0.0f)))
                {
                    editId = id;
                    editG  = static_cast<int>(profit / 10000);
                    editS  = static_cast<int>((profit % 10000) / 100);
                    editC  = static_cast<int>(profit % 100);
                }
                ImGui::PopStyleColor(3);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("custom_profit_edit_tooltip"));

                ImGui::SameLine(0.0f, 4.0f);

                // Delete button
                PushDeleteBtnStyle();
                if (ImGui::Button((std::string("X") + btnDel).c_str(), ImVec2(22.0f, 0.0f)))
                {
                    if (editId == id) editId = -1;
                    CustomProfitManager::RemoveCustomProfit(id);
                }
                ImGui::PopStyleColor(3);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("custom_profit_remove_tooltip"));
            }

            ImGui::PopStyleVar();
        }

        if (isItems)
            UIContextMenu::RenderItemContextMenu(ctxMenuId, ctxType);
        else
            UIContextMenu::RenderCurrencyContextMenu(ctxMenuId, ctxType);

        ImGui::EndTable();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// RenderCustomProfitTab — main entry point
// ═════════════════════════════════════════════════════════════════════════════
void RenderCustomProfitTab()
{
    static int s_AddId     = 0;
    static int s_AddGold   = 0;
    static int s_AddSilver = 0;
    static int s_AddCopper = 0;
    static int s_ActiveTab = 0; // 0 = Items, 1 = Currencies

    static int s_EditItemId      = -1;
    static int s_EditItemG       = 0, s_EditItemS = 0, s_EditItemC = 0;
    static int s_EditCurrencyId  = -1;
    static int s_EditCurrencyG   = 0, s_EditCurrencyS = 0, s_EditCurrencyC = 0;

    // ── Top bar ──────────────────────────────────────────────────────────────
    ImGui::Spacing();

    // Import/Export button (orange gradient)
    if (UICommon::OrangeGradientButton("Import/Export", "##custprofit_ie"))
        ImGui::OpenPopup("CustomProfitImportExportPopup");

    if (ImGui::BeginPopup("CustomProfitImportExportPopup"))
    {
        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "%s", Localization::GetText("export_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export_json")))
        {
            std::string json = CustomProfitManager::ExportToJson();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\custom_profit_export.json";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(json.data(), 1, json.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("export_csv")))
        {
            std::string csv = CustomProfitManager::ExportToCsv();
            const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string path = std::string(dir ? dir : "") + "\\custom_profit_export.csv";
            FILE* f = nullptr; fopen_s(&f, path.c_str(), "wb");
            if (f) { fwrite(csv.data(), 1, csv.size(), f); fclose(f); }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("export_csv_tooltip"));
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "%s", Localization::GetText("import_label"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import_json")))
        {
            LoadImportFile();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("import_json_tooltip"));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("import_csv")))
        {
            LoadImportFileCsv();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("custom_profit_import_tooltip"));
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    // Clear all button — red gradient design
    if (UICommon::RedGradientButton(Localization::GetText("clear_all_custom_profits"), "##clear_custom"))
        ImGui::OpenPopup("ClearAllCustomProfitsConfirm");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_all_custom_profits_tooltip"));

    // Confirm popup
    if (ImGui::BeginPopup("ClearAllCustomProfitsConfirm"))
    {
        ImGui::TextUnformatted(Localization::GetText("reset_confirm"));
        ImGui::TextUnformatted(Localization::GetText("clear_all_custom_profits_warning"));
        ImGui::Spacing();
        if (UICommon::GreenGradientButton(Localization::GetText("yes_clear"), "##yes_clear_custom"))
        {
            CustomProfitManager::ClearAll();
            s_EditItemId = s_EditCurrencyId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("no_cancel")))
            ImGui::CloseCurrentPopup();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("cancel_tooltip"));
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Add form ─────────────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::TextUnformatted(s_ActiveTab == 0
        ? Localization::GetText("add_custom_profit_item")
        : Localization::GetText("add_custom_profit_currency"));
    ImGui::PopStyleColor();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
    ImGui::SetNextItemWidth(90.0f);
    ImGui::InputInt(s_ActiveTab == 0 ? "ID##AddId" : "ID##AddIdC", &s_AddId, 0);
    ImGui::SameLine(0.0f, 10.0f);

    // Gold
    ImGui::SetNextItemWidth(48.0f);
    ImGui::InputInt("##AddG", &s_AddGold, 0);
    ImGui::SameLine(0.0f, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.86f, 0.57f, 0.12f, 1.0f));
    ImGui::TextUnformatted("G");
    ImGui::PopStyleColor();
    ImGui::SameLine(0.0f, 8.0f);

    // Silver
    ImGui::SetNextItemWidth(48.0f);
    ImGui::InputInt("##AddS", &s_AddSilver, 0);
    ImGui::SameLine(0.0f, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::TextUnformatted("S");
    ImGui::PopStyleColor();
    ImGui::SameLine(0.0f, 8.0f);

    // Copper
    ImGui::SetNextItemWidth(48.0f);
    ImGui::InputInt("##AddC", &s_AddCopper, 0);
    ImGui::SameLine(0.0f, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.32f, 0.18f, 1.0f));
    ImGui::TextUnformatted("C");
    ImGui::PopStyleColor();
    ImGui::SameLine(0.0f, 10.0f);
    ImGui::PopStyleVar();

    // Set button (orange gradient)
    if (UICommon::OrangeGradientButton(Localization::GetText("custom_profit_set_profit"), "##set_profit"))
    {
        if (s_AddId > 0)
        {
            long long totalCopper = (long long)s_AddGold   * 10000
                                  + (long long)s_AddSilver * 100
                                  + s_AddCopper;
            StatType type = (s_ActiveTab == 0) ? StatType::Item : StatType::Currency;
            CustomProfitManager::SetCustomProfit(s_AddId, totalCopper, type);
            s_AddId = s_AddGold = s_AddSilver = s_AddCopper = 0;
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("custom_profit_set_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Segmented control + entry count ──────────────────────────────────────
    auto allDetailed = CustomProfitManager::GetAllCustomProfitsDetailed();

    std::vector<std::pair<int, long long>> cpItems, cpCurrencies;
    for (auto& [id, entry] : allDetailed)
    {
        if (entry.type == StatType::Item)
            cpItems.push_back({id, entry.customProfitCopper});
        else
            cpCurrencies.push_back({id, entry.customProfitCopper});
    }

    // Tab buttons
    char lblItems[64], lblCur[64];
    snprintf(lblItems, sizeof(lblItems), "%s (%zu)",
        Localization::GetText("custom_profit_items_header"), cpItems.size());
    snprintf(lblCur,   sizeof(lblCur),   "%s (%zu)",
        Localization::GetText("custom_profit_currencies_header"), cpCurrencies.size());

    UITabIcons::RenderSubPillTabBar({
        { "items",      lblItems },
        { "currencies", lblCur   }
    }, s_ActiveTab);

    // Entry count pill (right-aligned)
    const auto& curArr = (s_ActiveTab == 0) ? cpItems : cpCurrencies;
    char countLbl[32];
    snprintf(countLbl, sizeof(countLbl), "%zu %s",
        curArr.size(),
        Localization::GetText("entries_label"));
    float pillW = ImGui::CalcTextSize(countLbl).x + 16.0f;
    float pillX = ImGui::GetContentRegionMax().x - pillW;
    if (pillX > ImGui::GetCursorPosX() + 8.0f)
    {
        ImGui::SameLine(pillX);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.70f, 1.00f, 1.0f));
        ImGui::TextUnformatted(countLbl);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    // ── Table ─────────────────────────────────────────────────────────────────
    if (s_ActiveTab == 0)
    {
        RenderTable(cpItems, StatType::Item,
            "CustomProfitItemsTable",
            "CustomProfitItemCtx",
            UIContextMenu::ContextMenuType::CustomProfit,
            s_EditItemId, s_EditItemG, s_EditItemS, s_EditItemC);
    }
    else
    {
        RenderTable(cpCurrencies, StatType::Currency,
            "CustomProfitCurrenciesTable",
            "CustomProfitCurrencyCtx",
            UIContextMenu::ContextMenuType::CustomProfit,
            s_EditCurrencyId, s_EditCurrencyG, s_EditCurrencyS, s_EditCurrencyC);
    }
}

} // namespace UICustomProfit
