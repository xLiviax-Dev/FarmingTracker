#include "ui_loot_filter.h"
#include "ui_favorites.h"
#include "ui_ignored.h"
#include "ui_filter.h"
#include "ui_tab_icons.h"
#include "localization.h"

namespace UILootFilter
{
static int s_SubTab = 0; // 0 = Favorites, 1 = Ignored, 2 = Filter

void RenderLootFilterTab()
{
    UITabIcons::RenderSubPillTabBar({
        { "favorites", Localization::GetText("tab_favorites") },
        { "ignored",   Localization::GetText("tab_ignored")   },
        { "filter",    Localization::GetText("tab_filter")    }
    }, s_SubTab);

    switch (s_SubTab)
    {
        case 0: UIFavorites::RenderFavoritesTab(); break;
        case 1: UIIgnored::RenderIgnoredTab();     break;
        case 2: UIFilter::RenderFilterTab();       break;
    }
}
}
