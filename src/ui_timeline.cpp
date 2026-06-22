#include "ui_timeline.h"
#include "item_tracker.h"
#include "custom_profit.h"
#include "session_history.h"
#include "settings.h"
#include "localization.h"
#include "ui_common.h"
#include "ui_context_menu.h"
#include "ui_tooltips.h"
#include <algorithm>
#include <ctime>
#include <map>

namespace UITimeline
{
    void RenderTimelineTab()
    {
        // 1. Header Info (Profit, G/h, etc. like in the photo)
        long long tpSell = ItemTracker::CalcTotalTpSellProfit();
        long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
        auto duration = ItemTracker::GetSessionDuration();
        long long seconds = duration.count();

        // === KPI Header ===
        long long custom       = ItemTracker::CalcTotalCustomProfit();
        long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
        long long coins        = ItemTracker::GetCurrencyStat(1).count; // Coins (currency ID 1)
        const ImVec4 kGold  = { 1.00f, 0.84f, 0.00f, 1.f };
        const ImVec4 kRed   = { 0.90f, 0.20f, 0.20f, 1.f };
        const ImVec4 kMuted = { 1.00f, 1.00f, 1.00f, 1.f };

        auto ProfitColor = [&](long long v) -> ImVec4 {
            return v > 0 ? kGold : (v < 0 ? kRed : ImVec4(1,1,1,1));
        };

        // Helper: draw one KPI card using a Child window with gradient background
        // cardId    - unique ImGui id string
        // label     - muted top label
        // value     - main value string
        // valueCol  - color for the value
        // cardW     - width
        // cardH     - height
        auto KpiCard = [&](const char* cardId, const char* label, const char* value,
                           ImVec4 valueCol, float cardW, float cardH)
        {
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImVec2 cardMin = cursor;
            ImVec2 cardMax = ImVec2(cursor.x + cardW, cursor.y + cardH);

            // Draw gradient background (matching Debug tab design)
            const float acR = g_Settings.accentColorR, acG = g_Settings.accentColorG, acB = g_Settings.accentColorB;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
            ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
            ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
            dl->AddRectFilledMultiColor(cardMin, cardMax, top, top, bot, bot);
            dl->AddRect(cardMin, cardMax, border, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(cardMin.x, cardMin.y), ImVec2(cardMin.x + 3.f, cardMax.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.f)), 2.f);

            // Draw text content (centered)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));
            ImGui::BeginChild(cardId, ImVec2(cardW, cardH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            float labelWidth = ImGui::CalcTextSize(label).x;
            float valueWidth = ImGui::CalcTextSize(value).x;
            float centerX = (cardW - labelWidth) * 0.5f;
            centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
            ImGui::SetCursorPosX(centerX);
            ImGui::TextColored(kMuted, "%s", label);
            centerX = (cardW - valueWidth) * 0.5f;
            centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
            ImGui::SetCursorPosX(centerX);
            ImGui::TextColored(valueCol, "%s", value);
            ImGui::EndChild();
            ImGui::PopStyleVar();
        };

        float avail = ImGui::GetContentRegionAvail().x;
        float cardH = 82.f;

        // --- Row 1: 4 equal cards ---
        float kw4 = (avail - 3.f * ImGui::GetStyle().ItemSpacing.x) / 4.f;

        KpiCard("##tlk1",
                Localization::GetText("approx_profits_label"),
                UICommon::FormatCoin(custom).c_str(),
                ProfitColor(custom), kw4, cardH);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("approx_profits_tooltip"));
        ImGui::SameLine();

        KpiCard("##tlk2",
                Localization::GetText("approx_gold_per_hour_label"),
                UICommon::FormatCoin(profitPerHour).c_str(),
                ProfitColor(profitPerHour), kw4, cardH);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("approx_gold_per_hour_tooltip"));
        ImGui::SameLine();

        KpiCard("##tlk3",
                Localization::GetText("approx_trading_profits_listings_label"),
                UICommon::FormatCoin(tpSell).c_str(),
                kGold, kw4, cardH);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("approx_trading_profits_listings_tooltip"));
        ImGui::SameLine();

        KpiCard("##tlk4",
                Localization::GetText("approx_trading_profits_instant_label"),
                UICommon::FormatCoin(tpInstant).c_str(),
                kGold, kw4, cardH);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("approx_trading_profits_instant_tooltip"));

        ImGui::Spacing();

        // --- Row 2: Coins card + 2 /h cards + Best Drop card ---

        // Left: Coins card
        {
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImVec2 cardMin = cursor;
            ImVec2 cardMax = ImVec2(cursor.x + kw4, cursor.y + cardH);

            // Draw gradient background (matching Debug tab design)
            const float acR = g_Settings.accentColorR, acG = g_Settings.accentColorG, acB = g_Settings.accentColorB;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
            ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
            ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
            dl->AddRectFilledMultiColor(cardMin, cardMax, top, top, bot, bot);
            dl->AddRect(cardMin, cardMax, border, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(cardMin.x, cardMin.y), ImVec2(cardMin.x + 3.f, cardMax.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.f)), 2.f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));
            ImGui::BeginChild("##tlkcoins", ImVec2(kw4, cardH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            const char* label = Localization::GetText("timeline_liquid_coins");
            std::string value = UICommon::FormatCoin(coins);
            float labelWidth = ImGui::CalcTextSize(label).x;
            float valueWidth = ImGui::CalcTextSize(value.c_str()).x;
            float centerX = (kw4 - labelWidth) * 0.5f;
            centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
            ImGui::SetCursorPosX(centerX);
            ImGui::TextColored(kMuted, "%s", label);
            centerX = (kw4 - valueWidth) * 0.5f;
            centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
            ImGui::SetCursorPosX(centerX);
            ImGui::TextColored(kGold, "%s", value.c_str());

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("coins_tooltip"));

        ImGui::SameLine();

        // Middle: TP /h listings
        {
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImVec2 cardMin = cursor;
            ImVec2 cardMax = ImVec2(cursor.x + kw4, cursor.y + cardH);

            // Draw gradient background (matching Debug tab design)
            const float acR = g_Settings.accentColorR, acG = g_Settings.accentColorG, acB = g_Settings.accentColorB;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
            ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
            ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
            dl->AddRectFilledMultiColor(cardMin, cardMax, top, top, bot, bot);
            dl->AddRect(cardMin, cardMax, border, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(cardMin.x, cardMin.y), ImVec2(cardMin.x + 3.f, cardMax.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.f)), 2.f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));
            ImGui::BeginChild("##tlkph1", ImVec2(kw4, cardH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            if (seconds > 0)
            {
                long long gphSell = (tpSell * 3600LL) / seconds;
                const char* label = Localization::GetText("timeline_profit_hour_listings");
                std::string value = UICommon::FormatCoin(gphSell);
                float labelWidth = ImGui::CalcTextSize(label).x;
                float valueWidth = ImGui::CalcTextSize(value.c_str()).x;
                float centerX = (kw4 - labelWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kMuted, "%s", label);
                centerX = (kw4 - valueWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kGold, "%s", value.c_str());
            }
            else
            {
                const char* label = Localization::GetText("timeline_profit_hour_listings");
                float labelWidth = ImGui::CalcTextSize(label).x;
                float centerX = (kw4 - labelWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kMuted, "%s", label);
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("timeline_profit_hour_listings_tooltip"));

        ImGui::SameLine();

        // Right: TP /h instant
        {
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImVec2 cardMin = cursor;
            ImVec2 cardMax = ImVec2(cursor.x + kw4, cursor.y + cardH);

            // Draw gradient background (matching Debug tab design)
            const float acR = g_Settings.accentColorR, acG = g_Settings.accentColorG, acB = g_Settings.accentColorB;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
            ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
            ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
            dl->AddRectFilledMultiColor(cardMin, cardMax, top, top, bot, bot);
            dl->AddRect(cardMin, cardMax, border, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(cardMin.x, cardMin.y), ImVec2(cardMin.x + 3.f, cardMax.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.f)), 2.f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));
            ImGui::BeginChild("##tlkph2", ImVec2(kw4, cardH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            if (seconds > 0)
            {
                long long gphInstant = (tpInstant * 3600LL) / seconds;
                const char* label = Localization::GetText("timeline_profit_hour_instant");
                std::string value = UICommon::FormatCoin(gphInstant);
                float labelWidth = ImGui::CalcTextSize(label).x;
                float valueWidth = ImGui::CalcTextSize(value.c_str()).x;
                float centerX = (kw4 - labelWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kMuted, "%s", label);
                centerX = (kw4 - valueWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kGold, "%s", value.c_str());
            }
            else
            {
                const char* label = Localization::GetText("timeline_profit_hour_instant");
                float labelWidth = ImGui::CalcTextSize(label).x;
                float centerX = (kw4 - labelWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kMuted, "%s", label);
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("timeline_profit_hour_instant_tooltip"));

        ImGui::SameLine();

        // Right: Best Drop card
        {
            auto [bestId, bestSt] = ItemTracker::GetBestDrop();

            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImVec2 cardMin = cursor;
            ImVec2 cardMax = ImVec2(cursor.x + kw4, cursor.y + cardH);

            // Draw gradient background (matching Debug tab design)
            const float acR = g_Settings.accentColorR, acG = g_Settings.accentColorG, acB = g_Settings.accentColorB;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 top    = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*2.f),std::min(1.f,acG*2.f),std::min(1.f,acB*2.f),1.f));
            ImU32 bot    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR*.5f,acG*.5f,acB*.5f,1.f));
            ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(std::min(1.f,acR*1.5f),std::min(1.f,acG*1.5f),std::min(1.f,acB*1.5f),1.f));
            dl->AddRectFilledMultiColor(cardMin, cardMax, top, top, bot, bot);
            dl->AddRect(cardMin, cardMax, border, 4.f, 0, 0.5f);
            dl->AddRectFilled(ImVec2(cardMin.x, cardMin.y), ImVec2(cardMin.x + 3.f, cardMax.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 1.f)), 2.f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));
            ImGui::BeginChild("##tlkbest", ImVec2(kw4, cardH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            const char* label = Localization::GetText("best_drop");
            float labelWidth = ImGui::CalcTextSize(label).x;
            float centerX = (kw4 - labelWidth) * 0.5f;
            centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
            ImGui::SetCursorPosX(centerX);
            ImGui::TextColored(kMuted, "%s", label);

            if (bestId != 0 && bestSt.count > 0)
            {
                float icoSz = ImGui::GetTextLineHeight() * 1.4f;
                std::string bestName = bestSt.details.loaded
                    ? bestSt.details.name
                    : Localization::GetText("loading");
                
                // Calculate total width of icon + name
                float nameWidth = ImGui::CalcTextSize(bestName.c_str()).x;
                float totalWidth = icoSz + 6.f + nameWidth;
                float centerX = (kw4 - totalWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let content go left of padding
                ImGui::SetCursorPosX(centerX);
                
                UICommon::DrawItemIconCell(bestId, bestSt.details.iconUrl, icoSz, bestSt.details.rarity);
                if (ImGui::IsItemHovered())
                {
                    UITooltips::ItemTooltipOptions opt;
                    opt.showTrading = opt.showAccountFlags = opt.showId = true;
                    if (bestSt.details.loaded)
                        UITooltips::RenderItemTooltip(bestSt.details, bestId, opt);
                    else
                        UITooltips::RenderItemTooltipFallback(
                            bestSt.details.name.empty() ? Localization::GetText("ellipsis") : bestSt.details.name,
                            bestSt.details.rarity, bestId, opt);
                }
                ImGui::SameLine(0, 6);
                ImGui::BeginGroup();
                // Clip name so it never overflows the card
                float nameMaxW = kw4 - icoSz - 10.f - 20.f;
                ImGui::PushClipRect(
                    ImGui::GetCursorScreenPos(),
                    ImVec2(ImGui::GetCursorScreenPos().x + nameMaxW,
                           ImGui::GetCursorScreenPos().y + ImGui::GetTextLineHeight()),
                    true);
                ImGui::Text("%s", bestName.c_str());
                ImGui::PopClipRect();
                std::string profit = UICommon::FormatCoin(ItemTracker::GetStatProfit(bestSt));
                float profitWidth = ImGui::CalcTextSize(profit.c_str()).x;
                centerX = (kw4 - profitWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kGold, "%s", profit.c_str());
                ImGui::EndGroup();
            }
            else
            {
                float labelWidth = ImGui::CalcTextSize(Localization::GetText("dash")).x;
                float centerX = (kw4 - labelWidth) * 0.5f;
                centerX = std::max(centerX, 10.0f); // Don't let text go left of padding
                ImGui::SetCursorPosX(centerX);
                ImGui::TextColored(kMuted, "%s", Localization::GetText("dash"));
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("best_drop_tooltip"));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 2. Timeline List (Grouped by Timestamp)
        auto allDrops = ItemTracker::GetSessionDropsCopy();
        std::vector<SessionHistory::DropEntry> drops;
        
        // Filter out ignored items/currencies
        for (const auto& drop : allDrops)
        {
            if (drop.isCurrency)
            {
                if (ItemTracker::IsCurrencyIgnored(drop.itemId)) continue;
            }
            else
            {
                if (ItemTracker::IsItemIgnored(drop.itemId)) continue;
            }
            drops.push_back(drop);
        }
        
        // Group drops by timestamp
        std::map<std::string, std::vector<SessionHistory::DropEntry>> groupedDrops;
        std::vector<std::string> timestamps; // Keep order
        
        for (const auto& drop : drops)
        {
            if (groupedDrops.find(drop.timestamp) == groupedDrops.end())
            {
                timestamps.push_back(drop.timestamp);
            }
            groupedDrops[drop.timestamp].push_back(drop);
        }
        
        std::reverse(timestamps.begin(), timestamps.end()); // Newest first

        int openItemMenuId = -1;
        std::string openItemMenuName;
        int openCurrencyMenuId = -1;
        std::string openCurrencyMenuName;

        if (ImGui::BeginChild("TimelineDropsScroll", ImVec2(0, 0), true))
        {
            if (timestamps.empty())
            {
                ImGui::TextDisabled("%s", Localization::GetText("timeline_no_drops"));
            }
            else
            {
                for (const auto& ts : timestamps)
                {
                    const auto& group = groupedDrops[ts];
                    
                    ImGui::PushID(ts.c_str());
                    
                    // Calculate total value and check for MF
                    long long groupValue = 0;
                    int groupMF = -1;
                    for (const auto& d : group)
                    {
                        groupValue += d.totalValue;
                        if (d.magicFind >= 0) groupMF = d.magicFind;
                    }

                    // 1. Time Header
                    // Use character name from the first drop in this group
                    std::string characterName = group.empty() ? "" : group[0].characterName;
                    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), characterName.empty() ? UICommon::s_AccountNameBuf : characterName.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.5f, 0.65f, 0.8f, 1.0f), "%s", ts.c_str());

                    // 2. MF if available
                    if (groupMF >= 0)
                    {
                        // Light green color for MF
                        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s: %d%%", Localization::GetText("magic_find"), groupMF);
                    }

                    // 3. Gold Value and Currencies in one row below "Item Drops"
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s:", Localization::GetText("timeline_liquid_coins"));
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "%s", UICommon::FormatCoin(groupValue).c_str());
                    
                    // Check if there are currencies to display
                    bool hasCurrencies = false;
                    for (const auto& d : group) if (d.isCurrency) { hasCurrencies = true; break; }
                    
                    if (hasCurrencies)
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", Localization::GetText("pipe"));
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s:", Localization::GetText("timeline_currencies"));
                        
                        // Group currencies by itemId and sum counts
                        std::map<int, int> currencyCounts;
                        std::map<int, std::string> currencyNames;
                        std::map<int, std::string> currencyIcons;
                        std::map<int, std::string> currencyRarities;
                        
                        for (const auto& d : group)
                        {
                            if (!d.isCurrency) continue;
                            currencyCounts[d.itemId] += d.count;
                            if (currencyNames.find(d.itemId) == currencyNames.end())
                            {
                                currencyNames[d.itemId] = d.itemName;
                                auto st = ItemTracker::GetCurrencyStat(d.itemId);
                                currencyIcons[d.itemId] = st.details.iconUrl;
                                currencyRarities[d.itemId] = st.details.rarity;
                            }
                        }
                        
                        // Display summed currencies with wrapping
                        float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                        float curIconSize = static_cast<float>(g_Settings.timelineIconSizeCurrencies);

                        bool firstCurrency = true;
                        for (const auto& [itemId, count] : currencyCounts)
                        {
                            ImGui::PushID(itemId);
                            
                            // itemId == 1 is the coin currency → format as G/S/C, everything else plain number
                            // But show profit if custom profit is set
                            long long currencyProfit = ItemTracker::GetStatProfit(ItemTracker::GetCurrencyStat(itemId)) / count * count; // This is a bit redundant but ensures we use the current session's count
                            // Actually, just use GetStatProfit with a dummy stat to get the unit profit, or just check if it has custom profit.
                            
                            auto st = ItemTracker::GetCurrencyStat(itemId);
                            long long totalProfit = ItemTracker::GetStatProfit(st);
                            // We need to be careful: the 'st' from ItemTracker might have a different count than the 'count' in this timeline group.
                            // So we calculate profit based on the unit profit from CustomProfitManager.
                            long long unitProfit = CustomProfitManager::HasCustomProfit(itemId) ? CustomProfitManager::GetCustomProfit(itemId) : 0;
                            long long displayProfit = unitProfit * count;

                            std::string countStr;
                            if (itemId == 1)
                                countStr = UICommon::FormatCoin(count);
                            else if (displayProfit != 0)
                                countStr = UICommon::FormatCoin(displayProfit);
                            else
                                countStr = UICommon::FormatCompact(count);

                            float itemWidth = ImGui::CalcTextSize(countStr.c_str()).x + curIconSize + ImGui::GetStyle().ItemSpacing.x * 2.0f;
                            
                            if (firstCurrency)
                            {
                                ImGui::SameLine();
                                firstCurrency = false;
                            }
                            else
                            {
                                float nextX2 = ImGui::GetCursorScreenPos().x + itemWidth;
                                if (nextX2 > windowVisibleX2)
                                    ImGui::NewLine();
                                else
                                    ImGui::SameLine();
                            }

                            UICommon::DrawItemIconCell(itemId, currencyIcons[itemId], curIconSize, currencyRarities[itemId]);
                            if (ImGui::IsItemHovered())
                            {
                                UITooltips::CurrencyTooltipOptions opt;
                                opt.showCount = true;
                                opt.count = count;
                                opt.showProfit = (displayProfit != 0);
                                opt.profit = displayProfit;
                                opt.showRarity = true;
                                opt.showId = true;
                                UITooltips::RenderCurrencyTooltipFallback(currencyNames[itemId], currencyRarities[itemId], itemId, opt);
                            }
                            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                            {
                                openCurrencyMenuId = itemId;
                                openCurrencyMenuName = currencyNames[itemId];
                            }
                            ImGui::SameLine();
                            
                            // Color currency text same as overview
                            ImVec4 textCol = (displayProfit != 0 || itemId == 1) ? (displayProfit < 0 ? ImVec4(0.9f,0.3f,0.3f,1.f) : ImVec4(0.95f,0.7f,0.1f,1.f)) : ImVec4(0.8f,0.8f,0.8f,1.f);
                            ImGui::TextColored(textCol, "%s", countStr.c_str());
                            if (ImGui::IsItemHovered())
                            {
                                UITooltips::CurrencyTooltipOptions opt;
                                opt.showCount = true;
                                opt.count = count;
                                opt.showProfit = (displayProfit != 0);
                                opt.profit = displayProfit;
                                opt.showRarity = false;
                                opt.showId = true;
                                auto st = ItemTracker::GetCurrencyStat(itemId);
                                if (st.details.loaded)
                                    UITooltips::RenderCurrencyTooltip(st.details, itemId, opt);
                                else
                                    UITooltips::RenderCurrencyTooltipFallback(currencyNames[itemId], "", itemId, opt);
                            }
                            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                            {
                                openCurrencyMenuId = itemId;
                                openCurrencyMenuName = currencyNames[itemId];
                            }
                            
                            ImGui::PopID();
                        }
                    }

                    ImGui::NewLine();

                    // 4. Icons Row
                    ImGui::Spacing();
                    float iconSize = static_cast<float>(g_Settings.timelineIconSizeItems);

                    // Group item drops by itemId and sum counts within the same timestamp
                    // Include items AND currencies that have a custom profit
                    std::map<int, SessionHistory::DropEntry> mergedItemMap;
                    std::vector<int> mergedItemOrder;
                    for (const auto& d : group)
                    {
                        bool isCustomProfitCurrency = d.isCurrency && CustomProfitManager::HasCustomProfit(d.itemId);
                        if (d.isCurrency && !isCustomProfitCurrency) continue;

                        if (mergedItemMap.find(d.itemId) == mergedItemMap.end())
                        {
                            mergedItemMap[d.itemId] = d;
                            // If it's a currency, ensure we have details for the icon row display
                            if (d.isCurrency)
                            {
                                auto st = ItemTracker::GetCurrencyStat(d.itemId);
                                mergedItemMap[d.itemId].iconUrl = st.details.iconUrl;
                                mergedItemMap[d.itemId].rarity = st.details.rarity;
                                mergedItemMap[d.itemId].itemName = st.details.name;
                            }
                            mergedItemOrder.push_back(d.itemId);
                        }
                        else
                        {
                            mergedItemMap[d.itemId].count      += d.count;
                            mergedItemMap[d.itemId].totalValue += d.totalValue;
                        }
                    }
                    std::vector<const SessionHistory::DropEntry*> itemDrops;
                    itemDrops.reserve(mergedItemOrder.size());
                    for (int id : mergedItemOrder)
                        itemDrops.push_back(&mergedItemMap[id]);

                    float spacingX = ImGui::GetStyle().ItemSpacing.x;
                    float availW = ImGui::GetContentRegionAvail().x;
                    int columns = std::max(1, static_cast<int>((availW + spacingX) / (iconSize + spacingX)));

                    for (int itemIndex = 0; itemIndex < static_cast<int>(itemDrops.size()); itemIndex++)
                    {
                        const auto& d = *itemDrops[itemIndex];

                        ImGui::PushID(itemIndex);
                        
                        if (itemIndex > 0)
                        {
                            if ((itemIndex % columns) == 0)
                                ImGui::NewLine();
                            else
                                ImGui::SameLine();
                        }

                        ImGui::BeginGroup();
                        UICommon::DrawItemIconCell(d.itemId, d.iconUrl, iconSize, d.rarity);
                        
                        // Draw count with new design (white + black outline)
                        char countStr[32];
                        snprintf(countStr, sizeof(countStr), "%d", d.count);
                        
                        ImVec2 origin = ImGui::GetItemRectMin();
                        
                        // Calculate font size as 54% of icon size (same as overview)
                        float desiredPixelSize = iconSize * 0.54f;
                        ImFont* font = ImGui::GetFont();
                        
                        // Text size calculation
                        ImVec2 textSize = font->CalcTextSizeA(desiredPixelSize, FLT_MAX, 0.0f, countStr);
                        
                        // Position: bottom right of icon
                        ImVec2 pos = ImVec2(origin.x + iconSize - textSize.x - 2.0f,
                                                 origin.y + iconSize - textSize.y - 2.0f);
                        
                        // 1. Very thin white outline (outer 8-way, 5 pixels)
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
                            ImGui::GetWindowDrawList()->AddText(font, desiredPixelSize, ImVec2(pos.x + off.x, pos.y + off.y), 
                                IM_COL32(255, 255, 255, 255), countStr);
                        }
                        
                        // 2. Thicker black outline (inner, 4 pixels)
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
                            ImGui::GetWindowDrawList()->AddText(font, desiredPixelSize, ImVec2(pos.x + off.x, pos.y + off.y), 
                                IM_COL32(0,0,0,255), countStr);
                        }
                        
                        // 3. Actual text with proper colors (same as overview)
                        ImVec4 col = d.count < 0 ? ImVec4(0.9f,0.3f,0.3f,1.f) : ImVec4(0.95f,0.7f,0.1f,1.f);
                        ImGui::GetWindowDrawList()->AddText(font, desiredPixelSize, pos, ImGui::ColorConvertFloat4ToU32(col), countStr);

                        if (ImGui::IsItemHovered())
                        {
                            if (d.isCurrency)
                            {
                                long long unitProfit = CustomProfitManager::HasCustomProfit(d.itemId) ? CustomProfitManager::GetCustomProfit(d.itemId) : 0;
                                long long displayProfit = unitProfit * d.count;
                                UITooltips::CurrencyTooltipOptions opt;
                                opt.showCount = true;
                                opt.count = d.count;
                                opt.showProfit = (displayProfit != 0);
                                opt.profit = displayProfit;
                                opt.showRarity = false;
                                opt.showId = true;
                                auto st = ItemTracker::GetCurrencyStat(d.itemId);
                                if (st.details.loaded)
                                    UITooltips::RenderCurrencyTooltip(st.details, d.itemId, opt);
                                else
                                    UITooltips::RenderCurrencyTooltipFallback(d.itemName, "", d.itemId, opt);
                            }
                            else
                            {
                                Stat st = ItemTracker::GetItemStat(d.itemId);
                                if (st.details.loaded)
                                {
                                    UITooltips::ItemTooltipOptions opt;
                                    opt.showCount = true;
                                    opt.count = d.count;
                                    opt.showProfit = false;
                                    opt.showTrading = true;
                                    opt.showAccountFlags = true;
                                    opt.showId = true;
                                    UITooltips::RenderItemTooltip(st.details, d.itemId, opt);
                                }
                                else
                                {
                                    UITooltips::ItemTooltipOptions opt;
                                    opt.showCount = true;
                                    opt.count = d.count;
                                    opt.showProfit = false;
                                    opt.showTrading = false;
                                    opt.showAccountFlags = false;
                                    opt.showId = true;
                                    UITooltips::RenderItemTooltipFallback(d.itemName, d.rarity, d.itemId, opt);
                                }
                            }
                        }
                        
                        // Right-click context menu
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                        {
                            if (d.isCurrency)
                            {
                                openCurrencyMenuId = d.itemId;
                                openCurrencyMenuName = d.itemName;
                            }
                            else
                            {
                                openItemMenuId = d.itemId;
                                openItemMenuName = d.itemName;
                            }
                        }
                        
                        ImGui::EndGroup();
                        ImGui::PopID();
                    }
                    ImGui::NewLine();

                    ImGui::PopID();
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
            }

        ImGui::EndChild();
        }

        // Open context menus if requested (Outside the child window ID scope)
        if (openCurrencyMenuId != -1)
            UIContextMenu::OpenContextMenu("TimelineCurrencyContextMenu", openCurrencyMenuId, openCurrencyMenuName);
        if (openItemMenuId != -1)
            UIContextMenu::OpenContextMenu("TimelineItemContextMenu", openItemMenuId, openItemMenuName);

        // Render context menus (Outside the child window ID scope)
        UIContextMenu::RenderCurrencyContextMenu("TimelineCurrencyContextMenu", UIContextMenu::ContextMenuType::General);
        UIContextMenu::RenderItemContextMenu("TimelineItemContextMenu", UIContextMenu::ContextMenuType::General);
    }
}
