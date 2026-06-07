// ---------------------------------------------------------------------------
// localization_hu.cpp – Hungarian translations for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>

namespace Localization
{
    const std::unordered_map<std::string, const char*> GetHungarianTranslations()
    {
        static const std::unordered_map<std::string, const char*> translations = {
            // Status texts
            {"status_disconnected", "Szétkapcsolva"},
            {"status_connecting", "Kapcsolódás..."},
            {"status_connected", "Kapcsolódva"},
            {"status_auth_failed", "Hitelesítés sikertelen – ellenőrizze a tokent"},
            {"status_reconnecting", "Újrakapcsolódás..."},
            {"status_error", "Hiba"},
            {"status_unknown", "Ismeretlen"},

            // Mini Window
            {"mini_window_title", "Farming Tracker Mini"},
            {"profit", "Profit"},
            {"profit_per_hour", "Profit/óra"},
            {"tp_sell", "TP Eladás"},
            {"tp_instant", "TP Azonnali"},
            {"total_items", "Összes tárgy"},
            {"session", "Munkamenet"},

            // Main Window
            {"main_window_title", "Farming Tracker"},
            {"drf_label", "DRF"},
            {"gw2_api_label", "GW2 API"},
            {"session_time_label", "Munkamenet idő"},
            {"reset_button", "Visszaállítás"},
            {"reset_tooltip", "Összes farming számláló visszaállítása (kézi visszaállítás)"},

            // Tabs
            {"tab_summary", "Dashboard"},
            {"tab_drops", "Drops"},
            {"tab_loot_filter", "Loot Filter"},
            {"tab_items", "Tárgyak"},
            {"tab_currencies", "Valuták"},
            {"tab_dashboard", "Vezérlőpult"},
            {"tab_favorites", "Kedvencek"},
            {"tab_ignored", "Mellőzött"},
            {"tab_timeline", "Idővonal"},
            {"timeline_profit_hour_listings", "Becsült kereskedési profit óránként (Listázások)"},
            {"timeline_profit_hour_instant", "Becsült kereskedési profit óránként (Azonnali eladás)"},
            {"timeline_no_drops", "Még nincs rögzített drop ebben a munkamenetben."},
            {"timeline_item_drops", "Tárgy dropok"},
            {"timeline_currencies", "Valuták"},
            {"tab_filter", "Szűrő"},
            {"tab_custom_profit", "Egyedi profit"},
            {"tab_debug", "Hibakeresés"},

            // Summary Tab
            {"warning_drf_not_connected", "⚠️ DRF nincs kapcsolódva"},
            {"warning_drf_not_connected_desc", "Ez a bővítmény DRF-et igényel az adatátvitelhez."},
            {"warning_drf_install", "Telepítse a DRF-et a Nexus Addon Manager vagy https://drf.rs/ segítségével"},
            {"warning_drf_token_invalid", "⚠️ DRF Token érvénytelen"},
            {"warning_drf_token_invalid_desc", "Kérjük, ellenőrizze a DRF Tokent a Beállításokban."},
            {"warning_gw2_api_key_not_set", "⚠️ GW2 API kulcs nincs beállítva"},
            {"warning_gw2_api_key_not_set_desc", "Kérjük, állítsa be a GW2 API kulcsot a Beállításokban a tárgy részletekhez."},
            {"gold", "Arany"},
            {"silver", "Ezüst"},
            {"copper", "Réz"},
            {"total_profit", "Teljes profit"},
            {"total_profit_tooltip", "Teljes egyedi profit minden tárgyból"},
            {"total_items_count", "Összes tárgy"},
            {"total_items_tooltip", "Követett egyedi tárgyak teljes száma"},
            {"total_currencies", "Összes valuta"},
            {"total_currencies_tooltip", "Követett egyedi valuták teljes száma"},
            {"profit_per_hour_label", "Profit óránként"},
            {"profit_per_hour_tooltip", "Profit óránként a munkamenet időtartama alapján"},
            {"magic_find", "Magic Find"},
            {"magic_find_tooltip", "Jelenlegi vagy utoljára rögzített Magic Find a DRF-től"},
            {"session_duration", "Munkamenet időtartama"},
            {"session_duration_tooltip", "Jelenlegi farming munkamenet időtartama"},
            {"date_tooltip", "Munkamenet kezdési idő"},
            {"duration_tooltip", "Munkamenet időtartama"},
            {"profit_tooltip", "Teljes munkamenet profit"},
            {"profit_per_hour_tooltip", "Profit óránként"},
            {"drops_tooltip", "Dropok száma"},
            {"best_drop_tooltip", "A munkamenet legértékesebb dropja"},
            {"top_items_profit", "Top tárgyak (Profit)"},
            {"top_items_profit_tooltip", "Top 5 tárgy profit érték szerint"},
            {"loading", "Betöltés..."},
            {"coin", "Érme"},
            {"top_items_count", "Top tárgyak (Darab)"},
            {"top_items_count_tooltip", "Top 5 tárgy darabszám szerint"},
            {"top_currencies", "Top valuták"},
            {"top_currencies_tooltip", "Top 5 valuta darabszám szerint"},
            {"quick_statistics", "Gyors statisztikák"},
            {"quick_statistics_tooltip", "Farming statisztikák áttekintése"},
            {"average_item_value", "Átlagos tárgyérték"},
            {"average_item_value_na", "N/A"},
            {"total_unique_items", "Összes egyedi tárgy"},
            {"warning_no_data", "⚠️ Nincs adat betöltve"},
            {"warning_no_data_desc", "Várakozás a DRF adatokra..."},
            {"export", "Exportálás"},
            {"export_tooltip", "Farming adatok exportálása fájlba"},
            {"export_json", "Exportálás JSON-ként"},
            {"export_csv", "Exportálás CSV-ként"},
            {"import_json", "Importálás JSON-ből"},

            // Items Tab
            {"search_hint", "Tárgyak keresése..."},
            {"clear", "Törlés"},
            {"sort_count_high", "Rendezés: |Darab| magas"},
            {"sort_count_low", "Rendezés: |Darab| alacsony"},
            {"sort_id_up", "Rendezés: Tárgy ID felfelé"},
            {"sort_id_down", "Rendezés: Tárgy ID lefelé"},
            {"sort_name_az", "Rendezés: Név A–Z"},
            {"sort_tooltip", "Tárgyak rendezése darab, ID vagy név szerint"},
            {"rarity_all", "Ritkaság: összes"},
            {"rarity_basic", "Ritkaság: Alap+"},
            {"rarity_fine", "Ritkaság: Finom+"},
            {"rarity_masterwork", "Ritkaság: Mester+"},
            {"rarity_rare", "Ritkaság: Ritka+"},
            {"rarity_exotic", "Ritkaság: Egzotikus+"},
            {"rarity_ascended", "Ritkaság: Ascended+"},
            {"rarity_legendary", "Ritkaság: Csak Legendás"},
            {"rarity_tooltip", "Tárgyak szűrése minimum ritkaság szerint"},
            {"rarity", "Ritkaság"},
            {"type", "Típus"},
            {"vendor_value", "Eladói érték"},
            {"tp_buy_net", "TP Vásárlás (Nettó)"},
            {"account_bound", "Fiókhoz kötött"},
            {"yes", "Igen"},
            {"no", "Nem"},
            {"nosell", "NoSell"},
            {"favorite", "Kedvenc"},
            {"ignore", "Mellőzés"},

            // Currencies Tab
            {"search_currencies_hint", "Valuták keresése..."},
            {"api_id", "API ID"},
            {"currency_name", "Valuta neve"},
            {"count", "Darab"},

            // Favorites Tab
            {"unfavorite_item", "Kedvenc eltávolítása"},
            {"unfavorite_selected", "Kedvencek eltávolítása"},
            {"no_favorites_yet", "Még nincsenek kedvencek. Jobb klikk egy tárgyon a hozzáadáshoz."},
            {"toggle_favorite_tooltip", "Kedvenc kapcsolása"},
            {"profits", "Profitok"},
            {"profits_tooltip", "Teljes profit a farmingból"},
            {"approx_profits", "Becsült profitok"},
            {"approx_gold_per_hour", "Becsült arany óránként"},
            {"trading_profits", "Kereskedési profitok"},

            // Profit Tab
            {"profits", "Profitok"},
            {"profits_tooltip", "Teljes profit a farmingból"},
            {"approx_profits", "Becsült profitok"},
            {"approx_gold_per_hour", "Becsült arany óránként"},
            {"trading_profits", "Kereskedési profitok"},
            {"trading_profits_tooltip", "Profit a Trading Postból"},
            {"approx_trading_profits_listings", "Becsült kereskedési profitok (Listázások)"},
            {"approx_trading_profits_instant", "Becsült kereskedési profitok (Azonnali eladás)"},
            {"trading_details", "Kereskedési részletek (Lehetőség-költség)"},
            {"trading_details_tooltip", "Lehetőség-költség a tárgyak használata helyett az eladásból"},
            {"lost_profit_vs_tp_sell", "Elvesztett profit (vs TP Eladás)"},
            {"lost_profit_per_hour_vs_tp_sell", "Elvesztett profit óránként (vs TP Eladás)"},
            {"efficiency_score", "Hatékonysági pontszám"},
            {"efficiency_score_label", "Hatékonysági pontszám:"},
            {"efficiency_score_tooltip", "Mennyit értél el a lehetséges maximum profitból (Azonnali eladás vs. TP Listázások)."},
            {"efficiency_score_desc", "Elérted a maximális profit %.1f%%-át!"},
            {"session_duration_label", "Munkamenet időtartama"},
            {"session_duration_tooltip", "Jelenlegi farming munkamenet időtartama"},

            // Filter Tab
            {"sell_method_filters", "Eladási módszer szűrők"},
            {"sellable_to_vendor", "Eladható eladónak"},
            {"sellable_to_vendor_tooltip", "Eladónak eladható tárgyak megjelenítése"},
            {"sellable_on_tp", "Eladható TP-n"},
            {"sellable_on_tp_tooltip", "TP-n eladható tárgyak megjelenítése"},
            {"has_custom_profit", "Van egyedi profit"},
            {"has_custom_profit_tooltip", "Egyedi profit értékkel rendelkező tárgyak megjelenítése"},
            {"api_knowledge_filters", "API tudás szűrők"},
            {"known_by_api", "Ismert az API által"},
            {"known_by_api_tooltip", "GW2 API által ismert tárgyak megjelenítése"},
            {"unknown_by_api", "Ismeretlen az API számára"},
            {"unknown_by_api_tooltip", "GW2 API számára ismeretlen tárgyak megjelenítése"},
            {"item_type_filters", "Tárgy típus szűrők"},
            {"type_armor", "Páncél"},
            {"type_armor_tooltip", "Páncél tárgyak megjelenítése"},
            {"type_weapon", "Fegyver"},
            {"type_weapon_tooltip", "Fegyver tárgyak megjelenítése"},
            {"type_trinket", "Ékszer"},
            {"type_trinket_tooltip", "Ékszer tárgyak megjelenítése"},
            {"type_gizmo", "Gizmo"},
            {"type_gizmo_tooltip", "Gizmo tárgyak megjelenítése"},
            {"type_crafting_material", "Crafting anyag"},
            {"type_crafting_material_tooltip", "Crafting anyagok megjelenítése"},
            {"type_consumable", "Fogyóeszköz"},
            {"type_consumable_tooltip", "Fogyóeszköz tárgyak megjelenítése"},
            {"type_gathering_tool", "Gyűjtő eszköz"},
            {"type_gathering_tool_tooltip", "Gyűjtő eszközök megjelenítése"},
            {"type_bag", "Zsák"},
            {"type_bag_tooltip", "Zsákok megjelenítése"},
            {"type_container", "Konténer"},
            {"type_container_tooltip", "Konténerek megjelenítése"},
            {"type_mini_pet", "Mini kisállat"},
            {"type_mini_pet_tooltip", "Mini kisállatok megjelenítése"},
            {"currency_filters_label", "Valuta Szűrők"},
            {"currency_general", "Általános"},
            {"currency_main", "Fő Valuták"},
            {"currency_fractal", "Fractal/Raid/Dungeon valuták"},
            {"currency_wvw_pvp", "WvW/PvP valuták"},
            {"currency_map", "Pálya-specifikus valuták"},
            {"filter_karma", "Karma"},
            {"currency_karma_tooltip", "Karma valuta megjelenítése"},
            {"currency_laurel", "Laurel"},
            {"currency_laurel_tooltip", "Laurel valuta megjelenítése"},
            {"currency_gem", "Gem"},
            {"currency_gem_tooltip", "Gem valuta megjelenítése"},
            {"currency_fractal_relic", "Fractal relikvia"},
            {"currency_fractal_relic_tooltip", "Fractal relikvia valuta megjelenítése"},
            {"currency_badge_of_honor", "Becsület jelvény"},
            {"currency_badge_of_honor_tooltip", "Becsület jelvény valuta megjelenítése"},
            {"currency_guild_commendation", "Céh ajánlás"},
            {"currency_guild_commendation_tooltip", "Céh ajánlás valuta megjelenítése"},
            {"currency_transmutation_charge", "Transmutációs töltés"},
            {"currency_transmutation_charge_tooltip", "Transmutációs töltés valuta megjelenítése"},
            {"currency_spirit_shards", "Szellem töredékek"},
            {"currency_spirit_shards_tooltip", "Szellem töredékek valuta megjelenítése"},
            {"currency_unbound_magic", "Kötetlen mágia"},
            {"currency_unbound_magic_tooltip", "Kötetlen mágia valuta megjelenítése"},
            {"currency_volatile_magic", "Volatile mágia"},
            {"currency_volatile_magic_tooltip", "Volatile mágia valuta megjelenítése"},
            {"currency_airship_parts", "Léghajó alkatrészek"},
            {"currency_airship_parts_tooltip", "Léghajó alkatrészek valuta megjelenítése"},
            {"currency_geode", "Geoda"},
            {"currency_geode_tooltip", "Geoda valuta megjelenítése"},
            {"currency_ley_line_crystals", "Ley-vonal kristályok"},
            {"currency_ley_line_crystals_tooltip", "Ley-vonal kristályok valuta megjelenítése"},
            {"currency_trade_contracts", "Kereskedelmi szerződések"},
            {"currency_trade_contracts_tooltip", "Kereskedelmi szerződések valuta megjelenítése"},
            {"currency_elegy_mosaic", "Elegy mozaik"},
            {"currency_elegy_mosaic_tooltip", "Elegy mozaik valuta megjelenítése"},
            {"currency_uncommon_coins", "Ritka érmék"},
            {"currency_uncommon_coins_tooltip", "Ritka érmék valuta megjelenítése"},
            {"currency_astral_acclaim", "Astrális elismerés"},
            {"currency_astral_acclaim_tooltip", "Astrális elismerés valuta megjelenítése"},
            {"currency_pristine_fractal_relics", "Pristine fractal relikviák"},
            {"currency_pristine_fractal_relics_tooltip", "Pristine fractal relikviák valuta megjelenítése"},
            {"currency_unstable_fractal_essence", "Instabil fractal esszencia"},
            {"currency_unstable_fractal_essence_tooltip", "Instabil fractal esszencia valuta megjelenítése"},
            {"currency_magnetite_shards", "Magnetit töredékek"},
            {"currency_magnetite_shards_tooltip", "Magnetit töredékek valuta megjelenítése"},
            {"currency_gaeting_crystals", "Gaeting kristályok"},
            {"currency_gaeting_crystals_tooltip", "Gaeting kristályok valuta megjelenítése"},
            {"currency_prophet_shards", "Próféta töredékek"},
            {"currency_prophet_shards_tooltip", "Próféta töredékek valuta megjelenítése"},
            {"currency_green_prophet_shards", "Zöld próféta töredékek"},
            {"currency_green_prophet_shards_tooltip", "Zöld próféta töredékek valuta megjelenítése"},
            {"currency_wvw_skirmish_tickets", "WvW skirmish jegyek"},
            {"currency_wvw_skirmish_tickets_tooltip", "WvW skirmish jegyek valuta megjelenítése"},
            {"currency_proofs_of_heroics", "Hősiesség bizonyítványai"},
            {"currency_proofs_of_heroics_tooltip", "Hősiesség bizonyítványai valuta megjelenítése"},
            {"currency_pvp_league_tickets", "PvP liga jegyek"},
            {"currency_pvp_league_tickets_tooltip", "PvP liga jegyek valuta megjelenítése"},
            {"currency_ascended_shards_of_glory", "Ascended dicsőség töredékek"},
            {"currency_ascended_shards_of_glory_tooltip", "Ascended dicsőség töredékek valuta megjelenítése"},
            {"currency_research_notes", "Kutatási jegyzetek"},
            {"currency_research_notes_tooltip", "Kutatási jegyzetek valuta megjelenítése"},
            {"currency_tyrian_defense_seal", "Tyriai védelem pecsét"},
            {"currency_tyrian_defense_seal_tooltip", "Tyriai védelem pecsét valuta megjelenítése"},
            {"currency_testimony_of_desert_heroics", "Sivatagi hősiesség tanúsítványa"},
            {"currency_testimony_of_desert_heroics_tooltip", "Sivatagi hősiesség tanúsítványa valuta megjelenítése"},
            {"currency_testimony_of_jade_heroics", "Jade hősiesség tanúsítványa"},
            {"currency_testimony_of_jade_heroics_tooltip", "Jade hősiesség tanúsítványa valuta megjelenítése"},
            {"currency_testimony_of_castoran_heroics", "Castor hősiesség tanúsítványa"},
            {"currency_testimony_of_castoran_heroics_tooltip", "Castor hősiesség tanúsítványa valuta megjelenítése"},
            {"currency_legendary_insight", "Legendás belátás"},
            {"currency_legendary_insight_tooltip", "Legendás belátás valuta megjelenítése"},
            {"currency_tales_of_dungeon_delving", "Dungeon felfedezés történetei"},
            {"currency_tales_of_dungeon_delving_tooltip", "Dungeon felfedezés történetei valuta megjelenítése"},
            {"currency_imperial_favor", "Császári kegy"},
            {"currency_imperial_favor_tooltip", "Császári kegy valuta megjelenítése"},
            {"currency_canach_coins", "Canach érmék"},
            {"currency_canach_coins_tooltip", "Canach érmék valuta megjelenítése"},
            {"currency_ancient_coin", "Ókori érme"},
            {"currency_ancient_coin_tooltip", "Ókori érme valuta megjelenítése"},
            {"currency_unusual_coin", "Szokatlan érme"},
            {"currency_unusual_coin_tooltip", "Szokatlan érme valuta megjelenítése"},
            {"currency_jade_sliver", "Jade töredék"},
            {"currency_jade_sliver_tooltip", "Jade töredék valuta megjelenítése"},
            {"currency_static_charge", "Statikus töltés"},
            {"currency_static_charge_tooltip", "Statikus töltés valuta megjelenítése"},
            {"currency_pinch_of_stardust", "Csipet csillagpor"},
            {"currency_pinch_of_stardust_tooltip", "Csipet csillagpor valuta megjelenítése"},
            {"currency_calcified_gasp", "Kalcifikált sóhajtás"},
            {"currency_calcified_gasp_tooltip", "Kalcifikált sóhajtás valuta megjelenítése"},
            {"currency_ursus_oblige", "Ursus Oblige"},
            {"currency_ursus_oblige_tooltip", "Ursus Oblige valuta megjelenítése"},
            {"currency_gaeting_crystal_janthir", "Gaeting kristály (Janthir)"},
            {"currency_gaeting_crystal_janthir_tooltip", "Gaeting kristály (janthir) valuta megjelenítése"},
            {"currency_antiquated_ducat", "Elavult ducát"},
            {"currency_antiquated_ducat_tooltip", "Elavult ducát valuta megjelenítése"},
            {"currency_aether_rich_sap", "Aether-gazdag nedű"},
            {"currency_aether_rich_sap_tooltip", "Aether-gazdag nedű valuta megjelenítése"},

            // Additional Filters
            {"additional_filters", "További szűrők"},
            {"account_bound", "Fiókhoz kötött"},
            {"account_bound_tooltip", "Fiókhoz kötött tárgyak megjelenítése"},
            {"not_account_bound", "Nem fiókhoz kötött"},
            {"not_account_bound_tooltip", "Nem fiókhoz kötött tárgyak megjelenítése"},
            {"nosell_items", "NoSell"},
            {"nosell_items_tooltip", "NoSell tárgyak megjelenítése"},
            {"not_nosell", "Nem NoSell"},
            {"not_nosell_tooltip", "Eladható tárgyak megjelenítése"},
            {"favorite_items", "Kedvenc"},
            {"favorite_items_tooltip", "Kedvenc tárgyak megjelenítése"},
            {"not_favorite", "Nem kedvenc"},
            {"not_favorite_tooltip", "Nem kedvenc tárgyak megjelenítése"},
            {"ignored_items", "Mellőzött"},
            {"ignored_items_tooltip", "Mellőzött tárgyak megjelenítése"},
            {"not_ignored", "Nem mellőzött"},
            {"not_ignored_tooltip", "Nem mellőzött tárgyak megjelenítése"},

            // Range Filters
            {"range_filters", "Tartomány szűrők"},
            {"show_range_filters", "Tartomány szűrők megjelenítése"},
            {"filter_min_price", "Szűrő min. ár"},
            {"filter_max_price", "Szűrő max. ár"},
            {"filter_min_quantity", "Szűrő min. mennyiség"},
            {"filter_max_quantity", "Szűrő max. mennyiség"},

            // Mini Window Settings
            {"mini_window_settings", "Mini ablak"},
            {"show_profit", "Profit megjelenítése"},
            {"show_profit_tooltip", "Teljes profit megjelenítése a mini ablakban"},
            {"show_profit_per_hour", "Profit/óra megjelenítése"},
            {"show_profit_per_hour_tooltip", "Profit óránként megjelenítése a mini ablakban"},
            {"show_tp_sell", "TP Eladás megjelenítése (Listázások)"},
            {"show_tp_sell_tooltip", "TP eladási profit megjelenítése (listázások) a mini ablakban"},
            {"show_tp_instant", "TP Azonnali megjelenítése (Azonnali eladás)"},
            {"show_tp_instant_tooltip", "TP azonnali eladási profit megjelenítése a mini ablakban"},
            {"show_total_items", "Összes tárgy megjelenítése"},
            {"show_total_items_tooltip", "Összes tárgy számláló megjelenítése a mini ablakban"},
            {"show_session_duration", "Munkamenet időtartam megjelenítése"},
            {"show_session_duration_tooltip", "Munkamenet időtartam megjelenítése a mini ablakban"},
            {"window_click_through", "Ablak átkattintás"},
            {"window_click_through_tooltip", "Lehetővé teszi az átkattintást a mini ablakon a játékra"},

            // Main Window Settings
            {"main_window", "Fő ablak"},
            {"click_through", "Átkattintás"},
            {"click_through_tooltip", "Lehetővé teszi az átkattintást a fő ablakon a játékra"},

            // Advanced UI Settings
            {"advanced_ui_settings", "Haladó UI beállítások"},
            {"no_advanced_ui_settings", "(Nincsenek elérhető haladó UI beállítások)"},

            // Display Settings
            {"display_settings", "Kijelző beállítások"},
            {"show_item_icons", "Tárgy ikonok megjelenítése"},
            {"show_item_icons_tooltip", "Tárgy ikonok megjelenítése a listában"},
            {"show_rarity_borders", "Ritkaság keretek megjelenítése"},
            {"show_rarity_borders_tooltip", "Színezett keretek megjelenítése az ikonok körül ritkaság alapján"},
            {"enable_grid_view", "Rács nézet engedélyezése"},
            {"enable_grid_view_tooltip", "Tárgyak megjelenítése rács elrendezésben lista helyett"},
            {"grid_icon_size", "Rács ikon méret"},
            {"grid_icon_size_tooltip", "Ikonok mérete a rács nézetben"},

            // Count Display Settings
            {"count_display_settings", "Darab kijelző beállítások"},
            {"count_text_color", "Darab szöveg szín"},
            {"count_text_color_tooltip", "Darab szöveg színe"},
            {"count_background_color", "Darab háttér szín"},
            {"count_background_color_tooltip", "Darab háttér színe"},
            {"count_font_size", "Darab betűméret"},
            {"count_font_size_tooltip", "Darab betűméret"},
            {"count_horizontal_alignment", "Darab vízszintes igazítás"},
            {"count_horizontal_alignment_tooltip", "Darab szöveg vízszintes igazítása"},

            // Gradient Background Settings
            {"gradient_background_settings", "Gradiens háttér beállítások"},
            {"enable_gradient_backgrounds", "Gradiens háttér engedélyezése"},
            {"enable_gradient_backgrounds_tooltip", "Gradiens háttér engedélyezése az ablakokhoz"},
            {"gradient_top_color", "Gradiens felső szín"},
            {"gradient_top_color_tooltip", "Gradiens háttér felső színe"},
            {"gradient_bottom_color", "Gradiens alsó szín"},
            {"gradient_bottom_color_tooltip", "Gradiens háttér alsó színe"},

            // Custom Profit System
            {"custom_profit_system", "Egyedi profit rendszer"},
            {"enable_custom_profit", "Egyedi profit engedélyezése"},
            {"enable_custom_profit_tooltip", "Egyedi profit értékek engedélyezése a tárgyakhoz"},

            // Search
            {"search_settings", "Keresés"},
            {"enable_search", "Keresés engedélyezése"},
            {"enable_search_tooltip", "Keresési funkció engedélyezése"},

            // Ignored Items
            {"ignored_items_settings", "Mellőzött tárgyak"},
            {"enable_ignored_items", "Mellőzött tárgyak engedélyezése"},
            {"enable_ignored_items_tooltip", "Mellőzött tárgyak funkció engedélyezése"},

            // Auto Reset
            {"auto_reset_settings", "Auto visszaállítás"},
            {"enable_auto_reset", "Auto visszaállítás engedélyezése"},
            {"enable_auto_reset_tooltip", "Farming munkamenet automatikus visszaállítása egy időtartam után"},
            {"auto_reset_duration", "Auto visszaállítás időtartam (percek)"},
            {"auto_reset_duration_tooltip", "Időtartam percben az auto visszaállítás előtt"},

            // DRF Settings
            {"drf_settings", "DRF beállítások"},
            {"drf_token", "DRF Token"},
            {"drf_token_label", "DRF Token:"},
            {"drf_token_tooltip", "Az Ön DRF hitelesítési tokenje"},
            {"edit_token", "Token szerkesztése"},
            {"save_token", "Token mentése"},

            // GW2 API Settings
            {"gw2_api_settings", "GW2 API beállítások"},
            {"gw2_api_key", "GW2 API kulcs"},
            {"gw2_api_key_tooltip", "Az Ön GW2 API kulcsa a tárgy részletekhez"},
            {"edit_key", "Kulcs szerkesztése"},
            {"save_key", "Kulcs mentése"},

            // Language Settings
            {"language_settings", "Nyelv"},
            {"language_tooltip", "Válasszon felület nyelvet"},
            {"language_english", "Angol"},
            {"language_german", "Német"},
            {"language_french", "Francia"},
            {"language_spanish", "Spanyol"},
            {"language_chinese", "Kínai"},
            {"language_czech", "Cseh"},
            {"language_italian", "Olasz"},
            {"language_polish", "Lengyel"},
            {"language_portuguese", "Portugál"},
            {"language_russian", "Orosz"},

            // Additional hardcoded strings found in UI
            {"farming_tracker_title", "Farming Tracker"},
            {"no_accounts_configured", "Nincs fiók konfigurálva"},
            {"no_profiles_created", "Még nincs profil létrehozva"},
            {"count_label", "Darab:"},
            {"profit_label", "Profit:"},
            {"no_profit", "Nincs profit"},
            {"vendor_value_label", "Eladói érték:"},
            {"tp_sell_gross_label", "TP Eladás (Bruttó):"},
            {"tp_sell_net_label", "TP Eladás (Nettó):"},
            {"tp_buy_gross_label", "TP Vásárlás (Bruttó):"},
            {"tp_buy_net_label", "TP Vásárlás (Nettó):"},
            {"ignored_items_label", "Mellőzött tárgyak:"},
            {"ignored_currencies_label", "Mellőzött valuták:"},
            {"total_items_label", "Összes tárgy:"},
            {"total_currencies_label", "Összes valuta:"},
            {"total_profit_label", "Teljes profit:"},
            {"tp_sell_profit_label", "TP Eladás Profit:"},
            {"tp_sell_profit_tooltip", "Teljes profit, ha minden tárgy eladható a jelenlegi TP listázási árakon (minusz 15% díj)"},
            {"vendor_profit_label", "Eladói Profit:"},
            {"profit_per_hour_label", "Profit óránként:"},
            {"opportunity_cost_profit_label", "Lehetőség-költség Profit:"},
            {"opportunity_cost_profit_per_hour_label", "Lehetőség-költség Profit/óra:"},
            {"custom_profit_feature_placeholder", "Funkció implementálva - UI követi"},
            {"custom_profit_items_header", "Egyedi profitú tárgyak"},
            {"custom_profit_currencies_header", "Egyedi profitú valuták"},
            {"add_custom_profit_item", "Egyedi profit hozzáadása tárgyhoz"},
            {"add_custom_profit_currency", "Egyedi profit hozzáadása valutához"},
            {"custom_profit_set_profit", "Profit beállítása"},
            {"custom_profit_remove", "Eltávolítás"},
            {"custom_profit_value", "Profit érték (Réz)"},
            {"custom_profit_set_tooltip", "Egyedi profit érték beállítása ehhez a tárgyhoz"},
            {"custom_profit_remove_tooltip", "Egyedi profit érték eltávolítása erről a tárgyról"},
            {"no_custom_profit_items", "(Nincs egyedi profitú tárgy)"},
            {"no_custom_profit_currencies", "(Nincs egyedi profitú valuta)"},
            {"clear_all_custom_profits", "Összes egyedi profit törlése"},
            {"clear_all_custom_profits_tooltip", "Összes egyedi profit érték törlése"},
            {"tabs_settings", "Egyéb fülek"},
            {"tabs_description", "Egyéb fülek megjelenítése vagy elrejtése"},
            {"tab_settings", "Fül beállítások"},
            {"tab_settings_description", "Fül sorrend és viselkedés"},
            {"enable_dashboard_tab", "Vezérlőpult fül engedélyezése"},
            {"enable_dashboard_tab_tooltip", "Vezérlőpult fül megjelenítése"},
            {"enable_items_tab", "Tárgyak fül engedélyezése"},
            {"enable_items_tab_tooltip", "Tárgyak fül megjelenítése"},
            {"enable_currencies_tab", "Valuták fül engedélyezése"},
            {"enable_currencies_tab_tooltip", "Valuták fül megjelenítése"},
            {"enable_ignored_tab", "Mellőzött fül engedélyezése"},
            {"enable_ignored_tab_tooltip", "Mellőzött tárgyak fül megjelenítése"},
            {"enable_session_history_tab", "Munkamenet előzmények fül engedélyezése"},
            {"enable_session_history_tab_tooltip", "Munkamenet előzmények fül megjelenítése"},
            {"enable_timeline_tab", "Idővonal fül engedélyezése"},
            {"enable_timeline_tab_tooltip", "Idővonal fül megjelenítése részletes drop előzményekkel"},
            {"enable_filter_tab", "Szűrő fül engedélyezése"},
            {"enable_filter_tab_tooltip", "Szűrő fül megjelenítése"},
            {"lock_tab_order", "Fül sorrend zárolása"},
            {"lock_tab_order_tooltip", "Fülek átrendezésének tiltása a fő ablakban"},
            {"enable_summaries_tab", "Összefoglaló fül engedélyezése"},
            {"enable_summaries_tab_tooltip", "Napi/heti/havi összefoglaló fül megjelenítése a munkamenet előzményekben"},
            {"custom_profit_settings", "Egyedi profit"},
            {"total_profit_label_simple", "Teljes profit"},
            {"total_items_label_simple", "Összes tárgy"},
            {"total_currencies_label_simple", "Összes valuta"},
            {"profit_per_hour_label_simple", "Profit óránként"},
            {"session_duration_label_simple", "Munkamenet időtartama"},
            {"next_reset_label_simple", "Következő visszaállítás"},
            {"export_label", "Exportálás:"},
            {"quick_actions", "Gyors műveletek:"},
            {"reset_confirm", "Biztosan visszaállítja az összes beállítást az alapértékekre?"},
            {"reset_warning", "Ez a művelet nem vonható vissza."},
            {"hotkeys", "Gyorsbillentyűk"},
            {"mini_window_toggle_hotkey", "Mini ablak váltó gyorsbillentyű"},
            {"backup_restore", "Biztonsági mentés & Visszaállítás"},
            {"appearance_settings", "Megjelenés"},
            {"enable_tooltips", "Buborékok engedélyezése"},
            {"enable_tooltips_tooltip", "Buborékok megjelenítése az UI elemek fölé húzáskor"},
            {"enable_grid_view_tooltip", "Elemek megjelenítése rács elrendezésben lista helyett"},
            {"favorites_first_tooltip", "Kedvencelemek megjelenítése a lista tetején"},
            {"group_by_rarity_tooltip", "Elemek csoportosítása ritkaságuk szerint"},
            {"show_rarity_as_tabs_tooltip", "Minden ritkaság megjelenítése külön fülként"},
            {"group_by_category_tooltip", "Elemek csoportosítása kategóriájuk szerint"},
            {"show_group_as_tabs_tooltip", "Minden kategória megjelenítése külön fülként"},
            {"mass_ignore_rarity_tooltip", "Minden elem figyelmen kívül hagyása ebből a ritkaságból"},
            {"icons_borders", "Ikonok & Keretek"},
            {"colors_gradients", "Színek & Gradiensek"},
            {"window_opacity", "Ablak átlátszóság"},
            {"windows_settings", "Ablakok"},
            {"advanced_settings", "Haladó"},
            {"export_settings", "Beállítások exportálása fájlba:"},
            {"import_settings", "Beállítások importálása fájlból:"},
            {"edit_account", "Fiók szerkesztése: %s"},
            {"account_name", "Fiók neve:"},
            {"gw2_api_key_label", "GW2 API kulcs:"},
            {"reload_config", "Konfiguráció újratöltése:"},
            {"auto_reset_label", "Automatikus visszaállítás:"},
            {"next_reset_utc", "Következő tervezett visszaállítás (UTC): %s"},
            {"favorites_ui", "Kedvencek UI:"},
            {"favorites_colors", "Kedvencek színek:"},
            {"visual_enhancements", "Vizuális fejlesztések:"},
            {"show_profit_sparkline", "Profit sparkline megjelenítése"},
            {"show_profit_sparkline_tooltip", "Kis vonaldiagram megjelenítése profit óránként trenddel"},
            {"mini_window_widget", "Mini ablak (Overlay widget):"},
            {"main_window_label", "Fő ablak:"},
            {"profiles_description", "A profilok lehetővé teszik különböző konfigurációk mentését és gyors váltást köztük."},
            {"create_new_profile", "Új profil létrehozása:"},
            {"current_profile", "Jelenlegi profil: %s"},
            {"auto_backup", "Automatikus biztonsági mentés a fontos változtatások előtt"},
            {"notifications", "Játékbeli értesítések konfigurálása fontos eseményekhez"},
            {"profit_goal", "Profit cél:"},
            {"reset_warning_label", "Visszaállítás figyelmeztetés:"},
            {"session_complete", "Munkamenet befejezve:"},
            {"manage_ignored_items", "Mellőzött tárgyak kezelése"},
            {"manage_ignored_currencies", "Mellőzött valuták kezelése"},
            {"rarity_label", "Ritkaság: %s"},
            {"type_label", "Típus: %d"},
            {"account_bound_label", "Fiókhoz kötött: %s"},
            {"nosell_label", "NoSell: %s"},
            {"item_id_label", "Tárgy ID: %d"},
            {"currency_id_label", "Valuta ID: %d"},
            {"context_menu_add_favorites", "Hozzáadás a kedvencekhez"},
            {"context_menu_remove_favorites", "Eltávolítás a kedvencekből"},
            {"context_menu_ignore", "Tárgy mellőzése"},
            {"context_menu_unignore", "Eltávolítás a mellőzöttekből"},
            {"context_menu_copy_name", "Név másolása"},
            {"context_menu_copy_id", "ID másolása"},
            {"sell_method_filters_label", "Eladási módszer szűrők:"},
            {"api_knowledge_filters_label", "API tudás szűrők:"},
            {"additional_filters_label", "További szűrők:"},
            {"item_type_filters_label", "Tárgy típus szűrők:"},
            {"currency_filters_label", "Valuta szűrők:"},
            {"price_range", "Ártartomány (Réz):"},
            {"quantity_range", "Mennyiségi tartomány:"},
            {"debug_info", "Hibakeresési információ"},
            {"drf_status", "DRF státusz: %s"},
            {"drf_reconnect_count", "DRF újrakapcsolódás számláló: %d"},
            {"gw2_api_status", "GW2 API státusz: %s"},
            {"gw2_api_reconnect_count", "GW2 API újrakapcsolódás számláló: %d"},
            {"session_duration_debug", "Munkamenet időtartam: %s"},
            {"gw2_memory", "GW2 folyamat memória: %zu MB"},
            {"gw2_api_request_count", "GW2 API kérés számláló: %d"},
            {"ignored_items_count", "Mellőzött tárgyak: %d"},
            {"ignored_currencies_count", "Mellőzött valuták: %d"},
            {"drf_logs", "DRF naplók:"},
            {"last_100_entries", "(Utolsó 100 bejegyzés)"},
            {"gw2_api_logs", "GW2 API naplók:"},
            {"item_currency_details", "Tárgy/Valuta részletek (Első 5):"},
            {"item_label", "Tárgy %d: %s (Darab: "},
            {"loaded_label", ", Betöltve: %s)"},
            {"currency_label", "Valuta %d: %s (Darab: "},
            {"custom_profit_items", "Egyedi profitú tárgyak (Első 5):"},
            {"custom_profit_item", "Tárgy %d: %s (Egyedi profit: "},
            {"no_custom_profit_items", "(Nincs egyedi profitú tárgy)"},
            {"ignored_items_debug", "Mellőzött tárgyak (Első 5):"},
            {"yes_label", "Igen"},
            {"no_label", "Nem"},
            {"profits_label", "Profitok:"},
            {"profits_tooltip", "Teljes profit a farmingból"},
            {"approx_profits_label", "Becsült profitok:"},
            {"approx_profits_tooltip", "Teljes profit MAX(Eladó, TP Eladás 15% díjjal) vagy Egyedi Profitból"},
            {"approx_gold_per_hour_label", "Becsült arany óránként:"},
            {"approx_gold_per_hour_tooltip", "Profit óránként a munkamenet időtartama alapján"},
            {"trading_profits_label", "Kereskedési profitok:"},
            {"trading_profits_tooltip", "Profit a tárgyak eladásából a Trading Poston"},
            {"approx_trading_profits_listings_label", "Becsült kereskedési profitok (Listázások):"},
            {"approx_trading_profits_listings_tooltip", "Teljes profit, ha eladva TP listázásokon (15% díj levonva)"},
            {"approx_trading_profits_instant_label", "Becsült kereskedési profitok (Azonnali eladás):"},
            {"approx_trading_profits_instant_tooltip", "Teljes profit, ha eladva TP azonnali vásárlási rendeléseken (15% díj levonva)"},
            {"trading_details_label", "Kereskedési részletek (Lehetőség-költség):"},
            {"trading_details_tooltip", "Profit elvesztése a TP listázásokon történő eladás elmulasztása miatt"},
            {"lost_profit_vs_tp_sell_label", "Elvesztett profit (vs TP Eladás):"},
            {"lost_profit_vs_tp_sell_tooltip", "Lehetőség-költség: Profit elvesztése a TP-n történő eladás elmulasztása miatt (15% díjjal)"},
            {"lost_profit_per_hour_vs_tp_sell_label", "Elvesztett profit óránként (vs TP Eladás):"},
            {"lost_profit_per_hour_vs_tp_sell_tooltip", "Lehetőség-költség óránként"},
            {"session_duration_debug_label", "Munkamenet időtartam: %s"},
            {"session_duration_debug_tooltip", "Jelenlegi farming munkamenet időtartama"},
            {"tab_items", "Tárgyak"},
            {"manage_ignored_items", "Mellőzött tárgyak kezelése"},
            {"clear_all_ignored_items", "Összes mellőzött tárgy törlése"},
            {"unignore_item", "Tárgy de-mellőzése"},
            {"manage_favorite_items", "Kedvenc tárgyak kezelése"},
            {"favorite_items_label", "Kedvenc tárgyak:"},
            {"clear_all_favorite_items", "Összes kedvenc tárgy törlése"},
            {"tab_currencies", "Valuták"},
            {"manage_ignored_currencies", "Mellőzött valuták kezelése"},
            {"clear_all_ignored_currencies", "Összes mellőzött valuta törlése"},
            {"unignore_currency", "Valuta de-mellőzése"},
            {"manage_favorite_currencies", "Kedvenc valuták kezelése"},
            {"favorite_currencies_label", "Kedvenc valuták:"},
            {"clear_all_favorite_currencies", "Összes kedvenc valuta törlése"},
            {"filter_active",   "Aktív"},
            {"filter_inactive", "Inaktív"},
            {"filter_all", "Összes"},
            {"filter_none", "Nincs"},
            {"filter_reset_all", "Összes visszaállítása"},
            {"filter_search_hint", "Szűrő keresése..."},
            {"filter_active_count", "%d szűrő aktív"},
            {"sell_method_filters_label", "Eladási módszer szűrők:"},
            {"api_knowledge_filters_label", "API tudás szűrők:"},
            {"additional_filters_label", "További szűrők:"},
            {"item_type_filters_label", "Tárgy típus szűrők:"},
            {"currency_filters_label", "Valuta szűrők:"},
            {"price_range", "Ártartomány (Réz):"},
            {"quantity_range", "Mennyiségi tartomány:"},
            {"debug_connection_status", "Kapcsolat státusz"},
            {"debug_session_snapshot", "Munkamenet pillanatkép"},
            {"debug_profit_breakdown", "Profit felbontás"},
            {"debug_data_state", "Adat állapot"},
            {"debug_logs", "Naplók"},
            {"debug_favorites", "Kedvencek"},
            {"debug_total_session", "összes ebben a munkamenetben"},
            {"debug_after_tp_fee", "15% díj után"},
            {"debug_direct_sell", "közvetlen eladás"},
            {"debug_rolling_avg", "futó átlag"},
            {"debug_vs_tp_sell", "vs TP eladás"},
            {"debug_per_hour", "óránként"},
            {"settings_api_key", "API kulcs"},
            {"settings_drf_token", "DRF Token"},
            {"debug_information", "Hibakeresési információ"},
            {"drf_status_label", "DRF státusz: %s"},
            {"drf_status_tooltip", "Jelenlegi DRF kapcsolat státusz"},
            {"drf_reconnect_count_label", "DRF újrakapcsolódás számláló: %d"},
            {"drf_reconnect_count_tooltip", "DRF újrakapcsolódási kísérletek száma"},
            {"gw2_api_status_label", "GW2 API státusz: %s"},
            {"gw2_api_status_tooltip", "Jelenlegi GW2 API kapcsolat státusz"},
            {"gw2_api_reconnect_count_label", "GW2 API újrakapcsolódás számláló: %d"},
            {"gw2_api_reconnect_count_tooltip", "GW2 API újrakapcsolódási kísérletek száma"},
            {"session_duration_debug", "Munkamenet időtartam: %s"},
            {"session_duration_debug_tooltip", "Jelenlegi farming munkamenet időtartama"},
            {"gw2_process_memory_label", "GW2 folyamat memória: %zu MB"},
            {"gw2_process_memory_tooltip", "Jelenlegi GW2 folyamat memória használat"},
            {"gw2_api_request_count_label", "GW2 API kérés számláló: %d"},
            {"gw2_api_request_count_tooltip", "Összes GW2 API kérés száma"},
            {"ignored_items_debug_label", "Mellőzött tárgyak: %d"},
            {"ignored_items_debug_tooltip", "Mellőzött tárgyak száma"},
            {"ignored_currencies_debug_label", "Mellőzött valuták: %d"},
            {"ignored_currencies_debug_tooltip", "Mellőzött valuták száma"},
            {"drf_logs_label", "DRF naplók:"},
            {"clear_drf_logs", "DRF naplók törlése"},
            {"clear_drf_logs_tooltip", "Összes DRF naplóbejegyzés törlése"},
            {"last_100_entries", "(Utolsó 100 bejegyzés)"},
            {"gw2_api_logs_label", "GW2 API naplók:"},
            {"clear_gw2_logs", "GW2 naplók törlése"},
            {"clear_gw2_logs_tooltip", "Összes GW2 API naplóbejegyzés törlése"},
            {"settings_label", "Beállítások:"},
            {"api_key_tooltip", "GW2 API kulcs státusz"},
            {"not_set", "Nincs beállítva"},
            {"set", "Beállítva"},
            {"drf_token_tooltip", "DRF Token státusz"},
            {"toggle_hotkey_label", "Váltó gyorsbillentyű: %s"},
            {"toggle_hotkey_tooltip", "Fő ablak váltó gyorsbillentyű"},
            {"auto_reset_mode_label", "Auto-visszaállítás mód: %d"},
            {"auto_reset_mode_tooltip", "Jelenlegi automatikus visszaállítás mód"},
            {"next_reset_label", "Következő visszaállítás: %s"},
            {"next_reset_tooltip", "Következő tervezett visszaállítás idő (UTC)"},
            {"fake_drf_server_label", "Hamis DRF szerver:"},
            {"use_fake_drf_server", "Hamis DRF szerver használata"},
            {"use_fake_drf_server_tooltip", "Csak tesztelési célokra"},
            {"reset_all_data", "Összes adat visszaállítása"},
            {"reset_all_data_tooltip", "Összes farming adat visszaállítása"},
            {"coin", "Érme"},
            {"info_button", "Info"},
            {"info_title", "FarmingTracker Info"},
            {"info_text", "A súgó szöveg később kerül ide..."},
            {"close_button", "Bezárás"},
            {"rarity_label", "Ritkaság: %s"},
            {"type_label", "Típus: %d"},
            {"account_bound_label", "Fiókhoz kötött: %s"},
            {"nosell_label", "NoSell: %s"},
            {"yes_label", "Igen"},
            {"no_label", "Nem"},
            {"sort_price_down", "Rendezés: Tárgy ár lefelé"},
            {"sort_price_up", "Rendezés: Tárgyár felfelé"},
            {"sort_count_high", "Rendezés: |Darab| magas"},
            {"sort_count_low", "Rendezés: |Darab| alacsony"},
            {"sort_name_az", "Rendezés: Név A–Z"},
            {"sort_name_za", "Rendezés: Név Z–A"},
            {"last_reset_label", "Visszaállítás"},
            {"last_reset_tooltip", "Idő az utolsó visszaállítás óta"},
            {"custom_profit_edit_tooltip",    "Profit érték szerkesztése"},
            {"custom_profit_confirm_tooltip", "Változtatások mentése"},
            {"accent_color", "Accent Szín (Buttons, Füls, UI)"}
            {"accent_color_tooltip", "Accent color for buttons, tabs, and UI elements"}
            {"account_management", "Fiók Management"}
            {"account_prefix", "Fiók"}
            {"actions", "Actions"}
            {"add_account", "+ Hozzáadás Fiók"}
            {"api_key_invalid_format", "(Invalid Format: 9 Blocks required)"}
            {"auto_reset_custom_days", "Egyéni (days)"}
            {"auto_reset_daily", "Daily reset (00:00 UTC)"}
            {"auto_reset_done_msg", "The tracker has been reset."}
            {"auto_reset_done_title", "Visszaállítás Complete"}
            {"auto_reset_minutes_unload", "Minutes after last unload"}
            {"auto_reset_never", "Never (manual Visszaállítás only)"}
            {"auto_reset_on_load", "Be addon load"}
            {"auto_reset_tooltip", "When to automatically reset farming counters"}
            {"auto_reset_weekly", "Weekly (Mon 07:30 UTC)"}
            {"auto_reset_weekly_eu_wvw", "Weekly EU WvW (Fri 18:00 UTC)"}
            {"auto_reset_weekly_map_bonus", "Weekly map bonus (Thu 20:00 UTC)"}
            {"auto_reset_weekly_na_wvw", "Weekly NA WvW (Sat 02:00 UTC)"}
            {"automatic_backups", "Automatic Biztonsági mentéss"}
            {"backup", "Biztonsági mentés"}
            {"backup_daily", "Daily"}
            {"backup_frequency", "Biztonsági mentés frequency"}
            {"backup_frequency_tooltip", "How often to create automatic backups"}
            {"backup_manual_only", "Kézi only"}
            {"backup_weekly", "Weekly"}
            {"best_drop", "Legjobb Drop"}
            {"border_size", "Szegély Méret"}
            {"border_size_tooltip", "Adjust the thickness of rarity borders (1.0 - 10.0)"}
            {"bottom_gradient_color", "Bottom"}
            {"bottom_gradient_color_tooltip", "Bottom gradient color"}
            {"browse_for_file", "Browse for file..."}
            {"cancel", "Mégse"}
            {"clear_all_custom_profits_warning", "Mind custom profit values will be deleted. This action cannot be undone."}
            {"clear_compare_selection", "Törlés selection"}
            {"clear_history", "Törlés Előzmények"}
            {"clear_history_confirm", "Törlés all session history?"}
            {"clear_history_tooltip", "Törlés all saved session history"}
            {"clear_history_warning", "This action cannot be undone!"}
            {"clear_search", "Törlés"}
            {"clear_search_favorites", "Törlés"}
            {"clear_search_tooltip", "Törléss the current search"}
            {"column_count", "Szám"}
            {"column_currency", "Currency"}
            {"column_favorite", "Favorite"}
            {"column_icon", "Ikon"}
            {"column_ignore", "Ignore"}
            {"column_item", "Item"}
            {"column_label", "Címke"}
            {"column_name", "Name"}
            {"column_profit", "Profit"}
            {"column_value", "Érték"}
            {"comparison_previous_period", "Comparison with previous period:"}
            {"count_format", "Szám: %lld"}
            {"create", "Create"}
            {"create_new_profile_tooltip", "Create a new profile with current settings"}
            {"create_tooltip", "Create a new profile with current settings"}
            {"currencies_header", "Pénznemek"}
            {"currency_cat_common", "Common"}
            {"currency_cat_fractal", "Fractals"}
            {"currency_cat_janthir", "Janthir Wilds"}
            {"currency_cat_map", "Map Pénznemek"}
            {"currency_cat_other", "Other"}
            {"currency_cat_pvp", "PvP"}
            {"currency_cat_raid_strike", "Raids & Strikes"}
            {"currency_cat_wvw", "WvW"}
            {"currency_group_by_category", "Csoportosítás by category"}
            {"currency_group_by_category_tooltip", "Csoportosítás currencies by category with collapsible sections or tabs"}
            {"currency_show_as_tabs", "Megjelenítés as tabs"}
            {"currency_show_as_tabs_tooltip", "Megjelenítés category groups as tabs instead of collapsible sections"}
            {"currency_table_favorite_tooltip", "Hozzáadás/remove favorite. Favorites appear in the Favorites tab. Tip: Right-click the icon/name for more actions."}
            {"currency_table_ignore_tooltip", "Hozzáadás/remove ignored. Ignored currencies appear in the Ignored tab. Tip: Right-click the icon/name for more actions."}
            {"date", "Dátum"}
            {"debug_settings", "Hibakeresés Beállítások"}
            {"default_no_profile", "Alapértelmezett (Nem Profile)"}
            {"delete_profile", "Törlés Profile"}
            {"delete_profile_tooltip", "Törlés current profile"}
            {"details", "Details"}
            {"drops", "Drops"}
            {"duration", "Időtartam"}
            {"enable_automatic_backups", "Engedélyezés automatic backups"}
            {"enable_automatic_backups_tooltip", "Automatically create backups before changes"}
            {"enable_best_drop_highlight", "Highlight Legjobb Drop"}
            {"enable_best_drop_highlight_tooltip", "Highlight the most valuable drop with a golden border in the Tárgyak tab"}
            {"enable_best_drop_in_mini_window", "Megjelenítés Legjobb Drop in Mini Ablak"}
            {"enable_best_drop_in_mini_window_tooltip", "Megjelenítés the most valuable drop in the mini window overlay"}
            {"enable_debug_tab", "Engedélyezés Hibakeresés Fül"}
            {"enable_debug_tab_tooltip", "Megjelenítéss the debug tab with additional information"}
            {"enable_favorite_row_color", "Engedélyezés favorite row color"}
            {"enable_favorite_row_color_tooltip", "Highlights favorite items/currencies with custom row background color"}
            {"enable_favorite_text_color", "Engedélyezés favorite text color"}
            {"enable_favorite_text_color_tooltip", "Highlights favorite items/currencies with custom text color"}
            {"enable_favorites", "Engedélyezés Favorites"}
            {"enable_favorites_tab", "Engedélyezés Favorites Fül"}
            {"enable_favorites_tab_tooltip", "Megjelenítéss a separate favorites tab"}
            {"enable_grid_view_currencies", "Engedélyezés Grid View (Pénznemek)"}
            {"enable_grid_view_currencies_tooltip", "Toggle between list and grid view in Pénznemek tab"}
            {"enable_grid_view_items", "Engedélyezés Grid View (Tárgyak)"}
            {"enable_grid_view_items_tooltip", "Toggle between list and grid view in Tárgyak tab"}
            {"enable_icon_cache", "Engedélyezés Ikon Cache"}
            {"enable_icon_cache_tooltip", "Cache item icons on disk to speed up loading after the first session"}
            {"enable_notifications", "Engedélyezés notifications"}
            {"enable_notifications_tooltip", "Engedélyezés in-game notifications"}
            {"enable_session_history", "Engedélyezés Munkamenet Előzmények"}
            {"enable_session_history_tooltip", "Mentés farming session history for later viewing"}
            {"enable_session_timeline", "Engedélyezés Munkamenet Idővonal"}
            {"enable_session_timeline_tooltip", "Mentés detailed drop timeline with timestamps for session details"}
            {"export_history", "Exportálás Előzmények"}
            {"export_history_tooltip", "Exportálás session history to a JSON file"}
            {"export_logs", "Exportálás Logs"}
            {"favorite_items_header", "Favorite Tárgyak"}
            {"favorites_first", "Favorites First"}
            {"favorites_first_tooltip", "Megjelenítéss favorites first in item/currency lists"}
            {"favorites_settings", "Favorites Beállítások"}
            {"filter_account_bound", "Fiók-bound"}
            {"filter_account_bound_tooltip", "Megjelenítés account-bound items"}
            {"filter_aether_rich_sap", "Aether-Rich Sap"}
            {"filter_aether_rich_sap_tooltip", "Megjelenítés aether-rich sap currency"}
            {"filter_airship_parts", "Airship Parts"}
            {"filter_airship_parts_tooltip", "Megjelenítés airship parts currency"}
            {"filter_ancient_coin", "Ancient Coin"}
            {"filter_ancient_coin_tooltip", "Megjelenítés ancient coin currency"}
            {"filter_antiquated_ducat", "Antiquated Ducat"}
            {"filter_antiquated_ducat_tooltip", "Megjelenítés antiquated ducat currency"}
            {"filter_ascended_shards_of_glory", "Ascended Shards of Glory"}
            {"filter_ascended_shards_of_glory_tooltip", "Megjelenítés ascended shards of glory currency"}
            {"filter_astral_acclaim", "Astral Acclaim"}
            {"filter_astral_acclaim_tooltip", "Megjelenítés astral acclaim currency"}
            {"filter_badge_of_honor", "Badge of Honor"}
            {"filter_badge_of_honor_tooltip", "Megjelenítés badge of honor currency"}
            {"filter_calcified_gasp", "Calcified Gasp"}
            {"filter_calcified_gasp_tooltip", "Megjelenítés calcified gasp currency"}
            {"filter_canach_coins", "Canach Coins"}
            {"filter_canach_coins_tooltip", "Megjelenítés canach coins currency"}
            {"filter_custom_profit", "Has custom profit"}
            {"filter_custom_profit_tooltip", "Megjelenítés items with custom profit set"}
            {"filter_elegy_mosaic", "Elegy Mosaic"}
            {"filter_elegy_mosaic_tooltip", "Megjelenítés elegy mosaic currency"}
            {"filter_favorite", "Favorite"}
            {"filter_favorite_tooltip", "Megjelenítés favorite items (outside Favorites tab)"}
            {"filter_fractal_relic", "Fractal Relic"}
            {"filter_fractal_relic_tooltip", "Megjelenítés fractal relic currency"}
            {"filter_gaeting_crystal_janthir", "Gaeting Crystal (Janthir)"}
            {"filter_gaeting_crystal_janthir_tooltip", "Megjelenítés gaeting crystal (janthir) currency"}
            {"filter_gaeting_crystals", "Gaeting Crystals"}
            {"filter_gaeting_crystals_tooltip", "Megjelenítés gaeting crystals currency"}
            {"filter_gem", "Gem"}
            {"filter_gem_tooltip", "Megjelenítés gem currency"}
            {"filter_geode", "Geode"}
            {"filter_geode_tooltip", "Megjelenítés geode currency"}
            {"filter_green_prophet_shards", "Green Prophet Shards"}
            {"filter_green_prophet_shards_tooltip", "Megjelenítés green prophet shards currency"}
            {"filter_guild_commendation", "Guild Commendation"}
            {"filter_guild_commendation_tooltip", "Megjelenítés guild commendation currency"}
            {"filter_ignored", "Ignored"}
            {"filter_ignored_tooltip", "Megjelenítés ignored items"}
            {"filter_imperial_favor", "Imperial Favor"}
            {"filter_imperial_favor_tooltip", "Megjelenítés imperial favor currency"}
            {"filter_items", "Szűrő Tárgyak"}
            {"filter_jade_sliver", "Jade Sliver"}
            {"filter_jade_sliver_tooltip", "Megjelenítés jade sliver currency"}
            {"filter_karma_tooltip", "Megjelenítés karma currency"}
            {"filter_known_by_api", "Known by API"}
            {"filter_known_by_api_tooltip", "Megjelenítés items known by GW2 API"}
            {"filter_laurel", "Laurel"}
            {"filter_laurel_tooltip", "Megjelenítés laurel currency"}
            {"filter_legendary_insight", "Legendary Insight"}
            {"filter_legendary_insight_tooltip", "Megjelenítés legendary insight currency"}
            {"filter_ley_line_crystals", "Ley-Line Crystals"}
            {"filter_ley_line_crystals_tooltip", "Megjelenítés ley-line crystals currency"}
            {"filter_magnetite_shards", "Magnetite Shards"}
            {"filter_magnetite_shards_tooltip", "Megjelenítés magnetite shards currency"}
            {"filter_max_price_tooltip", "Maximum price filter (0 = disabled)"}
            {"filter_max_quantity_tooltip", "Maximum quantity filter (0 = disabled)"}
            {"filter_min_price_tooltip", "Minimum price filter (0 = disabled)"}
            {"filter_min_quantity_tooltip", "Minimum quantity filter (0 = disabled)"}
            {"filter_nosell", "NemSell"}
            {"filter_nosell_tooltip", "Megjelenítés NemSell items"}
            {"filter_not_account_bound", "Nemt Fiók-bound"}
            {"filter_not_account_bound_tooltip", "Megjelenítés non-account-bound items"}
            {"filter_not_favorite", "Nemt Favorite"}
            {"filter_not_favorite_tooltip", "Megjelenítés items that are not marked as favorite"}
            {"filter_not_ignored", "Nemt Ignored"}
            {"filter_not_ignored_tooltip", "Megjelenítés non-ignored items"}
            {"filter_not_nosell", "Nemt NemSell"}
            {"filter_not_nosell_tooltip", "Megjelenítés sellable items"}
            {"filter_pinch_of_stardust", "Pinch of Stardust"}
            {"filter_pinch_of_stardust_tooltip", "Megjelenítés pinch of stardust currency"}
            {"filter_pristine_fractal_relics", "Pristine Fractal Relics"}
            {"filter_pristine_fractal_relics_tooltip", "Megjelenítés pristine fractal relics currency"}
            {"filter_proofs_of_heroics", "Proofs of Heroics"}
            {"filter_proofs_of_heroics_tooltip", "Megjelenítés proofs of heroics currency"}
            {"filter_prophet_shards", "Prophet Shards"}
            {"filter_prophet_shards_tooltip", "Megjelenítés prophet shards currency"}
            {"filter_pvp_league_tickets", "PvP League Tickets"}
            {"filter_pvp_league_tickets_tooltip", "Megjelenítés PvP league tickets currency"}
            {"filter_rarity", "Szűrő Rarity"}
            {"filter_research_notes", "Research Nemtes"}
            {"filter_research_notes_tooltip", "Megjelenítés research notes currency"}
            {"filter_sellable_on_tp", "Sellable on TP"}
            {"filter_sellable_on_tp_tooltip", "Megjelenítés items sellable on Trading Post"}
            {"filter_sellable_to_vendor", "Sellable to vendor"}
            {"filter_sellable_to_vendor_tooltip", "Megjelenítés items sellable to vendor"}
            {"filter_spirit_shards", "Spirit Shards"}
            {"filter_spirit_shards_tooltip", "Megjelenítés spirit shards currency"}
            {"filter_static_charge", "Static Charge"}
            {"filter_static_charge_tooltip", "Megjelenítés static charge currency"}
            {"filter_tales_of_dungeon_delving", "Tales of Dungeon Delving"}
            {"filter_tales_of_dungeon_delving_tooltip", "Megjelenítés tales of dungeon delving currency"}
            {"filter_testimony_of_castoran_heroics", "Tesztimony of Castoran Heroics"}
            {"filter_testimony_of_castoran_heroics_tooltip", "Megjelenítés testimony of castoran heroics currency"}
            {"filter_testimony_of_desert_heroics", "Tesztimony of Desert Heroics"}
            {"filter_testimony_of_desert_heroics_tooltip", "Megjelenítés testimony of desert heroics currency"}
            {"filter_testimony_of_jade_heroics", "Tesztimony of Jade Heroics"}
            {"filter_testimony_of_jade_heroics_tooltip", "Megjelenítés testimony of jade heroics currency"}
            {"filter_trade_contracts", "Trade Contracts"}
            {"filter_trade_contracts_tooltip", "Megjelenítés trade contracts currency"}
            {"filter_transmutation_charge", "Transmutation Charge"}
            {"filter_transmutation_charge_tooltip", "Megjelenítés transmutation charge currency"}
            {"filter_type_armor", "Armor"}
            {"filter_type_armor_tooltip", "Megjelenítés armor items"}
            {"filter_type_backpack", "Backpack"}
            {"filter_type_backpack_tooltip", "Megjelenítés backpack items"}
            {"filter_type_bag", "Bag"}
            {"filter_type_bag_tooltip", "Megjelenítés bags"}
            {"filter_type_consumable", "Consumable"}
            {"filter_type_consumable_tooltip", "Megjelenítés consumable items"}
            {"filter_type_container", "Container"}
            {"filter_type_container_tooltip", "Megjelenítés containers"}
            {"filter_type_crafting_material", "Crafting Material"}
            {"filter_type_crafting_material_tooltip", "Megjelenítés crafting materials"}
            {"filter_type_gathering_tool", "Gathering Tool"}
            {"filter_type_gathering_tool_tooltip", "Megjelenítés gathering tools"}
            {"filter_type_gizmo", "Gizmo"}
            {"filter_type_gizmo_container", "Gizmo Container"}
            {"filter_type_gizmo_container_tooltip", "Megjelenítés gizmo container items"}
            {"filter_type_gizmo_tooltip", "Megjelenítés gizmo items"}
            {"filter_type_mini_pet", "Mini Pet"}
            {"filter_type_mini_pet_tooltip", "Megjelenítés mini pets"}
            {"filter_type_tool", "Tool"}
            {"filter_type_tool_tooltip", "Megjelenítés tool items"}
            {"filter_type_trinket", "Trinket"}
            {"filter_type_trinket_tooltip", "Megjelenítés trinket items"}
            {"filter_type_trophy", "Trophy"}
            {"filter_type_trophy_tooltip", "Megjelenítés trophy items"}
            {"filter_type_unlock", "Feloldás"}
            {"filter_type_unlock_tooltip", "Megjelenítés unlock items"}
            {"filter_type_upgrade_component", "Upgrade Component"}
            {"filter_type_upgrade_component_tooltip", "Megjelenítés upgrade components"}
            {"filter_type_weapon", "Weapon"}
            {"filter_type_weapon_tooltip", "Megjelenítés weapon items"}
            {"filter_tyrian_defense_seal", "Tyrian Defense Seal"}
            {"filter_tyrian_defense_seal_tooltip", "Megjelenítés tyrian defense seal currency"}
            {"filter_unbound_magic", "Unbound Magic"}
            {"filter_unbound_magic_tooltip", "Megjelenítés unbound magic currency"}
            {"filter_uncommon_coins", "Uncommon Coins"}
            {"filter_uncommon_coins_tooltip", "Megjelenítés uncommon coins currency"}
            {"filter_unknown_by_api", "Unknown by API"}
            {"filter_unknown_by_api_tooltip", "Megjelenítés items not known by GW2 API"}
            {"filter_unstable_fractal_essence", "Unstable Fractal Essence"}
            {"filter_unstable_fractal_essence_tooltip", "Megjelenítés unstable fractal essence currency"}
            {"filter_unusual_coin", "Unusual Coin"}
            {"filter_unusual_coin_tooltip", "Megjelenítés unusual coin currency"}
            {"filter_ursus_oblige", "Ursus Oblige"}
            {"filter_ursus_oblige_tooltip", "Megjelenítés ursus oblige currency"}
            {"filter_volatile_magic", "Volatile Magic"}
            {"filter_volatile_magic_tooltip", "Megjelenítés volatile magic currency"}
            {"filter_wvw_skirmish_tickets", "WvW Skirmish Tickets"}
            {"filter_wvw_skirmish_tickets_tooltip", "Megjelenítés WvW skirmish tickets currency"}
            {"first_5_custom_profit", "First 5 items with custom profit set"}
            {"first_5_custom_profit_tooltip", "First 5 items with custom profit set"}
            {"first_5_ignored_items", "First 5 ignored items"}
            {"first_5_ignored_items_tooltip", "First 5 ignored items"}
            {"first_5_tracked_items", "First 5 tracked items and currencies with details"}
            {"first_5_tracked_items_tooltip", "First 5 tracked items and currencies with details"}
            {"full_backup", "Full Biztonsági mentés"}
            {"full_backup_tooltip", "Biztonsági mentés all data (settings, session history, favorites, ignored items, custom profit) to a JSON file"}
            {"full_restore", "Full Visszaállítás"}
            {"full_restore_tooltip", "Visszaállítás all data from a backup JSON file"}
            {"general_settings", "Általános Beállítások"}
            {"gold_format", "Gold: %lld"}
            {"gradient_backgrounds", "Gradient backgrounds"}
            {"gradient_backgrounds_tooltip", "Engedélyezéss smooth gradient backgrounds for a more modern look"}
            {"grid_icon_size_currencies", "Grid Ikon size (Pénznemek)"}
            {"grid_icon_size_currencies_tooltip", "Méret of icons in grid view for Pénznemek (16-128)"}
            {"grid_icon_size_items", "Grid Ikon size (Tárgyak)"}
            {"grid_icon_size_items_tooltip", "Méret of icons in grid view for Tárgyak (16-128)"}
            {"group_by_rarity", "Csoportosítás by Rarity"}
            {"group_by_type", "Csoportosítás by Category"}
            {"icon_cache_max_icons", "Max Cached Ikons"}
            {"icon_cache_max_icons_tooltip", "Maximum number of icons to keep in cache (older icons are deleted when limit is reached)"}
            {"icon_size", "Ikon size"}
            {"icon_size_tooltip", "Méret of item icons in pixels (16-96)"}
            {"import", "Importálás"}
            {"import_history", "Importálás Előzmények"}
            {"import_history_tooltip", "Importálás session history from a JSON file"}
            {"import_tooltip", "Importálás settings from a JSON file"}
            {"infusion_drop_label", "Infusion Drop!"}
            {"item", "Item"}
            {"items_header", "Tárgyak"}
            {"magic_find_abbreviation", "MF: %d%%"}
            {"main_window_click_through", "Click through"}
            {"main_window_click_through_tooltip", "Mindows clicking through the main window to the game"}
            {"main_window_opacity", "Main Ablak Transparency"}
            {"main_window_opacity_tooltip", "Main window background transparency (0-100%)"}
            {"main_window_settings", "Main Ablak"}
            {"map", "Map"}
            {"mass_actions_clear_ignore", "Törlés ignore list"}
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
            {"max_backups", "Max Biztonsági mentéss"}
            {"max_backups_tooltip", "Maximum number of backups to keep (1-20)"}
            {"max_session_history", "Max Munkamenets"}
            {"max_session_history_tooltip", "Maximum number of sessions to save (1-50). Oldest session is deleted when limit is reached if overwrite is enabled."}
            {"min_value", "Min Érték"}
            {"mini_window_click_through", "Ablak click through"}
            {"mini_window_click_through_tooltip", "Mindows clicking through the mini window to the game"}
            {"mini_window_hide_title_bar", "Elrejtés Mini Ablak Title Bar"}
            {"mini_window_hide_title_bar_tooltip", "Elrejtés the title bar of the mini window"}
            {"mini_window_locked", "Zárolás Mini Ablak"}
            {"mini_window_locked_tooltip", "Fix the mini window position and size (no longer movable or resizable)"}
            {"mini_window_opacity", "Mini Ablak Transparency"}
            {"mini_window_opacity_tooltip", "Mini window background transparency (0-100%)"}
            {"mini_window_show_profit", "Megjelenítés Profit"}
            {"mini_window_show_profit_per_hour", "Megjelenítés Profit/Hour"}
            {"mini_window_show_profit_per_hour_tooltip", "Display profit per hour in mini window"}
            {"mini_window_show_profit_tooltip", "Display total profit in mini window"}
            {"mini_window_show_session_duration", "Megjelenítés Munkamenet Időtartam"}
            {"mini_window_show_session_duration_tooltip", "Display session duration in mini window"}
            {"mini_window_show_total_items", "Megjelenítés Összesen Tárgyak"}
            {"mini_window_show_total_items_tooltip", "Display total item count in mini window"}
            {"mini_window_show_tp_instant", "Megjelenítés TP Instant (Instant Sell)"}
            {"mini_window_show_tp_instant_tooltip", "Display TP instant sell profit in mini window"}
            {"mini_window_show_tp_sell", "Megjelenítés TP Sell (Listings)"}
            {"mini_window_show_tp_sell_tooltip", "Display TP sell profit (listings) in mini window"}
            {"minutes_after_unload_tooltip", "Minutes after addon unload before automatic reset"}
            {"no_cancel", "Nem, Mégse"}
            {"no_items_in_session", "Nem items in this session"}
            {"no_sessions_recorded", "Nem sessions recorded yet."}
            {"notification_combine_logic", "Combine Szűrős (AND)"}
            {"notification_combine_logic_tooltip", "If enabled, BOTH conditions (Érték AND Rarity) must be met. If disabled, ANY one of them is enough."}
            {"notification_duration", "Display Időtartam"}
            {"notification_duration_tooltip", "How long the notification stays visible (seconds)"}
            {"notification_general", "Általános Beállítások"}
            {"notification_include_agony", "Include Agony Infusions"}
            {"notification_include_agony_tooltip", "If enabled, Agony Infusions (+1 to +30) will also trigger an alert."}
            {"notification_include_non_profit", "Include Nemn-Profit Tárgyak"}
            {"notification_include_non_profit_tooltip", "If enabled, items with no gold value (0c) will also trigger alerts if they meet the rarity requirement."}
            {"notification_infusion_alert", "Infusion Alert"}
            {"notification_infusion_alert_tooltip", "Always notify when an Infusion is found (ignores Érték/Rarity filters)"}
            {"notification_item_alerts", "Item Alerts"}
            {"notification_min_rarity", "Min. Rarity"}
            {"notification_min_rarity_tooltip", "Trigger notification if item rarity is at least this level"}
            {"notification_min_value", "Min. Érték (Gold)"}
            {"notification_min_value_tooltip", "Trigger notification if item value is at least this amount"}
            {"notification_play_sound", "Lejátszás Hang"}
            {"notification_play_sound_tooltip", "Lejátszás a sound effect when a notification appears"}
            {"notification_precursor_alert", "Pre-Cursor Alert"}
            {"notification_precursor_alert_tooltip", "Always notify when a Pre-Cursor is found (ignores Érték/Rarity filters)"}
            {"notification_session_alerts", "Progress & Idő"}
            {"notification_settings", "Értesítés Beállítások"}
            {"notification_setup_hint", "[Drag to reposition notifications]"}
            {"notification_stacking", "Stack Értesítéss"}
            {"notification_stacking_tooltip", "Megjelenítés multiple notifications at once instead of replacing the old one immediately"}
            {"notification_triggers", "Értesítés Triggers"}
            {"notification_volume", "Master Hangerő"}
            {"notification_volume_tooltip", "Hangerő for notification sounds"}
            {"notify_profit_goal", "Nemtify when profit goal reached"}
            {"notify_profit_goal_tooltip", "Nemtify when you reach your profit goal"}
            {"notify_reset_warning", "Nemtify before reset"}
            {"notify_reset_warning_tooltip", "Nemtify before automatic reset occurs"}
            {"notify_session_complete", "Nemtify after session duration"}
            {"notify_session_complete_tooltip", "Nemtify after farming for a certain duration"}
            {"opportunity_cost_per_hour", "Opportunity cost per hour"}
            {"opportunity_cost_per_hour_tooltip", "Opportunity cost per hour"}
            {"opportunity_cost_vs_tp_sell", "Opportunity cost vs TP sell"}
            {"opportunity_cost_vs_tp_sell_tooltip", "Opportunity cost vs TP sell"}
            {"overwrite_session_history", "Overwrite Munkamenets"}
            {"overwrite_session_history_tooltip", "If enabled, oldest session is deleted when limit is reached"}
            {"performance_settings", "Performance Beállítások"}
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
            {"range_filters_tooltip", "Megjelenítés price and quantity range filters"}
            {"rare_drop_label", "Rare Drop!"}
            {"rarity_border_thickness", "Rarity Szegély Thickness"}
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
            {"remove_account", "- Eltávolítás Fiók"}
            {"reset_all", "Visszaállítás Mind"}
            {"reset_all_tooltip", "Visszaállítás all settings to default values"}
            {"reset_interval_days", "Visszaállítás interval (days)"}
            {"reset_interval_days_tooltip", "Egyéni reset interval in days (1-30 days)"}
            {"reset_settings", "Auto Visszaállítás"}
            {"reset_warning_minutes", "Visszaállítás Figyelmeztetés (Minutes)"}
            {"reset_warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"reset_warning_msg", "The tracker will reset in %d minutes!"}
            {"reset_warning_title", "Visszaállítás Figyelmeztetés"}
            {"restore", "Visszaállítás"}
            {"row_color", "Row Szín"}
            {"save", "Mentés"}
            {"save_account", "Mentés Fiók"}
            {"save_all_items_confirm", "Engedélyezés session timeline?"}
            {"save_all_items_warning", "This will significantly increase file size!"}
            {"save_current_session", "Mentés Current Munkamenet"}
            {"save_current_session_tooltip", "Mentés the current farming session without resetting"}
            {"save_tooltip", "Mentés current settings"}
            {"search_favorite_currencies_hint", "Keresés favorite currencies..."}
            {"search_favorite_items_hint", "Keresés favorite items..."}
            {"search_items", "Keresés Tárgyak"}
            {"search_items_hint", "Keresés items..."}
            {"select_profile", "Kijelölés a profile to apply its settings"}
            {"select_profile_tooltip", "Kijelölés a profile to apply its settings"}
            {"session_complete_hours", "Munkamenet Complete (Hours)"}
            {"session_complete_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_complete_msg", "You have been farming for %d hours!"}
            {"session_complete_title", "Munkamenet Complete"}
            {"session_count", "Munkamenet Szám"}
            {"session_details", "Munkamenet Details"}
            {"session_history", "Munkamenet Előzmények"}
            {"session_hours", "Munkamenet hours"}
            {"session_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_note", "Nemte"}
            {"session_profit_trend", "Profit Trend"}
            {"session_search_hint", "Keresés sessions, items, notes..."}
            {"sessions_selected", "sessions selected"}
            {"sessions_stored", "Munkamenets Stored"}
            {"settings_profiles", "Beállítások Profiles"}
            {"show_ignored_items", "Megjelenítés ignored items"}
            {"show_ignored_items_tooltip", "Megjelenítés ignored items/currencies in Tárgyak and Pénznemek tabs (disable to hide). Difference from 'Ignored' filter: This filter controls display in Tárgyak/Pénznemek tabs, the 'Ignored' filter controls display in Szűrő tab."}
            {"show_main_window", "Megjelenítés main window"}
            {"show_mini_window", "Megjelenítés mini window"}
            {"show_mini_window_tooltip", "Megjelenítéss a small overlay widget with key statistics"}
            {"show_notification_setup", "Setup Mode (Positioning)"}
            {"show_notification_setup_tooltip", "Makes the notification window visible so you can move it"}
            {"show_rarity_as_tabs", "Megjelenítés as Füls"}
            {"show_summaries", "Megjelenítés Summaries"}
            {"show_summaries_tooltip", "Megjelenítés daily/weekly/monthly profit summaries"}
            {"show_type_as_tabs", "Megjelenítés as Füls"}
            {"showing", "Megjelenítésing"}
            {"sort_profit_high", "Rendezés: Profit high"}
            {"sort_profit_low", "Rendezés: Profit low"}
            {"sort_rarity_high", "Rendezés: Rarity high to low"}
            {"sort_rarity_low", "Rendezés: Rarity low to high"}
            {"sound_alert", "Alert Hang"}
            {"sound_infusion", "Infusion Hang"}
            {"sound_path_hint", "Path to sound file (empty = default)"}
            {"sound_precursor", "Pre-Cursor Hang"}
            {"sound_standard", "Standard Hang"}
            {"sound_test", "Teszt"}
            {"stat_avg_profit_per_hour", "Avg Profit/h"}
            {"stat_best_session", "Legjobb Munkamenet"}
            {"stat_total_profit", "Összesen Profit"}
            {"stat_total_time", "Összesen Idő"}
            {"summaries_coming_soon", "Summaries feature coming soon..."}
            {"summaries_label", "Summaries"}
            {"summaries_tooltip", "Daily/Weekly/Monthly profit summaries"}
            {"summary_period", "Period:"}
            {"summary_this_month", "This Month"}
            {"summary_this_week", "This Week"}
            {"summary_today", "Today"}
            {"tab_session_history", "Munkamenet Előzmények"}
            {"tab_sessions", "Munkamenets"}
            {"tab_summaries", "Summaries"}
            {"test_item_label", "Teszt Item"}
            {"text_color", "Text Szín"}
            {"time", "Idő"}
            {"time_ago_seconds", "%llds ago"}
            {"timeline_icon_size_currencies", "Idővonal Ikon Méret (Pénznemek)"}
            {"timeline_icon_size_currencies_tooltip", "Méret of currency icons in Idővonal tab (16-48)"}
            {"timeline_icon_size_items", "Idővonal Ikon Méret (Tárgyak)"}
            {"timeline_icon_size_items_tooltip", "Méret of item icons in Idővonal tab (16-96)"}
            {"toggle_favorite", "Toggle favorite"}
            {"toggle_favorite_tooltip", "Toggle favorite"}
            {"toggle_ignore", "Toggle ignore"}
            {"toggle_ignore_tooltip", "Toggle ignore"}
            {"top_currencies_count_header", "Top 5 Pénznemek (Szám)"}
            {"top_currencies_count_tooltip", "Top 5 currencies by count"}
            {"top_drops", "Top Drops"}
            {"top_gradient_color", "Top"}
            {"top_gradient_color_tooltip", "Top gradient color"}
            {"top_items_count_header", "Top 5 Tárgyak (Szám)"}
            {"top_items_profit_header", "Top 5 Tárgyak by Profit"}
            {"total_custom_profit", "Összesen custom profit"}
            {"total_custom_profit_tooltip", "Összesen custom profit"}
            {"total_drops", "Összesen Drops"}
            {"total_duration", "Összesen Időtartam"}
            {"total_tp_sell_profit", "Összesen TP sell profit"}
            {"total_tp_sell_profit_tooltip", "Összesen TP sell profit"}
            {"total_tracked_currencies", "Összesen number of tracked currencies"}
            {"total_tracked_currencies_tooltip", "Összesen number of tracked currencies"}
            {"total_tracked_items", "Összesen number of tracked items"}
            {"total_tracked_items_tooltip", "Összesen number of tracked items"}
            {"total_vendor_sell_profit", "Összesen vendor sell profit"}
            {"total_vendor_sell_profit_tooltip", "Összesen vendor sell profit"}
            {"tp_buy_gross_format", "TP Buy (Gross): %s"}
            {"tp_buy_net_format", "TP Buy (Net): %s"}
            {"tp_sell_gross_format", "TP Sell (Gross): %s"}
            {"tp_sell_net_format", "TP Sell (Net): %s"}
            {"trigger_drops", "Item Drops"}
            {"trigger_profit_goal", "Profit Goal"}
            {"trigger_time_reset", "Idő & Visszaállítás"}
            {"type_backpack", "Backpack"}
            {"type_gizmo_container", "Gizmo Container"}
            {"type_tool", "Tool"}
            {"type_trophy", "Trophy"}
            {"type_unlock", "Feloldás"}
            {"type_upgrade_component", "Upgrade Component"}
            {"unknown_map", "Unknown"}
            {"update_profile", "Frissítés Profile"}
            {"update_profile_tooltip", "Frissítés current profile with current settings"}
            {"value", "Érték"}
            {"vendor_value_format", "Vendor Érték: %s"}
            {"visual_settings", "Visual Beállítások"}
            {"warning_minutes", "Figyelmeztetés minutes"}
            {"warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"yes_clear", "Igen, Törlés"}
            {"yes_enable", "Igen, Engedélyezés"}
            {"yes_reset", "Igen, Visszaállítás"},

            // Drops Tab
            {"settings_tab", "Beállítások"},
        
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
