#include "ui_loot_log.h"
#include "loot_logger.h"
#include "settings.h"
#include "localization.h"
#include "ui_common.h"
#include "ui_tab_icons.h"
#include "shared.h"
#include "item_tracker.h"

#include <imgui/imgui.h>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>
#include <shellapi.h>
#include <process.h>

// Converts ItemType enum to a log-friendly string
static std::string ItemTypeToString(ItemType t)
{
    switch (t)
    {
        case ItemType::Armor:            return "Armor";
        case ItemType::Weapon:           return "Weapon";
        case ItemType::Trinket:          return "Trinket";
        case ItemType::Gizmo:            return "Gizmo";
        case ItemType::CraftingMaterial: return "CraftingMaterial";
        case ItemType::Consumable:       return "Consumable";
        case ItemType::Container:        return "Container";
        case ItemType::Bag:              return "Bag";
        case ItemType::Backpack:         return "Backpack";
        case ItemType::UpgradeComponent: return "UpgradeComponent";
        case ItemType::Tool:             return "Tool";
        case ItemType::Trophy:           return "Trophy";
        case ItemType::Unlock:           return "Unlock";
        case ItemType::MiniPet:          return "MiniPet";
        default:                         return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
namespace
{
    // Format copper value as "1g 23s 45c", or "—" if unknown
    std::string FormatCoin(long long copper)
    {
        if (copper < 0) return "\xe2\x80\x94"; // em-dash
        long long g = copper / 10000;
        long long s = (copper % 10000) / 100;
        long long c = copper % 100;
        std::string out;
        if (g) out += std::to_string(g) + "g ";
        if (s || g) out += std::to_string(s) + "s ";
        out += std::to_string(c) + "c";
        return out;
    }

    // Colour for rarity
    ImVec4 RarityColor(const std::string& rarity)
    {
        if (rarity == "Junk")        return ImVec4(0.60f, 0.60f, 0.60f, 1.f);
        if (rarity == "Basic")       return ImVec4(1.00f, 1.00f, 1.00f, 1.f);
        if (rarity == "Fine")        return ImVec4(0.35f, 0.55f, 0.90f, 1.f);
        if (rarity == "Masterwork")  return ImVec4(0.20f, 0.65f, 0.20f, 1.f);
        if (rarity == "Rare")        return ImVec4(0.90f, 0.80f, 0.10f, 1.f);
        if (rarity == "Exotic")      return ImVec4(0.95f, 0.55f, 0.10f, 1.f);
        if (rarity == "Ascended")    return ImVec4(0.85f, 0.20f, 0.50f, 1.f);
        if (rarity == "Legendary")   return ImVec4(0.55f, 0.20f, 0.90f, 1.f);
        return ImVec4(0.80f, 0.80f, 0.80f, 1.f);
    }

    // Filter state (persists across frames, reset on tab open is fine)
    static char  s_SearchBuf[256]  = "";
    static bool  s_FilterItems     = true;
    static bool  s_FilterCurrencies= true;
    static int   s_MaxRows         = 200; // cap live view rows

    // Summarise a filtered list of drops
    struct SummaryRow { std::string name; long long quantity; long long totalValue; };
    std::vector<SummaryRow> Summarise(const std::vector<LootLogger::DropEntry>& entries)
    {
        std::vector<SummaryRow> rows;
        for (const auto& e : entries)
        {
            auto it = std::find_if(rows.begin(), rows.end(),
                [&](const SummaryRow& r){ return r.name == e.itemName; });
            if (it != rows.end())
            {
                it->quantity   += e.quantity;
                it->totalValue += (e.sellPriceTp >= 0 ? e.sellPriceTp * e.quantity : 0);
            }
            else
            {
                long long tv = e.sellPriceTp >= 0 ? e.sellPriceTp * e.quantity : -1;
                rows.push_back({e.itemName, e.quantity, tv});
            }
        }
        // Sort by quantity descending
        std::sort(rows.begin(), rows.end(),
            [](const SummaryRow& a, const SummaryRow& b){ return a.quantity > b.quantity; });
        return rows;
    }

    // -----------------------------------------------------------------------
    // Sub-tab: Live Log
    // -----------------------------------------------------------------------
    void RenderLiveLog()
    {
        // --- Toolbar ---
        ImGui::SetNextItemWidth(220.f);
        ImGui::InputText("##LootSearch", s_SearchBuf, sizeof(s_SearchBuf));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_by_item_name_or_map"));

        ImGui::SameLine();
        ImGui::Checkbox("Items##filter",      &s_FilterItems);
        ImGui::SameLine();
        ImGui::Checkbox("Currencies##filter", &s_FilterCurrencies);

        // Summarise button (orange gradient)
        ImGui::SameLine(0, 16.f);
        static bool s_ShowSummary = false;
        if (UICommon::OrangeGradientButton(s_ShowSummary ? "Show raw" : "Summarise", "##sum"))
            s_ShowSummary = !s_ShowSummary;
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(
                s_ShowSummary
                ? "Switch back to per-drop view"
                : "Aggregate drops by item name"
            );

        // Open folder button — right-aligned (orange gradient)
        float btnW = 130.f;
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - btnW
                        + ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
        if (UICommon::OrangeGradientButton(Localization::GetText("open_log_folder"), "##open_log_folder"))
        {
            std::string folder = LootLogger::GetLogFolder();
            if (!folder.empty())
                ShellExecuteA(NULL, "explore", folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- Fetch and filter entries ---
        auto allEntries = LootLogger::GetCurrentSessionEntries();
        std::string search(s_SearchBuf);
        // Lowercase search
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);

        std::vector<LootLogger::DropEntry> filtered;
        for (const auto& e : allEntries)
        {
            if (e.itemType == "Currency" && !s_FilterCurrencies) continue;
            if (e.itemType != "Currency" && !s_FilterItems)      continue;
            
            // Skip ignored items/currencies
            if (e.itemType == "Currency")
            {
                if (ItemTracker::IsCurrencyIgnored(e.itemId)) continue;
            }
            else
            {
                if (ItemTracker::IsItemIgnored(e.itemId)) continue;
            }
            
            if (!search.empty())
            {
                std::string nameLower = e.itemName;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                std::string mapLower  = e.mapName;
                std::transform(mapLower.begin(),  mapLower.end(),  mapLower.begin(),  ::tolower);
                if (nameLower.find(search) == std::string::npos &&
                    mapLower.find(search)  == std::string::npos)
                    continue;
            }
            filtered.push_back(e);
        }

        // Show entry count + avg/h
        {
            long long totalValue = 0;
            for (const auto& e : filtered)
            {
                // Only count items/currencies with a known price
                // Currencies without explicit price (sellPriceTp == 0) are skipped
                if (e.sellPriceTp > 0)
                    totalValue += e.sellPriceTp * e.quantity;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
            ImGui::Text("%zu drops shown  |  est. value: %s",
                filtered.size(), FormatCoin(totalValue).c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextDisabled("(at drop time)");
        }

        ImGui::Spacing();

        // --- SUMMARY VIEW ---
        if (s_ShowSummary)
        {
            auto rows = Summarise(filtered);
            if (ImGui::BeginTable("LootSummary", 3,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp,
                ImVec2(0, 0)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Item",      ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Quantity",  ImGuiTableColumnFlags_WidthFixed, 80.f);
                ImGui::TableSetupColumn("Est. value",ImGuiTableColumnFlags_WidthFixed, 110.f);
                ImGui::TableHeadersRow();

                for (const auto& row : rows)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(row.name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%lld", row.quantity);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(FormatCoin(row.totalValue).c_str());
                }
                ImGui::EndTable();
            }
            return;
        }

        // --- RAW DROP VIEW ---
        // Show newest drops first (reverse)
        // Cap at s_MaxRows for performance
        int start = std::max(0, (int)filtered.size() - s_MaxRows);

        if (ImGui::BeginTable("LootLogTable", 9,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable,
            ImVec2(0, 0)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Time",       ImGuiTableColumnFlags_WidthFixed,   80.f);
            ImGui::TableSetupColumn("ID",         ImGuiTableColumnFlags_WidthFixed,   60.f);
            ImGui::TableSetupColumn("Item",       ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Qty",        ImGuiTableColumnFlags_WidthFixed,   45.f);
            ImGui::TableSetupColumn("Type",       ImGuiTableColumnFlags_WidthFixed,   90.f);
            ImGui::TableSetupColumn("TP Price",   ImGuiTableColumnFlags_WidthFixed,   80.f);
            ImGui::TableSetupColumn("Vendor",     ImGuiTableColumnFlags_WidthFixed,   80.f);
            ImGui::TableSetupColumn("MF",         ImGuiTableColumnFlags_WidthFixed,   50.f);
            ImGui::TableSetupColumn("Map",        ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            // Newest at top
            for (int i = (int)filtered.size() - 1; i >= start; --i)
            {
                const auto& e = filtered[i];
                ImGui::TableNextRow();

                // Time — show HH:MM:SS from ISO timestamp
                ImGui::TableSetColumnIndex(0);
                std::string timeStr = e.timestampUtc.size() >= 19
                    ? e.timestampUtc.substr(11, 8) : e.timestampUtc;
                ImGui::TextDisabled("%s", timeStr.c_str());

                // Item ID
                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled("%d", e.itemId);

                // Item name with rarity colour
                ImGui::TableSetColumnIndex(2);
                
                // Dynamically update name if details are now loaded
                std::string displayName = e.itemName;
                std::string displayRarity = e.rarity;
                std::string displayType = e.itemType;
                long long displayPrice = e.sellPriceTp;
                long long displayVendorPrice = e.vendorPrice;
                
                // Check if details are now loaded from ItemTracker
                if (e.itemName.find("Item #") == 0 || e.itemName.find("Currency #") == 0)
                {
                    if (e.itemType == "Currency")
                    {
                        auto stat = ItemTracker::GetCurrencyStat(e.itemId);
                        if (stat.details.loaded && !stat.details.name.empty())
                        {
                            displayName = stat.details.name;
                            displayRarity = stat.details.rarity;
                            displayType = "Currency";
                            // Recalculate price if details are now loaded
                            if (e.itemId == 1) // Coin
                            {
                                displayPrice = 1; // 1 copper per coin
                            }
                        }
                    }
                    else
                    {
                        auto stat = ItemTracker::GetItemStat(e.itemId);
                        if (stat.details.loaded && !stat.details.name.empty())
                        {
                            displayName = stat.details.name;
                            displayRarity = stat.details.rarity;
                            ItemType itemType = stat.details.itemType;
                            displayType = ItemTypeToString(itemType);
                            // Recalculate price if details are now loaded
                            long long vendorPrice = ItemTracker::CanSellToVendor(stat.details) ? (long long)stat.details.vendorValue : 0;
                            long long tpSellPrice = ItemTracker::CanSellOnTp(stat.details) ? ItemTracker::TpSellProceedsPerUnitCopper(stat.details) : 0;
                            displayPrice = std::max(vendorPrice, tpSellPrice);
                            displayVendorPrice = vendorPrice;
                        }
                        else
                        {
                            // Keep original type if details not loaded
                            displayType = e.itemType;
                            displayVendorPrice = e.vendorPrice;
                        }
                    }
                }
                
                if (!displayRarity.empty())
                    ImGui::TextColored(RarityColor(displayRarity), "%s", displayName.c_str());
                else
                    ImGui::TextUnformatted(displayName.c_str());
                if (ImGui::IsItemHovered() && displayPrice >= 0)
                {
                    std::string tooltip = "TP price: " + FormatCoin(displayPrice);
                    if (displayVendorPrice > 0)
                        tooltip += "\nVendor price: " + FormatCoin(displayVendorPrice);
                    ImGui::SetTooltip("%s", tooltip.c_str());
                }

                // Quantity
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%lld", e.quantity);

                // Type
                ImGui::TableSetColumnIndex(4);
                ImGui::TextDisabled("%s", displayType.c_str());

                // TP Price
                ImGui::TableSetColumnIndex(5);
                if (displayPrice >= 0)
                    ImGui::Text("%s", FormatCoin(displayPrice).c_str());
                else
                    ImGui::TextDisabled("\xe2\x80\x94");

                // Vendor Price
                ImGui::TableSetColumnIndex(6);
                if (displayVendorPrice > 0)
                    ImGui::Text("%s", FormatCoin(displayVendorPrice).c_str());
                else
                    ImGui::TextDisabled("\xe2\x80\x94");

                // Magic Find
                ImGui::TableSetColumnIndex(7);
                if (g_Settings.lootLogIncludeMagicFind && e.magicFind >= 0)
                    ImGui::Text("%d%%", e.magicFind);
                else
                    ImGui::TextDisabled("\xe2\x80\x94");

                // Map
                ImGui::TableSetColumnIndex(8);
                ImGui::TextDisabled("%s", e.mapName.empty()
                    ? ("map_" + std::to_string(e.mapId)).c_str()
                    : e.mapName.c_str());
            }

            ImGui::EndTable();
        }
    }

    // -----------------------------------------------------------------------
    // Toggle switch (matches ui_drops.cpp style)
    // -----------------------------------------------------------------------
    static bool Toggle(const char* id, bool* value)
    {
        const float  w      = 36.f;
        const float  h      = 18.f;
        const float  r      = h * 0.5f;
        const float  knobR  = r - 2.f;
        const ImVec4 colOn  = ImVec4(0.165f, 0.604f, 0.165f, 1.f); // Green when active
        const ImVec4 colOff = ImVec4(0.333f, 0.333f, 0.333f, 1.f);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImGui::InvisibleButton(id, ImVec2(w, h));
        bool changed = false;
        if (ImGui::IsItemClicked())
        {
            *value  = !(*value);
            changed = true;
        }

        ImVec4 trackCol = *value ? colOn : colOff;
        dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
            ImGui::ColorConvertFloat4ToU32(trackCol), r);

        float knobX = *value ? (pos.x + w - r) : (pos.x + r);
        dl->AddCircleFilled(ImVec2(knobX, pos.y + r), knobR,
            IM_COL32(255, 255, 255, 255));

        return changed;
    }

    // Render toggle without click handling (for use with InvisibleButton)
    static void RenderToggleOnly(const char* id, bool* value)
    {
        const float  w      = 36.f;
        const float  h      = 18.f;
        const float  r      = h * 0.5f;
        const float  knobR  = r - 2.f;
        const ImVec4 colOn  = ImVec4(0.165f, 0.604f, 0.165f, 1.f); // Green when active
        const ImVec4 colOff = ImVec4(0.333f, 0.333f, 0.333f, 1.f);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec4 trackCol = *value ? colOn : colOff;
        dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
            ImGui::ColorConvertFloat4ToU32(trackCol), r);

        float knobX = *value ? (pos.x + w - r) : (pos.x + r);
        dl->AddCircleFilled(ImVec2(knobX, pos.y + r), knobR,
            IM_COL32(255, 255, 255, 255));
    }

    // -----------------------------------------------------------------------
    // Helper functions for HTML-style design
    // -----------------------------------------------------------------------
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

    static void CardHeader(const char* iconKey, const char* label)
    {
        const float acR = g_Settings.accentColorR;
        const float acG = g_Settings.accentColorG;
        const float acB = g_Settings.accentColorB;

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 region = ImGui::GetContentRegionAvail();
        float headerHeight = 32.f;

        // Background gradient (matching active main tab design)
        ImU32 bgColorTop = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.0f));
        ImU32 bgColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.0f));
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec2 headerMin = cursor;
        ImVec2 headerMax = ImVec2(cursor.x + region.x, cursor.y + headerHeight);

        dl->AddRectFilledMultiColor(headerMin, headerMax, bgColorTop, bgColorTop, bgColorBottom, bgColorBottom);

        // Border (matching active tab border)
        ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.0f));
        dl->AddRect(headerMin, headerMax, borderColor, 4.0f, 0, 0.5f);

        // Icon - white
        void* tex = UITabIcons::GetIcon(iconKey);
        float sz  = 14.f;
        float lineH = ImGui::GetTextLineHeight();

        if (tex)
        {
            float offY = (lineH - sz) * 0.5f;
            dl->AddImage(
                (ImTextureID)tex,
                ImVec2(headerMin.x + 8.f, headerMin.y + (headerHeight - sz) * 0.5f),
                ImVec2(headerMin.x + 8.f + sz, headerMin.y + (headerHeight - sz) * 0.5f + sz),
                ImVec2(0,0), ImVec2(1,1),
                IM_COL32(255, 255, 255, 255));
        }

        // Label - white
        dl->AddText(ImVec2(headerMin.x + 26.f, headerMin.y + (headerHeight - lineH) * 0.5f),
                    IM_COL32(255, 255, 255, 255), label);

        ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y + 4.f));
    }

    static bool IconToggleRow(const char* iconKey, const char* label, bool* value,
                              float colWidth, bool indent = false)
    {
        const float iconSz   = 14.f;
        const float iconGap  = 5.f;
        const float toggleW  = 36.f;
        const float toggleH  = 18.f;
        const float indentW  = indent ? 16.f : 0.f;
        const float padX     = 8.0f;
        const float padY     = 4.0f;

        float curX = ImGui::GetCursorPosX();
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        float rowH = ImGui::GetTextLineHeight() + padY * 2.0f;

        // Draw gradient background with border and left accent bar (matching Filter tab design)
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float acR = g_Settings.accentColorR;
        const float acG = g_Settings.accentColorG;
        const float acB = g_Settings.accentColorB;

        // Gradient background colors (matching Filter tab design, 70% active, 16% inactive)
        ImU32 bgTopOn    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.7f));
        ImU32 bgBotOn    = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.25f, acG * 0.25f, acB * 0.25f, 0.7f));
        ImU32 bgTopOff   = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.16f));
        ImU32 bgBotOff   = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.25f, acG * 0.25f, acB * 0.25f, 0.16f));
        ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(acR * 0.75f, acG * 0.75f, acB * 0.75f, 0.5f));
        ImU32 accentBar  = ImGui::ColorConvertFloat4ToU32(ImVec4(acR, acG, acB, 0.8f));

        float rounding = 4.f;

        // Gradient background
        if (*value)
        {
            dl->AddRectFilledMultiColor(cursorPos, ImVec2(cursorPos.x + colWidth, cursorPos.y + rowH),
                bgTopOn, bgTopOn, bgBotOn, bgBotOn);
        }
        else
        {
            dl->AddRectFilledMultiColor(cursorPos, ImVec2(cursorPos.x + colWidth, cursorPos.y + rowH),
                bgTopOff, bgTopOff, bgBotOff, bgBotOff);
        }

        // Border
        dl->AddRect(cursorPos, ImVec2(cursorPos.x + colWidth, cursorPos.y + rowH), borderColor, rounding, 0, 0.5f);

        // Left accent bar
        dl->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y), ImVec2(cursorPos.x + 3.f, cursorPos.y + rowH), accentBar, 2.f);

        // Invisible button for click detection
        ImGui::SetCursorScreenPos(cursorPos);
        ImGui::InvisibleButton(("##itog_btn_" + std::string(label)).c_str(), ImVec2(colWidth, rowH));
        bool changed = false;
        if (ImGui::IsItemClicked())
        {
            *value = !(*value);
            changed = true;
        }

        // Move cursor back to start of row for rendering content
        ImGui::SetCursorScreenPos(cursorPos);

        // Left: icon + label
        ImGui::SetCursorPosX(curX + indentW + padX + 3.f);
        InlineIcon(iconKey, iconSz);

        const ImVec4 lblCol = indent
            ? ImVec4(0.67f, 0.67f, 0.67f, 1.f)
            : ImVec4(0.85f, 0.85f, 0.85f, 1.f);
        ImGui::SameLine(0, iconGap);
        ImGui::TextColored(lblCol, "%s", label);

        // Right: toggle (directly next to text, render only, no click handling)
        ImGui::SameLine(0, 8.f);

        char id[64];
        snprintf(id, sizeof(id), "##itog_%s", label);
        // Render toggle without click handling (click is handled by InvisibleButton)
        RenderToggleOnly(id, value);

        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + rowH + 4.f));
        return changed;
    }

    static void PathRowNew(const char* iconKey, const char* sublabel,
                           const std::string& effectivePath)
    {
        const float acR = g_Settings.accentColorR;
        const float acG = g_Settings.accentColorG;
        const float acB = g_Settings.accentColorB;

        float panelW = ImGui::GetContentRegionAvail().x;
        const float btnW = 60.f;
        const float changeBtnW = 70.f; // Change button 10px wider
        const float gap  = 6.f;
        float pathW = panelW - (btnW + gap) * 2.f - (changeBtnW + gap) - 4.f;

        // Label row
        InlineIcon(iconKey, 13.f);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", sublabel);
        ImGui::Spacing();

        // Path input
        ImGui::PushStyleColor(ImGuiCol_FrameBg,    ImVec4(0.10f, 0.10f, 0.10f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border,     ImVec4(0.25f, 0.25f, 0.25f, 1.f));
        ImGui::SetNextItemWidth(pathW);
        char buf[1024];
        strncpy_s(buf, effectivePath.c_str(), sizeof(buf) - 1);
        ImGui::InputText(("##pr_" + std::string(sublabel)).c_str(),
            buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor(2);

        // Copy button (orange gradient)
        ImGui::SameLine(0, gap);
        if (UICommon::OrangeGradientButton("Copy", ("##cp_" + std::string(sublabel)).c_str()))
        {
            ImGui::SetClipboardText(effectivePath.c_str());
        }

        // Open button (orange gradient)
        ImGui::SameLine(0, gap);
        if (UICommon::OrangeGradientButton("Open", ("##op_" + std::string(sublabel)).c_str()))
        {
            std::string cmd = "explorer.exe \"" + effectivePath + "\"";
            _spawnlp(_P_DETACH, "explorer.exe", cmd.c_str(), NULL);
        }

        // Change button (orange gradient)
        ImGui::SameLine(0, gap);
        if (UICommon::OrangeGradientButton(Localization::GetText("change_button"), ("##ch_" + std::string(sublabel)).c_str()))
        {
            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
            g_Settings.lootLogFolder.clear();
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        }

        ImGui::Spacing();
        ImGui::Spacing();
    }

    static bool SettingsToggleRow(const char* label, bool* value, float colWidth)
    {
        float toggleW = 36.f;
        float labelW  = colWidth - toggleW - 12.f;

        ImGui::BeginGroup();
        ImGui::SetNextItemWidth(labelW);
        ImGui::TextUnformatted(label);
        ImGui::EndGroup();

        ImGui::SameLine(colWidth - toggleW);

        char toggleId[128];
        snprintf(toggleId, sizeof(toggleId), "##tog_%s", label);
        bool changed = Toggle(toggleId, value);

        ImGui::Separator();
        return changed;
    }

    static void ColHeader(const char* label)
    {
        ImGui::TextUnformatted(label);
        ImGui::Separator();
        ImGui::Spacing();
    }

    // -----------------------------------------------------------------------
    // Sub-tab: Settings (HTML-style design)
    // -----------------------------------------------------------------------
    void RenderSettings()
    {
        float panelW = ImGui::GetContentRegionAvail().x;
        float colW   = (panelW - 10.f) * 0.5f;

        ImGui::Spacing();

        // ── Two-column card layout ──────────────────────────────────────────
        ImGui::BeginGroup();

        // Left card: Items
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##col_items", ImVec2(colW, 220.f), true);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Spacing();

        CardHeader("sword", "Items");

        if (IconToggleRow("toggle-left", "Enable Item Logging", &g_Settings.lootLogItems, colW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Log item drops.");

        if (IconToggleRow("star", "Favorites First", &g_Settings.itemsFavoritesFirst, colW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Show favorite items first.");

        if (IconToggleRow("filter", "Apply Active Filters", &g_Settings.filterFavorite, colW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Apply active loot filter settings.");

        ImGui::EndChild();
        ImGui::EndGroup();

        ImGui::SameLine(0, 10.f);

        // Right card: Currencies
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##col_currencies", ImVec2(colW, 220.f), true);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Spacing();

        CardHeader("coin", "Currencies");

        if (IconToggleRow("toggle-left", "Enable Currency Logging", &g_Settings.lootLogCurrencies, colW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Log currency gains.");

        ImGui::EndChild();
        ImGui::EndGroup();

        // ── Two-column: Format + Storage ─────────────────────────────────
        ImGui::Spacing();
        ImGui::BeginGroup();

        // Format card
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##col_format", ImVec2(colW, 150.f), true);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Spacing();

        CardHeader("file-description", "Format");

        ImGui::TextUnformatted(Localization::GetText("output"));
        ImGui::SameLine(colW - 100.f);
        const char* formats[] = { "CSV", "JSON", "Both" };
        ImGui::SetNextItemWidth(90.f);
        if (ImGui::Combo("##LootFormat", &g_Settings.lootLogFormat, formats, 3))
            BackgroundJobs::EnqueueDebouncedSettingsSave();

        ImGui::Spacing();
        ImGui::TextUnformatted(Localization::GetText("folder"));
        ImGui::SameLine(colW - 100.f);
        const char* folders[] = { "Per day", "Per session", "Both" };
        ImGui::SetNextItemWidth(90.f);
        static int folderMode = 0;
        ImGui::Combo("##LootFolderMode", &folderMode, folders, 3);

        ImGui::EndChild();
        ImGui::EndGroup();

        ImGui::SameLine(0, 10.f);

        // Storage card
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##col_storage", ImVec2(colW, 150.f), true);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Spacing();

        CardHeader("database", "Storage");

        ImGui::TextUnformatted(Localization::GetText("max_size_mb"));
        ImGui::SameLine(colW - 110.f);
        ImGui::SetNextItemWidth(100.f);
        static int maxSize = 50;
        ImGui::InputInt("##MaxSize", &maxSize, 1, 10);

        ImGui::Spacing();
        ImGui::TextUnformatted(Localization::GetText("delete_after_days"));
        ImGui::SameLine(colW - 110.f);
        ImGui::SetNextItemWidth(100.f);
        if (ImGui::InputInt("##LootMaxDays", &g_Settings.lootLogMaxDays, 1, 10))
        {
            g_Settings.lootLogMaxDays = std::max(0, g_Settings.lootLogMaxDays);
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        }

        ImGui::Spacing();
        if (IconToggleRow("history", "Persist after reset", &g_Settings.enableSessionHistory, colW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();

        ImGui::EndChild();
        ImGui::EndGroup();

        // ── General card ─────────────────────────────────────────────────────
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##col_general", ImVec2(panelW, 200.f), true);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Spacing();

        CardHeader("settings", "General");

        if (IconToggleRow("map-pin", "Include map name", &g_Settings.lootLogIncludeMap, panelW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add map_id and map_name to each row.");

        if (IconToggleRow("wand", "Include Magic Find", &g_Settings.lootLogIncludeMagicFind, panelW))
            BackgroundJobs::EnqueueDebouncedSettingsSave();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add magic_find to each row.");

        ImGui::EndChild();

        // ── Log path card ─────────────────────────────────────────────────────
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.11f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
        ImGui::BeginChild("##col_path", ImVec2(panelW, 100.f), true);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Spacing();

        CardHeader("folder-open", "Log path");

        std::string logPath = LootLogger::GetLogFolder();
        if (logPath.empty())
        {
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            logPath = std::string(addonDir ? addonDir : "") + "\\loot-logs\\";
        }

        PathRowNew("folder", "Current path", logPath);

        ImGui::EndChild();
    }


} // anonymous namespace

// ---------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------
namespace UILootLog
{
static int s_SubTab = 0; // 0 = Live Log, 1 = Settings

void RenderLootLogTab()
{
    UITabIcons::RenderSubPillTabBar({
        { "live_log", Localization::GetText("live_log_tab") },
        { "settings", Localization::GetText("settings_tab") }
    }, s_SubTab);

    switch (s_SubTab)
    {
        case 0:
            ImGui::Spacing();
            RenderLiveLog();
            break;
        case 1:
            ImGui::Spacing();
            RenderSettings();
            break;
    }
}

} // namespace UILootLog
