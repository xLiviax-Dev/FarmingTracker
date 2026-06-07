// ---------------------------------------------------------------------------
// localization_no.cpp – Norwegian translations for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>

namespace Localization
{
    const std::unordered_map<std::string, const char*> GetNorwegianTranslations()
    {
        static const std::unordered_map<std::string, const char*> translations = {
            // Status texts
            {"status_disconnected", "Koblet fra"},
            {"status_connecting", "Kobler til..."},
            {"status_connected", "Koblet til"},
            {"status_auth_failed", "Autentisering mislyktes – sjekk token"},
            {"status_reconnecting", "Kobler til på nytt..."},
            {"status_error", "Feil"},
            {"status_unknown", "Ukjent"},

            // Mini Window
            {"mini_window_title", "Farming Tracker Mini"},
            {"profit", "Fortjeneste"},
            {"profit_per_hour", "Fortjeneste/time"},
            {"tp_sell", "TP Salg"},
            {"tp_instant", "TP Umiddelbart"},
            {"total_items", "Totalt antall gjenstander"},
            {"session", "Økt"},

            // Main Window
            {"main_window_title", "Farming Tracker"},
            {"drf_label", "DRF"},
            {"gw2_api_label", "GW2 API"},
            {"session_time_label", "Økttid"},
            {"reset_button", "Tilbakestill"},
            {"reset_tooltip", "Tilbakestill alle farming-tellere (manuell tilbakestilling)"},

            // Tabs
            {"tab_summary", "Dashboard"},
            {"tab_drops", "Drops"},
            {"tab_loot_filter", "Loot Filter"},
            {"tab_items", "Gjenstander"},
            {"tab_currencies", "Valutaer"},
            {"tab_dashboard", "Kontrollpanel"},
            {"tab_favorites", "Favoritter"},
            {"tab_ignored", "Ignorert"},
            {"tab_timeline", "Tidslinje"},
            {"timeline_profit_hour_listings", "Omtrentlige handelsfortjenester per time (Listinger)"},
            {"timeline_profit_hour_instant", "Omtrentlige handelsfortjenester per time (Umiddelbart salg)"},
            {"timeline_no_drops", "Ingen drops registrert i denne økten ennå."},
            {"timeline_item_drops", "Gjenstand-drops"},
            {"timeline_currencies", "Valutaer"},
            {"tab_filter", "Filter"},
            {"tab_custom_profit", "Tilpasset fortjeneste"},
            {"tab_debug", "Feilsøk"},

            // Summary Tab
            {"warning_drf_not_connected", "⚠️ DRF ikke koblet til"},
            {"warning_drf_not_connected_desc", "Denne plugin krever DRF for dataoverføring."},
            {"warning_drf_install", "Installer DRF via Nexus Addon Manager eller https://drf.rs/"},
            {"warning_drf_token_invalid", "⚠️ DRF Token ugyldig"},
            {"warning_drf_token_invalid_desc", "Vennligst sjekk DRF Token i Innstillinger."},
            {"warning_gw2_api_key_not_set", "⚠️ GW2 API-nøkkel ikke satt"},
            {"warning_gw2_api_key_not_set_desc", "Vennligst sett GW2 API-nøkkel i Innstillinger for gjenstandsdetaljer."},
            {"gold", "Gull"},
            {"silver", "Sølv"},
            {"copper", "Kobber"},
            {"total_profit", "Total fortjeneste"},
            {"total_profit_tooltip", "Total tilpasset fortjeneste fra alle gjenstander"},
            {"total_items_count", "Totalt antall gjenstander"},
            {"total_items_tooltip", "Totalt antall unike gjenstander sporet"},
            {"total_currencies", "Totalt antall valutaer"},
            {"total_currencies_tooltip", "Totalt antall unike valutaer sporet"},
            {"profit_per_hour_label", "Fortjeneste per time"},
            {"profit_per_hour_tooltip", "Fortjeneste per time basert på øktvarighet"},
            {"magic_find", "Magic Find"},
            {"magic_find_tooltip", "Nåværende eller sist registrert Magic Find fra DRF"},
            {"session_duration", "Øktvarighet"},
            {"session_duration_tooltip", "Nåværende farming-øktvarighet"},
            {"date_tooltip", "Øktstarttid"},
            {"duration_tooltip", "Øktvarighet"},
            {"profit_tooltip", "Total øktfortjeneste"},
            {"profit_per_hour_tooltip", "Fortjeneste per time"},
            {"drops_tooltip", "Antall drops"},
            {"best_drop_tooltip", "Mest verdifulle drop i økten"},
            {"top_items_profit", "Topp gjenstander (Fortjeneste)"},
            {"top_items_profit_tooltip", "Topp 5 gjenstander etter fortjenesteverdi"},
            {"loading", "Laster..."},
            {"coin", "Mynt"},
            {"top_items_count", "Topp gjenstander (Antall)"},
            {"top_items_count_tooltip", "Topp 5 gjenstander etter antall"},
            {"top_currencies", "Topp valutaer"},
            {"top_currencies_tooltip", "Topp 5 valutaer etter antall"},
            {"quick_statistics", "Hurtig statistikk"},
            {"quick_statistics_tooltip", "Oversikt over farming-statistikk"},
            {"average_item_value", "Gjennomsnittlig gjenstandsverdi"},
            {"average_item_value_na", "Ikke tilgjengelig"},
            {"total_unique_items", "Totalt antall unike gjenstander"},
            {"warning_no_data", "⚠️ Ingen data lastet"},
            {"warning_no_data_desc", "Venter på DRF-data..."},
            {"export", "Eksport"},
            {"export_tooltip", "Eksporter farming-data til fil"},
            {"export_json", "Eksporter som JSON"},
            {"export_csv", "Eksporter som CSV"},
            {"import_json", "Importer fra JSON"},

            // Items Tab
            {"search_hint", "Søk gjenstander..."},
            {"clear", "Slett"},
            {"sort_count_high", "Sorter: |Antall| høy"},
            {"sort_count_low", "Sorter: |Antall| lav"},
            {"sort_id_up", "Sorter: Gjenstand-ID opp"},
            {"sort_id_down", "Sorter: Gjenstand-ID ned"},
            {"sort_name_az", "Sorter: Navn A–Å"},
            {"sort_tooltip", "Sorter gjenstander etter antall, ID eller navn"},
            {"rarity_all", "Sjeldenhet: alle"},
            {"rarity_basic", "Sjeldenhet: Basic+"},
            {"rarity_fine", "Sjeldenhet: Fine+"},
            {"rarity_masterwork", "Sjeldenhet: Masterwork+"},
            {"rarity_rare", "Sjeldenhet: Rare+"},
            {"rarity_exotic", "Sjeldenhet: Exotic+"},
            {"rarity_ascended", "Sjeldenhet: Ascended+"},
            {"rarity_legendary", "Sjeldenhet: Kun Legendary"},
            {"rarity_tooltip", "Filtrer gjenstander etter minimum sjeldenhet"},
            {"rarity", "Sjeldenhet"},
            {"type", "Type"},
            {"vendor_value", "Selgerverdi"},
            {"tp_buy_net", "TP Kjøp (Netto)"},
            {"account_bound", "Kontobundet"},
            {"yes", "Ja"},
            {"no", "Nei"},
            {"nosell", "NoSell"},
            {"favorite", "Favoritt"},
            {"ignore", "Ignorer"},

            // Currencies Tab
            {"search_currencies_hint", "Søk valutaer..."},
            {"api_id", "API-ID"},
            {"currency_name", "Valutanavn"},
            {"count", "Antall"},

            // Favorites Tab
            {"unfavorite_item", "Avfavoritt"},
            {"unfavorite_selected", "Avfavoritt valgte"},
            {"no_favorites_yet", "Ingen favoritter ennå. Høyreklikk på en gjenstand for å legge den til."},
            {"toggle_favorite_tooltip", "Veksle favoritt"},
            {"profits", "Fortjenester"},
            {"profits_tooltip", "Total fortjeneste fra farming"},
            {"approx_profits", "Omtrentlige fortjenester"},
            {"approx_gold_per_hour", "Omtrentlig gull per time"},
            {"trading_profits", "Handelsfortjenester"},
            {"trading_profits_tooltip", "Fortjeneste fra Trading Post"},

            // Profit Tab
            {"profits", "Fortjenester"},
            {"profits_tooltip", "Total fortjeneste fra farming"},
            {"approx_profits", "Omtrentlige fortjenester"},
            {"approx_gold_per_hour", "Omtrentlig gull per time"},
            {"trading_profits", "Handelsfortjenester"},
            {"trading_profits_tooltip", "Fortjeneste fra Trading Post"},
            {"approx_trading_profits_listings", "Omtrentlige handelsfortjenester (Listinger)"},
            {"approx_trading_profits_instant", "Omtrentlige handelsfortjenester (Umiddelbart salg)"},
            {"trading_details", "Handelsdetaljer (Mulighetskostnad)"},
            {"trading_details_tooltip", "Mulighetskostnad ved å bruke gjenstander i stedet for å selge"},
            {"lost_profit_vs_tp_sell", "Tapt fortjeneste (vs TP Salg)"},
            {"lost_profit_per_hour_vs_tp_sell", "Tapt fortjeneste per time (vs TP Salg)"},
            {"efficiency_score", "Effektivitetspoeng"},
            {"efficiency_score_label", "Effektivitetspoeng:"},
            {"efficiency_score_tooltip", "Hvor mye av maksimal mulig fortjeneste du oppnådde (Umiddelbart salg vs. TP Listinger)."},
            {"efficiency_score_desc", "Du oppnådde %.1f%% av maksimal fortjeneste!"},
            {"session_duration_label", "Øktvarighet"},
            {"session_duration_tooltip", "Nåværende farming-øktvarighet"},

            // Filter Tab
            {"sell_method_filters", "Salgsmetode-filter"},
            {"sellable_to_vendor", "Selgbar til selger"},
            {"sellable_to_vendor_tooltip", "Vis gjenstander selgbare til selger"},
            {"sellable_on_tp", "Selgbar på TP"},
            {"sellable_on_tp_tooltip", "Vis gjenstander selgbare på Trading Post"},
            {"has_custom_profit", "Har tilpasset fortjeneste"},
            {"has_custom_profit_tooltip", "Vis gjenstander med tilpassede fortjenesteverdier"},
            {"api_knowledge_filters", "API-kunnskaps-filter"},
            {"known_by_api", "Kjent av API"},
            {"known_by_api_tooltip", "Vis gjenstander kjent av GW2 API"},
            {"unknown_by_api", "Ukjent av API"},
            {"unknown_by_api_tooltip", "Vis gjenstander ikke kjent av GW2 API"},
            {"item_type_filters", "Gjenstandstype-filter"},
            {"type_armor", "Rustning"},
            {"type_armor_tooltip", "Vis rustningsgjenstander"},
            {"type_weapon", "Våpen"},
            {"type_weapon_tooltip", "Vis våpengjenstander"},
            {"type_trinket", "Smykke"},
            {"type_trinket_tooltip", "Vis smykkergjenstander"},
            {"type_gizmo", "Gizmo"},
            {"type_gizmo_tooltip", "Vis gizmo-gjenstander"},
            {"type_crafting_material", "Håndverksmateriale"},
            {"type_crafting_material_tooltip", "Vis håndverksmaterialer"},
            {"type_consumable", "Forbruksvare"},
            {"type_consumable_tooltip", "Vis forbruksvarer"},
            {"type_gathering_tool", "Samle-redskap"},
            {"type_gathering_tool_tooltip", "Vis samle-redskaper"},
            {"type_bag", "Pose"},
            {"type_bag_tooltip", "Vis poser"},
            {"type_container", "Beholder"},
            {"type_container_tooltip", "Vis beholdere"},
            {"type_mini_pet", "Minidyr"},
            {"type_mini_pet_tooltip", "Vis minidyr"},
            {"currency_filters_label", "Valutafiltre"},
            {"currency_general", "Generelt"},
            {"currency_main", "Hovedvalutaer"},
            {"currency_fractal", "Fractal/Raid/Dungeon-valutaer"},
            {"currency_wvw_pvp", "WvW/PvP-valutaer"},
            {"currency_map", "Kartspesifikke valutaer"},
            {"filter_karma", "Karma"},
            {"currency_karma_tooltip", "Vis karma-valuta"},
            {"currency_laurel", "Laurbær"},
            {"currency_laurel_tooltip", "Vis laurbær-valuta"},
            {"currency_gem", "Edelstein"},
            {"currency_gem_tooltip", "Vis edelstein-valuta"},
            {"currency_fractal_relic", "Fractal-relikvie"},
            {"currency_fractal_relic_tooltip", "Vis fractal-relikvie-valuta"},
            {"currency_badge_of_honor", "Æresmedalje"},
            {"currency_badge_of_honor_tooltip", "Vis æresmedalje-valuta"},
            {"currency_guild_commendation", "Gilleanbefaling"},
            {"currency_guild_commendation_tooltip", "Vis gilleanbefaling-valuta"},
            {"currency_transmutation_charge", "Transmutationslad"},
            {"currency_transmutation_charge_tooltip", "Vis transmutationslad-valuta"},
            {"currency_spirit_shards", "Åndesplitter"},
            {"currency_spirit_shards_tooltip", "Vis åndesplitter-valuta"},
            {"currency_unbound_magic", "Ubundet magi"},
            {"currency_unbound_magic_tooltip", "Vis ubundet magi-valuta"},
            {"currency_volatile_magic", "Flyktig magi"},
            {"currency_volatile_magic_tooltip", "Vis flyktig magi-valuta"},
            {"currency_airship_parts", "Luftskip-deler"},
            {"currency_airship_parts_tooltip", "Vis luftskip-deler-valuta"},
            {"currency_geode", "Geode"},
            {"currency_geode_tooltip", "Vis geode-valuta"},
            {"currency_ley_line_crystals", "Ley-linjekrystaller"},
            {"currency_ley_line_crystals_tooltip", "Vis ley-linjekrystaller-valuta"},
            {"currency_trade_contracts", "Handelskontrakter"},
            {"currency_trade_contracts_tooltip", "Vis handelskontrakter-valuta"},
            {"currency_elegy_mosaic", "Elegy-mosaikk"},
            {"currency_elegy_mosaic_tooltip", "Vis elegy-mosaikk-valuta"},
            {"currency_uncommon_coins", "Uvanlige mynter"},
            {"currency_uncommon_coins_tooltip", "Vis uvanlige mynter-valuta"},
            {"currency_astral_acclaim", "Astral-acclaim"},
            {"currency_astral_acclaim_tooltip", "Vis astral-acclaim-valuta"},
            {"currency_pristine_fractal_relics", "Pristine fractal-relikvier"},
            {"currency_pristine_fractal_relics_tooltip", "Vis pristine fractal-relikvier-valuta"},
            {"currency_unstable_fractal_essence", "Ustabil fractal-essens"},
            {"currency_unstable_fractal_essence_tooltip", "Vis ustabil fractal-essens-valuta"},
            {"currency_magnetite_shards", "Magnetittsplitter"},
            {"currency_magnetite_shards_tooltip", "Vis magnetittsplitter-valuta"},
            {"currency_gaeting_crystals", "Gaeting-krystaller"},
            {"currency_gaeting_crystals_tooltip", "Vis gaeting-krystaller-valuta"},
            {"currency_prophet_shards", "Profetsplitter"},
            {"currency_prophet_shards_tooltip", "Vis profetsplitter-valuta"},
            {"currency_green_prophet_shards", "Grønne profetsplitter"},
            {"currency_green_prophet_shards_tooltip", "Vis grønne profetsplitter-valuta"},
            {"currency_wvw_skirmish_tickets", "WvW-skirmish-billetter"},
            {"currency_wvw_skirmish_tickets_tooltip", "Vis WvW-skirmish-billetter-valuta"},
            {"currency_proofs_of_heroics", "Bevis på heltemod"},
            {"currency_proofs_of_heroics_tooltip", "Vis bevis på heltemod-valuta"},
            {"currency_pvp_league_tickets", "PvP-ligabilletter"},
            {"currency_pvp_league_tickets_tooltip", "Vis PvP-ligabilletter-valuta"},
            {"currency_ascended_shards_of_glory", "Ascended-glory-splitter"},
            {"currency_ascended_shards_of_glory_tooltip", "Vis ascended-glory-splitter-valuta"},
            {"currency_research_notes", "Forskningsnotater"},
            {"currency_research_notes_tooltip", "Vis forskningsnotater-valuta"},
            {"currency_tyrian_defense_seal", "Tyrian-forsvarssegl"},
            {"currency_tyrian_defense_seal_tooltip", "Vis tyrian-forsvarssegl-valuta"},
            {"currency_testimony_of_desert_heroics", "Vitnesbyrd om økenheltemod"},
            {"currency_testimony_of_desert_heroics_tooltip", "Vis vitnesbyrd om økenheltemod-valuta"},
            {"currency_testimony_of_jade_heroics", "Vitnesbyrd om jadeheltemod"},
            {"currency_testimony_of_jade_heroics_tooltip", "Vis vitnesbyrd om jadeheltemod-valuta"},
            {"currency_testimony_of_castoran_heroics", "Vitnesbyrd om castoranheltemod"},
            {"currency_testimony_of_castoran_heroics_tooltip", "Vis vitnesbyrd om castoranheltemod-valuta"},
            {"currency_legendary_insight", "Legendarisk innsikt"},
            {"currency_legendary_insight_tooltip", "Vis legendarisk innsikt-valuta"},
            {"currency_tales_of_dungeon_delving", "Fortellinger om dungeon-utforskning"},
            {"currency_tales_of_dungeon_delving_tooltip", "Vis fortellinger om dungeon-utforskning-valuta"},
            {"currency_imperial_favor", "Imperial gunst"},
            {"currency_imperial_favor_tooltip", "Vis imperial gunst-valuta"},
            {"currency_canach_coins", "Canach-mynter"},
            {"currency_canach_coins_tooltip", "Vis canach-mynter-valuta"},
            {"currency_ancient_coin", "Antikk mynt"},
            {"currency_ancient_coin_tooltip", "Vis antikk mynt-valuta"},
            {"currency_unusual_coin", "Uvanlig mynt"},
            {"currency_unusual_coin_tooltip", "Vis uvanlig mynt-valuta"},
            {"currency_jade_sliver", "Jade-splint"},
            {"currency_jade_sliver_tooltip", "Vis jade-splint-valuta"},
            {"currency_static_charge", "Statisk ladning"},
            {"currency_static_charge_tooltip", "Vis statisk ladning-valuta"},
            {"currency_pinch_of_stardust", "Nyp stjernestøv"},
            {"currency_pinch_of_stardust_tooltip", "Vis nyp stjernestøv-valuta"},
            {"currency_calcified_gasp", "Kalkifisert gisp"},
            {"currency_calcified_gasp_tooltip", "Vis kalkifisert gisp-valuta"},
            {"currency_ursus_oblige", "Ursus Oblige"},
            {"currency_ursus_oblige_tooltip", "Vis ursus oblige-valuta"},
            {"currency_gaeting_crystal_janthir", "Gaeting-krystall (Janthir)"},
            {"currency_gaeting_crystal_janthir_tooltip", "Vis gaeting-krystall (janthir)-valuta"},
            {"currency_antiquated_ducat", "Antik ducat"},
            {"currency_antiquated_ducat_tooltip", "Vis antik ducat-valuta"},
            {"currency_aether_rich_sap", "Aether-rik sevje"},
            {"currency_aether_rich_sap_tooltip", "Vis aether-rik sevje-valuta"},

            // Additional Filters
            {"additional_filters", "Ytterligere filter"},
            {"account_bound", "Kontobundet"},
            {"account_bound_tooltip", "Vis kontobundne gjenstander"},
            {"not_account_bound", "Ikke kontobundet"},
            {"not_account_bound_tooltip", "Vis ikke-kontobundne gjenstander"},
            {"nosell_items", "NoSell"},
            {"nosell_items_tooltip", "Vis NoSell-gjenstander"},
            {"not_nosell", "Ikke NoSell"},
            {"not_nosell_tooltip", "Vis selgbare gjenstander"},
            {"favorite_items", "Favoritt"},
            {"favorite_items_tooltip", "Vis favorittgjenstander"},
            {"not_favorite", "Ikke favoritt"},
            {"not_favorite_tooltip", "Vis ikke-favorittgjenstander"},
            {"ignored_items", "Ignorert"},
            {"ignored_items_tooltip", "Vis ignorerte gjenstander"},
            {"not_ignored", "Ikke ignorert"},
            {"not_ignored_tooltip", "Vis ikke-ignorerte gjenstander"},

            // Range Filters
            {"range_filters", "Områdefilter"},
            {"show_range_filters", "Vis områdefilter"},
            {"filter_min_price", "Filter min. pris"},
            {"filter_max_price", "Filter maks. pris"},
            {"filter_min_quantity", "Filter min. mengde"},
            {"filter_max_quantity", "Filter maks. mengde"},

            // Mini Window Settings
            {"mini_window_settings", "Minivindu"},
            {"show_profit", "Vis fortjeneste"},
            {"show_profit_tooltip", "Vis total fortjeneste i minivinduet"},
            {"show_profit_per_hour", "Vis fortjeneste/time"},
            {"show_profit_per_hour_tooltip", "Vis fortjeneste per time i minivinduet"},
            {"show_tp_sell", "Vis TP Salg (Listinger)"},
            {"show_tp_sell_tooltip", "Vis TP-salgfortjeneste (listinger) i minivinduet"},
            {"show_tp_instant", "Vis TP Umiddelbart (Umiddelbart salg)"},
            {"show_tp_instant_tooltip", "Vis TP umiddelbar salgfortjeneste i minivinduet"},
            {"show_total_items", "Vis totalt antall gjenstander"},
            {"show_total_items_tooltip", "Vis totalt antall gjenstander i minivinduet"},
            {"show_session_duration", "Vis øktvarighet"},
            {"show_session_duration_tooltip", "Vis øktvarighet i minivinduet"},
            {"window_click_through", "Klikk gjennom vindu"},
            {"window_click_through_tooltip", "Tillater klikk gjennom minivinduet til spillet"},

            // Main Window Settings
            {"main_window", "Hovedvindu"},
            {"click_through", "Klikk gjennom"},
            {"click_through_tooltip", "Tillater klikk gjennom hovedvinduet til spillet"},

            // Advanced UI Settings
            {"advanced_ui_settings", "Avanserte UI-innstillinger"},
            {"no_advanced_ui_settings", "(Ingen avanserte UI-innstillinger tilgjengelig)"},

            // Display Settings
            {"display_settings", "Visningsinnstillinger"},
            {"show_item_icons", "Vis gjenstand-ikoner"},
            {"show_item_icons_tooltip", "Vis gjenstand-ikoner i listen"},
            {"show_rarity_borders", "Vis sjeldenhetskanter"},
            {"show_rarity_borders_tooltip", "Viser fargede kanter rundt ikoner basert på sjeldenhet"},
            {"enable_grid_view", "Aktiver rutenettvisning"},
            {"enable_grid_view_tooltip", "Vis gjenstander i rutenett-layout i stedet for liste"},
            {"grid_icon_size", "Rutenett-ikonstørrelse"},
            {"grid_icon_size_tooltip", "Størrelse på ikoner i rutenettvisning"},

            // Count Display Settings
            {"count_display_settings", "Antall-visningsinnstillinger"},
            {"count_text_color", "Antall-tekstfarge"},
            {"count_text_color_tooltip", "Farge på antall-tekst"},
            {"count_background_color", "Antall-bakgrunnsfarge"},
            {"count_background_color_tooltip", "Farge på antall-bakgrunn"},
            {"count_font_size", "Antall-skriftstørrelse"},
            {"count_font_size_tooltip", "Størrelse på antall-skrift"},
            {"count_horizontal_alignment", "Antall-horisontal justering"},
            {"count_horizontal_alignment_tooltip", "Horisontal justering av antall-tekst"},

            // Gradient Background Settings
            {"gradient_background_settings", "Gradient-bakgrunnsinnstillinger"},
            {"enable_gradient_backgrounds", "Aktiver gradient-bakgrunn"},
            {"enable_gradient_backgrounds_tooltip", "Aktiver gradient-bakgrunn for vinduer"},
            {"gradient_top_color", "Gradient-toppfarge"},
            {"gradient_top_color_tooltip", "Toppfarge på gradient-bakgrunn"},
            {"gradient_bottom_color", "Gradient-bunnfarge"},
            {"gradient_bottom_color_tooltip", "Bunnfarge på gradient-bakgrunn"},

            // Custom Profit System
            {"custom_profit_system", "Tilpasset fortjenestesystem"},
            {"enable_custom_profit", "Aktiver tilpasset fortjeneste"},
            {"enable_custom_profit_tooltip", "Aktiver tilpassede fortjenesteverdier for gjenstander"},

            // Search
            {"search_settings", "Søk"},
            {"enable_search", "Aktiver søk"},
            {"enable_search_tooltip", "Aktiver søkefunksjonalitet"},

            // Ignored Items
            {"ignored_items_settings", "Ignorerte gjenstander"},
            {"enable_ignored_items", "Aktiver ignorerte gjenstander"},
            {"enable_ignored_items_tooltip", "Aktiver funksjonalitet for ignorerte gjenstander"},

            // Auto Reset
            {"auto_reset_settings", "Automatisk tilbakestilling"},
            {"enable_auto_reset", "Aktiver automatisk tilbakestilling"},
            {"enable_auto_reset_tooltip", "Tilbakestill farming-økt automatisk etter en varighet"},
            {"auto_reset_duration", "Automatisk tilbakestillingsvarighet (minutter)"},
            {"auto_reset_duration_tooltip", "Varighet i minutter før automatisk tilbakestilling"},

            // DRF Settings
            {"drf_settings", "DRF-innstillinger"},
            {"drf_token", "DRF Token"},
            {"drf_token_label", "DRF Token:"},
            {"drf_token_tooltip", "Ditt DRF-autentiseringstoken"},
            {"edit_token", "Rediger token"},
            {"save_token", "Lagre token"},

            // GW2 API Settings
            {"gw2_api_settings", "GW2 API-innstillinger"},
            {"gw2_api_key", "GW2 API-nøkkel"},
            {"gw2_api_key_tooltip", "Din GW2 API-nøkkel for gjenstandsdetaljer"},
            {"edit_key", "Rediger nøkkel"},
            {"save_key", "Lagre nøkkel"},

            // Language Settings
            {"language_settings", "Språk"},
            {"language_tooltip", "Velg grensesnittsspråk"},
            {"language_english", "Engelsk"},
            {"language_german", "Tysk"},
            {"language_french", "Fransk"},
            {"language_spanish", "Spansk"},
            {"language_chinese", "Kinesisk"},
            {"language_czech", "Tsjekkisk"},
            {"language_italian", "Italiensk"},
            {"language_polish", "Polsk"},
            {"language_portuguese", "Portugisisk"},
            {"language_russian", "Russisk"},

            // Additional hardcoded strings found in UI
            {"farming_tracker_title", "Farming Tracker"},
            {"no_accounts_configured", "Ingen kontoer konfigurert"},
            {"no_profiles_created", "Ingen profiler opprettet ennå"},
            {"count_label", "Antall:"},
            {"profit_label", "Fortjeneste:"},
            {"no_profit", "Ingen fortjeneste"},
            {"vendor_value_label", "Selgerverdi:"},
            {"tp_sell_gross_label", "TP Salg (Brutto):"},
            {"tp_sell_net_label", "TP Salg (Netto):"},
            {"tp_buy_gross_label", "TP Kjøp (Brutto):"},
            {"tp_buy_net_label", "TP Kjøp (Netto):"},
            {"ignored_items_label", "Ignorerte gjenstander:"},
            {"ignored_currencies_label", "Ignorerte valutaer:"},
            {"total_items_label", "Totalt antall gjenstander:"},
            {"total_currencies_label", "Totalt antall valutaer:"},
            {"total_profit_label", "Total fortjeneste:"},
            {"tp_sell_profit_label", "TP-salgsfortjeneste:"},
            {"tp_sell_profit_tooltip", "Total fortjeneste hvis alle gjenstander ble solgt til nåværende TP-listingspriser (minus 15% avgift)"},
            {"vendor_profit_label", "Selgerfortjeneste:"},
            {"profit_per_hour_label", "Fortjeneste per time:"},
            {"opportunity_cost_profit_label", "Mulighetskostnadsfortjeneste:"},
            {"opportunity_cost_profit_per_hour_label", "Mulighetskostnadsfortjeneste/time:"},
            {"custom_profit_feature_placeholder", "Funksjon implementert - UI følger"},
            {"custom_profit_items_header", "Gjenstander med tilpasset fortjeneste"},
            {"custom_profit_currencies_header", "Valutaer med tilpasset fortjeneste"},
            {"add_custom_profit_item", "Legg til tilpasset fortjeneste for gjenstand"},
            {"add_custom_profit_currency", "Legg til tilpasset fortjeneste for valuta"},
            {"custom_profit_set_profit", "Sett fortjeneste"},
            {"custom_profit_remove", "Fjern"},
            {"custom_profit_value", "Fortjenesteverdi (Kobber)"},
            {"custom_profit_set_tooltip", "Sett tilpasset fortjenesteverdi for denne gjenstanden"},
            {"custom_profit_remove_tooltip", "Fjern tilpasset fortjenesteverdi for denne gjenstanden"},
            {"no_custom_profit_items", "(Ingen gjenstander med tilpasset fortjeneste)"},
            {"no_custom_profit_currencies", "(Ingen valutaer med tilpasset fortjeneste)"},
            {"clear_all_custom_profits", "Slett alle tilpassede fortjenester"},
            {"clear_all_custom_profits_tooltip", "Slett alle tilpassede fortjenesteverdier"},
            {"tabs_settings", "Andre faner"},
            {"tabs_description", "Vis eller skjul andre faner"},
            {"tab_settings", "Faneinnstillinger"},
            {"tab_settings_description", "Faneordning og oppførsel"},
            {"enable_dashboard_tab", "Aktiver kontrollpanel-fane"},
            {"enable_dashboard_tab_tooltip", "Vis kontrollpanel-fanen"},
            {"enable_items_tab", "Aktiver gjenstand-fane"},
            {"enable_items_tab_tooltip", "Vis gjenstand-fanen"},
            {"enable_currencies_tab", "Aktiver valuta-fane"},
            {"enable_currencies_tab_tooltip", "Vis valuta-fanen"},
            {"enable_ignored_tab", "Aktiver ignorert-fane"},
            {"enable_ignored_tab_tooltip", "Vis ignorert-gjenstand-fanen"},
            {"enable_session_history_tab", "Aktiver økt-historikk-fane"},
            {"enable_session_history_tab_tooltip", "Vis økt-historikk-fanen"},
            {"enable_timeline_tab", "Aktiver tidslinje-fane"},
            {"enable_timeline_tab_tooltip", "Vis tidslinje-fanen med detaljert drop-historikk"},
            {"enable_loot_log_tab", "Aktiver Loot Log-fane"},
            {"enable_loot_log_tab_tooltip", "Vis Loot Log-fanen"},
            {"enable_filter_tab", "Aktiver filter-fane"},
            {"enable_filter_tab_tooltip", "Vis filter-fanen"},
            {"lock_tab_order", "Lås faneordning"},
            {"lock_tab_order_tooltip", "Deaktiver omordning av faner i hovedvinduet"},
            {"enable_summaries_tab", "Aktiver sammendrag-fane"},
            {"enable_summaries_tab_tooltip", "Vis daglig/ukentlig/månedlig sammendrag-fane i økt-historikk"},
            {"custom_profit_settings", "Tilpasset fortjeneste"},
            {"total_profit_label_simple", "Total fortjeneste"},
            {"total_items_label_simple", "Totalt antall gjenstander"},
            {"total_currencies_label_simple", "Totalt antall valutaer"},
            {"profit_per_hour_label_simple", "Fortjeneste per time"},
            {"session_duration_label_simple", "Øktvarighet"},
            {"next_reset_label_simple", "Neste tilbakestilling"},
            {"export_label", "Eksport:"},
            {"quick_actions", "Hurtige handlinger:"},
            {"reset_confirm", "Er du sikker på at du vil tilbakestille alle innstillinger til standardverdier?"},
            {"reset_warning", "Denne handlingen kan ikke angres."},
            {"hotkeys", "Hurtigtaster"},
            {"mini_window_toggle_hotkey", "Minivindu-veksle-hurtigtast"},
            {"backup_restore", "Backup & Gjenopprett"},
            {"appearance_settings", "Utseende"},
            {"enable_tooltips", "Aktiver verktøytips"},
            {"enable_tooltips_tooltip", "Vis verktøytips når du svever over UI-elementer"},
            {"enable_grid_view_tooltip", "Vis elementer i rutenett i stedet for liste"},
            {"favorites_first_tooltip", "Vis favorittelementer øverst i listen"},
            {"group_by_rarity_tooltip", "Grupper elementer etter deres sjeldenhet"},
            {"show_rarity_as_tabs_tooltip", "Vis hver sjeldenhet som en egen fane"},
            {"group_by_category_tooltip", "Grupper elementer etter deres kategori"},
            {"show_group_as_tabs_tooltip", "Vis hver kategori som en egen fane"},
            {"mass_ignore_rarity_tooltip", "Ignorer alle elementer av denne sjeldenheten"},
            {"icons_borders", "Ikoner & Kanter"},
            {"colors_gradients", "Farger & Gradienter"},
            {"window_opacity", "Vindusopasitet"},
            {"windows_settings", "Vinduer"},
            {"advanced_settings", "Avansert"},
            {"export_settings", "Eksporter innstillinger til fil:"},
            {"import_settings", "Importer innstillinger fra fil:"},
            {"edit_account", "Rediger konto: %s"},
            {"account_name", "Kontonavn:"},
            {"gw2_api_key_label", "GW2 API-nøkkel:"},
            {"reload_config", "Last inn konfigurasjon på nytt:"},
            {"auto_reset_label", "Automatisk tilbakestilling:"},
            {"next_reset_utc", "Neste planlagte tilbakestilling (UTC): %s"},
            {"favorites_ui", "Favoritt-UI:"},
            {"favorites_colors", "Favoritt-farger:"},
            {"visual_enhancements", "Visuelle forbedringer:"},
            {"show_profit_sparkline", "Vis fortjeneste-linjediagram"},
            {"show_profit_sparkline_tooltip", "Vis et lite linjediagram som viser fortjeneste per time-trend"},
            {"mini_window_widget", "Minivindu (Overlay-widget):"},
            {"main_window_label", "Hovedvindu:"},
            {"profiles_description", "Profiler lar deg lagre forskjellige konfigurasjoner og bytte mellom dem raskt."},
            {"create_new_profile", "Opprett ny profil:"},
            {"current_profile", "Nåværende profil: %s"},
            {"auto_backup", "Sikkerhetskopier automatisk innstillingene dine før større endringer"},
            {"notifications", "Konfigurer in-game-varsler for viktige hendelser"},
            {"profit_goal", "Fortjenestemål:"},
            {"reset_warning_label", "Tilbakestillingsvarsel:"},
            {"session_complete", "Økt fullført:"},
            {"manage_ignored_items", "Behandle ignorerte gjenstander"},
            {"manage_ignored_currencies", "Behandle ignorerte valutaer"},
            {"rarity_label", "Sjeldenhet: %s"},
            {"type_label", "Type: %d"},
            {"account_bound_label", "Kontobundet: %s"},
            {"nosell_label", "NoSell: %s"},
            {"item_id_label", "Gjenstand-ID: %d"},
            {"currency_id_label", "Valuta-ID: %d"},
            {"context_menu_add_favorites", "Legg til favoritter"},
            {"context_menu_remove_favorites", "Fjern fra favoritter"},
            {"context_menu_ignore", "Ignorer gjenstand"},
            {"context_menu_unignore", "Fjern fra ignorerte"},
            {"context_menu_copy_name", "Kopier navn"},
            {"context_menu_copy_id", "Kopier ID"},
            {"sell_method_filters_label", "Salgsmetode-filter:"},
            {"api_knowledge_filters_label", "API-kunnskaps-filter:"},
            {"additional_filters_label", "Ytterligere filter:"},
            {"item_type_filters_label", "Gjenstandstype-filter:"},
            {"currency_filters_label", "Valuta-filter:"},
            {"price_range", "Prisområde (Kobber):"},
            {"quantity_range", "Mengdeområde:"},
            {"debug_info", "Feilsøkingsinformasjon"},
            {"drf_status", "DRF-status: %s"},
            {"drf_reconnect_count", "DRF-tilkoblingsantall: %d"},
            {"gw2_api_status", "GW2 API-status: %s"},
            {"gw2_api_reconnect_count", "GW2 API-tilkoblingsantall: %d"},
            {"session_duration_debug", "Øktvarighet"},
            {"gw2_memory", "GW2-prosessminne: %zu MB"},
            {"gw2_api_request_count", "GW2 API-forespørselsantall: %d"},
            {"ignored_items_count", "Ignorerte gjenstander: %d"},
            {"ignored_currencies_count", "Ignorerte valutaer: %d"},
            {"drf_logs", "DRF-logg:"},
            {"last_100_entries", "(Siste 100 oppføringer)"},
            {"gw2_api_logs", "GW2 API-logg:"},
            {"item_currency_details", "Gjenstand/Valuta-detaljer (Første 5):"},
            {"item_label", "Gjenstand %d: %s (Antall: "},
            {"loaded_label", ", Lastet: %s)"},
            {"currency_label", "Valuta %d: %s (Antall: "},
            {"custom_profit_items", "Gjenstander med tilpasset fortjeneste (Første 5):"},
            {"custom_profit_item", "Gjenstand %d: %s (Tilpasset fortjeneste: "},
            {"no_custom_profit_items", "(Ingen gjenstander med tilpasset fortjeneste)"},
            {"ignored_items_debug", "Ignorerte gjenstander (Første 5):"},
            {"yes_label", "Ja"},
            {"no_label", "Nei"},
            {"profits_label", "Fortjenester:"},
            {"profits_tooltip", "Total fortjeneste fra farming"},
            {"approx_profits_label", "Omtrentlige fortjenester:"},
            {"approx_profits_tooltip", "Total fortjeneste fra MAX(Selger, TP Salg med 15% avgift) eller Tilpasset fortjeneste"},
            {"approx_gold_per_hour_label", "Omtrentlig gull per time:"},
            {"approx_gold_per_hour_tooltip", "Fortjeneste per time basert på øktvarighet"},
            {"trading_profits_label", "Handelsfortjenester:"},
            {"trading_profits_tooltip", "Fortjeneste fra salg av gjenstander på Trading Post"},
            {"approx_trading_profits_listings_label", "Omtrentlige handelsfortjenester (Listinger):"},
            {"approx_trading_profits_listings_tooltip", "Total fortjeneste hvis solgt via TP-listinger (15% avgift trukket fra)"},
            {"approx_trading_profits_instant_label", "Omtrentlige handelsfortjenester (Umiddelbart salg):"},
            {"approx_trading_profits_instant_tooltip", "Total fortjeneste hvis solgt via TP umiddelbar kjøpsordre (15% avgift trukket fra)"},
            {"trading_details_label", "Handelsdetaljer (Mulighetskostnad):"},
            {"trading_details_tooltip", "Fortjeneste tapt ved ikke å selge via TP-listinger"},
            {"lost_profit_vs_tp_sell_label", "Tapt fortjeneste (vs TP Salg):"},
            {"lost_profit_vs_tp_sell_tooltip", "Mulighetskostnad: Fortjeneste tapt ved ikke å selge via TP (med 15% avgift)"},
            {"lost_profit_per_hour_vs_tp_sell_label", "Tapt fortjeneste per time (vs TP Salg):"},
            {"lost_profit_per_hour_vs_tp_sell_tooltip", "Mulighetskostnad per time"},
            {"session_duration_debug_label", "Øktvarighet: %s"},
            {"session_duration_debug_tooltip", "Nåværende farming-øktvarighet"},
            {"tab_items", "Gjenstander"},
            {"manage_ignored_items", "Behandle ignorerte gjenstander"},
            {"clear_all_ignored_items", "Slett alle ignorerte gjenstander"},
            {"unignore_item", "Avignorer gjenstand"},
            {"manage_favorite_items", "Behandle favorittgjenstander"},
            {"favorite_items_label", "Favorittgjenstander:"},
            {"clear_all_favorite_items", "Slett alle favorittgjenstander"},
            {"tab_currencies", "Valutaer"},
            {"manage_ignored_currencies", "Behandle ignorerte valutaer"},
            {"clear_all_ignored_currencies", "Slett alle ignorerte valutaer"},
            {"unignore_currency", "Avignorer valuta"},
            {"manage_favorite_currencies", "Behandle favorittvalutaer"},
            {"favorite_currencies_label", "Favorittvalutaer:"},
            {"clear_all_favorite_currencies", "Slett alle favorittvalutaer"},
            {"filter_active",   "Aktiv"},
            {"filter_inactive", "Inaktiv"},
            {"filter_all", "Alle"},
            {"filter_none", "Ingen"},
            {"filter_reset_all", "Tilbakestill Alt"},
            {"filter_search_hint", "Søk filter..."},
            {"filter_active_count", "%d filter aktive"},
            {"sell_method_filters_label", "Salgsmetode-filter:"},
            {"api_knowledge_filters_label", "API-kunnskaps-filter:"},
            {"additional_filters_label", "Ytterligere filter:"},
            {"item_type_filters_label", "Gjenstandstype-filter:"},
            {"currency_filters_label", "Valuta-filter:"},
            {"price_range", "Prisområde (Kobber):"},
            {"quantity_range", "Mengdeområde:"},
            {"debug_connection_status", "Tilkoblingsstatus"},
            {"debug_session_snapshot", "Økt-øyeblikksbilde"},
            {"debug_profit_breakdown", "Fortjenesteoppdeling"},
            {"debug_data_state", "Datastatus"},
            {"debug_logs", "Logg"},
            {"debug_favorites", "Favoritter"},
            {"debug_total_session", "total i denne økten"},
            {"debug_after_tp_fee", "etter 15% avgift"},
            {"debug_direct_sell", "direkte salg"},
            {"debug_rolling_avg", "rullende gjennomsnitt"},
            {"debug_vs_tp_sell", "vs TP salg"},
            {"debug_per_hour", "per time"},
            {"settings_api_key", "API-nøkkel"},
            {"settings_drf_token", "DRF Token"},
            {"debug_information", "Feilsøkingsinformasjon"},
            {"drf_status_label", "DRF-status: %s"},
            {"drf_status_tooltip", "Nåværende DRF-tilkoblingsstatus"},
            {"drf_reconnect_count_label", "DRF-tilkoblingsantall: %d"},
            {"drf_reconnect_count_tooltip", "Antall DRF-tilkoblingsforsøk"},
            {"gw2_api_status_label", "GW2 API-status: %s"},
            {"gw2_api_status_tooltip", "Nåværende GW2 API-tilkoblingsstatus"},
            {"gw2_api_reconnect_count_label", "GW2 API-tilkoblingsantall: %d"},
            {"gw2_api_reconnect_count_tooltip", "Antall GW2 API-tilkoblingsforsøk"},
            {"session_duration_debug", "Øktvarighet"},
            {"session_duration_debug_tooltip", "Nåværende farming-øktvarighet"},
            {"gw2_process_memory_label", "GW2-prosessminne"},
            {"gw2_process_memory_tooltip", "Nåværende GW2-prosessminnebruk"},
            {"gw2_api_request_count_label", "GW2 API-forespørselsantall"},
            {"gw2_api_request_count_tooltip", "Totalt antall GW2 API-forespørsler gjort"},
            {"ignored_items_debug_label", "Ignorerte gjenstander: %d"},
            {"ignored_items_debug_tooltip", "Antall ignorerte gjenstander"},
            {"ignored_currencies_debug_label", "Ignorerte valutaer: %d"},
            {"ignored_currencies_debug_tooltip", "Antall ignorerte valutaer"},
            {"drf_logs_label", "DRF-logg:"},
            {"clear_drf_logs", "Slett DRF-logg"},
            {"clear_drf_logs_tooltip", "Slett alle DRF-loggoppføringer"},
            {"last_100_entries", "(Siste 100 oppføringer)"},
            {"gw2_api_logs_label", "GW2 API-logg:"},
            {"clear_gw2_logs", "Slett GW2-logg"},
            {"clear_gw2_logs_tooltip", "Slett alle GW2 API-loggoppføringer"},
            {"settings_label", "Innstillinger:"},
            {"api_key_tooltip", "GW2 API-nøkkelstatus"},
            {"not_set", "Ikke satt"},
            {"set", "Satt"},
            {"drf_token_tooltip", "DRF Token-status"},
            {"toggle_hotkey_label", "Veksle-hurtigtast: %s"},
            {"toggle_hotkey_tooltip", "Hovedvindu-veksle-hurtigtast"},
            {"auto_reset_mode_label", "Automatisk tilbakestillingsmodus: %d"},
            {"auto_reset_mode_tooltip", "Nåværende automatisk tilbakestillingsmodus"},
            {"next_reset_label", "Neste tilbakestilling: %s"},
            {"next_reset_tooltip", "Neste planlagte tilbakestillingstid (UTC)"},
            {"fake_drf_server_label", "Falsk DRF-server:"},
            {"use_fake_drf_server", "Bruk falsk DRF-server"},
            {"use_fake_drf_server_tooltip", "Kun for testformål"},
            {"reset_all_data", "Tilbakestill Alle Data"},
            {"reset_all_data_tooltip", "Tilbakestill all farming-data"},
            {"coin", "Mynt"},
            {"info_button", "Info"},
            {"info_title", "FarmingTracker Info"},
            {"info_text", "Hjelpetekst vil bli lagt til her senere..."},
            {"close_button", "Lukk"},
            {"rarity_label", "Sjeldenhet: %s"},
            {"type_label", "Type: %d"},
            {"account_bound_label", "Kontobundet: %s"},
            {"nosell_label", "NoSell: %s"},
            {"yes_label", "Ja"},
            {"no_label", "Nei"},
            {"sort_price_down", "Sorter: Gjenstandspris ned"},
            {"sort_price_up", "Sorter: Gjenstandspris opp"},
            {"sort_count_high", "Sorter: |Antall| høy"},
            {"sort_count_low", "Sorter: |Antall| lav"},
            {"sort_name_az", "Sorter: Navn A–Å"},
            {"sort_name_za", "Sorter: Navn Å–A"},
            {"last_reset_label", "Tilbakestilling"},
            {"last_reset_tooltip", "Tid siden siste tilbakestilling"},
            {"custom_profit_edit_tooltip",    "Rediger fortjenesteverdi"},
            {"custom_profit_confirm_tooltip", "Lagre endringer"},
            {"accent_color", "Accent Farge (Buttons, Fanes, UI)"}
            {"accent_color_tooltip", "Accent color for buttons, tabs, and UI elements"}
            {"account_management", "Konto Management"}
            {"account_prefix", "Konto"}
            {"actions", "Actions"}
            {"add_account", "+ Legg til Konto"}
            {"api_key_invalid_format", "(Invalid Format: 9 Blocks required)"}
            {"auto_reset_custom_days", "Egendefinert (days)"}
            {"auto_reset_daily", "Daily reset (00:00 UTC)"}
            {"auto_reset_done_msg", "The tracker has been reset."}
            {"auto_reset_done_title", "Tilbakestill Complete"}
            {"auto_reset_minutes_unload", "Minutes after last unload"}
            {"auto_reset_never", "Never (manual Tilbakestill only)"}
            {"auto_reset_on_load", "På addon load"}
            {"auto_reset_tooltip", "When to automatically reset farming counters"}
            {"auto_reset_weekly", "Weekly (Mon 07:30 UTC)"}
            {"auto_reset_weekly_eu_wvw", "Weekly EU WvW (Fri 18:00 UTC)"}
            {"auto_reset_weekly_map_bonus", "Weekly map bonus (Thu 20:00 UTC)"}
            {"auto_reset_weekly_na_wvw", "Weekly NA WvW (Sat 02:00 UTC)"}
            {"automatic_backups", "Automatic Sikkerhetskopis"}
            {"backup", "Sikkerhetskopi"}
            {"backup_daily", "Daily"}
            {"backup_frequency", "Sikkerhetskopi frequency"}
            {"backup_frequency_tooltip", "How often to create automatic backups"}
            {"backup_manual_only", "Manuell only"}
            {"backup_weekly", "Weekly"}
            {"best_drop", "Beste Drop"}
            {"border_size", "Kant Størrelse"}
            {"border_size_tooltip", "Adjust the thickness of rarity borders (1.0 - 10.0)"}
            {"bottom_gradient_color", "Bottom"}
            {"bottom_gradient_color_tooltip", "Bottom gradient color"}
            {"browse_for_file", "Browse for file..."}
            {"cancel", "Avbryt"}
            {"clear_all_custom_profits_warning", "Alle custom profit values will be deleted. This action cannot be undone."}
            {"clear_compare_selection", "Tøm selection"}
            {"clear_history", "Tøm Historikk"}
            {"clear_history_confirm", "Tøm all session history?"}
            {"clear_history_tooltip", "Slett all saved session history"}
            {"clear_history_warning", "This action cannot be undone!"}
            {"clear_search", "Tøm"}
            {"clear_search_favorites", "Tøm"}
            {"clear_search_tooltip", "Tøms the current search"}
            {"column_count", "Antall"}
            {"column_currency", "Currency"}
            {"column_favorite", "Favorite"}
            {"column_icon", "Ikon"}
            {"column_ignore", "Ignore"}
            {"column_item", "Item"}
            {"column_label", "Etikett"}
            {"column_name", "Name"}
            {"column_profit", "Profit"}
            {"column_value", "Verdi"}
            {"comparison_previous_period", "Comparison with previous period:"}
            {"count_format", "Antall: %lld"}
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
            {"currency_table_favorite_tooltip", "Legg til/remove favorite. Favorites appear in the Favorites tab. Tip: Right-click the icon/name for more actions."}
            {"currency_table_ignore_tooltip", "Legg til/remove ignored. Ignored currencies appear in the Ignored tab. Tip: Right-click the icon/name for more actions."}
            {"date", "Dato"}
            {"debug_settings", "Feilsøking Innstillinger"}
            {"default_no_profile", "Standard (Nei Profile)"}
            {"delete_profile", "Slett Profile"}
            {"delete_profile_tooltip", "Slett current profile"}
            {"details", "Details"}
            {"drops", "Drops"}
            {"duration", "Varighet"}
            {"enable_automatic_backups", "Aktiver automatic backups"}
            {"enable_automatic_backups_tooltip", "Automatically create backups before changes"}
            {"enable_best_drop_highlight", "Highlight Beste Drop"}
            {"enable_best_drop_highlight_tooltip", "Highlight the most valuable drop with a golden border in the Gjenstander tab"}
            {"enable_best_drop_in_mini_window", "Vis Beste Drop in Mini Vindu"}
            {"enable_best_drop_in_mini_window_tooltip", "Vis the most valuable drop in the mini window overlay"}
            {"enable_debug_tab", "Aktiver Feilsøking Fane"}
            {"enable_debug_tab_tooltip", "Viss the debug tab with additional information"}
            {"enable_favorite_row_color", "Aktiver favorite row color"}
            {"enable_favorite_row_color_tooltip", "Highlights favorite items/currencies with custom row background color"}
            {"enable_favorite_text_color", "Aktiver favorite text color"}
            {"enable_favorite_text_color_tooltip", "Highlights favorite items/currencies with custom text color"}
            {"enable_favorites", "Aktiver Favorites"}
            {"enable_favorites_tab", "Aktiver Favorites Fane"}
            {"enable_favorites_tab_tooltip", "Viss a separate favorites tab"}
            {"enable_grid_view_currencies", "Aktiver Grid View (Valutaer)"}
            {"enable_grid_view_currencies_tooltip", "Toggle between list and grid view in Valutaer tab"}
            {"enable_grid_view_items", "Aktiver Grid View (Gjenstander)"}
            {"enable_grid_view_items_tooltip", "Toggle between list and grid view in Gjenstander tab"}
            {"enable_icon_cache", "Aktiver Ikon Cache"}
            {"enable_icon_cache_tooltip", "Cache item icons on disk to speed up loading after the first session"}
            {"enable_notifications", "Aktiver notifications"}
            {"enable_notifications_tooltip", "Aktiver in-game notifications"}
            {"enable_session_history", "Aktiver Økt Historikk"}
            {"enable_session_history_tooltip", "Lagre farming session history for later viewing"}
            {"enable_session_timeline", "Aktiver Økt Tidslinje"}
            {"enable_session_timeline_tooltip", "Lagre detailed drop timeline with timestamps for session details"}
            {"export_history", "Eksporter Historikk"}
            {"export_history_tooltip", "Eksporter session history to a JSON file"}
            {"export_logs", "Eksporter Logs"}
            {"favorite_items_header", "Favorite Gjenstander"}
            {"favorites_first", "Favorites First"}
            {"favorites_first_tooltip", "Viss favorites first in item/currency lists"}
            {"favorites_settings", "Favorites Innstillinger"}
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
            {"filter_items", "Filter Gjenstander"}
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
            {"filter_nosell", "NeiSell"}
            {"filter_nosell_tooltip", "Vis NeiSell items"}
            {"filter_not_account_bound", "Neit Konto-bound"}
            {"filter_not_account_bound_tooltip", "Vis non-account-bound items"}
            {"filter_not_favorite", "Neit Favorite"}
            {"filter_not_favorite_tooltip", "Vis items that are not marked as favorite"}
            {"filter_not_ignored", "Neit Ignored"}
            {"filter_not_ignored_tooltip", "Vis non-ignored items"}
            {"filter_not_nosell", "Neit NeiSell"}
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
            {"filter_research_notes", "Research Neites"}
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
            {"filter_type_unlock", "Lås opp"}
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
            {"full_backup", "Full Sikkerhetskopi"}
            {"full_backup_tooltip", "Sikkerhetskopi all data (settings, session history, favorites, ignored items, custom profit) to a JSON file"}
            {"full_restore", "Full Gjenopprett"}
            {"full_restore_tooltip", "Gjenopprett all data from a backup JSON file"}
            {"general_settings", "Generelt Innstillinger"}
            {"gold_format", "Gold: %lld"}
            {"gradient_backgrounds", "Gradient backgrounds"}
            {"gradient_backgrounds_tooltip", "Aktivers smooth gradient backgrounds for a more modern look"}
            {"grid_icon_size_currencies", "Grid Ikon size (Valutaer)"}
            {"grid_icon_size_currencies_tooltip", "Størrelse of icons in grid view for Valutaer (16-128)"}
            {"grid_icon_size_items", "Grid Ikon size (Gjenstander)"}
            {"grid_icon_size_items_tooltip", "Størrelse of icons in grid view for Gjenstander (16-128)"}
            {"group_by_rarity", "Grupper by Rarity"}
            {"group_by_type", "Grupper by Category"}
            {"icon_cache_max_icons", "Max Cached Ikons"}
            {"icon_cache_max_icons_tooltip", "Maximum number of icons to keep in cache (older icons are deleted when limit is reached)"}
            {"icon_size", "Ikon size"}
            {"icon_size_tooltip", "Størrelse of item icons in pixels (16-96)"}
            {"import", "Importer"}
            {"import_history", "Importer Historikk"}
            {"import_history_tooltip", "Importer session history from a JSON file"}
            {"import_tooltip", "Importer settings from a JSON file"}
            {"infusion_drop_label", "Infusion Drop!"}
            {"item", "Item"}
            {"items_header", "Gjenstander"}
            {"magic_find_abbreviation", "MF: %d%%"}
            {"main_window_click_through", "Click through"}
            {"main_window_click_through_tooltip", "Alleows clicking through the main window to the game"}
            {"main_window_opacity", "Main Vindu Transparency"}
            {"main_window_opacity_tooltip", "Main window background transparency (0-100%)"}
            {"main_window_settings", "Main Vindu"}
            {"map", "Map"}
            {"mass_actions_clear_ignore", "Tøm ignore list"}
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
            {"max_backups", "Max Sikkerhetskopis"}
            {"max_backups_tooltip", "Maximum number of backups to keep (1-20)"}
            {"max_session_history", "Max Økts"}
            {"max_session_history_tooltip", "Maximum number of sessions to save (1-50). Oldest session is deleted when limit is reached if overwrite is enabled."}
            {"min_value", "Min Verdi"}
            {"mini_window_click_through", "Vindu click through"}
            {"mini_window_click_through_tooltip", "Alleows clicking through the mini window to the game"}
            {"mini_window_hide_title_bar", "Skjul Mini Vindu Title Bar"}
            {"mini_window_hide_title_bar_tooltip", "Skjul the title bar of the mini window"}
            {"mini_window_locked", "Lås Mini Vindu"}
            {"mini_window_locked_tooltip", "Fix the mini window position and size (no longer movable or resizable)"}
            {"mini_window_opacity", "Mini Vindu Transparency"}
            {"mini_window_opacity_tooltip", "Mini window background transparency (0-100%)"}
            {"mini_window_show_profit", "Vis Profit"}
            {"mini_window_show_profit_per_hour", "Vis Profit/Hour"}
            {"mini_window_show_profit_per_hour_tooltip", "Display profit per hour in mini window"}
            {"mini_window_show_profit_tooltip", "Display total profit in mini window"}
            {"mini_window_show_session_duration", "Vis Økt Varighet"}
            {"mini_window_show_session_duration_tooltip", "Display session duration in mini window"}
            {"mini_window_show_total_items", "Vis Totalt Gjenstander"}
            {"mini_window_show_total_items_tooltip", "Display total item count in mini window"}
            {"mini_window_show_tp_instant", "Vis TP Instant (Instant Sell)"}
            {"mini_window_show_tp_instant_tooltip", "Display TP instant sell profit in mini window"}
            {"mini_window_show_tp_sell", "Vis TP Sell (Listings)"}
            {"mini_window_show_tp_sell_tooltip", "Display TP sell profit (listings) in mini window"}
            {"minutes_after_unload_tooltip", "Minutes after addon unload before automatic reset"}
            {"no_cancel", "Nei, Avbryt"}
            {"no_items_in_session", "Nei items in this session"}
            {"no_sessions_recorded", "Nei sessions recorded yet."}
            {"notification_combine_logic", "Combine Filters (AND)"}
            {"notification_combine_logic_tooltip", "If enabled, BOTH conditions (Verdi AND Rarity) must be met. If disabled, ANY one of them is enough."}
            {"notification_duration", "Display Varighet"}
            {"notification_duration_tooltip", "How long the notification stays visible (seconds)"}
            {"notification_general", "Generelt Innstillinger"}
            {"notification_include_agony", "Include Agony Infusions"}
            {"notification_include_agony_tooltip", "If enabled, Agony Infusions (+1 to +30) will also trigger an alert."}
            {"notification_include_non_profit", "Include Nein-Profit Gjenstander"}
            {"notification_include_non_profit_tooltip", "If enabled, items with no gold value (0c) will also trigger alerts if they meet the rarity requirement."}
            {"notification_infusion_alert", "Infusion Alert"}
            {"notification_infusion_alert_tooltip", "Always notify when an Infusion is found (ignores Verdi/Rarity filters)"}
            {"notification_item_alerts", "Item Alerts"}
            {"notification_min_rarity", "Min. Rarity"}
            {"notification_min_rarity_tooltip", "Trigger notification if item rarity is at least this level"}
            {"notification_min_value", "Min. Verdi (Gold)"}
            {"notification_min_value_tooltip", "Trigger notification if item value is at least this amount"}
            {"notification_play_sound", "Spill av Lyd"}
            {"notification_play_sound_tooltip", "Spill av a sound effect when a notification appears"}
            {"notification_precursor_alert", "Pre-Cursor Alert"}
            {"notification_precursor_alert_tooltip", "Always notify when a Pre-Cursor is found (ignores Verdi/Rarity filters)"}
            {"notification_session_alerts", "Progress & Tid"}
            {"notification_settings", "Varsling Innstillinger"}
            {"notification_setup_hint", "[Drag to reposition notifications]"}
            {"notification_stacking", "Stack Varslings"}
            {"notification_stacking_tooltip", "Vis multiple notifications at once instead of replacing the old one immediately"}
            {"notification_triggers", "Varsling Triggers"}
            {"notification_volume", "Master Volum"}
            {"notification_volume_tooltip", "Volum for notification sounds"}
            {"notify_profit_goal", "Neitify when profit goal reached"}
            {"notify_profit_goal_tooltip", "Neitify when you reach your profit goal"}
            {"notify_reset_warning", "Neitify before reset"}
            {"notify_reset_warning_tooltip", "Neitify before automatic reset occurs"}
            {"notify_session_complete", "Neitify after session duration"}
            {"notify_session_complete_tooltip", "Neitify after farming for a certain duration"}
            {"opportunity_cost_per_hour", "Opportunity cost per hour"}
            {"opportunity_cost_per_hour_tooltip", "Opportunity cost per hour"}
            {"opportunity_cost_vs_tp_sell", "Opportunity cost vs TP sell"}
            {"opportunity_cost_vs_tp_sell_tooltip", "Opportunity cost vs TP sell"}
            {"overwrite_session_history", "Overwrite Økts"}
            {"overwrite_session_history_tooltip", "If enabled, oldest session is deleted when limit is reached"}
            {"performance_settings", "Performance Innstillinger"}
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
            {"reset_all", "Tilbakestill Alle"}
            {"reset_all_tooltip", "Tilbakestill all settings to default values"}
            {"reset_interval_days", "Tilbakestill interval (days)"}
            {"reset_interval_days_tooltip", "Egendefinert reset interval in days (1-30 days)"}
            {"reset_settings", "Auto Tilbakestill"}
            {"reset_warning_minutes", "Tilbakestill Advarsel (Minutes)"}
            {"reset_warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"reset_warning_msg", "The tracker will reset in %d minutes!"}
            {"reset_warning_title", "Tilbakestill Advarsel"}
            {"restore", "Gjenopprett"}
            {"row_color", "Row Farge"}
            {"save", "Lagre"}
            {"save_account", "Lagre Konto"}
            {"save_all_items_confirm", "Aktiver session timeline?"}
            {"save_all_items_warning", "This will significantly increase file size!"}
            {"save_current_session", "Lagre Current Økt"}
            {"save_current_session_tooltip", "Lagre the current farming session without resetting"}
            {"save_tooltip", "Lagre current settings"}
            {"search_favorite_currencies_hint", "Søk favorite currencies..."}
            {"search_favorite_items_hint", "Søk favorite items..."}
            {"search_items", "Søk Gjenstander"}
            {"search_items_hint", "Søk items..."}
            {"select_profile", "Velg a profile to apply its settings"}
            {"select_profile_tooltip", "Velg a profile to apply its settings"}
            {"session_complete_hours", "Økt Complete (Hours)"}
            {"session_complete_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_complete_msg", "You have been farming for %d hours!"}
            {"session_complete_title", "Økt Complete"}
            {"session_count", "Økt Antall"}
            {"session_details", "Økt Details"}
            {"session_history", "Økt Historikk"}
            {"session_hours", "Økt hours"}
            {"session_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_note", "Neite"}
            {"session_profit_trend", "Profit Trend"}
            {"session_search_hint", "Søk sessions, items, notes..."}
            {"sessions_selected", "sessions selected"}
            {"sessions_stored", "Økts Stored"}
            {"settings_profiles", "Innstillinger Profiles"}
            {"show_ignored_items", "Vis ignored items"}
            {"show_ignored_items_tooltip", "Vis ignored items/currencies in Gjenstander and Valutaer tabs (disable to hide). Difference from 'Ignored' filter: This filter controls display in Gjenstander/Valutaer tabs, the 'Ignored' filter controls display in Filter tab."}
            {"show_main_window", "Vis main window"}
            {"show_mini_window", "Vis mini window"}
            {"show_mini_window_tooltip", "Viss a small overlay widget with key statistics"}
            {"show_notification_setup", "Setup Mode (Positioning)"}
            {"show_notification_setup_tooltip", "Makes the notification window visible so you can move it"}
            {"show_rarity_as_tabs", "Vis as Fanes"}
            {"show_summaries", "Vis Summaries"}
            {"show_summaries_tooltip", "Vis daily/weekly/monthly profit summaries"}
            {"show_type_as_tabs", "Vis as Fanes"}
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
            {"stat_best_session", "Beste Økt"}
            {"stat_total_profit", "Totalt Profit"}
            {"stat_total_time", "Totalt Tid"}
            {"summaries_coming_soon", "Summaries feature coming soon..."}
            {"summaries_label", "Summaries"}
            {"summaries_tooltip", "Daily/Weekly/Monthly profit summaries"}
            {"summary_period", "Period:"}
            {"summary_this_month", "This Month"}
            {"summary_this_week", "This Week"}
            {"summary_today", "Today"}
            {"tab_session_history", "Økt Historikk"}
            {"tab_sessions", "Økts"}
            {"tab_summaries", "Summaries"}
            {"test_item_label", "Test Item"}
            {"text_color", "Text Farge"}
            {"time", "Tid"}
            {"time_ago_seconds", "%llds ago"}
            {"timeline_icon_size_currencies", "Tidslinje Ikon Størrelse (Valutaer)"}
            {"timeline_icon_size_currencies_tooltip", "Størrelse of currency icons in Tidslinje tab (16-48)"}
            {"timeline_icon_size_items", "Tidslinje Ikon Størrelse (Gjenstander)"}
            {"timeline_icon_size_items_tooltip", "Størrelse of item icons in Tidslinje tab (16-96)"}
            {"toggle_favorite", "Toggle favorite"}
            {"toggle_favorite_tooltip", "Toggle favorite"}
            {"toggle_ignore", "Toggle ignore"}
            {"toggle_ignore_tooltip", "Toggle ignore"}
            {"top_currencies_count_header", "Topp 5 Valutaer (Antall)"}
            {"top_currencies_count_tooltip", "Topp 5 currencies by count"}
            {"top_drops", "Topp Drops"}
            {"top_gradient_color", "Topp"}
            {"top_gradient_color_tooltip", "Topp gradient color"}
            {"top_items_count_header", "Topp 5 Gjenstander (Antall)"}
            {"top_items_profit_header", "Topp 5 Gjenstander by Profit"}
            {"total_custom_profit", "Totalt custom profit"}
            {"total_custom_profit_tooltip", "Totalt custom profit"}
            {"total_drops", "Totalt Drops"}
            {"total_duration", "Totalt Varighet"}
            {"total_tp_sell_profit", "Totalt TP sell profit"}
            {"total_tp_sell_profit_tooltip", "Totalt TP sell profit"}
            {"total_tracked_currencies", "Totalt number of tracked currencies"}
            {"total_tracked_currencies_tooltip", "Totalt number of tracked currencies"}
            {"total_tracked_items", "Totalt number of tracked items"}
            {"total_tracked_items_tooltip", "Totalt number of tracked items"}
            {"total_vendor_sell_profit", "Totalt vendor sell profit"}
            {"total_vendor_sell_profit_tooltip", "Totalt vendor sell profit"}
            {"tp_buy_gross_format", "TP Buy (Gross): %s"}
            {"tp_buy_net_format", "TP Buy (Net): %s"}
            {"tp_sell_gross_format", "TP Sell (Gross): %s"}
            {"tp_sell_net_format", "TP Sell (Net): %s"}
            {"trigger_drops", "Item Drops"}
            {"trigger_profit_goal", "Profit Goal"}
            {"trigger_time_reset", "Tid & Tilbakestill"}
            {"type_backpack", "Backpack"}
            {"type_gizmo_container", "Gizmo Container"}
            {"type_tool", "Tool"}
            {"type_trophy", "Trophy"}
            {"type_unlock", "Lås opp"}
            {"type_upgrade_component", "Upgrade Component"}
            {"unknown_map", "Unknown"}
            {"update_profile", "Oppdater Profile"}
            {"update_profile_tooltip", "Oppdater current profile with current settings"}
            {"value", "Verdi"}
            {"vendor_value_format", "Vendor Verdi: %s"}
            {"visual_settings", "Visual Innstillinger"}
            {"warning_minutes", "Advarsel minutes"}
            {"warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"yes_clear", "Ja, Tøm"}
            {"yes_enable", "Ja, Aktiver"}
            {"yes_reset", "Ja, Tilbakestill"},

            // Drops Tab
            {"settings_tab", "Innstillinger"},
        
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
