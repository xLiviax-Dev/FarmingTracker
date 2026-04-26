#include "ui_debug.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "gw2_api.h"
#include "gw2_fetcher.h"
#include "ignored_items.h"
#include "localization.h"
#include "shared.h"
#include <windows.h>
#include <psapi.h>
#include <ctime>

namespace UIDebug
{
void RenderDebugTab()
{
    ImGui::Text("%s", Localization::GetText("debug_information"));
    ImGui::Separator();

    // DRF Status
    char drfStatusLabel[256];
    snprintf(drfStatusLabel, sizeof(drfStatusLabel), Localization::GetText("drf_status_label"), UICommon::StatusText(DrfClient::GetStatus()));
    ImGui::Text("%s", drfStatusLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("drf_status_tooltip"));
    char drfReconnectLabel[256];
    snprintf(drfReconnectLabel, sizeof(drfReconnectLabel), Localization::GetText("drf_reconnect_count_label"), DrfClient::GetReconnectCount());
    ImGui::Text("%s", drfReconnectLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("drf_reconnect_count_tooltip"));

    // GW2 API Status
    const char* gw2StatusText = Localization::GetText("status_unknown");
    switch (Gw2Fetcher::GetStatus())
    {
        case Gw2Fetcher::Gw2Status::Disconnected: gw2StatusText = Localization::GetText("status_disconnected"); break;
        case Gw2Fetcher::Gw2Status::Connected: gw2StatusText = Localization::GetText("status_connected"); break;
        case Gw2Fetcher::Gw2Status::Error: gw2StatusText = Localization::GetText("status_error"); break;
    }
    char gw2ApiStatusLabel[256];
    snprintf(gw2ApiStatusLabel, sizeof(gw2ApiStatusLabel), Localization::GetText("gw2_api_status_label"), gw2StatusText);
    ImGui::Text("%s", gw2ApiStatusLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("gw2_api_status_tooltip"));
    char gw2ApiReconnectLabel[256];
    snprintf(gw2ApiReconnectLabel, sizeof(gw2ApiReconnectLabel), Localization::GetText("gw2_api_reconnect_count_label"), Gw2Fetcher::GetReconnectCount());
    ImGui::Text("%s", gw2ApiReconnectLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("gw2_api_reconnect_count_tooltip"));

    // Session Info
    char sessionDurationLabel[256];
    snprintf(sessionDurationLabel, sizeof(sessionDurationLabel), Localization::GetText("session_duration_debug"), UICommon::FormatDuration(ItemTracker::GetSessionDuration().count()).c_str());
    ImGui::Text("%s", sessionDurationLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("session_duration_debug_tooltip"));

    // Items/Currencies
    auto items = ItemTracker::GetItemsCopy();
    auto currencies = ItemTracker::GetCurrenciesCopy();
    size_t totalItems = items.size();
    ImVec4 totalItemsColor = totalItems > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalItems < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(totalItemsColor, "%s %zu", Localization::GetText("total_items_label"), totalItems);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("total_items_tooltip"));
    size_t totalCurrencies = currencies.size();
    ImVec4 totalCurrenciesColor = totalCurrencies > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalCurrencies < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(totalCurrenciesColor, "%s %zu", Localization::GetText("total_currencies_label"), totalCurrencies);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("total_currencies_tooltip"));

    // Memory Usage
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        SIZE_T memUsageMB = pmc.WorkingSetSize / (1024 * 1024);
        char memUsageLabel[256];
        snprintf(memUsageLabel, sizeof(memUsageLabel), Localization::GetText("gw2_process_memory_label"), memUsageMB);
        ImGui::Text("%s", memUsageLabel);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("gw2_process_memory_tooltip"));
    }

    // API Request Count
    char apiRequestLabel[256];
    snprintf(apiRequestLabel, sizeof(apiRequestLabel), Localization::GetText("gw2_api_request_count_label"), Gw2Api::GetRequestCount());
    ImGui::Text("%s", apiRequestLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("gw2_api_request_count_tooltip"));

    // Profit Info
    long long totalProfit = ItemTracker::CalcTotalCustomProfit();
    long long tpSellProfit = ItemTracker::CalcTotalTpSellProfit();
    long long vendorProfit = ItemTracker::CalcTotalVendorProfit();
    ImVec4 totalProfitColor = totalProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (totalProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(totalProfitColor, "%s %s", Localization::GetText("total_profit_label"), UICommon::FormatCoin(totalProfit).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("total_custom_profit_tooltip"));
    ImVec4 tpSellProfitColor = tpSellProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (tpSellProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(tpSellProfitColor, "%s %s", Localization::GetText("tp_sell_profit_label"), UICommon::FormatCoin(tpSellProfit).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("tp_sell_profit_tooltip"));
    ImVec4 vendorProfitColor = vendorProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (vendorProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(vendorProfitColor, "%s %s", Localization::GetText("vendor_profit_label"), UICommon::FormatCoin(vendorProfit).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Total vendor sell profit");

    // Profit per Hour
    auto duration = ItemTracker::GetSessionDuration();
    long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
    ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (profitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(profitPerHourColor, "%s %s", Localization::GetText("profit_per_hour_label"), UICommon::FormatCoin(profitPerHour).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("profit_per_hour_calculation_tooltip"));   

    // Opportunity Cost
    long long opportunityCostProfit = ItemTracker::GetOpportunityCostProfit();
    long long opportunityCostProfitPerHour = ItemTracker::GetOpportunityCostProfitPerHour(duration);
    ImVec4 opportunityCostProfitColor = opportunityCostProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (opportunityCostProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(opportunityCostProfitColor, "%s %s", Localization::GetText("opportunity_cost_profit_label"), UICommon::FormatCoin(opportunityCostProfit).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("opportunity_cost_vs_tp_sell_tooltip"));
    ImVec4 opportunityCostProfitPerHourColor = opportunityCostProfitPerHour > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (opportunityCostProfitPerHour < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextColored(opportunityCostProfitPerHourColor, "%s %s", Localization::GetText("opportunity_cost_profit_per_hour_label"), UICommon::FormatCoin(opportunityCostProfitPerHour).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("opportunity_cost_per_hour_tooltip"));

    // Filter Status
    int ignoredItemsCount = 0;
    int ignoredCurrenciesCount = 0;
    for (auto& [id, st] : items)
        if (st.isIgnored) ignoredItemsCount++;
    for (auto& [id, st] : currencies)
        if (st.isIgnored) ignoredCurrenciesCount++;
    char ignoredItemsLabel[256];
    snprintf(ignoredItemsLabel, sizeof(ignoredItemsLabel), Localization::GetText("ignored_items_debug_label"), ignoredItemsCount);
    ImGui::Text("%s", ignoredItemsLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("ignored_items_debug_tooltip"));
    char ignoredCurrenciesLabel[256];
    snprintf(ignoredCurrenciesLabel, sizeof(ignoredCurrenciesLabel), Localization::GetText("ignored_currencies_debug_label"), ignoredCurrenciesCount);
    ImGui::Text("%s", ignoredCurrenciesLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("ignored_currencies_debug_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();

    // DRF Logs
    ImGui::Text("%s", Localization::GetText("drf_logs_label"));
    if (ImGui::Button(Localization::GetText("clear_drf_logs")))
        DrfClient::ClearLogs();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_drf_logs_tooltip"));
    ImGui::SameLine();
    ImGui::Text("%s", Localization::GetText("last_100_entries"));

    if (ImGui::BeginChild("DRFLogs", ImVec2(0, 150), true))
    {
        auto logs = DrfClient::GetLogs();
        for (auto& log : logs)
        {
            auto time_t = std::chrono::system_clock::to_time_t(log.timestamp);
            char timeStr[64];
            struct tm timeinfo;
            localtime_s(&timeinfo, &time_t);
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

            ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f);
            if (log.type == "error") color = ImVec4(1.f, 0.2f, 0.2f, 1.f);
            else if (log.type == "data") color = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
            else if (log.type == "info") color = ImVec4(0.2f, 0.6f, 1.f, 1.f);

            ImGui::TextColored(color, "[%s] [%s] %s", timeStr, log.type.c_str(), log.message.c_str());
        }
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // GW2 API Logs
    ImGui::Text("%s", Localization::GetText("gw2_api_logs_label"));
    if (ImGui::Button(Localization::GetText("clear_gw2_logs")))
        Gw2Api::ClearLogs();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("clear_gw2_logs_tooltip"));
    ImGui::SameLine();
    ImGui::Text("%s", Localization::GetText("last_100_entries"));

    if (ImGui::BeginChild("GW2Logs", ImVec2(0, 150), true))
    {
        auto logs = Gw2Api::GetLogs();
        for (auto& log : logs)
        {
            auto time_t = std::chrono::system_clock::to_time_t(log.timestamp);
            char timeStr[64];
            struct tm timeinfo;
            localtime_s(&timeinfo, &time_t);
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

            ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f);
            if (log.type == "error") color = ImVec4(1.f, 0.2f, 0.2f, 1.f);
            else if (log.type == "request") color = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
            else if (log.type == "info") color = ImVec4(0.2f, 0.6f, 1.f, 1.f);

            ImGui::TextColored(color, "[%s] [%s] %s", timeStr, log.type.c_str(), log.message.c_str());
        }
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Item/Currency Details
    ImGui::Text("%s", Localization::GetText("first_5_tracked_items"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("first_5_tracked_items_tooltip"));
    int itemCount = 0;
    for (auto& [id, st] : items)
    {
        if (itemCount >= 5) break;
        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Text("Item %d: %s (Count: ", id, st.details.loaded ? st.details.name.c_str() : "Loading...");
        ImGui::SameLine();
        ImGui::TextColored(countColor, "%lld", st.count);
        ImGui::SameLine();
        ImGui::Text(", Loaded: %s)", st.details.loaded ? "Yes" : "No");
        itemCount++;
    }

    int currencyCount = 0;
    for (auto& [id, st] : currencies)
    {
        if (currencyCount >= 5) break;
        std::string name = st.details.loaded ? st.details.name : (id == 1 ? "Coin" : "Loading...");
        ImVec4 countColor = st.count > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (st.count < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Text("Currency %d: %s (Count: ", id, name.c_str());
        ImGui::SameLine();
        ImGui::TextColored(countColor, "%lld", st.count);
        ImGui::SameLine();
        ImGui::Text(", Loaded: %s)", st.details.loaded ? "Yes" : "No");
        currencyCount++;
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Custom Profit List
    ImGui::Text("%s", Localization::GetText("first_5_custom_profit"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("first_5_custom_profit_tooltip"));
    int customProfitCount = 0;
    for (auto& [id, st] : items)
    {
        if (customProfitCount >= 5) break;
        if (st.HasCustomProfit())
        {
            long long customProfit = st.GetCustomProfit();
            ImVec4 customProfitColor = customProfit > 0 ? ImVec4(1.f, 0.84f, 0.f, 1.f) : (customProfit < 0 ? ImVec4(0.9f, 0.2f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Text("Item %d: %s (Custom Profit: ", id, st.details.loaded ? st.details.name.c_str() : "Loading...");
            ImGui::SameLine();
            ImGui::TextColored(customProfitColor, "%s", UICommon::FormatCoin(customProfit).c_str());
            ImGui::SameLine();
            ImGui::Text(")");
            customProfitCount++;
        }
    }
    if (customProfitCount == 0)
        ImGui::Text("(No custom profit items)");

    ImGui::Spacing();
    ImGui::Separator();

    // Ignored Items List
    ImGui::Text("%s", Localization::GetText("first_5_ignored_items"));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("first_5_ignored_items_tooltip"));
    int ignoredCount = 0;
    for (auto& [id, st] : items)
    {
        if (ignoredCount >= 5) break;
        if (st.isIgnored)
        {
            ImGui::Text("Item %d: %s", id, st.details.loaded ? st.details.name.c_str() : "Loading...");
            ignoredCount++;
        }
    }
    if (ignoredCount == 0)
        ImGui::Text("(No ignored items)");

    ImGui::Spacing();
    ImGui::Separator();

    // Settings
    ImGui::Text("%s", Localization::GetText("settings_label"));
    char apiKeyLabel[256];
    snprintf(apiKeyLabel, sizeof(apiKeyLabel), Localization::GetText("api_key_label"), g_Settings.gw2ApiKey.empty() ? Localization::GetText("not_set") : Localization::GetText("set"));
    ImGui::Text("%s", apiKeyLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("api_key_tooltip"));
    char drfTokenLabel[256];
    snprintf(drfTokenLabel, sizeof(drfTokenLabel), Localization::GetText("drf_token_label"), g_Settings.drfToken.empty() ? Localization::GetText("not_set") : Localization::GetText("set"));
    ImGui::Text("%s", drfTokenLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("drf_token_tooltip"));
    char toggleHotkeyLabel[256];
    snprintf(toggleHotkeyLabel, sizeof(toggleHotkeyLabel), Localization::GetText("toggle_hotkey_label"), g_Settings.toggleHotkey.c_str());
    ImGui::Text("%s", toggleHotkeyLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("toggle_hotkey_tooltip"));
    char autoResetModeLabel[256];
    snprintf(autoResetModeLabel, sizeof(autoResetModeLabel), Localization::GetText("auto_reset_mode_label"), g_Settings.automaticResetMode);
    ImGui::Text("%s", autoResetModeLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("auto_reset_mode_tooltip"));
    char nextResetLabel[256];
    snprintf(nextResetLabel, sizeof(nextResetLabel), Localization::GetText("next_reset_label"), g_Settings.nextResetDateTimeUtc.c_str());
    ImGui::Text("%s", nextResetLabel);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("next_reset_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();

    // Fake DRF Server
    ImGui::Text("%s", Localization::GetText("fake_drf_server_label"));
    ImGui::Checkbox(Localization::GetText("use_fake_drf_server"), &g_Settings.useFakeDrfServer);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("use_fake_drf_server_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();

    // Reset
    if (ImGui::Button(Localization::GetText("reset_all_data")))
    {
        ItemTracker::Reset();
        const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
        ItemTracker::ClearPersistedData(addonDir);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("reset_all_data_tooltip"));
}
}
