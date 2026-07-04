#include "ui_debug.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "gw2_api.h"
#include "gw2_fetcher.h"
#include "ignored_items.h"
#include "custom_profit.h"
#include "localization.h"
#include "shared.h"
#include "ui_common.h"
#include "../include/imgui/imgui.h"
#include <windows.h>
#include <psapi.h>
#include <ctime>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

namespace UIDebug
{

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ImVec4 ProfitColor(long long v)
{
    if (v > 0) return ImVec4(0.78f, 0.60f, 0.18f, 1.f);
    if (v < 0) return ImVec4(0.90f, 0.20f, 0.20f, 1.f);
    return ImVec4(1.f, 1.f, 1.f, 1.f);
}

static ImVec4 CountColor(long long v)
{
    if (v > 0) return ImVec4(0.78f, 0.60f, 0.18f, 1.f);
    if (v < 0) return ImVec4(0.90f, 0.20f, 0.20f, 1.f);
    return ImVec4(1.f, 1.f, 1.f, 1.f);
}

static ImVec4 DrfStatusColor(DrfStatus s)
{
    switch (s)
    {
        case DrfStatus::Connected:    return ImVec4(0.20f, 0.75f, 0.35f, 1.f);
        case DrfStatus::Connecting:   return ImVec4(0.85f, 0.65f, 0.10f, 1.f);
        case DrfStatus::Disconnected: return ImVec4(0.90f, 0.20f, 0.20f, 1.f);
        default:                      return ImVec4(1.f, 1.f, 1.f, 1.f);
    }
}

static ImVec4 Gw2StatusColor(Gw2Fetcher::Gw2Status s)
{
    switch (s)
    {
        case Gw2Fetcher::Gw2Status::Connected:    return ImVec4(0.20f, 0.75f, 0.35f, 1.f);
        case Gw2Fetcher::Gw2Status::Connecting:   return ImVec4(0.85f, 0.65f, 0.10f, 1.f);
        case Gw2Fetcher::Gw2Status::Disconnected: return ImVec4(0.90f, 0.20f, 0.20f, 1.f);
        case Gw2Fetcher::Gw2Status::Error:        return ImVec4(0.90f, 0.20f, 0.20f, 1.f);
        default:                                   return ImVec4(0.55f, 0.55f, 0.55f, 1.f);
    }
}

// Filled circle dot, vertically centred on current line
static void StatusDot(ImVec4 color)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    float  r = 5.f;
    float  cy = p.y + ImGui::GetTextLineHeight() * 0.5f;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(p.x + r, cy), r,
        ImGui::ColorConvertFloat4ToU32(color), 16);
    ImGui::Dummy(ImVec2(r * 2.f + 6.f, ImGui::GetTextLineHeight()));
    ImGui::SameLine(0, 0);
}

// Thin separator + muted label as a section divider
static void SectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::Separator();
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

// Gradient background helper — drawn on parent draw list before BeginChild
static void DrawGradientBox(ImVec2 min, ImVec2 max)
{
    const float acR=g_Settings.accentColorR, acG=g_Settings.accentColorG, acB=g_Settings.accentColorB;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
    ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
    ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
    dl->AddRectFilledMultiColor(min, max, top, top, bot, bot);
    dl->AddRect(min, max, border, 4.f, 0, 0.5f);
    dl->AddRectFilled({min.x,min.y},{min.x+3.f,max.y},
        ImGui::ColorConvertFloat4ToU32(ImVec4(acR,acG,acB,1.f)),2.f);
}

// Small framed metric card: muted label above, large value below
static void MetricCard(const char* childId, const char* label, const char* value,
                       ImVec4 valueColor, float width)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    DrawGradientBox(pos, {pos.x+width, pos.y+54.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    if (ImGui::BeginChild(childId, ImVec2(width, 54.f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
        float labelWidth = ImGui::CalcTextSize(label).x;
        float centerX = (width - labelWidth) * 0.5f;
        ImGui::SetCursorPosX(centerX);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f,1.f,1.f,1.f));
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        float valueWidth = ImGui::CalcTextSize(value).x * 1.15f;
        centerX = (width - valueWidth) * 0.5f;
        ImGui::SetCursorPosX(centerX);
        ImGui::PushStyleColor(ImGuiCol_Text, valueColor);
        ImGui::SetWindowFontScale(1.15f);
        ImGui::TextUnformatted(value);
        ImGui::SetWindowFontScale(1.f);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// Framed profit card: muted label, gold/red coin value, tiny sublabel
static void ProfitCard(const char* childId, const char* label,
                       long long coinValue, const char* sublabel, float width)
{
    std::string valStr = UICommon::FormatCoin(coinValue);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    DrawGradientBox(pos, {pos.x+width, pos.y+110.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    if (ImGui::BeginChild(childId, ImVec2(width, 110.f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
        float labelWidth = ImGui::CalcTextSize(label).x;
        float centerX = (width - labelWidth) * 0.5f;
        ImGui::SetCursorPosX(centerX);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f,1.f,1.f,1.f));
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        float valueWidth = ImGui::CalcTextSize(valStr.c_str()).x;
        centerX = (width - valueWidth) * 0.5f;
        ImGui::SetCursorPosX(centerX);
        ImGui::PushStyleColor(ImGuiCol_Text, ProfitColor(coinValue));
        ImGui::TextUnformatted(valStr.c_str());
        ImGui::PopStyleColor();
        float sublabelWidth = ImGui::CalcTextSize(sublabel).x * 0.85f;
        centerX = (width - sublabelWidth) * 0.5f;
        ImGui::SetCursorPosX(centerX);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f,1.f,1.f,1.f));
        ImGui::SetWindowFontScale(0.85f);
        ImGui::TextUnformatted(sublabel);
        ImGui::SetWindowFontScale(1.f);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// Key/value row with muted left label, value right-aligned at colW
static void DataRow(const char* label, const char* value, float colW, const char* tooltip = nullptr)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    ImGui::TextUnformatted(value);
    if (tooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);
}

static void BoolRow(const char* label, bool set, float colW, const char* tooltip = nullptr)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    if (set)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.75f, 0.35f, 1.f));
        ImGui::TextUnformatted(Localization::GetText("set"));
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.25f, 0.25f, 1.f));
        ImGui::TextUnformatted(Localization::GetText("not_set"));
    }
    ImGui::PopStyleColor();
    if (tooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);
}

// Log panel renderer (templated so it works for both DrfClient and Gw2Api log vectors)
template<typename LogVec>
static void RenderLogPanel(const char* childId, const LogVec& logs)
{
    ImVec2 pos   = ImGui::GetCursorScreenPos();
    float  avail = ImGui::GetContentRegionAvail().x;
    DrawGradientBox(pos, {pos.x + avail, pos.y + 180.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    if (ImGui::BeginChild(childId, ImVec2(0.f, 180.f), false))
    {
        for (auto& log : logs)
        {
            auto tt = std::chrono::system_clock::to_time_t(log.timestamp);
            char ts[32]; struct tm ti{};
            localtime_s(&ti, &tt);
            strftime(ts, sizeof(ts), "%H:%M:%S", &ti);

            // Timestamp
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextUnformatted(ts);
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 6);

            // Coloured tag
            ImVec4 tagColor = ImVec4(1.f, 1.f, 1.f, 1.f);
            if      (log.type == "error")   tagColor = ImVec4(0.90f, 0.25f, 0.25f, 1.f);
            else if (log.type == "data")    tagColor = ImVec4(0.20f, 0.78f, 0.35f, 1.f);
            else if (log.type == "info")    tagColor = ImVec4(0.25f, 0.60f, 1.00f, 1.f);
            else if (log.type == "request") tagColor = ImVec4(0.20f, 0.78f, 0.35f, 1.f);
            else if (log.type == "warn")    tagColor = ImVec4(0.85f, 0.65f, 0.10f, 1.f);

            std::string tag = "[" + log.type + "]";
            ImGui::PushStyleColor(ImGuiCol_Text, tagColor);
            ImGui::TextUnformatted(tag.c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 6);

            ImGui::TextUnformatted(log.message.c_str());
        }

        // Auto-scroll to bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4.f)
            ImGui::SetScrollHereY(1.f);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// Export logs to file
template<typename LogVec>
static void ExportLogs(const LogVec& logs, const char* filename, const char* sourceName)
{
    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
    if (!addonDir) return;
    std::string path = std::string(addonDir) + "\\" + filename;
    std::ofstream f(path);
    if (!f.is_open()) return;
    for (auto& log : logs)
    {
        auto tt = std::chrono::system_clock::to_time_t(log.timestamp);
        char ts[32]; struct tm ti{};
        localtime_s(&ti, &tt);
        strftime(ts, sizeof(ts), "%H:%M:%S", &ti);
        f << "[" << ts << "] [" << log.type << "] " << log.message << "\n";
    }
    f.close();
    DrfClient::Log(std::string(sourceName) + " logs exported to: " + path, "info");
}

// ---------------------------------------------------------------------------
// Static state
// ---------------------------------------------------------------------------
static int s_ActiveLogTab = 0; // 0 = DRF, 1 = GW2 API

// ---------------------------------------------------------------------------
// Main render function
// ---------------------------------------------------------------------------
void RenderDebugTab()
{
    const float availW = ImGui::GetContentRegionAvail().x;
    const float colW   = availW * 0.42f; // label column width for data rows

    // ====================================================================
    // SECTION 1 — Connection Status
    // ====================================================================
    SectionHeader(Localization::GetText("debug_connection_status"));

    // Two-column layout: DRF left, GW2 API right
    const float halfW = (availW - 12.f) * 0.5f;

    // --- DRF ---
    ImGui::BeginGroup();
    {
        DrfStatus   drfSt    = DrfClient::GetStatus();
        ImVec4      drfColor = DrfStatusColor(drfSt);
        const char* drfText  = UICommon::StatusText(drfSt);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextUnformatted("DRF");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        StatusDot(drfColor);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, drfColor);
        ImGui::TextUnformatted(drfText);
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("drf_status_tooltip"));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        char drfReconnectBuf[128];
        snprintf(drfReconnectBuf, sizeof(drfReconnectBuf), Localization::GetText("drf_reconnect_count_label"), DrfClient::GetReconnectCount());
        ImGui::Text("%s", drfReconnectBuf);
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("drf_reconnect_count_tooltip"));
    }
    ImGui::EndGroup();

    ImGui::SameLine(halfW + 12.f);

    // --- GW2 API ---
    ImGui::BeginGroup();
    {
        auto        gw2St    = Gw2Fetcher::GetStatus();
        ImVec4      gw2Color = Gw2StatusColor(gw2St);
        const char* gw2Text  = Localization::GetText("status_unknown");
        switch (gw2St)
        {
            case Gw2Fetcher::Gw2Status::Disconnected: gw2Text = Localization::GetText("status_disconnected"); break;
            case Gw2Fetcher::Gw2Status::Connecting:   gw2Text = Localization::GetText("status_connecting");   break;
            case Gw2Fetcher::Gw2Status::Connected:    gw2Text = Localization::GetText("status_connected");    break;
            case Gw2Fetcher::Gw2Status::Error:        gw2Text = Localization::GetText("status_error");        break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextUnformatted("GW2 API");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        StatusDot(gw2Color);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, gw2Color);
        ImGui::TextUnformatted(gw2Text);
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("gw2_api_status_tooltip"));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        char gw2ReconnectBuf[128];
        snprintf(gw2ReconnectBuf, sizeof(gw2ReconnectBuf), Localization::GetText("gw2_api_reconnect_count_label"), Gw2Fetcher::GetReconnectCount());
        ImGui::Text("%s", gw2ReconnectBuf);
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("gw2_api_reconnect_count_tooltip"));
    }
    ImGui::EndGroup();

    // ====================================================================
    // SECTION 2 — Session Snapshot (4 metric cards)
    // ====================================================================
    SectionHeader(Localization::GetText("debug_session_snapshot"));

    auto items      = ItemTracker::GetFilteredItems();
    auto currencies = ItemTracker::GetFilteredCurrencies();
    auto duration   = ItemTracker::GetSessionDuration();

    std::string durStr   = UICommon::FormatDuration(duration.count());
    std::string itemsStr = std::to_string(items.size());
    std::string currStr  = std::to_string(currencies.size());
    std::string reqStr   = std::to_string(Gw2Api::GetRequestCount());

    PROCESS_MEMORY_COUNTERS pmc{};
    std::string memStr = "—";
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        memStr = std::to_string(pmc.WorkingSetSize / (1024 * 1024)) + " MB";

    const float cardGap = 6.f;
    const float cardW4  = (availW - cardGap * 3.f) / 4.f;

    ImVec4 goldColor = ImVec4(0.78f, 0.60f, 0.18f, 1.f);
    ImVec4 blueColor = ImVec4(0.25f, 0.60f, 1.00f, 1.f);
    ImVec4 normColor = ImVec4(1.f, 1.f, 1.f, 1.f);

    MetricCard("##mc_dur",  Localization::GetText("session_duration_debug"),       durStr.c_str(),   blueColor, cardW4);
    ImGui::SameLine(0, cardGap);
    MetricCard("##mc_itm",  Localization::GetText("total_items_label"),            itemsStr.c_str(), goldColor, cardW4);
    ImGui::SameLine(0, cardGap);
    MetricCard("##mc_cur",  Localization::GetText("total_currencies_label"),       currStr.c_str(),  goldColor, cardW4);
    ImGui::SameLine(0, cardGap);
    MetricCard("##mc_req",  Localization::GetText("gw2_api_request_count_label"),  reqStr.c_str(),   normColor, cardW4);

    ImGui::Spacing();
    MetricCard("##mc_mem",  Localization::GetText("gw2_process_memory_label"),     memStr.c_str(),   normColor, cardW4);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("gw2_process_memory_tooltip"));

    // ====================================================================
    // SECTION 3 — Profit Breakdown (3 × 2 cards)
    // ====================================================================
    SectionHeader(Localization::GetText("debug_profit_breakdown"));

    long long totalProfit   = ItemTracker::CalcTotalCustomProfit();
    long long tpSellProfit  = ItemTracker::CalcTotalTpSellProfit();
    long long vendorProfit  = ItemTracker::CalcTotalVendorProfit();
    long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
    long long oppCost       = ItemTracker::GetOpportunityCostProfit();
    long long oppCostPerH   = ItemTracker::GetOpportunityCostProfitPerHour(duration);

    const float pcGap = 6.f;
    const float pcW   = (availW - pcGap * 2.f) / 3.f;

    ProfitCard("##pc_tot", Localization::GetText("total_profit_label"),
               totalProfit,   Localization::GetText("debug_total_session"),  pcW);
    ImGui::SameLine(0, pcGap);
    ProfitCard("##pc_tp",  Localization::GetText("tp_sell_profit_label"),
               tpSellProfit,  Localization::GetText("debug_after_tp_fee"),   pcW);
    ImGui::SameLine(0, pcGap);
    ProfitCard("##pc_ven", Localization::GetText("vendor_profit_label"),
               vendorProfit,  Localization::GetText("debug_direct_sell"),    pcW);

    ImGui::Spacing();

    ProfitCard("##pc_pph", Localization::GetText("profit_per_hour_label"),
               profitPerHour, Localization::GetText("debug_rolling_avg"),    pcW);
    ImGui::SameLine(0, pcGap);
    ProfitCard("##pc_opp", Localization::GetText("opportunity_cost_profit_label"),
               oppCost,       Localization::GetText("debug_vs_tp_sell"),     pcW);
    ImGui::SameLine(0, pcGap);
    ProfitCard("##pc_oph", Localization::GetText("opportunity_cost_profit_per_hour_label"),
               oppCostPerH,   Localization::GetText("debug_per_hour"),       pcW);

    // ====================================================================
    // SECTION 4 — Data State
    // ====================================================================
    SectionHeader(Localization::GetText("debug_data_state"));

    int ignoredItemCnt = 0, ignoredCurrCnt = 0;
    for (auto& [id, st] : items)      if (st.isIgnored) ignoredItemCnt++;
    for (auto& [id, st] : currencies) if (st.isIgnored) ignoredCurrCnt++;

    auto favItemIds = ItemTracker::GetFavoriteItemIds();
    auto favCurrIds = ItemTracker::GetFavoriteCurrencyIds();

    int cpCount = 0;
    for (auto& [id, st] : items) if (st.HasCustomProfit()) cpCount++;

    bool apiKeySet = !SettingsManager::GetCurrentGw2ApiKey().empty();
    bool drfTokSet = !SettingsManager::GetCurrentDrfToken().empty();

    char ignoredBuf[64]; snprintf(ignoredBuf, sizeof(ignoredBuf), "%d items · %d currencies", ignoredItemCnt, ignoredCurrCnt);
    char favBuf[64];     snprintf(favBuf,     sizeof(favBuf),     "%d items · %d currencies", (int)favItemIds.size(), (int)favCurrIds.size());
    char cpBuf[32];      snprintf(cpBuf,      sizeof(cpBuf),      "%d items", cpCount);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted("Ignored Items");
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(ignoredBuf);
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("ignored_items_debug_tooltip"));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted("Favorites");
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(favBuf);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted("First 5 Custom Profit");
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(cpBuf);
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("first_5_custom_profit_tooltip"));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted("API Key");
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    if (apiKeySet)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.75f, 0.35f, 1.f));
        ImGui::TextUnformatted("Set");
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.25f, 0.25f, 1.f));
        ImGui::TextUnformatted("Not Set");
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("api_key_tooltip"));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted("DRF Token");
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    if (drfTokSet)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.75f, 0.35f, 1.f));
        ImGui::TextUnformatted("Set");
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.25f, 0.25f, 1.f));
        ImGui::TextUnformatted("Not Set");
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("drf_token_tooltip"));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted("Next Reset");
    ImGui::PopStyleColor();
    ImGui::SameLine(colW);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(g_Settings.nextResetDateTimeUtc.c_str());
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("next_reset_tooltip"));

    // First 5 items preview
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::TextUnformatted(Localization::GetText("first_5_tracked_items"));
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("first_5_tracked_items_tooltip"));

    int shown = 0;
    for (auto& [id, st] : items)
    {
        if (shown >= 5) break;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Text("  #%d  %s", id, st.details.loaded ? st.details.name.c_str() : Localization::GetText("loading"));
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextColored(CountColor(st.count), "x%lld", st.count);
        shown++;
    }
    if (shown == 0)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextUnformatted("  —");
        ImGui::PopStyleColor();
    }

    // ====================================================================
    // SECTION 5 — Logs (tabbed)
    // ====================================================================
    SectionHeader(Localization::GetText("debug_logs"));

    // Tab buttons
    auto TabBtn = [&](const char* label, int idx)
    {
        bool active = (s_ActiveLogTab == idx);
        const float acR = g_Settings.accentColorR;
        const float acG = g_Settings.accentColorG;
        const float acB = g_Settings.accentColorB;

        ImVec2 size = ImGui::CalcTextSize(label);
        size.x += 16.f;
        size.y += 8.f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cursor = ImGui::GetCursorScreenPos();

        if (active)
        {
            // Active tab gradient
            ImU32 bgTop = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
            ImU32 bgBot = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));
            ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));

            dl->AddRectFilledMultiColor(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y),
                bgTop, bgTop, bgBot, bgBot);
            dl->AddRect(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), border, 4.f, 0, 0.5f);
        }
        else
        {
            // Inactive tab
            ImU32 bg = IM_COL32(30, 30, 30, 255);
            ImU32 border = IM_COL32(60, 60, 60, 255);

            dl->AddRectFilled(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), bg, 4.f);
            dl->AddRect(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), border, 4.f, 0, 0.5f);
        }

        ImGui::InvisibleButton(("##tab_" + std::to_string(idx)).c_str(), size);
        if (ImGui::IsItemClicked())
            s_ActiveLogTab = idx;

        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos = ImVec2(cursor.x + (size.x - textSize.x) * 0.5f + 5.f, cursor.y + (size.y - textSize.y) * 0.5f);
        dl->AddText(textPos, active ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 180, 180, 255), label);

        ImGui::SameLine(0, 4);
    };

    TabBtn(Localization::GetText("drf_logs_label"),     0);
    TabBtn(Localization::GetText("gw2_api_logs_label"), 1);
    ImGui::NewLine();
    ImGui::Spacing();

    if (s_ActiveLogTab == 0)
    {
        if (UICommon::RedGradientButton(Localization::GetText("clear_drf_logs"), "##clear_drf_logs"))
            DrfClient::ClearLogs();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("clear_drf_logs_tooltip"));
        ImGui::SameLine(0, 6);
        if (UICommon::OrangeGradientButton(Localization::GetText("export_logs"), "##export_drf_logs"))
            ExportLogs(DrfClient::GetLogs(), "drf_logs.txt", "DRF");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextUnformatted(Localization::GetText("last_100_entries"));
        ImGui::PopStyleColor();
        ImGui::Spacing();
        RenderLogPanel("##DRFLogs", DrfClient::GetLogs());
    }
    else
    {
        if (UICommon::RedGradientButton(Localization::GetText("clear_gw2_logs"), "##clear_gw2_logs"))
            Gw2Api::ClearLogs();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("clear_gw2_logs_tooltip"));
        ImGui::SameLine(0, 6);
        if (UICommon::OrangeGradientButton(Localization::GetText("export_logs"), "##export_gw2_logs"))
            ExportLogs(Gw2Api::GetLogs(), "gw2_api_logs.txt", "GW2 API");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::TextUnformatted(Localization::GetText("last_100_entries"));
        ImGui::PopStyleColor();
        ImGui::Spacing();
        RenderLogPanel("##GW2Logs", Gw2Api::GetLogs());
    }

    // ====================================================================
    // SECTION 6 — Developer Options
    // ====================================================================
    SectionHeader(Localization::GetText("fake_drf_server_label"));

    if (ImGui::Checkbox(Localization::GetText("use_fake_drf_server"), &g_Settings.useFakeDrfServer))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("use_fake_drf_server_tooltip"));

    ImGui::Spacing();

    // Danger-coloured reset button — red gradient design
    if (UICommon::RedGradientButton(Localization::GetText("reset_all_data"), "##reset_all"))
    {
        ItemTracker::SafeReset();
        const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
        ItemTracker::ClearPersistedData(addonDir);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("reset_all_data_tooltip"));

    ImGui::Spacing();
}

} // namespace UIDebug
