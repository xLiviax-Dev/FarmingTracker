// ---------------------------------------------------------------------------
// localization.cpp – Localization system implementation for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>

namespace Localization
{
    static Language s_CurrentLanguage = Language::English;
    static std::unordered_map<std::string, const char*> s_Translations;

    // English translations
    static const std::unordered_map<std::string, const char*> s_EnglishTranslations = {
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
        {"tab_summary", "Summary"},
        {"tab_items", "Items"},
        {"tab_currencies", "Currencies"},
        {"tab_profit", "Profit"},
        {"tab_filter", "Filter"},
        {"tab_debug", "Debug"},

        // Summary Tab
        {"warning_drf_not_connected", "⚠️ DRF not connected"},
        {"warning_drf_not_connected_desc", "This plugin requires DRF for data transmission."},
        {"warning_drf_install", "Install DRF via Nexus Addon Manager or https://drf.rs/"},
        {"warning_drf_token_invalid", "⚠️ DRF Token invalid"},
        {"warning_drf_token_invalid_desc", "Please check your DRF Token in Settings."},
        {"warning_gw2_api_key_not_set", "⚠️ GW2 API Key not set"},
        {"warning_gw2_api_key_not_set_desc", "Please set your GW2 API Key in Settings for item details."},
        {"overview", "Overview"},
        {"overview_tooltip", "Current farming session overview"},
        {"total_profit", "Total Profit"},
        {"total_profit_tooltip", "Total custom profit from all items"},
        {"total_items_count", "Total Items"},
        {"total_items_tooltip", "Total number of unique items tracked"},
        {"total_currencies", "Total Currencies"},
        {"total_currencies_tooltip", "Total number of unique currencies tracked"},
        {"profit_per_hour_label", "Profit Per Hour"},
        {"profit_per_hour_tooltip", "Profit per hour based on session duration"},
        {"session_duration", "Session Duration"},
        {"session_duration_tooltip", "Current farming session duration"},
        {"top_items_profit", "Top Items (Profit)"},
        {"top_items_profit_tooltip", "Top 5 items by profit value"},
        {"loading", "Loading..."},
        {"coin", "Coin"},
        {"top_items_count", "Top Items (Count)"},
        {"top_items_count_tooltip", "Top 5 items by count"},
        {"top_currencies", "Top Currencies"},
        {"top_currencies_tooltip", "Top 5 currencies by count"},
        {"quick_statistics", "Quick Statistics"},
        {"quick_statistics_tooltip", "Overview of farming statistics"},
        {"average_item_value", "Average Item Value"},
        {"average_item_value_na", "N/A"},
        {"total_unique_items", "Total Unique Items"},
        {"warning_no_data", "⚠️ No data loaded"},
        {"warning_no_data_desc", "Waiting for DRF data..."},
        {"export", "Export"},
        {"export_tooltip", "Export farming data to file"},
        {"export_json", "Export as JSON"},
        {"export_csv", "Export as CSV"},

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
        {"currency_filters", "Currency Filters"},
        {"currency_karma", "Karma"},
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
        {"show_rarity_border", "Show Rarity Border"},
        {"show_rarity_border_tooltip", "Display colored border based on item rarity"},
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
        {"language_chinese", "中文"},
        {"language_czech", "Čeština"},
        {"language_italian", "Italiano"},
        {"language_polish", "Polski"},
        {"language_portuguese", "Português"},
        {"language_russian", "Русский"},
    };

    void Initialize(Language lang)
    {
        SetLanguage(lang);
    }

    void SetLanguage(Language lang)
    {
        s_CurrentLanguage = lang;
        s_Translations.clear();

        // Currently only English is implemented
        // Other languages will be added later
        s_Translations = s_EnglishTranslations;
    }

    Language GetLanguage()
    {
        return s_CurrentLanguage;
    }

    const char* GetText(const char* key)
    {
        auto it = s_Translations.find(key);
        if (it != s_Translations.end())
            return it->second;
        return key; // Return key if not found
    }

    const char* LanguageToString(Language lang)
    {
        switch (lang)
        {
            case Language::English: return "English";
            case Language::German: return "German";
            case Language::French: return "French";
            case Language::Spanish: return "Spanish";
            case Language::Chinese: return "Chinese";
            case Language::Czech: return "Czech";
            case Language::Italian: return "Italian";
            case Language::Polish: return "Polish";
            case Language::Portuguese: return "Portuguese";
            case Language::Russian: return "Russian";
            default: return "English";
        }
    }

    Language StringToLanguage(const std::string& str)
    {
        if (str == "German") return Language::German;
        if (str == "French") return Language::French;
        if (str == "Spanish") return Language::Spanish;
        if (str == "Chinese") return Language::Chinese;
        if (str == "Czech") return Language::Czech;
        if (str == "Italian") return Language::Italian;
        if (str == "Polish") return Language::Polish;
        if (str == "Portuguese") return Language::Portuguese;
        if (str == "Russian") return Language::Russian;
        return Language::English; // Default
    }
}
