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

    // Save Current Session Button
    if (ImGui::Button(Localization::GetText("save_current_session")))
    {
        ItemTracker::SaveCurrentSession();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("save_current_session_tooltip"));

    ImGui::SameLine();

    // Clear History Button
    if (ImGui::Button(Localization::GetText("clear_history")))
    {
        ImGui::OpenPopup("ClearHistoryConfirm");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_history_tooltip"));

    ImGui::SameLine();

    // Export History Button
    if (ImGui::Button(Localization::GetText("export_history")))
    {
        ImGui::OpenPopup("ExportHistoryConfirm");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("export_history_tooltip"));

    if (ImGui::BeginPopup("ExportHistoryConfirm"))
    {
        static char exportPath[MAX_PATH] = "";
        if (exportPath[0] == '\0')
        {
            strcpy_s(exportPath, "session_history_export.json");
        }
        ImGui::Text("%s", Localization::GetText("export_history"));
        ImGui::InputText("##ExportHistoryPath", exportPath, sizeof(exportPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export")))
        {
            std::string jsonData = SessionHistory::ExportToJson();
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string filename = std::string(addonDir ? addonDir : "") + "\\" + exportPath;

            FILE* file = nullptr;
            fopen_s(&file, filename.c_str(), "w");
            if (file)
            {
                fprintf(file, "%s", jsonData.c_str());
                fclose(file);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    // Import History Button
    if (ImGui::Button(Localization::GetText("import_history")))
    {
        ImGui::OpenPopup("ImportHistoryConfirm");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("import_history_tooltip"));

    if (ImGui::BeginPopup("ImportHistoryConfirm"))
    {
        static char importPath[MAX_PATH] = "";
        if (importPath[0] == '\0')
        {
            strcpy_s(importPath, "session_history_export.json");
        }
        ImGui::Text("%s", Localization::GetText("import_history"));
        ImGui::InputText("##ImportHistoryPath", importPath, sizeof(importPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import")))
        {
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string filename = std::string(addonDir ? addonDir : "") + "\\" + importPath;

            FILE* file = nullptr;
            fopen_s(&file, filename.c_str(), "r");
            if (file)
            {
                fseek(file, 0, SEEK_END);
                long fileSize = ftell(file);
                fseek(file, 0, SEEK_SET);

                std::string jsonData;
                jsonData.resize(fileSize);
                fread(&jsonData[0], 1, fileSize, file);
                fclose(file);

                if (SessionHistory::ImportFromJson(jsonData))
                {
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("ClearHistoryConfirm"))
    {
        ImGui::Text("%s", Localization::GetText("clear_history_confirm"));
        ImGui::Text("%s", Localization::GetText("clear_history_warning"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("yes_clear")))
        {
            SessionHistory::ClearHistory();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
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
    if (ImGui::BeginTable("SessionHistoryTable", 7, ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(Localization::GetText("date"));
        ImGui::TableSetupColumn(Localization::GetText("duration"));
        ImGui::TableSetupColumn(Localization::GetText("profit"));
        ImGui::TableSetupColumn(Localization::GetText("profit_per_hour"));
        ImGui::TableSetupColumn(Localization::GetText("drops"));
        ImGui::TableSetupColumn(Localization::GetText("best_drop"));
        ImGui::TableSetupColumn(Localization::GetText("map"));
        ImGui::TableHeadersRow();

        for (const auto& session : sessions)
        {
            ImGui::TableNextRow();

            // Date (start time)
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", session.startTime.c_str());

            // Duration
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", UICommon::FormatDuration(session.durationSeconds).c_str());

            // Profit
            ImGui::TableSetColumnIndex(2);
            ImVec4 profitColor = session.totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (session.totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextColored(profitColor, "%s", UICommon::FormatCoin(session.totalProfit).c_str());

            // Profit/h
            ImGui::TableSetColumnIndex(3);
            ImVec4 profitPerHourColor = session.profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (session.profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextColored(profitPerHourColor, "%s", UICommon::FormatCoin(session.profitPerHour).c_str());

            // Drops
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d", session.totalDrops);

            // Best Drop
            ImGui::TableSetColumnIndex(5);
            if (!session.topDrops.empty())
            {
                const auto& bestDrop = session.topDrops[0];
                ImGui::Text("%s (%s)", bestDrop.itemName.c_str(), UICommon::FormatCoin(bestDrop.totalValue).c_str());
            }
            else
            {
                ImGui::TextUnformatted("-");
            }

            // Map
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s", session.mapName.empty() ? Localization::GetText("unknown_map") : session.mapName.c_str());
        }

        ImGui::EndTable();
    }
}

} // namespace UISessionHistory
