#include "ui_context_menu.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "skip_once_manager.h"
#include "session_ignore_manager.h"
#include "localization.h"
#include "loot_logger.h"
#include "imgui/imgui.h"
#include <cstdio>
#include <map>
#include <string>

namespace UIContextMenu
{
    // Static variables to store context for each popup
    static std::map<std::string, int> s_ContextItemIds;
    static std::map<std::string, std::string> s_ContextItemNames;

    void OpenContextMenu(const char* popupName, int itemId, const std::string& itemName)
    {
        ImGui::OpenPopup(popupName);
        s_ContextItemIds[popupName] = itemId;
        s_ContextItemNames[popupName] = itemName;
    }

    void RenderItemContextMenu(const char* popupName, ContextMenuType type)
    {
        if (ImGui::BeginPopup(popupName))
        {
            int contextItemId = s_ContextItemIds[popupName];
            std::string contextItemName = s_ContextItemNames[popupName];
            
            bool isFavorite = ItemTracker::IsFavorite(contextItemId);
            bool isIgnored = IgnoredItemsManager::IsItemIgnored(contextItemId);

            // Favorites options
            if (type != ContextMenuType::CopyOnly)
            {
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit || type == ContextMenuType::Favorites || type == ContextMenuType::Ignored)
            {
                if (!isFavorite)
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_add_favorites")))
                    {
                        ItemTracker::SetFavorite(contextItemId, true);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_add_favorites_tooltip"));
                    }
                }
                else
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_remove_favorites")))
                    {
                        ItemTracker::SetFavorite(contextItemId, false);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_remove_favorites_tooltip"));
                    }
                }
            }

            ImGui::Separator();

            // Ignore options
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit || type == ContextMenuType::Favorites || type == ContextMenuType::Ignored)
            {
                if (!isIgnored)
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_ignore")))
                    {
                        // Remove from favorites first (UI layer enforces mutual exclusivity)
                        ItemTracker::SetFavorite(contextItemId, false);
                        IgnoredItemsManager::IgnoreItem(contextItemId);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_ignore_tooltip"));
                    }
                }
                else
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_unignore")))
                    {
                        IgnoredItemsManager::UnignoreItem(contextItemId);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_unignore_tooltip"));
                    }
                }
                
                if (ImGui::MenuItem(Localization::GetText("context_menu_ignore_for_session")))
                {
                    ItemTracker::ResetItemCount(contextItemId);
                    SessionIgnoreManager::IgnoreItemForSession(contextItemId);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", Localization::GetText("context_menu_ignore_for_session_tooltip"));
                }
                
                if (ImGui::MenuItem(Localization::GetText("context_menu_skip_once")))
                {
                    SkipOnceManager::SkipOnceItem(contextItemId);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", Localization::GetText("context_menu_skip_once_tooltip"));
                }
                
                if (ImGui::MenuItem(Localization::GetText("context_menu_delete")))
                {
                    ItemTracker::RemoveItem(contextItemId);
                    LootLogger::RemoveEntriesForItem(contextItemId);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", Localization::GetText("context_menu_delete_tooltip"));
                }
            }

            ImGui::Separator();
            }

            // Copy options
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_name")))
            {
                ImGui::SetClipboardText(contextItemName.c_str());
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", Localization::GetText("context_menu_copy_name_tooltip"));
            }
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_id")))
            {
                char idStr[32];
                snprintf(idStr, sizeof(idStr), "%d", contextItemId);
                ImGui::SetClipboardText(idStr);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", Localization::GetText("context_menu_copy_id_tooltip"));
            }

            ImGui::EndPopup();
        }
    }

    void RenderCurrencyContextMenu(const char* popupName, ContextMenuType type)
    {
        if (ImGui::BeginPopup(popupName))
        {
            int contextItemId = s_ContextItemIds[popupName];
            std::string contextItemName = s_ContextItemNames[popupName];
            
            bool isFavorite = ItemTracker::IsFavorite(contextItemId);
            bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(contextItemId);

            // Favorites options
            if (type != ContextMenuType::CopyOnly)
            {
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit || type == ContextMenuType::Favorites || type == ContextMenuType::Ignored)
            {
                if (!isFavorite)
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_add_favorites")))
                    {
                        ItemTracker::SetFavorite(contextItemId, true);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_add_favorites_tooltip"));
                    }
                }
                else
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_remove_favorites")))
                    {
                        ItemTracker::SetFavorite(contextItemId, false);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_remove_favorites_tooltip"));
                    }
                }
            }

            ImGui::Separator();

            // Ignore options
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit || type == ContextMenuType::Favorites || type == ContextMenuType::Ignored)
            {
                if (!isIgnored)
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_ignore")))
                    {
                        // Remove from favorites first (UI layer enforces mutual exclusivity)
                        ItemTracker::SetFavorite(contextItemId, false);
                        IgnoredItemsManager::IgnoreCurrency(contextItemId);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_ignore_tooltip"));
                    }
                }
                else
                {
                    if (ImGui::MenuItem(Localization::GetText("context_menu_unignore")))
                    {
                        IgnoredItemsManager::UnignoreCurrency(contextItemId);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", Localization::GetText("context_menu_unignore_tooltip"));
                    }
                }
                
                if (ImGui::MenuItem(Localization::GetText("context_menu_ignore_for_session")))
                {
                    ItemTracker::ResetCurrencyCount(contextItemId);
                    SessionIgnoreManager::IgnoreCurrencyForSession(contextItemId);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", Localization::GetText("context_menu_ignore_for_session_tooltip"));
                }
                
                if (ImGui::MenuItem(Localization::GetText("context_menu_skip_once")))
                {
                    SkipOnceManager::SkipOnceCurrency(contextItemId);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", Localization::GetText("context_menu_skip_once_tooltip"));
                }
                
                if (ImGui::MenuItem(Localization::GetText("context_menu_delete")))
                {
                    ItemTracker::RemoveCurrency(contextItemId);
                    LootLogger::RemoveEntriesForItem(contextItemId);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", Localization::GetText("context_menu_delete_tooltip"));
                }
            }

            ImGui::Separator();
            }

            // Copy options
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_name")))
            {
                ImGui::SetClipboardText(contextItemName.c_str());
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", Localization::GetText("context_menu_copy_name_tooltip"));
            }
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_id")))
            {
                char idStr[32];
                snprintf(idStr, sizeof(idStr), "%d", contextItemId);
                ImGui::SetClipboardText(idStr);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", Localization::GetText("context_menu_copy_id_tooltip"));
            }

            ImGui::EndPopup();
        }
    }
}
