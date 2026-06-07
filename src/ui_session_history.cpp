#include "ui_session_history.h"
#include "session_history.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "shared.h"
#include "localization.h"
#include "ui_common.h"
#include <algorithm>

namespace UISessionHistory
{
static SessionHistory::SessionData s_SelectedSession;
static bool s_ShowSessionDetails = false;
static int s_DetailsView = 0;

void RenderSessionHistoryTab()
{
    if (!g_Settings.enableSessionHistory)
    {
        ImGui::Text("%s", Localization::GetText("enable_session_history"));
        ImGui::Text("%s", Localization::GetText("enable_session_history_tooltip"));
        return;
    }

    // Load sessions
    auto sessions = SessionHistory::LoadSessions();
    int sessionCount = static_cast<int>(sessions.size());
    int maxSessions = SessionHistory::GetMaxSessions();

    // Show session count
    ImGui::Text("%s: %d/%d", Localization::GetText("sessions_stored"), sessionCount, maxSessions);
    ImGui::Spacing();

    // Save Current Session Button (green gradient)
    if (UICommon::GreenGradientButton(Localization::GetText("save_current_session"), "##save_current_session"))
    {
        ItemTracker::SaveCurrentSession();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("save_current_session_tooltip"));

    ImGui::SameLine();

    // Clear History Button — red gradient design
    if (UICommon::RedGradientButton(Localization::GetText("clear_history"), "##clear_hist"))
    {
        ImGui::OpenPopup("ClearHistoryConfirm");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_history_tooltip"));

    if (ImGui::BeginPopup("ClearHistoryConfirm"))
    {
        ImGui::TextUnformatted(Localization::GetText("reset_confirm"));
        ImGui::TextUnformatted(Localization::GetText("clear_history_warning"));
        ImGui::Spacing();
        if (UICommon::GreenGradientButton(Localization::GetText("yes_clear"), "##yes_clear_hist"))
        {
            SessionHistory::ClearHistory();
            ImGui::CloseCurrentPopup();
        }
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

    ImGui::Spacing();
    ImGui::Separator();

    if (sessions.empty())
    {
        ImGui::Text("%s", Localization::GetText("no_sessions_recorded"));
        return;
    }

    // Reverse to show newest first
    std::reverse(sessions.begin(), sessions.end());

    // Session History Table
    if (ImGui::BeginTable("SessionHistoryTable_v2", 7, ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoSavedSettings))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(Localization::GetText("date"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 200.0f); // Default was auto, now 200px
        ImGui::TableSetupColumn(Localization::GetText("duration"), ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn(Localization::GetText("profit"), ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(Localization::GetText("profit_per_hour"), ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(Localization::GetText("drops"), ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(Localization::GetText("best_drop"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(Localization::GetText("actions"), ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (const auto& session : sessions)
        {
            float rowH = UICommon::CalcTableRowHeight(0.0f);
            ImGui::TableNextRow(0, rowH);

            // Date (start time)
            ImGui::TableSetColumnIndex(0);
            UICommon::AlignTableCellText(rowH);
            ImGui::Text("%s", session.startTime.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("date_tooltip"));

            // Duration
            ImGui::TableSetColumnIndex(1);
            UICommon::AlignTableCellText(rowH);
            ImGui::Text("%s", UICommon::FormatDuration(session.durationSeconds).c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("duration_tooltip"));

            // Profit
            ImGui::TableSetColumnIndex(2);
            UICommon::AlignTableCellText(rowH);
            ImVec4 profitColor = session.totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (session.totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(session.totalProfit).c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("profit_tooltip"));

            // Profit/h
            ImGui::TableSetColumnIndex(3);
            UICommon::AlignTableCellText(rowH);
            ImVec4 profitPerHourColor = session.profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (session.profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(session.profitPerHour).c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("profit_per_hour_tooltip"));

            // Drops
            ImGui::TableSetColumnIndex(4);
            UICommon::AlignTableCellText(rowH);
            ImGui::Text("%d", session.totalDrops);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("drops_tooltip"));

            // Best Drop
            ImGui::TableSetColumnIndex(5);
            UICommon::AlignTableCellText(rowH);
            if (!session.topDrops.empty())
            {
                const auto& bestDrop = session.topDrops[0];
                ImGui::Text("%s (%s)", bestDrop.itemName.c_str(), UICommon::FormatCoin(bestDrop.totalValue).c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("best_drop_tooltip"));
            }
            else
            {
                ImGui::TextUnformatted("-");
            }

            // Actions
            ImGui::TableSetColumnIndex(6);
            UICommon::AlignTableCellFrame(rowH);
            ImGui::PushID(session.startTime.c_str());
            if (UICommon::OrangeGradientButton(Localization::GetText("details"), ("##details_" + std::string(session.startTime)).c_str()))
            {
                s_SelectedSession = session;
                s_ShowSessionDetails = true;
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // Session Details Modal
    if (s_ShowSessionDetails)
    {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(Localization::GetText("session_details"), &s_ShowSessionDetails, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // Session summary header
            ImGui::Text("%s: %s", Localization::GetText("date"), s_SelectedSession.startTime.c_str());
            ImGui::Text("%s: %s", Localization::GetText("duration"), UICommon::FormatDuration(s_SelectedSession.durationSeconds).c_str());
            ImGui::Text("%s: %s", Localization::GetText("profit"), UICommon::FormatCoin(s_SelectedSession.totalProfit).c_str());
            ImGui::Text("%s: %d", Localization::GetText("drops"), s_SelectedSession.totalDrops);
            if (s_SelectedSession.averageMagicFind >= 0)
                ImGui::Text("%s: %d%%", Localization::GetText("magic_find"), s_SelectedSession.averageMagicFind);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Use allDrops if available, otherwise topDrops
            const auto& dropsToShow = s_SelectedSession.allDrops.empty() ? s_SelectedSession.topDrops : s_SelectedSession.allDrops;

            if (dropsToShow.empty())
            {
                ImGui::Text("%s", Localization::GetText("no_items_in_session"));
            }
            else
            {
                // Filter controls
                static char searchFilter[256] = "";
                if (ImGui::BeginTabBar("SessionDetailsViewTabs"))
                {
                    if (ImGui::BeginTabItem(Localization::GetText("tab_items")))
                    {
                        s_DetailsView = 0;
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem(Localization::GetText("tab_currencies")))
                    {
                        s_DetailsView = 1;
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                ImGui::InputText(Localization::GetText("search_items"), searchFilter, sizeof(searchFilter));
                ImGui::Spacing();

                // Drops table
                if (ImGui::BeginTable("SessionDropsTable", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable, ImVec2(0, 300)))
                {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn(Localization::GetText("time"));
                    ImGui::TableSetupColumn(Localization::GetText("item"));
                    ImGui::TableSetupColumn(Localization::GetText("quantity"));
                    ImGui::TableSetupColumn(Localization::GetText("value"));
                    ImGui::TableSetupColumn(Localization::GetText("magic_find"));
                    ImGui::TableHeadersRow();

                    for (const auto& drop : dropsToShow)
                    {
                        if (s_DetailsView == 0 && drop.isCurrency)
                            continue;
                        if (s_DetailsView == 1 && !drop.isCurrency)
                            continue;

                        // Apply search filter
                        if (searchFilter[0] != '\0')
                        {
                            std::string searchLower = searchFilter;
                            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                            std::string itemNameLower = drop.itemName;
                            std::transform(itemNameLower.begin(), itemNameLower.end(), itemNameLower.begin(), ::tolower);
                            if (itemNameLower.find(searchLower) == std::string::npos)
                                continue;
                        }

                        float iconSz = static_cast<float>((std::min)(g_Settings.iconSize, 24));
                        float rowH = UICommon::CalcTableRowHeight(iconSz);
                        ImGui::TableNextRow(0, rowH);

                        // Time
                        ImGui::TableSetColumnIndex(0);
                        UICommon::AlignTableCellText(rowH);
                        if (!drop.timestamp.empty())
                        {
                            // Extract HH:MM:SS from timestamp
                            std::string timeStr = drop.timestamp;
                            size_t spacePos = timeStr.find(' ');
                            if (spacePos != std::string::npos)
                                timeStr = timeStr.substr(spacePos + 1);
                            ImGui::Text("%s", timeStr.c_str());
                        }
                        else
                        {
                            ImGui::TextUnformatted("-");
                        }

                        // Item
                        ImGui::TableSetColumnIndex(1);
                        float cellY = ImGui::GetCursorPosY();
                        std::string iconUrl = drop.iconUrl;
                        if (iconUrl.empty())
                        {
                            if (drop.isCurrency)
                            {
                                Stat cst = ItemTracker::GetCurrencyStat(drop.itemId);
                                if (cst.details.loaded && !cst.details.iconUrl.empty())
                                    iconUrl = cst.details.iconUrl;
                                else
                                {
                                    Stat st = ItemTracker::GetItemStat(drop.itemId);
                                    if (st.details.loaded && !st.details.iconUrl.empty())
                                        iconUrl = st.details.iconUrl;
                                }
                            }
                            else
                            {
                                Stat st = ItemTracker::GetItemStat(drop.itemId);
                                if (st.details.loaded && !st.details.iconUrl.empty())
                                    iconUrl = st.details.iconUrl;
                                else
                                {
                                    Stat cst = ItemTracker::GetCurrencyStat(drop.itemId);
                                    if (cst.details.loaded && !cst.details.iconUrl.empty())
                                        iconUrl = cst.details.iconUrl;
                                }
                            }
                        }
                        if (drop.itemId == 1 && iconUrl.empty())
                            iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";

                        ImGui::SetCursorPosY(cellY);
                        UICommon::AlignTableCellIcon(rowH, iconSz);
                        UICommon::DrawItemIconCell(drop.itemId, iconUrl, iconSz, drop.rarity);
                        ImGui::SameLine();
                        ImGui::SetCursorPosY(cellY);
                        UICommon::AlignTableCellText(rowH);
                        ImGui::Text("%s", drop.itemName.c_str());

                        // Quantity
                        ImGui::TableSetColumnIndex(2);
                        UICommon::AlignTableCellText(rowH);
                        ImGui::Text("%d", drop.count);

                        // Value
                        ImGui::TableSetColumnIndex(3);
                        UICommon::AlignTableCellText(rowH);
                        ImGui::Text("%s", UICommon::FormatCoin(drop.totalValue).c_str());

                        // Magic Find
                        ImGui::TableSetColumnIndex(4);
                        UICommon::AlignTableCellText(rowH);
                        if (drop.magicFind >= 0)
                            ImGui::Text("%d%%", drop.magicFind);
                        else
                            ImGui::TextUnformatted("-");
                    }

                    ImGui::EndTable();
                }

                ImGui::Spacing();

                // Export CSV button
                if (ImGui::Button(Localization::GetText("export_csv")))
                {
                    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
                    std::string filename = std::string(addonDir ? addonDir : "") + "\\session_details_export.csv";

                    FILE* file = nullptr;
                    fopen_s(&file, filename.c_str(), "w");
                    if (file)
                    {
                        fprintf(file, "Time,Item,Quantity,Value,MagicFind\n");
                        for (const auto& drop : dropsToShow)
                        {
                            if (s_DetailsView == 0 && drop.isCurrency)
                                continue;
                            if (s_DetailsView == 1 && !drop.isCurrency)
                                continue;

                            if (searchFilter[0] != '\0')
                            {
                                std::string searchLower = searchFilter;
                                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                                std::string itemNameLower = drop.itemName;
                                std::transform(itemNameLower.begin(), itemNameLower.end(), itemNameLower.begin(), ::tolower);
                                if (itemNameLower.find(searchLower) == std::string::npos)
                                    continue;
                            }

                            std::string timeStr = drop.timestamp.empty() ? "-" : drop.timestamp;
                            fprintf(file, "%s,%s,%d,%lld,%d\n", timeStr.c_str(), drop.itemName.c_str(), drop.count, drop.totalValue, drop.magicFind);
                        }
                        fclose(file);
                    }
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("export_csv_tooltip"));
            }

            ImGui::Spacing();
            if (ImGui::Button(Localization::GetText("cancel")))
            {
                s_ShowSessionDetails = false;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("cancel_tooltip"));
        }
        ImGui::End(); // Must always be called after Begin(), even if Begin() returns false
    }
}

} // namespace UISessionHistory
