#ifndef UI_CONTEXT_MENU_H
#define UI_CONTEXT_MENU_H

#include <string>

namespace UIContextMenu
{
    // Context menu types
    enum class ContextMenuType
    {
        General,        // General items/currencies (Items tab, Currencies tab)
        Favorites,      // Favorites tab (show "Remove from favorites")
        Ignored,        // Ignored tab (show "Remove from ignored")
        CustomProfit    // Custom Profit tab
    };

    // Render context menu for items
    // popupName: Unique name for the popup (e.g., "ItemContextMenu", "FavoriteItemContextMenu")
    // type: Type of context menu (determines which options are shown)
    void RenderItemContextMenu(const char* popupName, ContextMenuType type);

    // Render context menu for currencies
    // popupName: Unique name for the popup (e.g., "CurrencyContextMenu", "FavoriteCurrencyContextMenu")
    // type: Type of context menu (determines which options are shown)
    void RenderCurrencyContextMenu(const char* popupName, ContextMenuType type);

    // Helper function to open context menu
    // popupName: Unique name for the popup
    // itemId: ID of the item/currency
    // itemName: Name of the item/currency
    void OpenContextMenu(const char* popupName, int itemId, const std::string& itemName);
}

#endif // UI_CONTEXT_MENU_H
