#include "ui_context_menu.h"
#include "item_tracker.h"
#include "ignored_items.h"
#include "localization.h"
#include "imgui/imgui.h"
#include <cstdio>

namespace UIContextMenu
{
    // Static variables to store context for each popup
    static int s_ContextItemId = 0;
    static std::string s_ContextItemName;

    void OpenContextMenu(const char* popupName, int itemId, const std::string& itemName)
    {
        ImGui::OpenPopup(popupName);
        s_ContextItemId = itemId;
        s_ContextItemName = itemName;
    }

    void RenderItemContextMenu(const char* popupName, ContextMenuType type)
    {
        if (ImGui::BeginPopup(popupName))
        {
            // Favorites options
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_add_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, true);
                }
                if (ImGui::MenuItem(Localization::GetText("context_menu_remove_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, false);
                }
            }
            else if (type == ContextMenuType::Favorites)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_remove_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, false);
                }
            }
            else if (type == ContextMenuType::Ignored)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_add_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, true);
                }
            }

            ImGui::Separator();

            // Ignore options
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_ignore")))
                {
                    IgnoredItemsManager::IgnoreItem(s_ContextItemId);
                }
                if (ImGui::MenuItem(Localization::GetText("context_menu_unignore")))
                {
                    IgnoredItemsManager::UnignoreItem(s_ContextItemId);
                }
            }
            else if (type == ContextMenuType::Favorites)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_ignore")))
                {
                    IgnoredItemsManager::IgnoreItem(s_ContextItemId);
                }
            }
            else if (type == ContextMenuType::Ignored)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_unignore")))
                {
                    IgnoredItemsManager::UnignoreItem(s_ContextItemId);
                }
            }

            ImGui::Separator();

            // Copy options
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_name")))
            {
                ImGui::SetClipboardText(s_ContextItemName.c_str());
            }
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_id")))
            {
                char idStr[32];
                snprintf(idStr, sizeof(idStr), "%d", s_ContextItemId);
                ImGui::SetClipboardText(idStr);
            }

            ImGui::EndPopup();
        }
    }

    void RenderCurrencyContextMenu(const char* popupName, ContextMenuType type)
    {
        if (ImGui::BeginPopup(popupName))
        {
            // Favorites options
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_add_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, true);
                }
                if (ImGui::MenuItem(Localization::GetText("context_menu_remove_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, false);
                }
            }
            else if (type == ContextMenuType::Favorites)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_remove_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, false);
                }
            }
            else if (type == ContextMenuType::Ignored)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_add_favorites")))
                {
                    ItemTracker::SetFavorite(s_ContextItemId, true);
                }
            }

            ImGui::Separator();

            // Ignore options
            if (type == ContextMenuType::General || type == ContextMenuType::CustomProfit)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_ignore")))
                {
                    IgnoredItemsManager::IgnoreCurrency(s_ContextItemId);
                }
                if (ImGui::MenuItem(Localization::GetText("context_menu_unignore")))
                {
                    IgnoredItemsManager::UnignoreCurrency(s_ContextItemId);
                }
            }
            else if (type == ContextMenuType::Favorites)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_ignore")))
                {
                    IgnoredItemsManager::IgnoreCurrency(s_ContextItemId);
                }
            }
            else if (type == ContextMenuType::Ignored)
            {
                if (ImGui::MenuItem(Localization::GetText("context_menu_unignore")))
                {
                    IgnoredItemsManager::UnignoreCurrency(s_ContextItemId);
                }
            }

            ImGui::Separator();

            // Copy options
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_name")))
            {
                ImGui::SetClipboardText(s_ContextItemName.c_str());
            }
            if (ImGui::MenuItem(Localization::GetText("context_menu_copy_id")))
            {
                char idStr[32];
                snprintf(idStr, sizeof(idStr), "%d", s_ContextItemId);
                ImGui::SetClipboardText(idStr);
            }

            ImGui::EndPopup();
        }
    }
}
