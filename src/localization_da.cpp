// ---------------------------------------------------------------------------
// localization_da.cpp – Danish translations for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>

namespace Localization
{
    const std::unordered_map<std::string, const char*> GetDanishTranslations()
    {
        static const std::unordered_map<std::string, const char*> translations = {
            // Status texts
            {"status_disconnected", "Disconnected"},
            {"status_connecting", "Connecting..."},
            {"status_connected", "Connected"},
            {"status_auth_failed", "Auth Failed – check token"},
            {"status_reconnecting", "Reconnecting..."},
            {"status_error", "Error"},
            {"status_unknown", "Unknown"},

            // Mini Window
            {"mini_window_title", "Farming Tracker Mini"},
            {"profit", "Profit"},
            {"profit_per_hour", "Profit/Hour"},
            {"tp_sell", "TP Sell"},
            {"tp_instant", "TP Instant"},
            {"total_items", "Total Items"},
            {"session", "Session"},

            // Main Window
            {"main_window_title", "Farming Tracker"},
            {"drf_label", "DRF"},
            {"gw2_api_label", "GW2 API"},
            {"session_time_label", "Session Time"},
            {"reset_button", "Reset"},
            {"reset_tooltip", "Reset all farming counters (manual reset)"},

            // Tabs
            {"tab_summary", "Dashboard"},
            {"tab_drops", "Drops"},
            {"tab_loot_filter", "Loot Filter"},
            {"tab_items", "Items"},
            {"tab_currencies", "Currencies"},
            {"tab_dashboard", "Dashboard"},
            {"tab_favorites", "Favorites"},
            {"tab_ignored", "Ignored"},
            {"tab_timeline", "Timeline"},
            {"timeline_profit_hour_listings", "Approx. Trading Profits Per Hour (Listings)"},
            {"timeline_profit_hour_instant", "Approx. Trading Profits Per Hour (Instant Sell)"},
            {"timeline_no_drops", "No drops recorded in this session yet."},
            {"timeline_item_drops", "Item Drops"},
            {"timeline_currencies", "Currencies"},
            {"tab_filter", "Filter"},
            {"tab_custom_profit", "Custom Profit"},
            {"tab_debug", "Debug"},

            // Summary Tab
            {"warning_drf_not_connected", "⚠️ DRF not connected"},
            {"warning_drf_not_connected_desc", "This plugin requires DRF for data transmission."},
            {"warning_drf_install", "Install DRF via Nexus Addon Manager or https://drf.rs/"},
            {"warning_drf_token_invalid", "⚠️ DRF Token invalid"},
            {"warning_drf_token_invalid_desc", "Please check your DRF Token in Settings."},
            {"warning_gw2_api_key_not_set", "⚠️ GW2 API Key not set"},
            {"warning_gw2_api_key_not_set_desc", "Please set your GW2 API Key in Settings for item details."},
            {"gold", "Gold"},
            {"silver", "Silver"},
            {"copper", "Copper"},
            {"total_profit", "Total Profit"},
            {"total_profit_tooltip", "Total custom profit from all items"},
            {"total_items_count", "Total Items"},
            {"total_items_tooltip", "Total number of unique items tracked"},
            {"total_currencies", "Total Currencies"},
            {"total_currencies_tooltip", "Total number of unique currencies tracked"},
            {"profit_per_hour_label", "Profit Per Hour"},
            {"profit_per_hour_tooltip", "Profit per hour based on session duration"},
            {"magic_find", "Magic Find"},
            {"magic_find_tooltip", "Current or last recorded Magic Find from DRF"},
            {"session_duration", "Session duration"},
            {"session_duration_tooltip", "Current farming session duration"},
            {"date_tooltip", "Session start time"},
            {"duration_tooltip", "Session duration"},
            {"profit_tooltip", "Total session profit"},
            {"profit_per_hour_tooltip", "Profit per hour"},
            {"drops_tooltip", "Number of drops"},
            {"best_drop_tooltip", "Most valuable drop of the session"},
            {"top_items_profit", "Top Items (Profit)"},
            {"top_items_profit_tooltip", "Top 5 items by profit value"},
            {"loading", "Loading..."},
            {"coin", "Coin"},
            {"top_items_count", "Top Items (Count)"},
            {"top_items_count_tooltip", "Top 5 items by count"},
            {"top_currencies", "Top Currencies"},
            {"top_currencies_tooltip", "Top 5 currencies by count"},
            {"quick_statistics", "Quick Statistics"},
            {"quick_statistics_tooltip", "Farming statistics"},
            {"average_item_value", "Average Item Value"},
            {"average_item_value_na", "N/A"},
            {"total_unique_items", "Total Unique Items"},
            {"warning_no_data", "⚠️ No data loaded"},
            {"warning_no_data_desc", "Waiting for DRF data..."},
            {"export", "Export"},
            {"export_tooltip", "Export farming data to file"},
            {"export_json", "Export as JSON"},
            {"export_csv", "Export as CSV"},
            {"import_json", "Import from JSON"},

            // Items Tab
            {"search_hint", "Search items..."},
            {"clear", "Clear"},
            {"sort_count_high", "Sort: |Count| high"},
            {"sort_count_low", "Sort: |Count| low"},
            {"sort_id_up", "Sort: Item ID up"},
            {"sort_id_down", "Sort: Item ID down"},
            {"sort_name_az", "Sort: Name A–Z"},
            {"sort_tooltip", "Sort items by count, ID, or name"},
            {"rarity_all", "Rarity: all"},
            {"rarity_basic", "Rarity: Basic+"},
            {"rarity_fine", "Rarity: Fine+"},
            {"rarity_masterwork", "Rarity: Masterwork+"},
            {"rarity_rare", "Rarity: Rare+"},
            {"rarity_exotic", "Rarity: Exotic+"},
            {"rarity_ascended", "Rarity: Ascended+"},
            {"rarity_legendary", "Rarity: Legendary only"},
            {"rarity_tooltip", "Filter items by minimum rarity"},
            {"rarity", "Rarity"},
            {"type", "Type"},
            {"vendor_value", "Vendor Value"},
            {"tp_buy_net", "TP Buy (Net)"},
            {"account_bound", "Account-bound"},
            {"yes", "Yes"},
            {"no", "No"},
            {"nosell", "NoSell"},
            {"favorite", "Favorite"},
            {"ignore", "Ignore"},

            // Currencies Tab
            {"search_currencies_hint", "Search currencies..."},
            {"api_id", "API ID"},
            {"currency_name", "Currency Name"},
            {"count", "Count"},

            // Favorites Tab
            {"unfavorite_item", "Unfavorite"},
            {"unfavorite_selected", "Unfavorite Selected"},
            {"no_favorites_yet", "No favorites yet. Right-click on an item to add it."},
            {"toggle_favorite_tooltip", "Toggle favorite"},
            {"profits", "Profits"},
            {"profits_tooltip", "Total profits from farming"},
            {"approx_profits", "Approx. Profits"},
            {"approx_gold_per_hour", "Approx. Gold Per Hour"},
            {"trading_profits", "Trading Profits"},
            {"trading_profits_tooltip", "Profits from Trading Post"},

            // Profit Tab
            {"profits", "Profits"},
            {"profits_tooltip", "Total profits from farming"},
            {"approx_profits", "Approx. Profits"},
            {"approx_gold_per_hour", "Approx. Gold Per Hour"},
            {"trading_profits", "Trading Profits"},
            {"trading_profits_tooltip", "Profits from Trading Post"},
            {"approx_trading_profits_listings", "Approx. Trading Profits (Listings)"},
            {"approx_trading_profits_instant", "Approx. Trading Profits (Instant Sell)"},
            {"trading_details", "Trading Details (Opportunity Cost)"},
            {"trading_details_tooltip", "Opportunity cost of using items instead of selling"},
            {"lost_profit_vs_tp_sell", "Lost Profit (vs TP Sell)"},
            {"lost_profit_per_hour_vs_tp_sell", "Lost Profit Per Hour (vs TP Sell)"},
            {"efficiency_score", "Efficiency Score"},
            {"efficiency_score_label", "Efficiency Score:"},
            {"efficiency_score_tooltip", "How much of the maximum possible profit you achieved (Instant Sell vs. TP Listings)."},
            {"efficiency_score_desc", "You achieved %.1f%% of the maximum profit!"},
            {"session_duration_label", "Session duration"},
            {"session_duration_tooltip", "Current farming session duration"},

            // Filter Tab
            {"sell_method_filters", "Sell Method Filters"},
            {"sellable_to_vendor", "Sellable to vendor"},
            {"sellable_to_vendor_tooltip", "Show items sellable to vendor"},
            {"sellable_on_tp", "Sellable on TP"},
            {"sellable_on_tp_tooltip", "Show items sellable on Trading Post"},
            {"has_custom_profit", "Has custom profit"},
            {"has_custom_profit_tooltip", "Show items with custom profit values"},
            {"api_knowledge_filters", "API Knowledge Filters"},
            {"known_by_api", "Known by API"},
            {"known_by_api_tooltip", "Show items known by GW2 API"},
            {"unknown_by_api", "Unknown by API"},
            {"unknown_by_api_tooltip", "Show items not known by GW2 API"},
            {"item_type_filters", "Item Type Filters"},
            {"type_armor", "Armor"},
            {"type_armor_tooltip", "Show armor items"},
            {"type_weapon", "Weapon"},
            {"type_weapon_tooltip", "Show weapon items"},
            {"type_trinket", "Trinket"},
            {"type_trinket_tooltip", "Show trinket items"},
            {"type_gizmo", "Gizmo"},
            {"type_gizmo_tooltip", "Show gizmo items"},
            {"type_crafting_material", "Crafting Material"},
            {"type_crafting_material_tooltip", "Show crafting materials"},
            {"type_consumable", "Consumable"},
            {"type_consumable_tooltip", "Show consumable items"},
            {"type_gathering_tool", "Gathering Tool"},
            {"type_gathering_tool_tooltip", "Show gathering tools"},
            {"type_bag", "Bag"},
            {"type_bag_tooltip", "Show bags"},
            {"type_container", "Container"},
            {"type_container_tooltip", "Show containers"},
            {"type_mini_pet", "Mini Pet"},
            {"type_mini_pet_tooltip", "Show mini pets"},
            {"currency_filters_label", "Currency Filters"},
            {"currency_general", "General"},
            {"currency_main", "Main Currencies"},
            {"currency_fractal", "Fractal/Raid/Dungeon Currencies"},
            {"currency_wvw_pvp", "WvW/PvP Currencies"},
            {"currency_map", "Map-specific Currencies"},
            {"filter_karma", "Karma"},
            {"currency_karma_tooltip", "Show karma currency"},
            {"currency_laurel", "Laurel"},
            {"currency_laurel_tooltip", "Show laurel currency"},
            {"currency_gem", "Gem"},
            {"currency_gem_tooltip", "Show gem currency"},
            {"currency_fractal_relic", "Fractal Relic"},
            {"currency_fractal_relic_tooltip", "Show fractal relic currency"},
            {"currency_badge_of_honor", "Badge of Honor"},
            {"currency_badge_of_honor_tooltip", "Show badge of honor currency"},
            {"currency_guild_commendation", "Guild Commendation"},
            {"currency_guild_commendation_tooltip", "Show guild commendation currency"},
            {"currency_transmutation_charge", "Transmutation Charge"},
            {"currency_transmutation_charge_tooltip", "Show transmutation charge currency"},
            {"currency_spirit_shards", "Spirit Shards"},
            {"currency_spirit_shards_tooltip", "Show spirit shards currency"},
            {"currency_unbound_magic", "Unbound Magic"},
            {"currency_unbound_magic_tooltip", "Show unbound magic currency"},
            {"currency_volatile_magic", "Volatile Magic"},
            {"currency_volatile_magic_tooltip", "Show volatile magic currency"},
            {"currency_airship_parts", "Airship Parts"},
            {"currency_airship_parts_tooltip", "Show airship parts currency"},
            {"currency_geode", "Geode"},
            {"currency_geode_tooltip", "Show geode currency"},
            {"currency_ley_line_crystals", "Ley-Line Crystals"},
            {"currency_ley_line_crystals_tooltip", "Show ley-line crystals currency"},
            {"currency_trade_contracts", "Trade Contracts"},
            {"currency_trade_contracts_tooltip", "Show trade contracts currency"},
            {"currency_elegy_mosaic", "Elegy Mosaic"},
            {"currency_elegy_mosaic_tooltip", "Show elegy mosaic currency"},
            {"currency_uncommon_coins", "Uncommon Coins"},
            {"currency_uncommon_coins_tooltip", "Show uncommon coins currency"},
            {"currency_astral_acclaim", "Astral Acclaim"},
            {"currency_astral_acclaim_tooltip", "Show astral acclaim currency"},
            {"currency_pristine_fractal_relics", "Pristine Fractal Relics"},
            {"currency_pristine_fractal_relics_tooltip", "Show pristine fractal relics currency"},
            {"currency_unstable_fractal_essence", "Unstable Fractal Essence"},
            {"currency_unstable_fractal_essence_tooltip", "Show unstable fractal essence currency"},
            {"currency_magnetite_shards", "Magnetite Shards"},
            {"currency_magnetite_shards_tooltip", "Show magnetite shards currency"},
            {"currency_gaeting_crystals", "Gaeting Crystals"},
            {"currency_gaeting_crystals_tooltip", "Show gaeting crystals currency"},
            {"currency_prophet_shards", "Prophet Shards"},
            {"currency_prophet_shards_tooltip", "Show prophet shards currency"},
            {"currency_green_prophet_shards", "Green Prophet Shards"},
            {"currency_green_prophet_shards_tooltip", "Show green prophet shards currency"},
            {"currency_wvw_skirmish_tickets", "WvW Skirmish Tickets"},
            {"currency_wvw_skirmish_tickets_tooltip", "Show WvW skirmish tickets currency"},
            {"currency_proofs_of_heroics", "Proofs of Heroics"},
            {"currency_proofs_of_heroics_tooltip", "Show proofs of heroics currency"},
            {"currency_pvp_league_tickets", "PvP League Tickets"},
            {"currency_pvp_league_tickets_tooltip", "Show PvP league tickets currency"},
            {"currency_ascended_shards_of_glory", "Ascended Shards of Glory"},
            {"currency_ascended_shards_of_glory_tooltip", "Show ascended shards of glory currency"},
            {"currency_research_notes", "Research Notes"},
            {"currency_research_notes_tooltip", "Show research notes currency"},
            {"currency_tyrian_defense_seal", "Tyrian Defense Seal"},
            {"currency_tyrian_defense_seal_tooltip", "Show tyrian defense seal currency"},
            {"currency_testimony_of_desert_heroics", "Testimony of Desert Heroics"},
            {"currency_testimony_of_desert_heroics_tooltip", "Show testimony of desert heroics currency"},
            {"currency_testimony_of_jade_heroics", "Testimony of Jade Heroics"},
            {"currency_testimony_of_jade_heroics_tooltip", "Show testimony of jade heroics currency"},
            {"currency_testimony_of_castoran_heroics", "Testimony of Castoran Heroics"},
            {"currency_testimony_of_castoran_heroics_tooltip", "Show testimony of castoran heroics currency"},
            {"currency_legendary_insight", "Legendary Insight"},
            {"currency_legendary_insight_tooltip", "Show legendary insight currency"},
            {"currency_tales_of_dungeon_delving", "Tales of Dungeon Delving"},
            {"currency_tales_of_dungeon_delving_tooltip", "Show tales of dungeon delving currency"},
            {"currency_imperial_favor", "Imperial Favor"},
            {"currency_imperial_favor_tooltip", "Show imperial favor currency"},
            {"currency_canach_coins", "Canach Coins"},
            {"currency_canach_coins_tooltip", "Show canach coins currency"},
            {"currency_ancient_coin", "Ancient Coin"},
            {"currency_ancient_coin_tooltip", "Show ancient coin currency"},
            {"currency_unusual_coin", "Unusual Coin"},
            {"currency_unusual_coin_tooltip", "Show unusual coin currency"},
            {"currency_jade_sliver", "Jade Sliver"},
            {"currency_jade_sliver_tooltip", "Show jade sliver currency"},
            {"currency_static_charge", "Static Charge"},
            {"currency_static_charge_tooltip", "Show static charge currency"},
            {"currency_pinch_of_stardust", "Pinch of Stardust"},
            {"currency_pinch_of_stardust_tooltip", "Show pinch of stardust currency"},
            {"currency_calcified_gasp", "Calcified Gasp"},
            {"currency_calcified_gasp_tooltip", "Show calcified gasp currency"},
            {"currency_ursus_oblige", "Ursus Oblige"},
            {"currency_ursus_oblige_tooltip", "Show ursus oblige currency"},
            {"currency_gaeting_crystal_janthir", "Gaeting Crystal (Janthir)"},
            {"currency_gaeting_crystal_janthir_tooltip", "Show gaeting crystal (janthir) currency"},
            {"currency_antiquated_ducat", "Antiquated Ducat"},
            {"currency_antiquated_ducat_tooltip", "Show antiquated ducat currency"},
            {"currency_aether_rich_sap", "Aether-Rich Sap"},
            {"currency_aether_rich_sap_tooltip", "Show aether-rich sap currency"},

            // Additional Filters
            {"additional_filters", "Additional Filters"},
            {"account_bound", "Account-bound"},
            {"account_bound_tooltip", "Show account-bound items"},
            {"not_account_bound", "Not Account-bound"},
            {"not_account_bound_tooltip", "Show non-account-bound items"},
            {"nosell_items", "NoSell"},
            {"nosell_items_tooltip", "Show NoSell items"},
            {"not_nosell", "Not NoSell"},
            {"not_nosell_tooltip", "Show sellable items"},
            {"favorite_items", "Favorite"},
            {"favorite_items_tooltip", "Show favorite items"},
            {"not_favorite", "Not Favorite"},
            {"not_favorite_tooltip", "Show non-favorite items"},
            {"ignored_items", "Ignored"},
            {"ignored_items_tooltip", "Show ignored items"},
            {"not_ignored", "Not Ignored"},
            {"not_ignored_tooltip", "Show non-ignored items"},

            // Range Filters
            {"range_filters", "Range Filters"},
            {"show_range_filters", "Show Range Filters"},
            {"filter_min_price", "Filter Min Price"},
            {"filter_max_price", "Filter Max Price"},
            {"filter_min_quantity", "Filter Min Quantity"},
            {"filter_max_quantity", "Filter Max Quantity"},

            // Mini Window Settings
            {"mini_window_settings", "Mini Window"},
            {"show_profit", "Show Profit"},
            {"show_profit_tooltip", "Display total profit in mini window"},
            {"show_profit_per_hour", "Show Profit/Hour"},
            {"show_profit_per_hour_tooltip", "Display profit per hour in mini window"},
            {"show_tp_sell", "Show TP Sell (Listings)"},
            {"show_tp_sell_tooltip", "Display TP sell profit (listings) in mini window"},
            {"show_tp_instant", "Show TP Instant (Instant Sell)"},
            {"show_tp_instant_tooltip", "Display TP instant sell profit in mini window"},
            {"show_total_items", "Show Total Items"},
            {"show_total_items_tooltip", "Display total item count in mini window"},
            {"show_session_duration", "Show Session Duration"},
            {"show_session_duration_tooltip", "Display session duration in mini window"},
            {"window_click_through", "Window click through"},
            {"window_click_through_tooltip", "Allows clicking through the mini window to the game"},

            // Main Window Settings
            {"main_window", "Main Window"},
            {"click_through", "Click through"},
            {"click_through_tooltip", "Allows clicking through the main window to the game"},

            // Advanced UI Settings
            {"advanced_ui_settings", "Advanced UI Settings"},
            {"no_advanced_ui_settings", "(No advanced UI settings available)"},

            // Display Settings
            {"display_settings", "Display Settings"},
            {"show_item_icons", "Show Item Icons"},
            {"show_item_icons_tooltip", "Display item icons in the list"},
            {"show_rarity_borders", "Show Rarity Borders"},
            {"show_rarity_borders_tooltip", "Shows colored borders around icons based on rarity"},
            {"enable_grid_view", "Enable Grid View"},
            {"enable_grid_view_tooltip", "Display items in a grid layout instead of list"},
            {"grid_icon_size", "Grid Icon Size"},
            {"grid_icon_size_tooltip", "Size of icons in grid view"},

            // Count Display Settings
            {"count_display_settings", "Count Display Settings"},
            {"count_text_color", "Count Text Color"},
            {"count_text_color_tooltip", "Color of count text"},
            {"count_background_color", "Count Background Color"},
            {"count_background_color_tooltip", "Color of count background"},
            {"count_font_size", "Count Font Size"},
            {"count_font_size_tooltip", "Size of count font"},
            {"count_horizontal_alignment", "Count Horizontal Alignment"},
            {"count_horizontal_alignment_tooltip", "Horizontal alignment of count text"},

            // Gradient Background Settings
            {"gradient_background_settings", "Gradient Background Settings"},
            {"enable_gradient_backgrounds", "Enable Gradient Backgrounds"},
            {"enable_gradient_backgrounds_tooltip", "Enable gradient background for windows"},
            {"gradient_top_color", "Gradient Top Color"},
            {"gradient_top_color_tooltip", "Top color of gradient background"},
            {"gradient_bottom_color", "Gradient Bottom Color"},
            {"gradient_bottom_color_tooltip", "Bottom color of gradient background"},

            // Custom Profit System
            {"custom_profit_system", "Custom Profit System"},
            {"enable_custom_profit", "Enable Custom Profit"},
            {"enable_custom_profit_tooltip", "Enable custom profit values for items"},

            // Search
            {"search_settings", "Search"},
            {"enable_search", "Enable Search"},
            {"enable_search_tooltip", "Enable search functionality"},

            // Ignored Items
            {"ignored_items_settings", "Ignored Items"},
            {"enable_ignored_items", "Enable Ignored Items"},
            {"enable_ignored_items_tooltip", "Enable ignored items functionality"},

            // Auto Reset
            {"auto_reset_settings", "Auto Reset"},
            {"enable_auto_reset", "Enable Auto Reset"},
            {"enable_auto_reset_tooltip", "Automatically reset farming session after a duration"},
            {"auto_reset_duration", "Auto Reset Duration (minutes)"},
            {"auto_reset_duration_tooltip", "Duration in minutes before auto reset"},

            // DRF Settings
            {"drf_settings", "DRF Settings"},
            {"drf_token", "DRF Token"},
            {"drf_token_label", "DRF Token:"},
            {"drf_token_tooltip", "Your DRF authentication token"},
            {"edit_token", "Edit Token"},
            {"save_token", "Save Token"},

            // GW2 API Settings
            {"gw2_api_settings", "GW2 API Settings"},
            {"gw2_api_key", "GW2 API Key"},
            {"gw2_api_key_tooltip", "Your GW2 API key for item details"},
            {"edit_key", "Edit Key"},
            {"save_key", "Save Key"},

            // Language Settings
            {"language_settings", "Language"},
            {"language_tooltip", "Select interface language"},
            {"language_english", "English"},
            {"language_german", "Deutsch"},
            {"language_french", "Français"},
            {"language_spanish", "Español"},
            {"language_chinese", "Chinese"},
            {"language_czech", "Čeština"},
            {"language_italian", "Italiano"},
            {"language_polish", "Polski"},
            {"language_portuguese", "Português"},
            {"language_russian", "Русский"},

            // Additional hardcoded strings found in UI
            {"farming_tracker_title", "Farming Tracker"},
            {"no_accounts_configured", "No accounts configured"},
            {"no_profiles_created", "No profiles created yet"},
            {"count_label", "Count:"},
            {"profit_label", "Profit:"},
            {"no_profit", "No profit"},
            {"vendor_value_label", "Vendor Value:"},
            {"tp_sell_gross_label", "TP Sell (Gross):"},
            {"tp_sell_net_label", "TP Sell (Net):"},
            {"tp_buy_gross_label", "TP Buy (Gross):"},
            {"tp_buy_net_label", "TP Buy (Net):"},
            {"ignored_items_label", "Ignored Items:"},
            {"ignored_currencies_label", "Ignored Currencies:"},
            {"total_items_label", "Total Items:"},
            {"total_currencies_label", "Total Currencies:"},
            {"total_profit_label", "Total Profit:"},
            {"tp_sell_profit_label", "TP Sell Profit:"},
            {"tp_sell_profit_tooltip", "Total profit if all items were sold at current TP listing prices (minus 15% fee)"},
            {"vendor_profit_label", "Vendor Profit:"},
            {"profit_per_hour_label", "Profit Per Hour:"},
            {"opportunity_cost_profit_label", "Opportunity Cost Profit:"},
            {"opportunity_cost_profit_per_hour_label", "Opportunity Cost Profit/Hour:"},
            {"custom_profit_feature_placeholder", "Feature implemented - UI follows"},
            {"custom_profit_items_header", "Items with Custom Profit"},
            {"custom_profit_currencies_header", "Currencies with Custom Profit"},
            {"add_custom_profit_item", "Add Custom Profit for Item"},
            {"add_custom_profit_currency", "Add Custom Profit for Currency"},
            {"custom_profit_set_profit", "Set Profit"},
            {"custom_profit_remove", "Remove"},
            {"custom_profit_value", "Profit Value (Copper)"},
            {"custom_profit_set_tooltip", "Set custom profit value for this item"},
            {"custom_profit_remove_tooltip", "Remove custom profit value for this item"},
            {"no_custom_profit_items", "(No items with custom profit)"},
            {"no_custom_profit_currencies", "(No currencies with custom profit)"},
            {"clear_all_custom_profits", "Clear All Custom Profits"},
            {"clear_all_custom_profits_tooltip", "Clear all custom profit values"},
            {"tabs_settings", "Other Tabs"},
            {"tabs_description", "Show or hide other tabs"},
            {"tab_settings", "Tab Settings"},
            {"tab_settings_description", "Tab ordering and behavior"},
            {"enable_dashboard_tab", "Enable Dashboard Tab"},
            {"enable_dashboard_tab_tooltip", "Show the Dashboard tab"},
            {"enable_items_tab", "Enable Items Tab"},
            {"enable_items_tab_tooltip", "Show the Items tab"},
            {"enable_currencies_tab", "Enable Currencies Tab"},
            {"enable_currencies_tab_tooltip", "Show the Currencies tab"},
            {"enable_ignored_tab", "Enable Ignored Tab"},
            {"enable_ignored_tab_tooltip", "Show the ignored items tab"},
            {"enable_session_history_tab", "Enable Session History Tab"},
            {"enable_session_history_tab_tooltip", "Show the Session History tab"},
            {"enable_timeline_tab", "Enable Timeline Tab"},
            {"enable_timeline_tab_tooltip", "Show the Timeline tab with detailed drop history"},
            {"enable_filter_tab", "Enable Filter Tab"},
            {"enable_filter_tab_tooltip", "Show the Filter tab"},
            {"lock_tab_order", "Lock Tab Order"},
            {"lock_tab_order_tooltip", "Disable reordering of tabs in the main window"},
            {"enable_summaries_tab", "Enable Summaries Tab"},
            {"enable_summaries_tab_tooltip", "Show the daily/weekly/monthly summaries tab in session history"},
            {"custom_profit_settings", "Custom Profit"},
            {"total_profit_label_simple", "Total Profit"},
            {"total_items_label_simple", "Total Items"},
            {"total_currencies_label_simple", "Total Currencies"},
            {"profit_per_hour_label_simple", "Profit Per Hour"},
            {"session_duration_label_simple", "Session Duration"},
            {"next_reset_label_simple", "Next Reset"},
            {"export_label", "Export:"},
            {"quick_actions", "Quick Actions:"},
            {"reset_confirm", "Are you sure you want to reset all settings to defaults?"},
            {"reset_warning", "This action cannot be undone."},
            {"hotkeys", "Hotkeys"},
            {"mini_window_toggle_hotkey", "Mini Window Toggle Hotkey"},
            {"backup_restore", "Backup & Restore"},
            {"appearance_settings", "Appearance"},
            {"enable_tooltips", "Enable Tooltips"},
            {"enable_tooltips_tooltip", "Show tooltips when hovering over UI elements"},
            {"enable_grid_view_tooltip", "Display items in a grid layout instead of a list"},
            {"favorites_first_tooltip", "Show favorite items at the top of the list"},
            {"group_by_rarity_tooltip", "Group items by their rarity"},
            {"show_rarity_as_tabs_tooltip", "Display each rarity as a separate tab"},
            {"group_by_category_tooltip", "Group items by their category"},
            {"show_group_as_tabs_tooltip", "Display each category as a separate tab"},
            {"mass_ignore_rarity_tooltip", "Ignore all items of this rarity"},
            {"icons_borders", "Icons & Borders"},
            {"colors_gradients", "Colors & Gradients"},
            {"window_opacity", "Window Opacity"},
            {"windows_settings", "Windows"},
            {"advanced_settings", "Advanced"},
            {"export_settings", "Export Settings to File:"},
            {"import_settings", "Import Settings from File:"},
            {"edit_account", "Edit Account: %s"},
            {"account_name", "Account Name:"},
            {"gw2_api_key_label", "GW2 API Key:"},
            {"reload_config", "Reload Configuration:"},
            {"auto_reset_label", "Automatic reset:"},
            {"next_reset_utc", "Next scheduled reset (UTC): %s"},
            {"favorites_ui", "Favorites UI:"},
            {"favorites_colors", "Favorites Colors:"},
            {"visual_enhancements", "Visual Enhancements:"},
            {"show_profit_sparkline", "Show Profit Sparkline"},
            {"show_profit_sparkline_tooltip", "Display a small line chart showing profit per hour trend"},
            {"mini_window_widget", "Mini Window (Overlay Widget):"},
            {"main_window_label", "Main Window:"},
            {"profiles_description", "Profiles allow you to save different configurations and switch between them quickly."},
            {"create_new_profile", "Create New Profile:"},
            {"current_profile", "Current Profile: %s"},
            {"auto_backup", "Automatically backup your settings before major changes"},
            {"notifications", "Configure in-game notifications for important events"},
            {"profit_goal", "Profit Goal:"},
            {"reset_warning_label", "Reset Warning:"},
            {"session_complete", "Session Complete:"},
            {"manage_ignored_items", "Manage ignored items"},
            {"manage_ignored_currencies", "Manage ignored currencies"},
            {"rarity_label", "Rarity: %s"},
            {"type_label", "Type: %d"},
            {"account_bound_label", "Account Bound: %s"},
            {"nosell_label", "NoSell: %s"},
            {"item_id_label", "Item ID: %d"},
            {"currency_id_label", "Currency ID: %d"},
            {"context_menu_add_favorites", "Add to Favorites"},
            {"context_menu_remove_favorites", "Remove from Favorites"},
            {"context_menu_ignore", "Ignore Item"},
            {"context_menu_unignore", "Remove from Ignored"},
            {"context_menu_copy_name", "Copy Name"},
            {"context_menu_copy_id", "Copy ID"},
            {"sell_method_filters_label", "Sell Method Filters:"},
            {"api_knowledge_filters_label", "API Knowledge Filters:"},
            {"additional_filters_label", "Additional Filters:"},
            {"item_type_filters_label", "Item Type Filters:"},
            {"currency_filters_label", "Currency Filters:"},
            {"price_range", "Price Range (Copper):"},
            {"quantity_range", "Quantity Range:"},
            {"debug_info", "Debug Information"},
            {"drf_status", "DRF Status: %s"},
            {"drf_reconnect_count", "DRF Reconnect Count: %d"},
            {"gw2_api_status", "GW2 API Status: %s"},
            {"gw2_api_reconnect_count", "GW2 API Reconnect Count: %d"},
            {"session_duration_debug", "Session Duration: %s"},
            {"gw2_memory", "GW2 Process Memory: %zu MB"},
            {"gw2_api_request_count", "GW2 API Request Count: %d"},
            {"ignored_items_count", "Ignored Items: %d"},
            {"ignored_currencies_count", "Ignored Currencies: %d"},
            {"drf_logs", "DRF Logs:"},
            {"last_100_entries", "(Last 100 entries)"},
            {"gw2_api_logs", "GW2 API Logs:"},
            {"item_currency_details", "Item/Currency Details (First 5):"},
            {"item_label", "Item %d: %s (Count: "},
            {"loaded_label", ", Loaded: %s)"},
            {"currency_label", "Currency %d: %s (Count: "},
            {"custom_profit_items", "Custom Profit Items (First 5):"},
            {"custom_profit_item", "Item %d: %s (Custom Profit: "},
            {"no_custom_profit_items", "(No custom profit items)"},
            {"ignored_items_debug", "Ignored Items (First 5):"},
            {"yes_label", "Yes"},
            {"no_label", "No"},
            {"profits_label", "Profits:"},
            {"profits_tooltip", "Total profits from farming"},
            {"approx_profits_label", "Approx. Profits:"},
            {"approx_profits_tooltip", "Total profit from MAX(Vendor, TP Sell with 15% fee) or Custom Profit"},
            {"approx_gold_per_hour_label", "Approx. Gold Per Hour:"},
            {"approx_gold_per_hour_tooltip", "Profit per hour based on session duration"},
            {"trading_profits_label", "Trading Profits:"},
            {"trading_profits_tooltip", "Profits from selling items on Trading Post"},
            {"approx_trading_profits_listings_label", "Approx. Trading Profits (Listings):"},
            {"approx_trading_profits_listings_tooltip", "Total profit if sold via TP listings (15% fee deducted)"},
            {"approx_trading_profits_instant_label", "Approx. Trading Profits (Instant Sell):"},
            {"approx_trading_profits_instant_tooltip", "Total profit if sold via TP instant buy orders (15% fee deducted)"},
            {"trading_details_label", "Trading Details (Opportunity Cost):"},
            {"trading_details_tooltip", "Profit lost by not selling via TP listings"},
            {"lost_profit_vs_tp_sell_label", "Lost Profit (vs TP Sell):"},
            {"lost_profit_vs_tp_sell_tooltip", "Opportunity cost: Profit lost by not selling via TP (with 15% fee)"},
            {"lost_profit_per_hour_vs_tp_sell_label", "Lost Profit Per Hour (vs TP Sell):"},
            {"lost_profit_per_hour_vs_tp_sell_tooltip", "Opportunity cost per hour"},
            {"session_duration_debug_label", "Session duration: %s"},
            {"session_duration_debug_tooltip", "Current farming session duration"},
            {"tab_items", "Items"},
            {"manage_ignored_items", "Manage ignored items"},
            {"clear_all_ignored_items", "Clear all ignored items"},
            {"unignore_item", "Unignore item"},
            {"manage_favorite_items", "Manage favorite items"},
            {"favorite_items_label", "Favorite items:"},
            {"clear_all_favorite_items", "Clear all favorite items"},
            {"tab_currencies", "Currencies"},
            {"manage_ignored_currencies", "Manage ignored currencies"},
            {"clear_all_ignored_currencies", "Clear all ignored currencies"},
            {"unignore_currency", "Unignore currency"},
            {"manage_favorite_currencies", "Manage favorite currencies"},
            {"favorite_currencies_label", "Favorite currencies:"},
            {"clear_all_favorite_currencies", "Clear all favorite currencies"},
            {"filter_active",   "Active"},
            {"filter_inactive", "Inactive"},
            {"filter_all", "All"},
            {"filter_none", "None"},
            {"filter_reset_all", "Reset All"},
            {"filter_search_hint", "Search filter..."},
            {"filter_active_count", "%d filters active"},
            {"sell_method_filters_label", "Sell Method Filters:"},
            {"api_knowledge_filters_label", "API Knowledge Filters:"},
            {"additional_filters_label", "Additional Filters:"},
            {"item_type_filters_label", "Item Type Filters:"},
            {"currency_filters_label", "Currency Filters:"},
            {"price_range", "Price Range (Copper):"},
            {"quantity_range", "Quantity Range:"},
            {"debug_connection_status", "Connection Status"},
            {"debug_session_snapshot", "Session Snapshot"},
            {"debug_profit_breakdown", "Profit Breakdown"},
            {"debug_data_state", "Data State"},
            {"debug_logs", "Logs"},
            {"debug_favorites", "Favorites"},
            {"debug_total_session", "total this session"},
            {"debug_after_tp_fee", "after 15% fee"},
            {"debug_direct_sell", "direct sell"},
            {"debug_rolling_avg", "rolling average"},
            {"debug_vs_tp_sell", "vs TP sell"},
            {"debug_per_hour", "per hour"},
            {"settings_api_key", "API Key"},
            {"settings_drf_token", "DRF Token"},
            {"debug_information", "Debug Information"},
            {"drf_status_label", "DRF Status: %s"},
            {"drf_status_tooltip", "Current DRF connection status"},
            {"drf_reconnect_count_label", "DRF Reconnect Count: %d"},
            {"drf_reconnect_count_tooltip", "Number of DRF reconnection attempts"},
            {"gw2_api_status_label", "GW2 API Status: %s"},
            {"gw2_api_status_tooltip", "Current GW2 API connection status"},
            {"gw2_api_reconnect_count_label", "GW2 API Reconnect Count: %d"},
            {"gw2_api_reconnect_count_tooltip", "Number of GW2 API reconnection attempts"},
            {"session_duration_debug", "Session Duration: %s"},
            {"session_duration_debug_tooltip", "Current farming session duration"},
            {"gw2_process_memory_label", "GW2 Process Memory: %zu MB"},
            {"gw2_process_memory_tooltip", "Current GW2 process memory usage"},
            {"gw2_api_request_count_label", "GW2 API Request Count: %d"},
            {"gw2_api_request_count_tooltip", "Total number of GW2 API requests made"},
            {"ignored_items_debug_label", "Ignored Items: %d"},
            {"ignored_items_debug_tooltip", "Number of ignored items"},
            {"ignored_currencies_debug_label", "Ignored Currencies: %d"},
            {"ignored_currencies_debug_tooltip", "Number of ignored currencies"},
            {"drf_logs_label", "DRF Logs:"},
            {"clear_drf_logs", "Clear DRF Logs"},
            {"clear_drf_logs_tooltip", "Clear all DRF log entries"},
            {"last_100_entries", "(Last 100 entries)"},
            {"gw2_api_logs_label", "GW2 API Logs:"},
            {"clear_gw2_logs", "Clear GW2 Logs"},
            {"clear_gw2_logs_tooltip", "Clear all GW2 API log entries"},
            {"settings_label", "Settings:"},
            {"api_key_tooltip", "GW2 API Key Status"},
            {"not_set", "Not set"},
            {"set", "Set"},
            {"drf_token_tooltip", "DRF Token Status"},
            {"toggle_hotkey_label", "Toggle Hotkey: %s"},
            {"toggle_hotkey_tooltip", "Main window toggle hotkey"},
            {"auto_reset_mode_label", "Auto-Reset Mode: %d"},
            {"auto_reset_mode_tooltip", "Current automatic reset mode"},
            {"next_reset_label", "Next Reset: %s"},
            {"next_reset_tooltip", "Next scheduled reset time (UTC)"},
            {"fake_drf_server_label", "Fake DRF Server:"},
            {"use_fake_drf_server", "Use fake DRF server"},
            {"use_fake_drf_server_tooltip", "For testing purposes only"},
            {"reset_all_data", "Reset All Data"},
            {"reset_all_data_tooltip", "Reset all farming data"},
            {"coin", "Coin"},
            {"info_button", "Info"},
            {"info_title", "FarmingTracker Info"},
            {"info_text", "Help text will be added here later..."},
            {"close_button", "Close"},
            {"rarity_label", "Rarity: %s"},
            {"type_label", "Type: %d"},
            {"account_bound_label", "Account-bound: %s"},
            {"nosell_label", "NoSell: %s"},
            {"yes_label", "Yes"},
            {"no_label", "No"},
            {"sort_price_down", "Sort: Item Price down"},
            {"sort_price_up", "Sort: Item Price up"},
            {"sort_count_high", "Sort: |Count| high"},
            {"sort_count_low", "Sort: |Count| low"},
            {"sort_name_az", "Sort: Name A–Z"},
            {"sort_name_za", "Sort: Name Z–A"},
            {"last_reset_label", "Reset"},
            {"last_reset_tooltip", "Time since last reset"},
            {"custom_profit_edit_tooltip",    "Edit profit value"},
            {"custom_profit_confirm_tooltip", "Save changes"},
            {"accent_color", "Accent Farve (Buttons, Faneblads, UI)"}
            {"accent_color_tooltip", "Accent color for buttons, tabs, and UI elements"}
            {"account_management", "Konto Management"}
            {"account_prefix", "Konto"}
            {"actions", "Actions"}
            {"add_account", "+ Tilføj Konto"}
            {"api_key_invalid_format", "(Invalid Format: 9 Blocks required)"}
            {"auto_reset_custom_days", "Brugerdefineret (days)"}
            {"auto_reset_daily", "Daily reset (00:00 UTC)"}
            {"auto_reset_done_msg", "The tracker has been reset."}
            {"auto_reset_done_title", "Nulstil Complete"}
            {"auto_reset_minutes_unload", "Minutes after last unload"}
            {"auto_reset_never", "Never (manual Nulstil only)"}
            {"auto_reset_on_load", "Til addon load"}
            {"auto_reset_tooltip", "When to automatically reset farming counters"}
            {"auto_reset_weekly", "Weekly (Mon 07:30 UTC)"}
            {"auto_reset_weekly_eu_wvw", "Weekly EU WvW (Fri 18:00 UTC)"}
            {"auto_reset_weekly_map_bonus", "Weekly map bonus (Thu 20:00 UTC)"}
            {"auto_reset_weekly_na_wvw", "Weekly NA WvW (Sat 02:00 UTC)"}
            {"automatic_backups", "Automatic Backups"}
            {"backup", "Backup"}
            {"backup_daily", "Daily"}
            {"backup_frequency", "Backup frequency"}
            {"backup_frequency_tooltip", "How often to create automatic backups"}
            {"backup_manual_only", "Manuel only"}
            {"backup_weekly", "Weekly"}
            {"best_drop", "Bedste Drop"}
            {"border_size", "Kant Størrelse"}
            {"border_size_tooltip", "Adjust the thickness of rarity borders (1.0 - 10.0)"}
            {"bottom_gradient_color", "Bottom"}
            {"bottom_gradient_color_tooltip", "Bottom gradient color"}
            {"browse_for_file", "Browse for file..."}
            {"cancel", "Annuller"}
            {"clear_all_custom_profits_warning", "Alle custom profit values will be deleted. This action cannot be undone."}
            {"clear_compare_selection", "Ryd selection"}
            {"clear_history", "Ryd Historik"}
            {"clear_history_confirm", "Ryd all session history?"}
            {"clear_history_tooltip", "Slet all saved session history"}
            {"clear_history_warning", "This action cannot be undone!"}
            {"clear_search", "Ryd"}
            {"clear_search_favorites", "Ryd"}
            {"clear_search_tooltip", "Ryds the current search"}
            {"column_count", "Antal"}
            {"column_currency", "Currency"}
            {"column_favorite", "Favorite"}
            {"column_icon", "Ikon"}
            {"column_ignore", "Ignore"}
            {"column_item", "Item"}
            {"column_label", "Etiket"}
            {"column_name", "Name"}
            {"column_profit", "Profit"}
            {"column_value", "Værdi"}
            {"comparison_previous_period", "Comparison with previous period:"}
            {"count_format", "Antal: %lld"}
            {"create", "Create"}
            {"create_new_profile_tooltip", "Create a new profile with current settings"}
            {"create_tooltip", "Create a new profile with current settings"}
            {"currencies_header", "Valutaer"}
            {"currency_cat_common", "Common"}
            {"currency_cat_fractal", "Fractals"}
            {"currency_cat_janthir", "Janthir Wilds"}
            {"currency_cat_map", "Map Valutaer"}
            {"currency_cat_other", "Other"}
            {"currency_cat_pvp", "PvP"}
            {"currency_cat_raid_strike", "Raids & Strikes"}
            {"currency_cat_wvw", "WvW"}
            {"currency_group_by_category", "Grupper by category"}
            {"currency_group_by_category_tooltip", "Grupper currencies by category with collapsible sections or tabs"}
            {"currency_show_as_tabs", "Vis as tabs"}
            {"currency_show_as_tabs_tooltip", "Vis category groups as tabs instead of collapsible sections"}
            {"currency_table_favorite_tooltip", "Tilføj/remove favorite. Favorites appear in the Favorites tab. Tip: Right-click the icon/name for more actions."}
            {"currency_table_ignore_tooltip", "Tilføj/remove ignored. Ignored currencies appear in the Ignored tab. Tip: Right-click the icon/name for more actions."}
            {"date", "Dato"}
            {"debug_settings", "Fejlfinding Indstillinger"}
            {"default_no_profile", "Standard (Nej Profile)"}
            {"delete_profile", "Slet Profile"}
            {"delete_profile_tooltip", "Slet current profile"}
            {"details", "Details"}
            {"drops", "Drops"}
            {"duration", "Varighed"}
            {"enable_automatic_backups", "Aktiver automatic backups"}
            {"enable_automatic_backups_tooltip", "Automatically create backups before changes"}
            {"enable_best_drop_highlight", "Highlight Bedste Drop"}
            {"enable_best_drop_highlight_tooltip", "Highlight the most valuable drop with a golden border in the Genstande tab"}
            {"enable_best_drop_in_mini_window", "Vis Bedste Drop in Mini Vindue"}
            {"enable_best_drop_in_mini_window_tooltip", "Vis the most valuable drop in the mini window overlay"}
            {"enable_debug_tab", "Aktiver Fejlfinding Faneblad"}
            {"enable_debug_tab_tooltip", "Viss the debug tab with additional information"}
            {"enable_favorite_row_color", "Aktiver favorite row color"}
            {"enable_favorite_row_color_tooltip", "Highlights favorite items/currencies with custom row background color"}
            {"enable_favorite_text_color", "Aktiver favorite text color"}
            {"enable_favorite_text_color_tooltip", "Highlights favorite items/currencies with custom text color"}
            {"enable_favorites", "Aktiver Favorites"}
            {"enable_favorites_tab", "Aktiver Favorites Faneblad"}
            {"enable_favorites_tab_tooltip", "Viss a separate favorites tab"}
            {"enable_grid_view_currencies", "Aktiver Grid View (Valutaer)"}
            {"enable_grid_view_currencies_tooltip", "Toggle between list and grid view in Valutaer tab"}
            {"enable_grid_view_items", "Aktiver Grid View (Genstande)"}
            {"enable_grid_view_items_tooltip", "Toggle between list and grid view in Genstande tab"}
            {"enable_icon_cache", "Aktiver Ikon Cache"}
            {"enable_icon_cache_tooltip", "Cache item icons on disk to speed up loading after the first session"}
            {"enable_notifications", "Aktiver notifications"}
            {"enable_notifications_tooltip", "Aktiver in-game notifications"}
            {"enable_session_history", "Aktiver Session Historik"}
            {"enable_session_history_tooltip", "Gem farming session history for later viewing"}
            {"enable_session_timeline", "Aktiver Session Tidslinje"}
            {"enable_session_timeline_tooltip", "Gem detailed drop timeline with timestamps for session details"}
            {"export_history", "Eksporter Historik"}
            {"export_history_tooltip", "Eksporter session history to a JSON file"}
            {"export_logs", "Eksporter Logs"}
            {"favorite_items_header", "Favorite Genstande"}
            {"favorites_first", "Favorites First"}
            {"favorites_first_tooltip", "Viss favorites first in item/currency lists"}
            {"favorites_settings", "Favorites Indstillinger"}
            {"filter_account_bound", "Konto-bound"}
            {"filter_account_bound_tooltip", "Vis account-bound items"}
            {"filter_aether_rich_sap", "Aether-Rich Sap"}
            {"filter_aether_rich_sap_tooltip", "Vis aether-rich sap currency"}
            {"filter_airship_parts", "Airship Parts"}
            {"filter_airship_parts_tooltip", "Vis airship parts currency"}
            {"filter_ancient_coin", "Ancient Coin"}
            {"filter_ancient_coin_tooltip", "Vis ancient coin currency"}
            {"filter_antiquated_ducat", "Antiquated Ducat"}
            {"filter_antiquated_ducat_tooltip", "Vis antiquated ducat currency"}
            {"filter_ascended_shards_of_glory", "Ascended Shards of Glory"}
            {"filter_ascended_shards_of_glory_tooltip", "Vis ascended shards of glory currency"}
            {"filter_astral_acclaim", "Astral Acclaim"}
            {"filter_astral_acclaim_tooltip", "Vis astral acclaim currency"}
            {"filter_badge_of_honor", "Badge of Honor"}
            {"filter_badge_of_honor_tooltip", "Vis badge of honor currency"}
            {"filter_calcified_gasp", "Calcified Gasp"}
            {"filter_calcified_gasp_tooltip", "Vis calcified gasp currency"}
            {"filter_canach_coins", "Canach Coins"}
            {"filter_canach_coins_tooltip", "Vis canach coins currency"}
            {"filter_custom_profit", "Has custom profit"}
            {"filter_custom_profit_tooltip", "Vis items with custom profit set"}
            {"filter_elegy_mosaic", "Elegy Mosaic"}
            {"filter_elegy_mosaic_tooltip", "Vis elegy mosaic currency"}
            {"filter_favorite", "Favorite"}
            {"filter_favorite_tooltip", "Vis favorite items (outside Favorites tab)"}
            {"filter_fractal_relic", "Fractal Relic"}
            {"filter_fractal_relic_tooltip", "Vis fractal relic currency"}
            {"filter_gaeting_crystal_janthir", "Gaeting Crystal (Janthir)"}
            {"filter_gaeting_crystal_janthir_tooltip", "Vis gaeting crystal (janthir) currency"}
            {"filter_gaeting_crystals", "Gaeting Crystals"}
            {"filter_gaeting_crystals_tooltip", "Vis gaeting crystals currency"}
            {"filter_gem", "Gem"}
            {"filter_gem_tooltip", "Vis gem currency"}
            {"filter_geode", "Geode"}
            {"filter_geode_tooltip", "Vis geode currency"}
            {"filter_green_prophet_shards", "Green Prophet Shards"}
            {"filter_green_prophet_shards_tooltip", "Vis green prophet shards currency"}
            {"filter_guild_commendation", "Guild Commendation"}
            {"filter_guild_commendation_tooltip", "Vis guild commendation currency"}
            {"filter_ignored", "Ignored"}
            {"filter_ignored_tooltip", "Vis ignored items"}
            {"filter_imperial_favor", "Imperial Favor"}
            {"filter_imperial_favor_tooltip", "Vis imperial favor currency"}
            {"filter_items", "Filter Genstande"}
            {"filter_jade_sliver", "Jade Sliver"}
            {"filter_jade_sliver_tooltip", "Vis jade sliver currency"}
            {"filter_karma_tooltip", "Vis karma currency"}
            {"filter_known_by_api", "Known by API"}
            {"filter_known_by_api_tooltip", "Vis items known by GW2 API"}
            {"filter_laurel", "Laurel"}
            {"filter_laurel_tooltip", "Vis laurel currency"}
            {"filter_legendary_insight", "Legendary Insight"}
            {"filter_legendary_insight_tooltip", "Vis legendary insight currency"}
            {"filter_ley_line_crystals", "Ley-Line Crystals"}
            {"filter_ley_line_crystals_tooltip", "Vis ley-line crystals currency"}
            {"filter_magnetite_shards", "Magnetite Shards"}
            {"filter_magnetite_shards_tooltip", "Vis magnetite shards currency"}
            {"filter_max_price_tooltip", "Maximum price filter (0 = disabled)"}
            {"filter_max_quantity_tooltip", "Maximum quantity filter (0 = disabled)"}
            {"filter_min_price_tooltip", "Minimum price filter (0 = disabled)"}
            {"filter_min_quantity_tooltip", "Minimum quantity filter (0 = disabled)"}
            {"filter_nosell", "NejSell"}
            {"filter_nosell_tooltip", "Vis NejSell items"}
            {"filter_not_account_bound", "Nejt Konto-bound"}
            {"filter_not_account_bound_tooltip", "Vis non-account-bound items"}
            {"filter_not_favorite", "Nejt Favorite"}
            {"filter_not_favorite_tooltip", "Vis items that are not marked as favorite"}
            {"filter_not_ignored", "Nejt Ignored"}
            {"filter_not_ignored_tooltip", "Vis non-ignored items"}
            {"filter_not_nosell", "Nejt NejSell"}
            {"filter_not_nosell_tooltip", "Vis sellable items"}
            {"filter_pinch_of_stardust", "Pinch of Stardust"}
            {"filter_pinch_of_stardust_tooltip", "Vis pinch of stardust currency"}
            {"filter_pristine_fractal_relics", "Pristine Fractal Relics"}
            {"filter_pristine_fractal_relics_tooltip", "Vis pristine fractal relics currency"}
            {"filter_proofs_of_heroics", "Proofs of Heroics"}
            {"filter_proofs_of_heroics_tooltip", "Vis proofs of heroics currency"}
            {"filter_prophet_shards", "Prophet Shards"}
            {"filter_prophet_shards_tooltip", "Vis prophet shards currency"}
            {"filter_pvp_league_tickets", "PvP League Tickets"}
            {"filter_pvp_league_tickets_tooltip", "Vis PvP league tickets currency"}
            {"filter_rarity", "Filter Rarity"}
            {"filter_research_notes", "Research Nejtes"}
            {"filter_research_notes_tooltip", "Vis research notes currency"}
            {"filter_sellable_on_tp", "Sellable on TP"}
            {"filter_sellable_on_tp_tooltip", "Vis items sellable on Trading Post"}
            {"filter_sellable_to_vendor", "Sellable to vendor"}
            {"filter_sellable_to_vendor_tooltip", "Vis items sellable to vendor"}
            {"filter_spirit_shards", "Spirit Shards"}
            {"filter_spirit_shards_tooltip", "Vis spirit shards currency"}
            {"filter_static_charge", "Static Charge"}
            {"filter_static_charge_tooltip", "Vis static charge currency"}
            {"filter_tales_of_dungeon_delving", "Tales of Dungeon Delving"}
            {"filter_tales_of_dungeon_delving_tooltip", "Vis tales of dungeon delving currency"}
            {"filter_testimony_of_castoran_heroics", "Testimony of Castoran Heroics"}
            {"filter_testimony_of_castoran_heroics_tooltip", "Vis testimony of castoran heroics currency"}
            {"filter_testimony_of_desert_heroics", "Testimony of Desert Heroics"}
            {"filter_testimony_of_desert_heroics_tooltip", "Vis testimony of desert heroics currency"}
            {"filter_testimony_of_jade_heroics", "Testimony of Jade Heroics"}
            {"filter_testimony_of_jade_heroics_tooltip", "Vis testimony of jade heroics currency"}
            {"filter_trade_contracts", "Trade Contracts"}
            {"filter_trade_contracts_tooltip", "Vis trade contracts currency"}
            {"filter_transmutation_charge", "Transmutation Charge"}
            {"filter_transmutation_charge_tooltip", "Vis transmutation charge currency"}
            {"filter_type_armor", "Armor"}
            {"filter_type_armor_tooltip", "Vis armor items"}
            {"filter_type_backpack", "Backpack"}
            {"filter_type_backpack_tooltip", "Vis backpack items"}
            {"filter_type_bag", "Bag"}
            {"filter_type_bag_tooltip", "Vis bags"}
            {"filter_type_consumable", "Consumable"}
            {"filter_type_consumable_tooltip", "Vis consumable items"}
            {"filter_type_container", "Container"}
            {"filter_type_container_tooltip", "Vis containers"}
            {"filter_type_crafting_material", "Crafting Material"}
            {"filter_type_crafting_material_tooltip", "Vis crafting materials"}
            {"filter_type_gathering_tool", "Gathering Tool"}
            {"filter_type_gathering_tool_tooltip", "Vis gathering tools"}
            {"filter_type_gizmo", "Gizmo"}
            {"filter_type_gizmo_container", "Gizmo Container"}
            {"filter_type_gizmo_container_tooltip", "Vis gizmo container items"}
            {"filter_type_gizmo_tooltip", "Vis gizmo items"}
            {"filter_type_mini_pet", "Mini Pet"}
            {"filter_type_mini_pet_tooltip", "Vis mini pets"}
            {"filter_type_tool", "Tool"}
            {"filter_type_tool_tooltip", "Vis tool items"}
            {"filter_type_trinket", "Trinket"}
            {"filter_type_trinket_tooltip", "Vis trinket items"}
            {"filter_type_trophy", "Trophy"}
            {"filter_type_trophy_tooltip", "Vis trophy items"}
            {"filter_type_unlock", "Lås op"}
            {"filter_type_unlock_tooltip", "Vis unlock items"}
            {"filter_type_upgrade_component", "Upgrade Component"}
            {"filter_type_upgrade_component_tooltip", "Vis upgrade components"}
            {"filter_type_weapon", "Weapon"}
            {"filter_type_weapon_tooltip", "Vis weapon items"}
            {"filter_tyrian_defense_seal", "Tyrian Defense Seal"}
            {"filter_tyrian_defense_seal_tooltip", "Vis tyrian defense seal currency"}
            {"filter_unbound_magic", "Unbound Magic"}
            {"filter_unbound_magic_tooltip", "Vis unbound magic currency"}
            {"filter_uncommon_coins", "Uncommon Coins"}
            {"filter_uncommon_coins_tooltip", "Vis uncommon coins currency"}
            {"filter_unknown_by_api", "Unknown by API"}
            {"filter_unknown_by_api_tooltip", "Vis items not known by GW2 API"}
            {"filter_unstable_fractal_essence", "Unstable Fractal Essence"}
            {"filter_unstable_fractal_essence_tooltip", "Vis unstable fractal essence currency"}
            {"filter_unusual_coin", "Unusual Coin"}
            {"filter_unusual_coin_tooltip", "Vis unusual coin currency"}
            {"filter_ursus_oblige", "Ursus Oblige"}
            {"filter_ursus_oblige_tooltip", "Vis ursus oblige currency"}
            {"filter_volatile_magic", "Volatile Magic"}
            {"filter_volatile_magic_tooltip", "Vis volatile magic currency"}
            {"filter_wvw_skirmish_tickets", "WvW Skirmish Tickets"}
            {"filter_wvw_skirmish_tickets_tooltip", "Vis WvW skirmish tickets currency"}
            {"first_5_custom_profit", "First 5 items with custom profit set"}
            {"first_5_custom_profit_tooltip", "First 5 items with custom profit set"}
            {"first_5_ignored_items", "First 5 ignored items"}
            {"first_5_ignored_items_tooltip", "First 5 ignored items"}
            {"first_5_tracked_items", "First 5 tracked items and currencies with details"}
            {"first_5_tracked_items_tooltip", "First 5 tracked items and currencies with details"}
            {"full_backup", "Full Backup"}
            {"full_backup_tooltip", "Backup all data (settings, session history, favorites, ignored items, custom profit) to a JSON file"}
            {"full_restore", "Full Gendan"}
            {"full_restore_tooltip", "Gendan all data from a backup JSON file"}
            {"general_settings", "Generelt Indstillinger"}
            {"gold_format", "Gold: %lld"}
            {"gradient_backgrounds", "Gradient backgrounds"}
            {"gradient_backgrounds_tooltip", "Aktivers smooth gradient backgrounds for a more modern look"}
            {"grid_icon_size_currencies", "Grid Ikon size (Valutaer)"}
            {"grid_icon_size_currencies_tooltip", "Størrelse of icons in grid view for Valutaer (16-128)"}
            {"grid_icon_size_items", "Grid Ikon size (Genstande)"}
            {"grid_icon_size_items_tooltip", "Størrelse of icons in grid view for Genstande (16-128)"}
            {"group_by_rarity", "Grupper by Rarity"}
            {"group_by_type", "Grupper by Category"}
            {"icon_cache_max_icons", "Max Cached Ikons"}
            {"icon_cache_max_icons_tooltip", "Maximum number of icons to keep in cache (older icons are deleted when limit is reached)"}
            {"icon_size", "Ikon size"}
            {"icon_size_tooltip", "Størrelse of item icons in pixels (16-96)"}
            {"import", "Importer"}
            {"import_history", "Importer Historik"}
            {"import_history_tooltip", "Importer session history from a JSON file"}
            {"import_tooltip", "Importer settings from a JSON file"}
            {"infusion_drop_label", "Infusion Drop!"}
            {"item", "Item"}
            {"items_header", "Genstande"}
            {"magic_find_abbreviation", "MF: %d%%"}
            {"main_window_click_through", "Click through"}
            {"main_window_click_through_tooltip", "Alleows clicking through the main window to the game"}
            {"main_window_opacity", "Main Vindue Transparency"}
            {"main_window_opacity_tooltip", "Main window background transparency (0-100%)"}
            {"main_window_settings", "Main Vindue"}
            {"map", "Map"}
            {"mass_actions_clear_ignore", "Ryd ignore list"}
            {"mass_actions_ignore_ascended", "Ignore all Ascended items"}
            {"mass_actions_ignore_basic", "Ignore all Basic items"}
            {"mass_actions_ignore_exotic", "Ignore all Exotic items"}
            {"mass_actions_ignore_fine", "Ignore all Fine items"}
            {"mass_actions_ignore_junk", "Ignore all Junk items"}
            {"mass_actions_ignore_legendary", "Ignore all Legendary items"}
            {"mass_actions_ignore_masterwork", "Ignore all Masterwork items"}
            {"mass_actions_ignore_rare", "Ignore all Rare items"}
            {"mass_actions_label", "Mass Actions"}
            {"max_backup_count", "Max backup count"}
            {"max_backup_count_tooltip", "Maximum number of backups to keep (1-20)"}
            {"max_backups", "Max Backups"}
            {"max_backups_tooltip", "Maximum number of backups to keep (1-20)"}
            {"max_session_history", "Max Sessions"}
            {"max_session_history_tooltip", "Maximum number of sessions to save (1-50). Oldest session is deleted when limit is reached if overwrite is enabled."}
            {"min_value", "Min Værdi"}
            {"mini_window_click_through", "Vindue click through"}
            {"mini_window_click_through_tooltip", "Alleows clicking through the mini window to the game"}
            {"mini_window_hide_title_bar", "Skjul Mini Vindue Title Bar"}
            {"mini_window_hide_title_bar_tooltip", "Skjul the title bar of the mini window"}
            {"mini_window_locked", "Lås Mini Vindue"}
            {"mini_window_locked_tooltip", "Fix the mini window position and size (no longer movable or resizable)"}
            {"mini_window_opacity", "Mini Vindue Transparency"}
            {"mini_window_opacity_tooltip", "Mini window background transparency (0-100%)"}
            {"mini_window_show_profit", "Vis Profit"}
            {"mini_window_show_profit_per_hour", "Vis Profit/Hour"}
            {"mini_window_show_profit_per_hour_tooltip", "Display profit per hour in mini window"}
            {"mini_window_show_profit_tooltip", "Display total profit in mini window"}
            {"mini_window_show_session_duration", "Vis Session Varighed"}
            {"mini_window_show_session_duration_tooltip", "Display session duration in mini window"}
            {"mini_window_show_total_items", "Vis Total Genstande"}
            {"mini_window_show_total_items_tooltip", "Display total item count in mini window"}
            {"mini_window_show_tp_instant", "Vis TP Instant (Instant Sell)"}
            {"mini_window_show_tp_instant_tooltip", "Display TP instant sell profit in mini window"}
            {"mini_window_show_tp_sell", "Vis TP Sell (Listings)"}
            {"mini_window_show_tp_sell_tooltip", "Display TP sell profit (listings) in mini window"}
            {"minutes_after_unload_tooltip", "Minutes after addon unload before automatic reset"}
            {"no_cancel", "Nej, Annuller"}
            {"no_items_in_session", "Nej items in this session"}
            {"no_sessions_recorded", "Nej sessions recorded yet."}
            {"notification_combine_logic", "Combine Filters (AND)"}
            {"notification_combine_logic_tooltip", "If enabled, BOTH conditions (Værdi AND Rarity) must be met. If disabled, ANY one of them is enough."}
            {"notification_duration", "Display Varighed"}
            {"notification_duration_tooltip", "How long the notification stays visible (seconds)"}
            {"notification_general", "Generelt Indstillinger"}
            {"notification_include_agony", "Include Agony Infusions"}
            {"notification_include_agony_tooltip", "If enabled, Agony Infusions (+1 to +30) will also trigger an alert."}
            {"notification_include_non_profit", "Include Nejn-Profit Genstande"}
            {"notification_include_non_profit_tooltip", "If enabled, items with no gold value (0c) will also trigger alerts if they meet the rarity requirement."}
            {"notification_infusion_alert", "Infusion Alert"}
            {"notification_infusion_alert_tooltip", "Always notify when an Infusion is found (ignores Værdi/Rarity filters)"}
            {"notification_item_alerts", "Item Alerts"}
            {"notification_min_rarity", "Min. Rarity"}
            {"notification_min_rarity_tooltip", "Trigger notification if item rarity is at least this level"}
            {"notification_min_value", "Min. Værdi (Gold)"}
            {"notification_min_value_tooltip", "Trigger notification if item value is at least this amount"}
            {"notification_play_sound", "Afspil Lyd"}
            {"notification_play_sound_tooltip", "Afspil a sound effect when a notification appears"}
            {"notification_precursor_alert", "Pre-Cursor Alert"}
            {"notification_precursor_alert_tooltip", "Always notify when a Pre-Cursor is found (ignores Værdi/Rarity filters)"}
            {"notification_session_alerts", "Progress & Tid"}
            {"notification_settings", "Nejtifikation Indstillinger"}
            {"notification_setup_hint", "[Drag to reposition notifications]"}
            {"notification_stacking", "Stack Nejtifikations"}
            {"notification_stacking_tooltip", "Vis multiple notifications at once instead of replacing the old one immediately"}
            {"notification_triggers", "Nejtifikation Triggers"}
            {"notification_volume", "Master Lydstyrke"}
            {"notification_volume_tooltip", "Lydstyrke for notification sounds"}
            {"notify_profit_goal", "Nejtify when profit goal reached"}
            {"notify_profit_goal_tooltip", "Nejtify when you reach your profit goal"}
            {"notify_reset_warning", "Nejtify before reset"}
            {"notify_reset_warning_tooltip", "Nejtify before automatic reset occurs"}
            {"notify_session_complete", "Nejtify after session duration"}
            {"notify_session_complete_tooltip", "Nejtify after farming for a certain duration"}
            {"opportunity_cost_per_hour", "Opportunity cost per hour"}
            {"opportunity_cost_per_hour_tooltip", "Opportunity cost per hour"}
            {"opportunity_cost_vs_tp_sell", "Opportunity cost vs TP sell"}
            {"opportunity_cost_vs_tp_sell_tooltip", "Opportunity cost vs TP sell"}
            {"overwrite_session_history", "Overwrite Sessions"}
            {"overwrite_session_history_tooltip", "If enabled, oldest session is deleted when limit is reached"}
            {"performance_settings", "Performance Indstillinger"}
            {"precursor_drop_label", "Pre-Cursor Drop!"}
            {"profit_change", "Profit Change"}
            {"profit_goal_amount", "Goal Amount (Gold)"}
            {"profit_goal_gold", "Profit goal (gold)"}
            {"profit_goal_gold_tooltip", "Profit goal in gold coins (1-1000)"}
            {"profit_goal_reached_msg", "You have reached your profit goal of %d gold!"}
            {"profit_goal_reached_title", "Profit Goal Reached"}
            {"profit_per_hour_calculation", "Profit per hour calculation"}
            {"profit_per_hour_calculation_tooltip", "Profit per hour calculation"}
            {"quantity", "Quantity"}
            {"range_filters_tooltip", "Vis price and quantity range filters"}
            {"rare_drop_label", "Rare Drop!"}
            {"rarity_border_thickness", "Rarity Kant Thickness"}
            {"rarity_border_thickness_tooltip", "Adjust the thickness of rarity borders (1.0 - 10.0)"}
            {"rarity_name_ascended", "Ascended"}
            {"rarity_name_basic", "Basic"}
            {"rarity_name_exotic", "Exotic"}
            {"rarity_name_fine", "Fine"}
            {"rarity_name_junk", "Junk"}
            {"rarity_name_legendary", "Legendary"}
            {"rarity_name_masterwork", "Masterwork"}
            {"rarity_name_rare", "Rare"}
            {"rarity_name_unknown", "Unknown"}
            {"reconnect_drf_token", "Reconnect to DRF with the current token"}
            {"reload_drf_token", "Reload DRF Token"}
            {"reload_gw2_api_key", "Reload GW2 API Key"}
            {"reload_gw2_api_key_tooltip", "Reload GW2 API key for item data fetching"}
            {"remove_account", "- Fjern Konto"}
            {"reset_all", "Nulstil Alle"}
            {"reset_all_tooltip", "Nulstil all settings to default values"}
            {"reset_interval_days", "Nulstil interval (days)"}
            {"reset_interval_days_tooltip", "Brugerdefineret reset interval in days (1-30 days)"}
            {"reset_settings", "Auto Nulstil"}
            {"reset_warning_minutes", "Nulstil Advarsel (Minutes)"}
            {"reset_warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"reset_warning_msg", "The tracker will reset in %d minutes!"}
            {"reset_warning_title", "Nulstil Advarsel"}
            {"restore", "Gendan"}
            {"row_color", "Row Farve"}
            {"save", "Gem"}
            {"save_account", "Gem Konto"}
            {"save_all_items_confirm", "Aktiver session timeline?"}
            {"save_all_items_warning", "This will significantly increase file size!"}
            {"save_current_session", "Gem Current Session"}
            {"save_current_session_tooltip", "Gem the current farming session without resetting"}
            {"save_tooltip", "Gem current settings"}
            {"search_favorite_currencies_hint", "Søg favorite currencies..."}
            {"search_favorite_items_hint", "Søg favorite items..."}
            {"search_items", "Søg Genstande"}
            {"search_items_hint", "Søg items..."}
            {"select_profile", "Vælg a profile to apply its settings"}
            {"select_profile_tooltip", "Vælg a profile to apply its settings"}
            {"session_complete_hours", "Session Complete (Hours)"}
            {"session_complete_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_complete_msg", "You have been farming for %d hours!"}
            {"session_complete_title", "Session Complete"}
            {"session_count", "Session Antal"}
            {"session_details", "Session Details"}
            {"session_history", "Session Historik"}
            {"session_hours", "Session hours"}
            {"session_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_note", "Nejte"}
            {"session_profit_trend", "Profit Trend"}
            {"session_search_hint", "Søg sessions, items, notes..."}
            {"sessions_selected", "sessions selected"}
            {"sessions_stored", "Sessions Stored"}
            {"settings_profiles", "Indstillinger Profiles"}
            {"show_ignored_items", "Vis ignored items"}
            {"show_ignored_items_tooltip", "Vis ignored items/currencies in Genstande and Valutaer tabs (disable to hide). Difference from 'Ignored' filter: This filter controls display in Genstande/Valutaer tabs, the 'Ignored' filter controls display in Filter tab."}
            {"show_main_window", "Vis main window"}
            {"show_mini_window", "Vis mini window"}
            {"show_mini_window_tooltip", "Viss a small overlay widget with key statistics"}
            {"show_notification_setup", "Setup Mode (Positioning)"}
            {"show_notification_setup_tooltip", "Makes the notification window visible so you can move it"}
            {"show_rarity_as_tabs", "Vis as Faneblads"}
            {"show_summaries", "Vis Summaries"}
            {"show_summaries_tooltip", "Vis daily/weekly/monthly profit summaries"}
            {"show_type_as_tabs", "Vis as Faneblads"}
            {"showing", "Vising"}
            {"sort_profit_high", "Sorter: Profit high"}
            {"sort_profit_low", "Sorter: Profit low"}
            {"sort_rarity_high", "Sorter: Rarity high to low"}
            {"sort_rarity_low", "Sorter: Rarity low to high"}
            {"sound_alert", "Alert Lyd"}
            {"sound_infusion", "Infusion Lyd"}
            {"sound_path_hint", "Path to sound file (empty = default)"}
            {"sound_precursor", "Pre-Cursor Lyd"}
            {"sound_standard", "Standard Lyd"}
            {"sound_test", "Test"}
            {"stat_avg_profit_per_hour", "Avg Profit/h"}
            {"stat_best_session", "Bedste Session"}
            {"stat_total_profit", "Total Profit"}
            {"stat_total_time", "Total Tid"}
            {"summaries_coming_soon", "Summaries feature coming soon..."}
            {"summaries_label", "Summaries"}
            {"summaries_tooltip", "Daily/Weekly/Monthly profit summaries"}
            {"summary_period", "Period:"}
            {"summary_this_month", "This Month"}
            {"summary_this_week", "This Week"}
            {"summary_today", "Today"}
            {"tab_session_history", "Session Historik"}
            {"tab_sessions", "Sessions"}
            {"tab_summaries", "Summaries"}
            {"test_item_label", "Test Item"}
            {"text_color", "Text Farve"}
            {"time", "Tid"}
            {"time_ago_seconds", "%llds ago"}
            {"timeline_icon_size_currencies", "Tidslinje Ikon Størrelse (Valutaer)"}
            {"timeline_icon_size_currencies_tooltip", "Størrelse of currency icons in Tidslinje tab (16-48)"}
            {"timeline_icon_size_items", "Tidslinje Ikon Størrelse (Genstande)"}
            {"timeline_icon_size_items_tooltip", "Størrelse of item icons in Tidslinje tab (16-96)"}
            {"toggle_favorite", "Toggle favorite"}
            {"toggle_favorite_tooltip", "Toggle favorite"}
            {"toggle_ignore", "Toggle ignore"}
            {"toggle_ignore_tooltip", "Toggle ignore"}
            {"top_currencies_count_header", "Top 5 Valutaer (Antal)"}
            {"top_currencies_count_tooltip", "Top 5 currencies by count"}
            {"top_drops", "Top Drops"}
            {"top_gradient_color", "Top"}
            {"top_gradient_color_tooltip", "Top gradient color"}
            {"top_items_count_header", "Top 5 Genstande (Antal)"}
            {"top_items_profit_header", "Top 5 Genstande by Profit"}
            {"total_custom_profit", "Total custom profit"}
            {"total_custom_profit_tooltip", "Total custom profit"}
            {"total_drops", "Total Drops"}
            {"total_duration", "Total Varighed"}
            {"total_tp_sell_profit", "Total TP sell profit"}
            {"total_tp_sell_profit_tooltip", "Total TP sell profit"}
            {"total_tracked_currencies", "Total number of tracked currencies"}
            {"total_tracked_currencies_tooltip", "Total number of tracked currencies"}
            {"total_tracked_items", "Total number of tracked items"}
            {"total_tracked_items_tooltip", "Total number of tracked items"}
            {"total_vendor_sell_profit", "Total vendor sell profit"}
            {"total_vendor_sell_profit_tooltip", "Total vendor sell profit"}
            {"tp_buy_gross_format", "TP Buy (Gross): %s"}
            {"tp_buy_net_format", "TP Buy (Net): %s"}
            {"tp_sell_gross_format", "TP Sell (Gross): %s"}
            {"tp_sell_net_format", "TP Sell (Net): %s"}
            {"trigger_drops", "Item Drops"}
            {"trigger_profit_goal", "Profit Goal"}
            {"trigger_time_reset", "Tid & Nulstil"}
            {"type_backpack", "Backpack"}
            {"type_gizmo_container", "Gizmo Container"}
            {"type_tool", "Tool"}
            {"type_trophy", "Trophy"}
            {"type_unlock", "Lås op"}
            {"type_upgrade_component", "Upgrade Component"}
            {"unknown_map", "Unknown"}
            {"update_profile", "Opdater Profile"}
            {"update_profile_tooltip", "Opdater current profile with current settings"}
            {"value", "Værdi"}
            {"vendor_value_format", "Vendor Værdi: %s"}
            {"visual_settings", "Visual Indstillinger"}
            {"warning_minutes", "Advarsel minutes"}
            {"warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"yes_clear", "Ja, Ryd"}
            {"yes_enable", "Ja, Aktiver"}
            {"yes_reset", "Ja, Nulstil"},

            // Drops Tab
            {"settings_tab", "Indstillinger"},
        
            // Missing keys from English
            {"backup_path_default_tooltip", "Default: addon directory"}
            {"backup_path_label", "Backup folder:"}
            {"browse_for_folder_tooltip", "Browse for folder"}
            {"current_or_last_recorded", "Current or last recorded"}
            {"custom_profit_import_tooltip", "Loads custom_profit_import.json from the addon folder"}
            {"disable_complex_visuals", "Disable Complex Visuals on Low Performance"}
            {"disable_complex_visuals_tooltip", "Disables rarity borders, sparklines and other visual effects to improve performance"}
            {"efficiency_score_desc_short", "instant vs. listing"}
            {"enable_drops_tab", "Enable Drops Tab"}
            {"enable_drops_tab_tooltip", "Show the Drops tab"}
            {"entries_label", "entries"},
            {"open_folder_tooltip", "Open folder in Explorer"},

            // Custom Profit Tab
            {"custom_profit_edit_tooltip",    "Edit profit value"},
            {"custom_profit_confirm_tooltip", "Save changes"},
            {"custom_profit_remove_tooltip",  "Remove custom profit"},

            // Common Tooltips
            {"cancel_tooltip",               "Cancel action"},
            {"export_json_tooltip",           "Export to JSON format"},
            {"export_csv_tooltip",            "Export to CSV format"},
            {"import_json_tooltip",           "Import from JSON format"},
            {"import_csv_tooltip",            "Import from CSV format"},
            {"import_ignored_json_tooltip",   "Import ignored items from JSON"},
            {"import_items_json_tooltip",     "Import items from JSON"},
            {"import_currencies_json_tooltip","Import currencies from JSON"},
            {"import_favorites_json_tooltip", "Import favorites from JSON"},
            {"toggle_favorites_tooltip",      "Toggle favorites section"},
            {"toggle_magnetite_tooltip",      "Toggle magnetite section"},
            {"close_button_tooltip",          "Close this window"},
            {"clear_all_custom_profits_tooltip", "Clear all custom profit entries"},
            {"clear_all_ignored_tooltip",     "Clear all ignored items"},
            {"load_save_tooltip",             "Load or save settings"},
            {"mass_actions_tooltip",          "Mass actions for items"},
            {"group_by_rarity_tooltip",       "Group items by rarity"},
            {"clear_search_tooltip",          "Clear search field"}
};
        return translations;
    }
}
