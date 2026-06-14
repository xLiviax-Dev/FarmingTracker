#include "ui_profit.h"
#include "settings.h"
#include "item_tracker.h"
#include "custom_profit.h"
#include "localization.h"
#include "session_history.h"
#include "ui_common.h"
#include "drf_client.h"
#include "shared.h"
#include "ignored_items.h"
#include "ui_context_menu.h"
#include "auto_reset.h"
#include "ui_tooltips.h"
#include "ui_tab_icons.h"
#include "magnetite_tracker.h"
#include <chrono>

#define NOMINMAX
#include <algorithm>

namespace UIProfit
{
static int s_SummaryPeriod = 0;

static const ImVec4 kGold   = { 1.00f, 0.84f, 0.00f, 1.f };
static const ImVec4 kGreen  = { 0.23f, 0.62f, 0.23f, 1.f };
static const ImVec4 kRed    = { 0.90f, 0.20f, 0.20f, 1.f };
static const ImVec4 kMuted  = { 0.75f, 0.75f, 0.75f, 1.f };
static const ImVec4 kPurple = { 0.48f, 0.38f, 0.83f, 1.f };
static const ImVec4 kCardBg = { 0.12f, 0.12f, 0.12f, 1.f };

static ImVec4 ProfitColor(long long v)
{
    return v > 0 ? kGold : (v < 0 ? kRed : ImVec4(1,1,1,1));
}
static ImVec4 EfficiencyColor(float e)
{
    if (e >= 95.f) return kGreen;
    if (e >= 85.f) return { 0.50f, 1.00f, 0.00f, 1.f };
    if (e >= 70.f) return { 1.00f, 0.80f, 0.00f, 1.f };
    return kRed;
}

static void PushCard()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));
}
static void PopCard()
{
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

// 30% darker gradient box matching the Magnetite/stats bar design
static void DrawDarkGradientBox(ImVec2 min, ImVec2 max)
{
    const float d    = 0.7f;
    const float acR  = g_Settings.accentColorR * d;
    const float acG  = g_Settings.accentColorG * d;
    const float acB  = g_Settings.accentColorB * d;
    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
    ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f, acG*.5f, acB*.5f, 1.f));
    ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
    dl->AddRectFilledMultiColor(min, max, top, top, bot, bot);
    dl->AddRect(min, max, border, 6.f, 0, 0.5f);
    dl->AddRectFilled({min.x,min.y},{min.x+3.f,max.y},
        ImGui::ColorConvertFloat4ToU32(ImVec4(acR,acG,acB,1.f)),2.f);
}

static void HSep()
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    dl->AddLine(p, ImVec2(p.x+w, p.y), IM_COL32(64,64,64,200));
    ImGui::Dummy(ImVec2(w, 1.f));
}

static void KpiCard(const char* id, const char* label, const char* value,
                    ImVec4 col, const char* sub, float w, float h = 114.f,
                    const char* subValue = nullptr, ImVec4 subCol = {1,1,1,1})
{
    auto CenteredText = [&](const char* txt, ImVec4 c, float cardW)
    {
        float tw = ImGui::CalcTextSize(txt).x;
        float indent = std::max(0.f, (cardW - tw) * 0.5f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        ImGui::TextColored(c, "%s", txt);
    };

    ImVec2 pos = ImGui::GetCursorScreenPos();
    DrawDarkGradientBox(pos, {pos.x+w, pos.y+h});
    PushCard();
    ImGui::BeginChild(id, ImVec2(w, h), false, ImGuiWindowFlags_NoScrollbar);
    float inner = w - ImGui::GetStyle().WindowPadding.x * 2.f;
    CenteredText(label, ImVec4(1.f, 1.f, 1.f, 1.f), inner);
    CenteredText(value, col, inner);
    if (sub && sub[0])      CenteredText(sub,      ImVec4(1.f, 1.f, 1.f, 1.f), inner);
    if (subValue && subValue[0]) CenteredText(subValue, subCol, inner);
    ImGui::EndChild();
    PopCard();
}

static void SecLabel(const char* t)
{
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", t);
    ImGui::Spacing();
}

static void DropRow(int apiId, const std::string& name, long long count,
                    const std::string& valStr, ImVec4 valCol,
                    long long val, long long maxVal,
                    const std::string& iconUrl, const std::string& rarity, float icoSz,
                    bool showValue = true)
{
    float rowH = UICommon::CalcTableRowHeight(icoSz);
    ImGui::TableNextRow(0, rowH);

    ImGui::TableSetColumnIndex(0);
    UICommon::AlignTableCellIcon(rowH, icoSz);
    UICommon::DrawItemIconCell(apiId, iconUrl, icoSz, rarity);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        UIContextMenu::OpenContextMenu("DashDropMenu", apiId, name);

    ImGui::TableSetColumnIndex(1);
    UICommon::AlignTableCellText(rowH);
    ImGui::Text("%s", name.c_str());
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
        UIContextMenu::OpenContextMenu("DashDropMenu", apiId, name);

    ImGui::TableSetColumnIndex(2);
    UICommon::AlignTableCellText(rowH);
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "x%lld", count);

    if (maxVal > 0)
    {
        ImGui::TableSetColumnIndex(3);
        float frac = std::min(1.f, (float)val / (float)maxVal);
        ImVec2 bp = ImGui::GetCursorScreenPos();
        bp.y += (rowH - 4.f) * 0.5f;
        float bw = ImGui::GetContentRegionAvail().x - 2.f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(bp, {bp.x+bw, bp.y+4.f}, IM_COL32(50,50,50,200), 2.f);
        ImVec4 bc = (valCol.x > 0.7f && valCol.y > 0.7f)
            ? ImVec4(.78f,.59f,.08f,1.f) : ImVec4(.48f,.38f,.83f,1.f);
        dl->AddRectFilled(bp, {bp.x+bw*frac, bp.y+4.f},
                          ImGui::ColorConvertFloat4ToU32(bc), 2.f);
        ImGui::Dummy(ImVec2(bw, rowH));
        if (showValue)
        {
            ImGui::TableSetColumnIndex(4);
            UICommon::AlignTableCellText(rowH);
            ImGui::TextColored(valCol, "%s", valStr.c_str());
        }
    }
    else
    {
        ImGui::TableSetColumnIndex(3);
        if (showValue)
        {
            UICommon::AlignTableCellText(rowH);
            ImGui::TextColored(valCol, "%s", valStr.c_str());
        }
    }
}

static void RenderSparkline(float width, float height)
{
    if (!g_Settings.showProfitSparkline) return;
    auto history = ItemTracker::GetProfitHistory();
    if (history.size() < 2) return;

    std::vector<float> vals;
    std::vector<std::string> times;
    for (auto& e : history) {
        vals.push_back((float)e.second);
        // Convert time_point to local HH:MM:SS string
        std::time_t t = std::chrono::system_clock::to_time_t(e.first);
        struct tm tm_info;
        localtime_s(&tm_info, &t);
        char tbuf[16];
        strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &tm_info);
        times.push_back(tbuf);
    }
    float mn = *std::min_element(vals.begin(), vals.end());
    float mx = *std::max_element(vals.begin(), vals.end());

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = {p0.x+width, p0.y+height};
    dl->AddRectFilled(p0, p1, IM_COL32(20,20,20,160), 4.f);

    // Build point list
    std::vector<ImVec2> pts;
    if (mx != mn && vals.size() > 1)
    {
        // Convert sparkline color from int to RGBA
        ImVec4 sparklineColor = ImVec4(
            ((g_Settings.sparklineColor >> 16) & 0xFF) / 255.0f,
            ((g_Settings.sparklineColor >> 8) & 0xFF) / 255.0f,
            (g_Settings.sparklineColor & 0xFF) / 255.0f,
            1.0f
        );
        ImU32 sparklineFill = IM_COL32(
            (int)(sparklineColor.x * 255),
            (int)(sparklineColor.y * 255),
            (int)(sparklineColor.z * 255),
            30
        );
        ImU32 sparklineLine = IM_COL32(
            (int)(sparklineColor.x * 255),
            (int)(sparklineColor.y * 255),
            (int)(sparklineColor.z * 255),
            220
        );
        
        for (size_t i = 0; i < vals.size(); ++i)
            pts.push_back({p0.x + (float)i/(vals.size()-1)*width,
                           p1.y - (vals[i]-mn)/(mx-mn)*height});
        for (size_t i = 0; i+1 < pts.size(); ++i)
        {
            dl->AddQuadFilled(pts[i], pts[i+1], {pts[i+1].x,p1.y}, {pts[i].x,p1.y}, sparklineFill);
            dl->AddLine(pts[i], pts[i+1], sparklineLine, 1.8f);
        }
    }

    // Invisible button to capture mouse
    ImGui::InvisibleButton("##spark", ImVec2(width, height));

    if (ImGui::IsItemHovered() && vals.size() > 1)
    {
        ImVec2 mouse = ImGui::GetMousePos();

        // Find nearest data point by X
        float relX = (mouse.x - p0.x) / width;
        relX = std::max(0.f, std::min(1.f, relX));
        int idx = (int)std::round(relX * (vals.size() - 1));
        idx = std::max(0, std::min((int)vals.size()-1, idx));

        float ptX = pts.empty() ? mouse.x : pts[idx].x;
        float ptY = pts.empty() ? mouse.y : pts[idx].y;

        // Vertical crosshair line
        dl->AddLine({ptX, p0.y}, {ptX, p1.y}, IM_COL32(255,255,255,60), 1.f);

        // Dot on the line
        dl->AddCircleFilled({ptX, ptY}, 4.f, IM_COL32(255,200,30,255));
        dl->AddCircle      ({ptX, ptY}, 4.f, IM_COL32(255,255,255,180), 0, 1.5f);

        // Tooltip: profit + timestamp
        const std::string& ts = idx < (int)times.size() ? times[idx] : "";
        std::string profitStr = UICommon::FormatCoin((long long)vals[idx]);
        if (!ts.empty())
            ImGui::SetTooltip("%s\n%s", profitStr.c_str(), ts.c_str());
        else
            ImGui::SetTooltip("%s", profitStr.c_str());
    }
}

void RenderProfitTab()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 8.f));
    ImGui::Indent(4.f);

    // Warnings
    DrfStatus status = DrfClient::GetStatus();
    if (status == DrfStatus::Disconnected)
    {
        ImGui::TextColored(kRed, "%s", Localization::GetText("warning_drf_not_connected"));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", Localization::GetText("warning_drf_not_connected_desc"));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", Localization::GetText("warning_drf_install"));
        ImGui::Spacing(); HSep(); ImGui::Spacing();
    }
    else if (status == DrfStatus::AuthFailed)
    {
        ImGui::TextColored(kRed, "%s", Localization::GetText("warning_drf_token_invalid"));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", Localization::GetText("warning_drf_token_invalid_desc"));
        ImGui::Spacing(); HSep(); ImGui::Spacing();
    }
    if (g_Settings.gw2ApiKey.empty())
    {
        ImGui::TextColored({1.f,0.6f,0.f,1.f}, "%s", Localization::GetText("warning_gw2_api_key_not_set"));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", Localization::GetText("warning_gw2_api_key_not_set_desc"));
        ImGui::Spacing(); HSep(); ImGui::Spacing();
    }

    // Data
    auto duration    = ItemTracker::GetSessionDuration();
    long long profit = ItemTracker::CalcTotalCustomProfit();
    long long profPh = ItemTracker::GetTotalProfitPerHour(duration);
    long long tpSell = ItemTracker::CalcTotalTpSellProfit();
    long long tpInst = ItemTracker::CalcTotalTpInstantProfit();
    long long lostP  = ItemTracker::GetOpportunityCostProfit();
    long long lostPh = ItemTracker::GetOpportunityCostProfitPerHour(duration);
    int  mf          = ItemTracker::GetMagicFind();
    auto items       = ItemTracker::GetFilteredItems();
    auto currencies  = ItemTracker::GetFilteredCurrencies();
    float avail      = ImGui::GetContentRegionAvail().x - 8.f;
    float iconSz     = (float)g_Settings.profitIconSize;
    char buf[128];

    // KPI Row 1
    float kw = (avail - 3*ImGui::GetStyle().ItemSpacing.x) / 4.f;

    char subBuf[128];

    snprintf(buf, sizeof(buf), "%s", UICommon::FormatCoin(profit).c_str());
    snprintf(subBuf, sizeof(subBuf), "%zu items · %zu currencies", items.size(), currencies.size());
    KpiCard("##k1", Localization::GetText("total_profit_label_simple"), buf, ProfitColor(profit), Localization::GetText("dashboard"), kw, 114.f, subBuf);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("total_profit_tooltip"));
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%s", UICommon::FormatCoin(profPh).c_str());
    snprintf(subBuf, sizeof(subBuf), "%s", UICommon::FormatDuration(duration.count()).c_str());
    KpiCard("##k2", Localization::GetText("profit_per_hour_label_simple"), buf, ProfitColor(profPh), Localization::GetText("session_duration_label_simple"), kw, 114.f, subBuf);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("profit_per_hour_tooltip"));
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%zu", items.size());
    snprintf(subBuf, sizeof(subBuf), "%zu", currencies.size());
    KpiCard("##k3", Localization::GetText("total_items_label_simple"), buf, {1,1,1,1}, Localization::GetText("total_currencies_label_simple"), kw, 114.f, subBuf);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("total_items_tooltip"));
    ImGui::SameLine();

    mf >= 0 ? snprintf(buf, sizeof(buf), "%d%%", mf) : snprintf(buf, sizeof(buf), "N/A");
    KpiCard("##k4", Localization::GetText("magic_find"), buf, kPurple, Localization::GetText("current_or_last_recorded"), kw, 114.f);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("magic_find_tooltip"));

    ImGui::Spacing();

    // KPI Row 2
    char lostSub[64];
    snprintf(lostSub, sizeof(lostSub), "%s/h", UICommon::FormatCoin(lostPh).c_str());

    snprintf(buf, sizeof(buf), "%s", UICommon::FormatCoin(tpSell).c_str());
    KpiCard("##k5", Localization::GetText("approx_trading_profits_listings_label"), buf, ProfitColor(tpSell), Localization::GetText("column_value"), kw, 80.f);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("approx_trading_profits_listings_tooltip"));
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%s", UICommon::FormatCoin(tpInst).c_str());
    KpiCard("##k6", Localization::GetText("approx_trading_profits_instant_label"), buf, ProfitColor(tpInst), Localization::GetText("column_value"), kw, 80.f);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("approx_trading_profits_instant_tooltip"));
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%s", UICommon::FormatCoin(lostP).c_str());
    KpiCard("##k7", Localization::GetText("lost_profit_vs_tp_sell_label"), buf, ProfitColor(lostP), lostSub, kw, 80.f);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("lost_profit_vs_tp_sell_tooltip"));
    ImGui::SameLine();

    if (tpSell > 0)
    {
        float eff = (float)tpInst / (float)tpSell * 100.f;
        snprintf(buf, sizeof(buf), "%.1f%%", eff);
        char effTooltip[128];
        snprintf(effTooltip, sizeof(effTooltip), Localization::GetText("efficiency_score_desc"), eff);
        KpiCard("##k8", Localization::GetText("efficiency_score"), buf, EfficiencyColor(eff), Localization::GetText("efficiency_score_desc_short"), kw, 80.f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", effTooltip);
    }
    else
    {
        KpiCard("##k8", Localization::GetText("efficiency_score"), "\xe2\x80\x94", kMuted, "N/A", kw, 80.f);
    }

    ImGui::Spacing();

    // Two-column layout
    float sideW = 200.f;
    float mainW = avail - sideW - ImGui::GetStyle().ItemSpacing.x;

    // LEFT COLUMN
    ImGui::BeginGroup();

    // Best Drops Row (Single & Total)
    {
        auto [bestId, bestSt] = ItemTracker::GetBestDrop();
        auto [bestTotalId, bestTotalSt] = ItemTracker::GetBestDropTotalValue();
        
        bool hasBestSingle = (bestId != 0 && bestSt.count > 0 && ItemTracker::PassesFilter(bestSt));
        bool hasBestTotal = (bestTotalId != 0 && bestTotalSt.count > 0 && ItemTracker::PassesFilter(bestTotalSt));

        if (hasBestSingle || hasBestTotal)
        {
            float dropCardW = (mainW - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
            float dropCardH = iconSz + 52.f;

            // Single Best Drop
            if (hasBestSingle)
            {
                { ImVec2 p = ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p, { p.x + dropCardW, p.y + dropCardH }); }
                PushCard();
                ImGui::BeginChild("##best_single", ImVec2(dropCardW, dropCardH), false, ImGuiWindowFlags_NoScrollbar);

                std::string bn = bestSt.details.loaded ? bestSt.details.name : Localization::GetText("loading");
                long long unitProfit = 0;
                {
                    long long vendorPrice = ItemTracker::CanSellToVendor(bestSt.details) ? (long long)bestSt.details.vendorValue : 0;
                    long long tpSellPrice = ItemTracker::CanSellOnTp(bestSt.details) ? ItemTracker::TpSellProceedsPerUnitCopper(bestSt.details) : 0;
                    unitProfit = std::max(vendorPrice, tpSellPrice);
                }
                std::string profitStr = UICommon::FormatCoin(unitProfit);
                
                float labelWidth = ImGui::CalcTextSize(Localization::GetText("best_drop_single")).x;
                float nameWidth = ImGui::CalcTextSize(bn.c_str()).x;
                float profitWidth = ImGui::CalcTextSize(profitStr.c_str()).x;
                float maxTextWidth = std::max({ labelWidth, nameWidth, profitWidth });
                float totalWidth = iconSz + 10.f + maxTextWidth;
                float centerX = (dropCardW - totalWidth) * 0.5f;

                ImGui::SetCursorPosX(centerX);
                ImVec2 ipos = ImGui::GetCursorScreenPos();
                UICommon::DrawItemIconCell(bestId, bestSt.details.iconUrl, iconSz, bestSt.details.rarity);
                bool ih = ImGui::IsItemHovered();
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    UIContextMenu::OpenContextMenu("BestDropMenuSingle", bestId, bestSt.details.loaded ? bestSt.details.name : "...");
                ImGui::GetWindowDrawList()->AddRect(ipos, { ipos.x + iconSz, ipos.y + iconSz }, IM_COL32(255, 215, 0, 200), 4.f, 0, 2.f);
                ImGui::SameLine(0, 10);
                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", Localization::GetText("best_drop_single"));
                ImGui::Text("%s", bn.c_str());
                bool nh = ImGui::IsItemHovered();
                if (nh && ImGui::IsMouseClicked(1)) UIContextMenu::OpenContextMenu("BestDropMenuSingle", bestId, bn);
                ImGui::TextColored(kGold, "%s", profitStr.c_str());
                ImGui::EndGroup();
                if (ih || nh)
                {
                    UITooltips::ItemTooltipOptions opt; opt.showTrading = opt.showAccountFlags = opt.showId = true;
                    if (bestSt.details.loaded) UITooltips::RenderItemTooltip(bestSt.details, bestId, opt);
                    else UITooltips::RenderItemTooltipFallback(bn, bestSt.details.rarity, bestId, opt);
                }
                UIContextMenu::RenderItemContextMenu("BestDropMenuSingle", UIContextMenu::ContextMenuType::General);
                ImGui::EndChild(); PopCard();
                if (hasBestTotal) ImGui::SameLine();
            }

            // Total Best Drop
            if (hasBestTotal)
            {
                { ImVec2 p = ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p, { p.x + dropCardW, p.y + dropCardH }); }
                PushCard();
                ImGui::BeginChild("##best_total", ImVec2(dropCardW, dropCardH), false, ImGuiWindowFlags_NoScrollbar);

                std::string bn = bestTotalSt.details.loaded ? bestTotalSt.details.name : Localization::GetText("loading");
                long long totalProfit = ItemTracker::GetStatProfit(bestTotalSt);
                std::string profitStr = UICommon::FormatCoin(totalProfit);
                char countBuf[32]; snprintf(countBuf, sizeof(countBuf), " (x%zu)", bestTotalSt.count);
                std::string profitWithCount = profitStr + countBuf;

                float labelWidth = ImGui::CalcTextSize(Localization::GetText("best_drop_total")).x;
                float nameWidth = ImGui::CalcTextSize(bn.c_str()).x;
                float profitWidth = ImGui::CalcTextSize(profitWithCount.c_str()).x;
                float maxTextWidth = std::max({ labelWidth, nameWidth, profitWidth });
                float totalWidth = iconSz + 10.f + maxTextWidth;
                float centerX = (dropCardW - totalWidth) * 0.5f;

                ImGui::SetCursorPosX(centerX);
                ImVec2 ipos = ImGui::GetCursorScreenPos();
                UICommon::DrawItemIconCell(bestTotalId, bestTotalSt.details.iconUrl, iconSz, bestTotalSt.details.rarity);
                bool ih = ImGui::IsItemHovered();
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    UIContextMenu::OpenContextMenu("BestDropMenuTotal", bestTotalId, bestTotalSt.details.loaded ? bestTotalSt.details.name : "...");
                ImGui::GetWindowDrawList()->AddRect(ipos, { ipos.x + iconSz, ipos.y + iconSz }, IM_COL32(255, 215, 0, 200), 4.f, 0, 2.f);
                ImGui::SameLine(0, 10);
                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", Localization::GetText("best_drop_total"));
                ImGui::Text("%s", bn.c_str());
                bool nh = ImGui::IsItemHovered();
                if (nh && ImGui::IsMouseClicked(1)) UIContextMenu::OpenContextMenu("BestDropMenuTotal", bestTotalId, bn);
                ImGui::TextColored(kGold, "%s", profitStr.c_str());
                ImGui::SameLine(0, 2); ImGui::TextColored(kMuted, "x%zu", bestTotalSt.count);
                ImGui::EndGroup();
                if (ih || nh)
                {
                    UITooltips::ItemTooltipOptions opt; opt.showTrading = opt.showAccountFlags = opt.showId = true;
                    if (bestTotalSt.details.loaded) UITooltips::RenderItemTooltip(bestTotalSt.details, bestTotalId, opt);
                    else UITooltips::RenderItemTooltipFallback(bn, bestTotalSt.details.rarity, bestTotalId, opt);
                }
                UIContextMenu::RenderItemContextMenu("BestDropMenuTotal", UIContextMenu::ContextMenuType::General);
                ImGui::EndChild(); PopCard();
            }
            ImGui::Spacing();
        }
    }

    // Sparkline
    { ImVec2 p=ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p,{p.x+mainW,p.y+165.f}); }
    PushCard();
    ImGui::BeginChild("##sparkcard", ImVec2(mainW, 165.f), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Indent(4.f);
    SecLabel(Localization::GetText("profit_per_hour_label_simple"));
    RenderSparkline(mainW - 20.f, 127.f);
    ImGui::Unindent(4.f);
    ImGui::EndChild(); PopCard(); ImGui::Spacing();

    // ---------------------------------------------------------------------------
    // Magnetite Shard Weekly Tracker
    // Only shown when the tracker is enabled in Settings.
    // ---------------------------------------------------------------------------
    bool magnetiteEnabled;
    {
        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
        magnetiteEnabled = g_Settings.enableMagnetiteTracker;
    }

    if (magnetiteEnabled)
    {
        ImGui::Spacing();
        
        // Magnetite Shards Section (Collapsible)
        static bool magnetiteExpanded = true;
        
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
        if (magnetiteExpanded)
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
        void* iconTex = UITabIcons::GetIcon("magnetite");
        if (iconTex)
        {
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
                         "Magnetite Shards");
        
        // Invisible button for click detection
        ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        
        if (ImGui::Button("##MagnetiteHeaderButton", ImVec2(regionAvail.x, headerHeight)))
        {
            magnetiteExpanded = !magnetiteExpanded;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("toggle_magnetite_tooltip"));

        ImGui::PopStyleColor(3);
        
        ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + headerHeight + 8.0f));
        
        if (magnetiteExpanded)
        {
            ImGui::Spacing();
            ImGui::Indent(4.0f);

            int earned = MagnetiteTracker::GetWeeklyEarned();
            int cap    = MagnetiteTracker::WEEKLY_CAP;
            float progress = cap > 0 ? static_cast<float>(earned) / static_cast<float>(cap) : 0.f;

            // Progress bar
            char progressOverlay[64];
            snprintf(progressOverlay, sizeof(progressOverlay), "%d / %d", earned, cap);

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.55f, 0.35f, 0.75f, 1.f)); // purple tint
            ImGui::ProgressBar(
                progress,
                ImVec2(300.f, 18.f),
                progressOverlay
            );
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(
                    "Magnetite Shards earned this week\n"
                    "%d / %d  (%.1f%%)",
                    earned, cap, progress * 100.f
                );
            }

            ImGui::Spacing();

            // Remaining shards and percentage labels
            int remaining = std::max(0, cap - earned);
            ImGui::Text("%s", Localization::GetText("earned_this_week"));
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.75f, 0.55f, 1.f, 1.f), "%d", earned);

            ImGui::Text("%s", Localization::GetText("remaining"));
            ImGui::SameLine();
            ImGui::TextColored(remaining > 0
                ? ImVec4(1.f, 1.f, 1.f, 1.f)
                : ImVec4(0.4f, 0.9f, 0.4f, 1.f),
                "%d", remaining
            );

            // Weekly cap completed indicator
            if (earned >= cap)
            {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), " ✓ Weekly cap reached!");
            }

            ImGui::Spacing();

            // Last API check timestamp
            std::string lastCheck;
            {
                std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                lastCheck = g_Settings.magnetiteLastApiCheckUtc;
            }
            if (!lastCheck.empty())
            {
                std::string lastCheckLocal = MagnetiteTracker::UtcToLocal(lastCheck);
                ImGui::TextDisabled("Last wallet check: %s", lastCheckLocal.c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(
                        "Last time the GW2 wallet API was queried\n"
                        "to verify the Magnetite Shard total."
                    );
            }
            else
            {
                ImGui::TextDisabled("Wallet API not yet queried this session.");
            }

            ImGui::Unindent(4.0f);
            ImGui::Spacing();
        }
    }

    // Top Items & Currencies by Profit
    std::vector<std::pair<int, Stat>> combinedStats;
    {
        auto sItems = ItemTracker::GetSortedItems(ItemTracker::SortMode::ProfitDesc);
        auto sCurrencies = ItemTracker::GetSortedCurrencies(ItemTracker::SortMode::ProfitDesc);
        
        for (const auto& pair : sItems) combinedStats.push_back(pair);
        for (const auto& pair : sCurrencies) combinedStats.push_back(pair);

        std::sort(combinedStats.begin(), combinedStats.end(), [](const auto& a, const auto& b) {
            return ItemTracker::GetStatProfit(a.second) > ItemTracker::GetStatProfit(b.second);
        });
    }

    long long maxIV = 0;
    { 
        int c = 0; 
        for (auto& [id, st] : combinedStats) 
        { 
            if (c >= 5) break; 
            if (!st.count) continue; 
            long long p = ItemTracker::GetStatProfit(st); 
            if (p > 0) { maxIV = std::max(maxIV, p); c++; } 
        } 
    }

    { ImVec2 p = ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p, { p.x + mainW, p.y + 28.f + 5 * (iconSz + 8.f) + 16.f }); }
    PushCard();
    ImGui::BeginChild("##topI", ImVec2(mainW, 28.f + 5 * (iconSz + 8.f) + 16.f), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Indent(4.f);
    SecLabel(Localization::GetText("top_items_profit_header"));
    if (ImGui::BeginTable("##ti", 6, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, iconSz + 4);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 80.f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 90.f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.f); // Spacer to move right side 100px left
        int cnt = 0;
        for (auto& [id, st] : combinedStats)
        {
            if (cnt >= 5) break; if (!st.count) continue;
            long long p = ItemTracker::GetStatProfit(st); if (p <= 0) continue;
            std::string nm = st.details.loaded ? st.details.name : Localization::GetText("loading");
            
            char profitStr[64];
            snprintf(profitStr, sizeof(profitStr), "%s", UICommon::FormatCoin(p).c_str());
            
            DropRow(id, nm, st.count, profitStr, kGold, p, maxIV,
                    st.details.iconUrl, st.details.loaded ? st.details.rarity : "", iconSz);
            
            if (ImGui::IsItemHovered()) 
             { 
                 if (st.details.loaded) 
                 {
                     if (st.IsCurrency())
                     {
                         UITooltips::CurrencyTooltipOptions opt;
                         opt.showCount = true;
                         opt.count = st.count;
                         opt.showProfit = true;
                         opt.profit = p;
                         UITooltips::RenderCurrencyTooltip(st.details, id, opt);
                     }
                     else
                     {
                         UITooltips::ItemTooltipOptions opt;
                         opt.showCount = true;
                         opt.count = st.count;
                         opt.showProfit = true;
                         opt.profit = p;
                         UITooltips::RenderItemTooltip(st.details, id, opt);
                     }
                 }
                 else 
                 {
                     UITooltips::ItemTooltipOptions opt;
                     opt.showCount = true;
                     opt.count = st.count;
                     opt.showProfit = true;
                     opt.profit = p;
                     UITooltips::RenderItemTooltipFallback(nm, "", id, opt); 
                 }
             }
            cnt++;
        }
        UIContextMenu::RenderItemContextMenu("DashDropMenu", UIContextMenu::ContextMenuType::General);
        ImGui::EndTable();
    }
    ImGui::Unindent(4.f);
    ImGui::EndChild(); PopCard(); ImGui::Spacing();

    // Top Currencies
    auto sCurr = ItemTracker::GetSortedCurrencies(ItemTracker::SortMode::CountDesc);
    long long maxCV = 0;
    { int c=0; for (auto& [id,st]:sCurr) { if(c>=5)break; if(st.count>0){maxCV=std::max(maxCV,st.count);c++;} } }

    { ImVec2 p=ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p,{p.x+mainW,p.y+28.f+5*(iconSz+8.f)+16.f}); }
    PushCard();
    ImGui::BeginChild("##topC", ImVec2(mainW, 28.f+5*(iconSz+8.f)+16.f), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Indent(4.f);
    SecLabel(Localization::GetText("top_currencies_count_header"));
    if (ImGui::BeginTable("##tc", 6, ImGuiTableFlags_NoSavedSettings|ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthFixed,iconSz+4);
        ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthFixed,70.f);
        ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthFixed,80.f);
        ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthFixed,90.f);
        ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthFixed,100.f); // Spacer to move right side 100px left
        int cnt=0;
        for (auto& [id,st]:sCurr)
        {
            if (cnt>=5) break; if (st.count<=0) continue;
            std::string nm = st.details.loaded ? st.details.name : (id==1 ? Localization::GetText("coin") : Localization::GetText("loading"));
            std::string iconUrl = st.details.iconUrl;
            if (id == 1 && iconUrl.empty()) iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png";
            
            long long customProfit = CustomProfitManager::GetCustomProfit(id);
            char cb[64];
            if (customProfit != 0) {
                snprintf(cb, sizeof(cb), "%s", UICommon::FormatCoin(customProfit * st.count).c_str());
            } else {
                cb[0] = '\0'; // Show nothing if no custom profit
            }
            
            DropRow(id, nm, st.count, cb, kPurple, st.count, maxCV, iconUrl, st.details.loaded ? st.details.rarity : "", iconSz);
            cnt++;
        }
        UIContextMenu::RenderCurrencyContextMenu("DashDropMenu", UIContextMenu::ContextMenuType::General);
        ImGui::EndTable();
    }
    ImGui::Unindent(4.f);
    ImGui::EndChild(); PopCard();
    ImGui::EndGroup(); // left

    ImGui::SameLine();

    // RIGHT COLUMN
    ImGui::BeginGroup();

    // Rarity distribution
    {
        struct RC { const char* n; ImVec4 c; int v; };
        RC rc[] = {
            {"Legendary",{0.55f,0.25f,0.85f,1.f},0}, {"Exotic",{.78f,.59f,.08f,1.f},0},
            {"Rare",{.83f,.72f,.00f,1.f},0},       {"Masterwork",{.35f,.69f,.87f,1.f},0},
            {"Fine",{.40f,.56f,.84f,1.f},0},        {"Basic",{.55f,.55f,.55f,1.f},0}
        };
        for (auto& [id,st]:items)
        {
            const auto& r=st.details.rarity;
            if(r=="Legendary")rc[0].v++; else if(r=="Exotic")rc[1].v++; else if(r=="Rare")rc[2].v++;
            else if(r=="Masterwork")rc[3].v++; else if(r=="Fine")rc[4].v++; else rc[5].v++;
        }
        { ImVec2 p=ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p,{p.x+sideW,p.y+148.f}); }
        PushCard();
        ImGui::BeginChild("##rar", ImVec2(sideW, 148.f), false, ImGuiWindowFlags_NoScrollbar);
        SecLabel(Localization::GetText("total_items_label_simple"));
        for (auto& r:rc)
        {
            if (!r.v) continue;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos(); p.y += ImGui::GetTextLineHeight()*0.3f;
            dl->AddCircleFilled({p.x+5.f,p.y+6.f}, 4.f, ImGui::ColorConvertFloat4ToU32(r.c));
            ImGui::Dummy(ImVec2(12.f,ImGui::GetTextLineHeight())); ImGui::SameLine(0,2);
            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",r.n); ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+ImGui::GetContentRegionAvail().x-30.f);
            ImGui::Text("%d",r.v);
        }
        ImGui::EndChild(); PopCard(); ImGui::Spacing();
    }

    // Session info
    { ImVec2 p=ImGui::GetCursorScreenPos(); DrawDarkGradientBox(p,{p.x+sideW,p.y+102.f}); }
    PushCard();
    ImGui::BeginChild("##sess", ImVec2(sideW, 102.f), false, ImGuiWindowFlags_NoScrollbar);
    SecLabel(Localization::GetText("session_duration_label_simple"));
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("session_duration_label_simple")); ImGui::SameLine();
    ImGui::Text("%s", UICommon::FormatDuration(duration.count()).c_str());
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("next_reset_label_simple")); ImGui::SameLine();
    ImGui::TextColored({1.f,0.5f,0.f,1.f},"%s", AutoReset::GetNextResetDisplayUtc().c_str());
    ImGui::EndChild(); PopCard();

    ImGui::EndGroup(); // right

    ImGui::Spacing(); HSep(); ImGui::Spacing();

    // Summaries
    if (g_Settings.enableSessionHistory)
    {
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("show_summaries")); ImGui::SameLine();
        if (ImGui::Checkbox("##showSum",&g_Settings.enableSummariesInProfitTab)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",Localization::GetText("show_summaries_tooltip"));

        if (g_Settings.enableSummariesInProfitTab)
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("summary_period")); ImGui::SameLine();
            if (ImGui::RadioButton(Localization::GetText("summary_today"),      s_SummaryPeriod==0)) s_SummaryPeriod=0; ImGui::SameLine();
            if (ImGui::RadioButton(Localization::GetText("summary_this_week"),  s_SummaryPeriod==1)) s_SummaryPeriod=1; ImGui::SameLine();
            if (ImGui::RadioButton(Localization::GetText("summary_this_month"), s_SummaryPeriod==2)) s_SummaryPeriod=2;
            ImGui::Spacing();

            SessionHistory::SummaryPeriod period =
                s_SummaryPeriod==0 ? SessionHistory::SummaryPeriod::Today :
                s_SummaryPeriod==1 ? SessionHistory::SummaryPeriod::ThisWeek :
                                     SessionHistory::SummaryPeriod::ThisMonth;
            auto sum = SessionHistory::GetSummary(period);
            float sw = (avail - 4*ImGui::GetStyle().ItemSpacing.x) / 5.f;
            char sv[64];

            snprintf(sv,sizeof(sv),"%s",UICommon::FormatCoin(sum.totalProfit).c_str());
            KpiCard("##s1",Localization::GetText("total_profit"),sv,ProfitColor(sum.totalProfit),"",sw,52.f); ImGui::SameLine();
            snprintf(sv,sizeof(sv),"%s",UICommon::FormatCoin(sum.profitPerHour).c_str());
            KpiCard("##s2",Localization::GetText("profit_per_hour"),sv,ProfitColor(sum.profitPerHour),"",sw,52.f); ImGui::SameLine();
            snprintf(sv,sizeof(sv),"%d",sum.totalDrops);
            KpiCard("##s3",Localization::GetText("total_drops"),sv,{1,1,1,1},"",sw,52.f); ImGui::SameLine();
            snprintf(sv,sizeof(sv),"%d",sum.sessionCount);
            KpiCard("##s4",Localization::GetText("session_count"),sv,{1,1,1,1},"",sw,52.f); ImGui::SameLine();
            snprintf(sv,sizeof(sv),"%s",UICommon::FormatDuration(sum.totalDurationSeconds).c_str());
            KpiCard("##s5",Localization::GetText("total_duration"),sv,{1,1,1,1},"",sw,52.f);

            if (sum.previousPeriodProfit != 0)
            {
                ImGui::Spacing();
                long long diff = sum.totalProfit - sum.previousPeriodProfit;
                float pct = (float)diff / (float)sum.previousPeriodProfit * 100.f;
                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("comparison_previous_period")); ImGui::SameLine();
                ImGui::TextColored(diff>=0?kGreen:kRed, "%s (%.1f%%)", UICommon::FormatCoin(diff).c_str(), pct);
            }

            if (!sum.topDrops.empty())
            {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("top_drops")); ImGui::Spacing();
                if (ImGui::BeginTable("##sd",4,ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable|ImGuiTableFlags_NoSavedSettings))
                {
                    float icW = std::max(iconSz+10.f,70.f);
                    ImGui::TableSetupColumn(Localization::GetText("column_icon"), ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide,icW);
                    ImGui::TableSetupColumn(Localization::GetText("item"),        ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide,350.f);
                    ImGui::TableSetupColumn(Localization::GetText("count"),       ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoHide,100.f);
                    ImGui::TableSetupColumn(Localization::GetText("value"),       ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (const auto& drop:sum.topDrops)
                    {
                        if (drop.totalValue<=0) continue;
                        Stat st = ItemTracker::GetItemStat(drop.itemId);
                        st.apiId=drop.itemId; st.count=drop.count;
                        if (!ItemTracker::PassesFilter(st)) continue;
                        float rh = UICommon::CalcTableRowHeight(iconSz);
                        ImGui::TableNextRow(0,rh);
                        ImGui::TableSetColumnIndex(0); UICommon::AlignTableCellIcon(rh,iconSz);
                        std::string iu = !drop.iconUrl.empty() ? drop.iconUrl : st.details.iconUrl;
                        UICommon::DrawItemIconCell(drop.itemId,iu,iconSz,drop.rarity);
                        bool ih=ImGui::IsItemHovered();
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) UIContextMenu::OpenContextMenu("SumDrop",drop.itemId,drop.itemName);
                        ImGui::TableSetColumnIndex(1); UICommon::AlignTableCellText(rh);
                        ImGui::Text("%s",drop.itemName.c_str()); bool nh=ImGui::IsItemHovered();
                        if (nh&&ImGui::IsMouseClicked(1)) UIContextMenu::OpenContextMenu("SumDrop",drop.itemId,drop.itemName);
                        if (ih||nh)
                        {
                            UITooltips::ItemTooltipOptions opt; opt.showCount=true; opt.count=drop.count; opt.showValue=true; opt.value=drop.totalValue; opt.valueLabelKey="column_value"; opt.showTrading=st.details.loaded; opt.showAccountFlags=st.details.loaded; opt.showId=true;
                            if (st.details.loaded) UITooltips::RenderItemTooltip(st.details,drop.itemId,opt);
                            else UITooltips::RenderItemTooltipFallback(drop.itemName,drop.rarity,drop.itemId,opt);
                        }
                        ImGui::TableSetColumnIndex(2); UICommon::AlignTableCellText(rh); ImGui::Text("%d",drop.count);
                        ImGui::TableSetColumnIndex(3); UICommon::AlignTableCellText(rh); ImGui::Text("%s",UICommon::FormatCoin(drop.totalValue).c_str());
                    }
                    UIContextMenu::RenderItemContextMenu("SumDrop",UIContextMenu::ContextMenuType::General);
                    ImGui::EndTable();
                }
            }
        }
        ImGui::Spacing(); HSep(); ImGui::Spacing();
    }

    // No Data
    if (items.empty() && currencies.empty())
    {
        ImGui::TextColored({1.f,0.6f,0.f,1.f},"%s",Localization::GetText("warning_no_data"));
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f),"%s",Localization::GetText("warning_no_data_desc"));
        ImGui::Spacing();
    }

    // Import/Export button (orange gradient)
    ImGui::Unindent(4.f);
    ImGui::PopStyleVar();
} // RenderProfitTab

} // namespace UIProfit
