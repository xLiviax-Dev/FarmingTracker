
# List of missing keys from the script
$missingKeys = @(
    "blacklist_add_item",
    "blacklist_add_item_tooltip",
    "blacklist_item_id",
    "blacklist_item_id_tooltip",
    "column_magic_find",
    "context_menu_delete",
    "context_menu_ignore_for_session",
    "context_menu_skip_once",
    "favorite_currencies",
    "history_icon_size",
    "history_icon_size_tooltip",
    "items_icon_size",
    "items_icon_size_tooltip",
    "main_window_hide_title_bar",
    "main_window_hide_title_bar_tooltip",
    "mini_window_best_drop_icon_size",
    "mini_window_best_drop_icon_size_tooltip",
    "mini_window_element_order",
    "mini_window_element_order_tooltip",
    "mini_window_enable_text_shadow",
    "mini_window_enable_text_shadow_tooltip",
    "mini_window_font_size",
    "mini_window_font_size_tooltip",
    "mini_window_hide_border",
    "mini_window_hide_border_tooltip",
    "mini_window_show_best_drop_icons",
    "mini_window_show_best_drop_icons_tooltip",
    "mini_window_text_color",
    "mini_window_text_color_tooltip",
    "notification_blacklist",
    "notification_blacklist_tooltip",
    "overview_favorites_icon_size",
    "overview_favorites_icon_size_tooltip",
    "profit_icon_size",
    "profit_icon_size_tooltip",
    "show_short_icon",
    "show_short_icon_tooltip",
    "sparkline_color",
    "sparkline_color_tooltip",
    "tab_content_font_size",
    "tab_content_font_size_tooltip"
)

# Read localization_en.cpp and extract the key-value pairs
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$output = @()
Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
        $key = $matches[1]
        $value = $matches[2]
        if ($missingKeys -contains $key) {
            $output += "            {`"$key`", `"$value`"},"
        }
    }
}

# Output the key-value pairs
Write-Output "Missing key-value pairs from localization_en.cpp:"
Write-Output "-----------------------------------------------"
$output | Write-Output
