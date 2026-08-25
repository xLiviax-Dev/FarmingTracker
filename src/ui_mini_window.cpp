#include "ui_mini_window.h"
#include "ui.h"
#include "ui_common.h"
#include "settings.h"
#include "shared.h"
#include "item_tracker.h"
#include "pinned_items.h"
#include "localization.h"
#include "material_storage_manager.h"

namespace UIMiniWindow
{
void RenderMiniWindow()
{
    if (!g_Settings.showMiniWindow) return;

    bool inCombat = IsInCombat();
    bool shouldShow = false;
    
    // Convert old InCombat mode to OutOfCombat (for backward compatibility)
    auto mode = g_Settings.miniWindowVisibilityMode;
    if (static_cast<int>(mode) > 1) {
        mode = MiniWindowVisibilityMode::OutOfCombat;
    }
    
    switch (mode)
    {
        case MiniWindowVisibilityMode::Always:
            shouldShow = true;
            break;
        case MiniWindowVisibilityMode::OutOfCombat:
            shouldShow = !inCombat;
            break;
    }
    if (!shouldShow) return;

    ImGui::SetNextWindowPos(ImVec2(g_Settings.miniWindowPosX, g_Settings.miniWindowPosY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(g_Settings.miniWindowWidth, g_Settings.miniWindowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(g_Settings.miniWindowOpacity);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (g_Settings.miniWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (g_Settings.miniWindowHideTitleBar)
        flags |= ImGuiWindowFlags_NoTitleBar;  // Title bar only — keep resize grip!

    if (g_Settings.miniWindowLocked)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("Farming Tracker Mini##FT_Mini", &g_Settings.showMiniWindow, flags))
    {
        PushAccentColor();

        // Apply font scale
        ImGui::SetWindowFontScale(g_Settings.miniWindowFontSize / 14.0f);

        // Convert text color from int to ImVec4 (RGB format)
        ImVec4 textColor = ImVec4(
            ((g_Settings.miniWindowTextColor >> 16) & 0xFF) / 255.0f,
            ((g_Settings.miniWindowTextColor >> 8) & 0xFF) / 255.0f,
            (g_Settings.miniWindowTextColor & 0xFF) / 255.0f,
            1.0f
        );

        // Save position
        ImVec2 pos = ImGui::GetWindowPos();
        g_Settings.miniWindowPosX = pos.x;
        g_Settings.miniWindowPosY = pos.y;

        // Save size
        ImVec2 size = ImGui::GetWindowSize();
        g_Settings.miniWindowWidth = size.x;
        g_Settings.miniWindowHeight = size.y;

        // Display selected metrics in custom order
        for (const auto& element : g_Settings.miniWindowElementOrder)
        {
            if (element == "Profit" && g_Settings.miniWindowShowProfit)
            {
                long long totalProfit = ItemTracker::CalcTotalCustomProfit();
                ImVec4 profitColor = totalProfit > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (totalProfit < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));

                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit"));
                ImGui::SameLine();

                std::string profitStr = UICommon::FormatCoin(totalProfit);
                UICommon::TextColoredWithSimpleOutline(profitColor, ImVec4(0, 0, 0, 1.0f), "%s", profitStr.c_str());
            }
            else if (element == "Profit/Hour" && g_Settings.miniWindowShowProfitPerHour)
            {
                auto duration = ItemTracker::GetSessionDuration();
                long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
                ImVec4 profitPerHourColor = profitPerHour > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (profitPerHour < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("profit_per_hour"));
                ImGui::SameLine();

                std::string profitPerHourStr = UICommon::FormatCoin(profitPerHour);
                UICommon::TextColoredWithSimpleOutline(profitPerHourColor, ImVec4(0, 0, 0, 1.0f), "%s", profitPerHourStr.c_str());
            }
            else if (element == "TP Sell" && g_Settings.miniWindowShowTradingProfitSell)
            {
                long long tpSell = ItemTracker::CalcTotalTpSellProfit();
                ImVec4 tpSellColor = tpSell > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (tpSell < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));

                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_sell"));
                ImGui::SameLine();

                std::string tpSellStr = UICommon::FormatCoin(tpSell);
                UICommon::TextColoredWithSimpleOutline(tpSellColor, ImVec4(0, 0, 0, 1.0f), "%s", tpSellStr.c_str());
            }
            else if (element == "TP Instant" && g_Settings.miniWindowShowTradingProfitInstant)
            {
                long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
                ImVec4 tpInstantColor = tpInstant > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (tpInstant < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));

                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("tp_instant"));
                ImGui::SameLine();

                std::string tpInstantStr = UICommon::FormatCoin(tpInstant);
                UICommon::TextColoredWithSimpleOutline(tpInstantColor, ImVec4(0, 0, 0, 1.0f), "%s", tpInstantStr.c_str());
            }
            else if (element == "Total Items" && g_Settings.miniWindowShowTotalItems)
            {
                const auto& items = ItemTracker::GetFilteredItemsView();
                size_t totalItems = items.size();
                ImVec4 totalItemsColor = totalItems > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (totalItems < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));

                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("total_items"));
                ImGui::SameLine();

                std::string totalItemsStr = std::to_string(totalItems);
                UICommon::TextColoredWithSimpleOutline(totalItemsColor, ImVec4(0, 0, 0, 1.0f), "%s", totalItemsStr.c_str());
            }
            else if (element == "Session Duration" && g_Settings.miniWindowShowSessionDuration)
            {
                auto duration = ItemTracker::GetSessionDuration();
                std::string durationStr = UICommon::FormatDuration(duration.count());

                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("session"));
                ImGui::SameLine();

                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s", durationStr.c_str());
            }
        }

        bool showSingle = false;
        if (g_Settings.miniWindowShowBestDropSingle)
        {
            auto bestDrop = ItemTracker::GetBestDrop();

            // Always show the section now instead of hiding it completely.
            // This avoids the "slot is just non-existent" user complaint.
            // If nothing found, we render a placeholder so the layout stays consistent.
            ImGui::Separator();

            // Best Drop Label
            UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_single"));

            if (bestDrop.first != 0 && bestDrop.second.count > 0)
            {
                // New line for icon and item name
                // Show icon if enabled
                if (g_Settings.miniWindowShowBestDropIcons)
                {
                    // Draw the icon
                    float iconSize = static_cast<float>(g_Settings.miniWindowBestDropIconSize);
                    UICommon::DrawItemIconCell(bestDrop.first, bestDrop.second.details.iconUrl, iconSize, bestDrop.second.details.rarity, true);
                    // Move cursor to the right of the icon
                    ImGui::SameLine(0.0f, 4.0f);
                }
                
                // For single drop, we show the unit value in the mini window to differentiate
                long long totalProfit = ItemTracker::GetStatProfit(bestDrop.second);
                long long unitProfit = totalProfit / bestDrop.second.count;
                
                ImVec4 bestDropColor = unitProfit > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (unitProfit < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));
                
                // Best Drop Name — show Item #<id> as a FALLBACK instead of a generic
                // "Loading..." so users can at least identify the item while API is busy.
                std::string bestDropName;
                if (bestDrop.second.details.loaded && !bestDrop.second.details.name.empty())
                    bestDropName = bestDrop.second.details.name;
                else
                    bestDropName = std::string("Item #") + std::to_string(bestDrop.first);
                UICommon::TextColoredWithSimpleOutline(bestDropColor, ImVec4(0, 0, 0, 1.0f), "%s", bestDropName.c_str());
                
                // Add small gap
                ImGui::Spacing();

                // Unit Value Label
                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("unit_value"));
                ImGui::SameLine();

                // Unit Value
                std::string unitProfitStr = UICommon::FormatCoin(unitProfit);
                UICommon::TextColoredWithSimpleOutline(bestDropColor, ImVec4(0, 0, 0, 1.0f), "%s", unitProfitStr.c_str());
                
                showSingle = true;
            }
            else
            {
                // Placeholder: no data yet
                ImGui::Spacing();
                ImGui::TextDisabled("  — %s", "No drops yet");
                ImGui::Spacing();
            }
        }

        if (g_Settings.miniWindowShowBestDropTotalValue)
        {
            auto bestTotal = ItemTracker::GetBestDropTotalValue();

            // Consistent layout: always show the section header
            if (!showSingle) ImGui::Separator();

            // Best Drop Total Label
            UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s: ", Localization::GetText("best_drop_total"));

            if (bestTotal.first != 0 && bestTotal.second.count > 0)
            {
                // New line for icon and item name
                
                // Show icon if enabled
                if (g_Settings.miniWindowShowBestDropIcons)
                {
                    // Draw the icon
                    float iconSize = static_cast<float>(g_Settings.miniWindowBestDropIconSize);
                    UICommon::DrawItemIconCell(bestTotal.first, bestTotal.second.details.iconUrl, iconSize, bestTotal.second.details.rarity, true);
                    // Move cursor to the right of the icon
                    ImGui::SameLine(0.0f, 4.0f);
                }
                
                long long bestTotalProfit = ItemTracker::GetStatProfit(bestTotal.second);
                ImVec4 bestTotalColor = bestTotalProfit > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : (bestTotalProfit < 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f));

                // Best Drop Total Name — same fallback as Single.
                std::string bestTotalName;
                if (bestTotal.second.details.loaded && !bestTotal.second.details.name.empty())
                    bestTotalName = bestTotal.second.details.name;
                else
                    bestTotalName = std::string("Item #") + std::to_string(bestTotal.first);
                UICommon::TextColoredWithSimpleOutline(bestTotalColor, ImVec4(0, 0, 0, 1.0f), "%s", bestTotalName.c_str());
                
                // Add small gap
                ImGui::Spacing();

                // Total Value Label
                std::string totalLabel = std::string(Localization::GetText("total_value")) + " (" + std::to_string(bestTotal.second.count) + "): ";
                UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s", totalLabel.c_str());
                ImGui::SameLine();

                // Total Value
                std::string totalProfitStr = UICommon::FormatCoin(bestTotalProfit);
                UICommon::TextColoredWithSimpleOutline(bestTotalColor, ImVec4(0, 0, 0, 1.0f), "%s", totalProfitStr.c_str());
            }
            else
            {
                // Placeholder: no data yet
                ImGui::Spacing();
                ImGui::TextDisabled("  — %s", "No drops yet");
                ImGui::Spacing();
            }
        }

        // Display pinned items at the bottom
        auto pinnedItems = PinnedItemsManager::GetPinnedItems();
        if (!pinnedItems.empty())
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Iterate in normal order so newest pins appear at the bottom
            for (size_t i = 0; i < pinnedItems.size(); ++i)
            {
                const auto& pinnedEntry = pinnedItems[i];
                Stat stat;
                if (pinnedEntry.type == StatType::Item)
                {
                    stat = ItemTracker::GetItemStat(pinnedEntry.apiId);
                }
                else
                {
                    stat = ItemTracker::GetCurrencyStat(pinnedEntry.apiId);
                }

                // Skip if item/currency doesn't exist or has no data
                if (stat.apiId == 0 && stat.count == 0)
                    continue;

                // Skip if hide_zero_drop_stats is enabled and drop count is 0
                if (g_Settings.miniWindowHideZeroDropStats && stat.count == 0)
                    continue;

                // Render icon + name + count
                ImGui::PushID(pinnedEntry.apiId);

                // Start of row - save cursor position
                ImVec2 rowStart = ImGui::GetCursorPos();

                float iconSize = g_Settings.miniWindowPinnedIconSize;
                float textLineHeight = ImGui::GetTextLineHeight();
                float rowHeight = std::max(iconSize, textLineHeight);
                float iconTopOffset = (rowHeight - iconSize) / 2.0f;
                float textTopOffset = (rowHeight - textLineHeight) / 2.0f;

                // Draw icon
                if (!g_Settings.miniWindowHideIcons && g_Settings.showItemIcons && stat.details.loaded)
                {
                    if (iconTopOffset > 0.0f)
                    {
                        ImGui::Dummy(ImVec2(0.0f, iconTopOffset));
                    }
                    UICommon::DrawItemIconCell(stat.apiId, stat.details.iconUrl, iconSize, stat.details.rarity, true);
                    ImGui::SameLine(0.0f, 4.0f);
                }

                // Align text vertically in row
                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, rowStart.y + textTopOffset));

                // Name with outline (only if hide_text_labels is not enabled)
                if (!g_Settings.miniWindowHideTextLabels)
                {
                    std::string displayName;
                    if (stat.details.loaded)
                        displayName = stat.details.name;
                    else
                        displayName = (stat.IsCurrency() ? "Currency #" : "Item #") + std::to_string(pinnedEntry.apiId);
                    UICommon::TextColoredWithSimpleOutline(textColor, ImVec4(0, 0, 0, 1.0f), "%s", displayName.c_str());
                    ImGui::SameLine();
                }

                // Count with outline — 5 separate blocks, each controlled by its own
                // toggle. Drop count is ALWAYS shown (session delta). Every other
                // source (Inventory, Bank, Material Storage, Wallet) is independently
                // toggleable so the user can pick exactly what they want to see.
                std::string countStr;
                if (g_Settings.miniWindowHideCountLabels)
                {
                    // Hide labels, show only numbers separated by |
                    countStr = std::to_string(stat.count);

                    if (g_Settings.miniWindowShowInventoryCount)
                    {
                        int invCount = MaterialStorageManager::GetInventoryCount(pinnedEntry.apiId);
                        if (invCount > 0 || pinnedEntry.type == StatType::Item)
                            countStr += " | " + std::to_string(invCount);
                    }
                    if (g_Settings.miniWindowShowBankCount)
                    {
                        int bankCount = MaterialStorageManager::GetBankCount(pinnedEntry.apiId);
                        if (bankCount > 0 || pinnedEntry.type == StatType::Item)
                            countStr += " | " + std::to_string(bankCount);
                    }
                    if (g_Settings.miniWindowShowMaterialStorageCount)
                    {
                        // Only show Mat count if the item can actually go into material storage
                        if (MaterialStorageManager::CanGoToMaterialStorage(pinnedEntry.apiId))
                        {
                            int storageCount = MaterialStorageManager::GetMaterialCount(pinnedEntry.apiId);
                            if (storageCount > 0 || pinnedEntry.type == StatType::Item)
                                countStr += " | " + std::to_string(storageCount);
                        }
                    }
                    if (g_Settings.miniWindowShowWalletCount)
                    {
                        int walletCount = MaterialStorageManager::GetWalletCount(pinnedEntry.apiId);
                        if (walletCount > 0 || pinnedEntry.type == StatType::Currency)
                            countStr += " | " + std::to_string(walletCount);
                    }
                }
                else
                {
                    // Show labels with numbers
                    std::string dropLabel = "(Drop)";
                    std::string invLabel = "Inv";
                    std::string bankLabel = "Bank";
                    std::string matLabel = "Mat";
                    std::string walletLabel = "Wallet";

                    // Use short labels if enabled
                    if (g_Settings.miniWindowShortCountLabels)
                    {
                        dropLabel = "(D)";
                        invLabel = "I";
                        bankLabel = "B";
                        matLabel = "M";
                        walletLabel = "W";
                    }

                    countStr = std::to_string(stat.count) + " " + dropLabel;

                    if (g_Settings.miniWindowShowInventoryCount)
                    {
                        int invCount = MaterialStorageManager::GetInventoryCount(pinnedEntry.apiId);
                        if (invCount > 0 || pinnedEntry.type == StatType::Item)
                            countStr += " | " + invLabel + ": " + std::to_string(invCount);
                    }
                    if (g_Settings.miniWindowShowBankCount)
                    {
                        int bankCount = MaterialStorageManager::GetBankCount(pinnedEntry.apiId);
                        if (bankCount > 0 || pinnedEntry.type == StatType::Item)
                            countStr += " | " + bankLabel + ": " + std::to_string(bankCount);
                    }
                    if (g_Settings.miniWindowShowMaterialStorageCount)
                    {
                        // Only show Mat count if the item can actually go into material storage
                        if (MaterialStorageManager::CanGoToMaterialStorage(pinnedEntry.apiId))
                        {
                            int storageCount = MaterialStorageManager::GetMaterialCount(pinnedEntry.apiId);
                            if (storageCount > 0 || pinnedEntry.type == StatType::Item)
                                countStr += " | " + matLabel + ": " + std::to_string(storageCount);
                        }
                    }
                    if (g_Settings.miniWindowShowWalletCount)
                    {
                        int walletCount = MaterialStorageManager::GetWalletCount(pinnedEntry.apiId);
                        if (walletCount > 0 || pinnedEntry.type == StatType::Currency)
                            countStr += " | " + walletLabel + ": " + std::to_string(walletCount);
                    }
                }

                ImVec4 countColor = stat.count > 0 ? ImVec4(1.f, 0.9f, 0.2f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f);
                UICommon::TextColoredWithSimpleOutline(countColor, ImVec4(0, 0, 0, 1.0f), "%s", countStr.c_str());

                // Set cursor to next row
                ImGui::SetCursorPosY(rowStart.y + rowHeight + 4.0f); // Add 4px spacing between rows

                // Invisible button over entire row for right-click detection
                ImGui::SetCursorPos(rowStart);
                ImGui::InvisibleButton("##pinned_item_row", ImVec2(g_Settings.miniWindowWidth - 20.0f, rowHeight));
                
                // Right-click to unpin (if enabled and not click-through)
                if (g_Settings.miniWindowAllowRightClickUnpin && !g_Settings.miniWindowClickThrough)
                {
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        PinnedItemsManager::Unpin(pinnedEntry.apiId, pinnedEntry.type);
                    }
                }

                ImGui::PopID();
            }
        }
    }

    PopAccentColor();
    ImGui::End();
}
}
