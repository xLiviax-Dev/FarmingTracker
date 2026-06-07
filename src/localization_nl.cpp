// ---------------------------------------------------------------------------
// localization_nl.cpp – Dutch translations for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>

namespace Localization
{
    const std::unordered_map<std::string, const char*> GetDutchTranslations()
    {
        static const std::unordered_map<std::string, const char*> translations = {
            // Status texts
            {"status_disconnected", "Verbroken"},
            {"status_connecting", "Verbinden..."},
            {"status_connected", "Verbonden"},
            {"status_auth_failed", "Authenticatie mislukt – controleer token"},
            {"status_reconnecting", "Opnieuw verbinden..."},
            {"status_error", "Fout"},
            {"status_unknown", "Onbekend"},

            // Mini Window
            {"mini_window_title", "Farming Tracker Mini"},
            {"profit", "Winst"},
            {"profit_per_hour", "Winst/uur"},
            {"tp_sell", "TP Verkoop"},
            {"tp_instant", "TP Direct"},
            {"total_items", "Totaal aantal items"},
            {"session", "Sessie"},

            // Main Window
            {"main_window_title", "Farming Tracker"},
            {"drf_label", "DRF"},
            {"gw2_api_label", "GW2 API"},
            {"session_time_label", "Sessietijd"},
            {"reset_button", "Reset"},
            {"reset_tooltip", "Reset alle farming-tellers (handmatige reset)"},

            // Tabs
            {"tab_summary", "Dashboard"},
            {"tab_drops", "Drops"},
            {"tab_loot_filter", "Loot Filter"},
            {"tab_items", "Items"},
            {"tab_currencies", "Valuta's"},
            {"tab_dashboard", "Dashboard"},
            {"tab_favorites", "Favorieten"},
            {"tab_ignored", "Genegeerd"},
            {"tab_timeline", "Tijdlijn"},
            {"timeline_profit_hour_listings", "Ongeveer handelswinsten per uur (Lijsten)"},
            {"timeline_profit_hour_instant", "Ongeveer handelswinsten per uur (Directe verkoop)"},
            {"timeline_no_drops", "Nog geen drops vastgelegd in deze sessie."},
            {"timeline_item_drops", "Item Drops"},
            {"timeline_currencies", "Valuta's"},
            {"tab_filter", "Filter"},
            {"tab_custom_profit", "Aangepaste winst"},
            {"tab_debug", "Debug"},

            // Summary Tab
            {"warning_drf_not_connected", "⚠️ DRF niet verbonden"},
            {"warning_drf_not_connected_desc", "Deze plugin vereist DRF voor gegevensoverdracht."},
            {"warning_drf_install", "Installeer DRF via Nexus Addon Manager of https://drf.rs/"},
            {"warning_drf_token_invalid", "⚠️ DRF Token ongeldig"},
            {"warning_drf_token_invalid_desc", "Controleer uw DRF Token in Instellingen."},
            {"warning_gw2_api_key_not_set", "⚠️ GW2 API-sleutel niet ingesteld"},
            {"warning_gw2_api_key_not_set_desc", "Stel uw GW2 API-sleutel in Instellingen in voor item-details."},
            {"gold", "Goud"},
            {"silver", "Zilver"},
            {"copper", "Koper"},
            {"total_profit", "Totale winst"},
            {"total_profit_tooltip", "Totale aangepaste winst van alle items"},
            {"total_items_count", "Totaal aantal items"},
            {"total_items_tooltip", "Totaal aantal unieke items bijgehouden"},
            {"total_currencies", "Totaal aantal valuta's"},
            {"total_currencies_tooltip", "Totaal aantal unieke valuta's bijgehouden"},
            {"profit_per_hour_label", "Winst per uur"},
            {"profit_per_hour_tooltip", "Winst per uur gebaseerd op sessieduur"},
            {"magic_find", "Magic Find"},
            {"magic_find_tooltip", "Huidige of laatst vastgelegde Magic Find van DRF"},
            {"session_duration", "Sessieduur"},
            {"session_duration_tooltip", "Huidige farming-sessieduur"},
            {"date_tooltip", "Sessie-starttijd"},
            {"duration_tooltip", "Sessieduur"},
            {"profit_tooltip", "Totale sessiewinst"},
            {"profit_per_hour_tooltip", "Winst per uur"},
            {"drops_tooltip", "Aantal drops"},
            {"best_drop_tooltip", "Meest waardevolle drop van de sessie"},
            {"top_items_profit", "Top items (Winst)"},
            {"top_items_profit_tooltip", "Top 5 items op winstwaarde"},
            {"loading", "Laden..."},
            {"coin", "Munt"},
            {"top_items_count", "Top items (Aantal)"},
            {"top_items_count_tooltip", "Top 5 items op aantal"},
            {"top_currencies", "Top valuta's"},
            {"top_currencies_tooltip", "Top 5 valuta's op aantal"},
            {"quick_statistics", "Snelle statistieken"},
            {"quick_statistics_tooltip", "Overzicht van farming-statistieken"},
            {"average_item_value", "Gemiddelde itemwaarde"},
            {"average_item_value_na", "N.v.t."},
            {"total_unique_items", "Totaal unieke items"},
            {"warning_no_data", "⚠️ Geen gegevens geladen"},
            {"warning_no_data_desc", "Wachten op DRF-gegevens..."},
            {"export", "Exporteer"},
            {"export_tooltip", "Exporteer farming-gegevens naar bestand"},
            {"export_json", "Exporteer als JSON"},
            {"export_csv", "Exporteer als CSV"},
            {"import_json", "Importeer uit JSON"},

            // Items Tab
            {"search_hint", "Zoek items..."},
            {"clear", "Wissen"},
            {"sort_count_high", "Sorteer: |Aantal| hoog"},
            {"sort_count_low", "Sorteer: |Aantal| laag"},
            {"sort_id_up", "Sorteer: Item-ID omhoog"},
            {"sort_id_down", "Sorteer: Item-ID omlaag"},
            {"sort_name_az", "Sorteer: Naam A–Z"},
            {"sort_tooltip", "Sorteer items op aantal, ID of naam"},
            {"rarity_all", "Zeldzaamheid: alle"},
            {"rarity_basic", "Zeldzaamheid: Basis+"},
            {"rarity_fine", "Zeldzaamheid: Fijn+"},
            {"rarity_masterwork", "Zeldzaamheid: Meesterwerk+"},
            {"rarity_rare", "Zeldzaamheid: Zeldzaam+"},
            {"rarity_exotic", "Zeldzaamheid: Exotisch+"},
            {"rarity_ascended", "Zeldzaamheid: Ascended+"},
            {"rarity_legendary", "Zeldzaamheid: Alleen Legendarisch"},
            {"rarity_tooltip", "Filter items op minimum zeldzaamheid"},
            {"rarity", "Zeldzaamheid"},
            {"type", "Type"},
            {"vendor_value", "Verkoperwaarde"},
            {"tp_buy_net", "TP Koop (Netto)"},
            {"account_bound", "Account-gebonden"},
            {"yes", "Ja"},
            {"no", "Nee"},
            {"nosell", "NoSell"},
            {"favorite", "Favoriet"},
            {"ignore", "Negeer"},

            // Currencies Tab
            {"search_currencies_hint", "Zoek valuta's..."},
            {"api_id", "API-ID"},
            {"currency_name", "Valutanaam"},
            {"count", "Aantal"},

            // Favorites Tab
            {"unfavorite_item", "Ontfavoriet"},
            {"unfavorite_selected", "Ontfavoriet geselecteerd"},
            {"no_favorites_yet", "Nog geen favorieten. Klik met de rechtermuisknop op een item om het toe te voegen."},
            {"toggle_favorite_tooltip", "Wissel favoriet"},
            {"profits", "Winsten"},
            {"profits_tooltip", "Totale winsten uit farming"},
            {"approx_profits", "Ongeveer winsten"},
            {"approx_gold_per_hour", "Ongeveer goud per uur"},
            {"trading_profits", "Handelswinsten"},
            {"trading_profits_tooltip", "Winsten van Trading Post"},

            // Profit Tab
            {"profits", "Winsten"},
            {"profits_tooltip", "Totale winsten uit farming"},
            {"approx_profits", "Ongeveer winsten"},
            {"approx_gold_per_hour", "Ongeveer goud per uur"},
            {"trading_profits", "Handelswinsten"},
            {"trading_profits_tooltip", "Winsten van Trading Post"},
            {"approx_trading_profits_listings", "Ongeveer handelswinsten (Lijsten)"},
            {"approx_trading_profits_instant", "Ongeveer handelswinsten (Directe verkoop)"},
            {"trading_details", "Handelsdetails (Kosten van afstand)"},
            {"trading_details_tooltip", "Kosten van afstand door items te gebruiken in plaats van te verkopen"},
            {"lost_profit_vs_tp_sell", "Verloren winst (vs TP Verkoop)"},
            {"lost_profit_per_hour_vs_tp_sell", "Verloren winst per uur (vs TP Verkoop)"},
            {"efficiency_score", "Efficiëntiescore"},
            {"efficiency_score_label", "Efficiëntiescore:"},
            {"efficiency_score_tooltip", "Hoeveel van de maximaal mogelijke winst u behaalde (Directe verkoop vs. TP Lijsten)."},
            {"efficiency_score_desc", "U behaalde %.1f%% van de maximale winst!"},
            {"session_duration_label", "Sessieduur"},
            {"session_duration_tooltip", "Huidige farming-sessieduur"},

            // Filter Tab
            {"sell_method_filters", "Verkoopmethode-filters"},
            {"sellable_to_vendor", "Verkoopbaar aan verkoper"},
            {"sellable_to_vendor_tooltip", "Toon items verkoopbaar aan verkoper"},
            {"sellable_on_tp", "Verkoopbaar op TP"},
            {"sellable_on_tp_tooltip", "Toon items verkoopbaar op Trading Post"},
            {"has_custom_profit", "Heeft aangepaste winst"},
            {"has_custom_profit_tooltip", "Toon items met aangepaste winstwaarden"},
            {"api_knowledge_filters", "API-kennis-filters"},
            {"known_by_api", "Bekend door API"},
            {"known_by_api_tooltip", "Toon items bekend door GW2 API"},
            {"unknown_by_api", "Onbekend door API"},
            {"unknown_by_api_tooltip", "Toon items niet bekend door GW2 API"},
            {"item_type_filters", "Itemtype-filters"},
            {"type_armor", "Harnas"},
            {"type_armor_tooltip", "Toon harnas-items"},
            {"type_weapon", "Wapen"},
            {"type_weapon_tooltip", "Toon wapen-items"},
            {"type_trinket", "Sieraad"},
            {"type_trinket_tooltip", "Toon sieraad-items"},
            {"type_gizmo", "Gizmo"},
            {"type_gizmo_tooltip", "Toon gizmo-items"},
            {"type_crafting_material", "Crafting-materiaal"},
            {"type_crafting_material_tooltip", "Toon crafting-materialen"},
            {"type_consumable", "Verbruiksartikel"},
            {"type_consumable_tooltip", "Toon verbruiksartikelen"},
            {"type_gathering_tool", "Verzamelingstool"},
            {"type_gathering_tool_tooltip", "Toon verzamelingstools"},
            {"type_bag", "Tas"},
            {"type_bag_tooltip", "Toon tassen"},
            {"type_container", "Container"},
            {"type_container_tooltip", "Toon containers"},
            {"type_mini_pet", "Mini-huisdier"},
            {"type_mini_pet_tooltip", "Toon mini-huisdieren"},
            {"currency_filters_label", "Valuta Filters"},
            {"currency_general", "Algemeen"},
            {"currency_main", "Hoofdvaluta's"},
            {"currency_fractal", "Fractal/Raid/Dungeon-valuta's"},
            {"currency_wvw_pvp", "WvW/PvP-valuta's"},
            {"currency_map", "Kaart-specifieke valuta's"},
            {"filter_karma", "Karma"},
            {"currency_karma_tooltip", "Toon karma-valuta"},
            {"currency_laurel", "Laurier"},
            {"currency_laurel_tooltip", "Toon laurier-valuta"},
            {"currency_gem", "Edelsteen"},
            {"currency_gem_tooltip", "Toon edelsteen-valuta"},
            {"currency_fractal_relic", "Fractal-relikwie"},
            {"currency_fractal_relic_tooltip", "Toon fractal-relikwie-valuta"},
            {"currency_badge_of_honor", "Erebadge"},
            {"currency_badge_of_honor_tooltip", "Toon erebadge-valuta"},
            {"currency_guild_commendation", "Gilden-aanbeveling"},
            {"currency_guild_commendation_tooltip", "Toon gilden-aanbeveling-valuta"},
            {"currency_transmutation_charge", "Transmutatielading"},
            {"currency_transmutation_charge_tooltip", "Toon transmutatielading-valuta"},
            {"currency_spirit_shards", "Geest-scherven"},
            {"currency_spirit_shards_tooltip", "Toon geest-scherven-valuta"},
            {"currency_unbound_magic", "Ongebonden magie"},
            {"currency_unbound_magic_tooltip", "Toon ongebonden magie-valuta"},
            {"currency_volatile_magic", "Vlottige magie"},
            {"currency_volatile_magic_tooltip", "Toon vlottige magie-valuta"},
            {"currency_airship_parts", "Luchtschip-onderdelen"},
            {"currency_airship_parts_tooltip", "Toon luchtschip-onderdelen-valuta"},
            {"currency_geode", "Geode"},
            {"currency_geode_tooltip", "Toon geode-valuta"},
            {"currency_ley_line_crystals", "Ley-lijn-kristallen"},
            {"currency_ley_line_crystals_tooltip", "Toon ley-lijn-kristallen-valuta"},
            {"currency_trade_contracts", "Handelscontracten"},
            {"currency_trade_contracts_tooltip", "Toon handelscontracten-valuta"},
            {"currency_elegy_mosaic", "Elegy-mozaïek"},
            {"currency_elegy_mosaic_tooltip", "Toon elegy-mozaïek-valuta"},
            {"currency_uncommon_coins", "Ongebruikelijke munten"},
            {"currency_uncommon_coins_tooltip", "Toon ongebruikelijke munten-valuta"},
            {"currency_astral_acclaim", "Astrale lof"},
            {"currency_astral_acclaim_tooltip", "Toon astrale lof-valuta"},
            {"currency_pristine_fractal_relics", "Pristine fractal-relikwieën"},
            {"currency_pristine_fractal_relics_tooltip", "Toon pristine fractal-relikwieën-valuta"},
            {"currency_unstable_fractal_essence", "Onstabiele fractal-essentie"},
            {"currency_unstable_fractal_essence_tooltip", "Toon onstabiele fractal-essentie-valuta"},
            {"currency_magnetite_shards", "Magnetiet-scherven"},
            {"currency_magnetite_shards_tooltip", "Toon magnetiet-scherven-valuta"},
            {"currency_gaeting_crystals", "Gaeting-kristallen"},
            {"currency_gaeting_crystals_tooltip", "Toon gaeting-kristallen-valuta"},
            {"currency_prophet_shards", "Profeet-scherven"},
            {"currency_prophet_shards_tooltip", "Toon profeet-scherven-valuta"},
            {"currency_green_prophet_shards", "Groene profeet-scherven"},
            {"currency_green_prophet_shards_tooltip", "Toon groene profeet-scherven-valuta"},
            {"currency_wvw_skirmish_tickets", "WvW-schermuts-biljetten"},
            {"currency_wvw_skirmish_tickets_tooltip", "Toon WvW-schermuts-biljetten-valuta"},
            {"currency_proofs_of_heroics", "Bewijzen van heldendom"},
            {"currency_proofs_of_heroics_tooltip", "Toon bewijzen van heldendom-valuta"},
            {"currency_pvp_league_tickets", "PvP-competitie-biljetten"},
            {"currency_pvp_league_tickets_tooltip", "Toon PvP-competitie-biljetten-valuta"},
            {"currency_ascended_shards_of_glory", "Ascended glorie-scherven"},
            {"currency_ascended_shards_of_glory_tooltip", "Toon ascended glorie-scherven-valuta"},
            {"currency_research_notes", "Onderzoek-notities"},
            {"currency_research_notes_tooltip", "Toon onderzoek-notities-valuta"},
            {"currency_tyrian_defense_seal", "Tyriaanse verdedigingszegel"},
            {"currency_tyrian_defense_seal_tooltip", "Toon Tyriaanse verdedigingszegel-valuta"},
            {"currency_testimony_of_desert_heroics", "Getuigenis van woestijn-heldendom"},
            {"currency_testimony_of_desert_heroics_tooltip", "Toon getuigenis van woestijn-heldendom-valuta"},
            {"currency_testimony_of_jade_heroics", "Getuigenis van jade-heldendom"},
            {"currency_testimony_of_jade_heroics_tooltip", "Toon getuigenis van jade-heldendom-valuta"},
            {"currency_testimony_of_castoran_heroics", "Getuigenis van castoraan-heldendom"},
            {"currency_testimony_of_castoran_heroics_tooltip", "Toon getuigenis van castoraan-heldendom-valuta"},
            {"currency_legendary_insight", "Legendarisch inzicht"},
            {"currency_legendary_insight_tooltip", "Toon legendarisch inzicht-valuta"},
            {"currency_tales_of_dungeon_delving", "Verhalen van dungeon-ontdekking"},
            {"currency_tales_of_dungeon_delving_tooltip", "Toon verhalen van dungeon-ontdekking-valuta"},
            {"currency_imperial_favor", "Keizerlijke gunst"},
            {"currency_imperial_favor_tooltip", "Toon keizerlijke gunst-valuta"},
            {"currency_canach_coins", "Canach-munten"},
            {"currency_canach_coins_tooltip", "Toon canach-munten-valuta"},
            {"currency_ancient_coin", "Oude munt"},
            {"currency_ancient_coin_tooltip", "Toon oude munt-valuta"},
            {"currency_unusual_coin", "Ongebruikelijke munt"},
            {"currency_unusual_coin_tooltip", "Toon ongebruikelijke munt-valuta"},
            {"currency_jade_sliver", "Jade-splinter"},
            {"currency_jade_sliver_tooltip", "Toon jade-splinter-valuta"},
            {"currency_static_charge", "Statische lading"},
            {"currency_static_charge_tooltip", "Toon statische lading-valuta"},
            {"currency_pinch_of_stardust", "Snufje sterrenstof"},
            {"currency_pinch_of_stardust_tooltip", "Toon snufje sterrenstof-valuta"},
            {"currency_calcified_gasp", "Gekalkt gehijg"},
            {"currency_calcified_gasp_tooltip", "Toon gekalkt gehijg-valuta"},
            {"currency_ursus_oblige", "Ursus Oblige"},
            {"currency_ursus_oblige_tooltip", "Toon ursus oblige-valuta"},
            {"currency_gaeting_crystal_janthir", "Gaeting-kristal (Janthir)"},
            {"currency_gaeting_crystal_janthir_tooltip", "Toon gaeting-kristal (janthir)-valuta"},
            {"currency_antiquated_ducat", "Oude dukaat"},
            {"currency_antiquated_ducat_tooltip", "Toon oude dukaat-valuta"},
            {"currency_aether_rich_sap", "Aether-rijke sap"},
            {"currency_aether_rich_sap_tooltip", "Toon aether-rijke sap-valuta"},

            // Additional Filters
            {"additional_filters", "Aanvullende filters"},
            {"account_bound", "Account-gebonden"},
            {"account_bound_tooltip", "Toon account-gebonden items"},
            {"not_account_bound", "Niet account-gebonden"},
            {"not_account_bound_tooltip", "Toon niet-account-gebonden items"},
            {"nosell_items", "NoSell"},
            {"nosell_items_tooltip", "Toon NoSell-items"},
            {"not_nosell", "Niet NoSell"},
            {"not_nosell_tooltip", "Toon verkoopbare items"},
            {"favorite_items", "Favoriet"},
            {"favorite_items_tooltip", "Toon favoriete items"},
            {"not_favorite", "Niet favoriet"},
            {"not_favorite_tooltip", "Toon niet-favoriete items"},
            {"ignored_items", "Genegeerd"},
            {"ignored_items_tooltip", "Toon genegeerde items"},
            {"not_ignored", "Niet genegeerd"},
            {"not_ignored_tooltip", "Toon niet-genegeerde items"},

            // Range Filters
            {"range_filters", "Bereik-filters"},
            {"show_range_filters", "Toon bereik-filters"},
            {"filter_min_price", "Filter min. prijs"},
            {"filter_max_price", "Filter max. prijs"},
            {"filter_min_quantity", "Filter min. hoeveelheid"},
            {"filter_max_quantity", "Filter max. hoeveelheid"},

            // Mini Window Settings
            {"mini_window_settings", "Mini-venster"},
            {"show_profit", "Toon winst"},
            {"show_profit_tooltip", "Toon totale winst in mini-venster"},
            {"show_profit_per_hour", "Toon winst/uur"},
            {"show_profit_per_hour_tooltip", "Toon winst per uur in mini-venster"},
            {"show_tp_sell", "Toon TP Verkoop (Lijsten)"},
            {"show_tp_sell_tooltip", "Toon TP-verkoopwinst (lijsten) in mini-venster"},
            {"show_tp_instant", "Toon TP Direct (Directe verkoop)"},
            {"show_tp_instant_tooltip", "Toon TP directe verkoopwinst in mini-venster"},
            {"show_total_items", "Toon totaal aantal items"},
            {"show_total_items_tooltip", "Toon totaal aantal items in mini-venster"},
            {"show_session_duration", "Toon sessieduur"},
            {"show_session_duration_tooltip", "Toon sessieduur in mini-venster"},
            {"window_click_through", "Venster klik-door"},
            {"window_click_through_tooltip", "Staat klikken door het mini-venster naar het spel toe"},

            // Main Window Settings
            {"main_window", "Hoofdvenster"},
            {"click_through", "Klik-door"},
            {"click_through_tooltip", "Staat klikken door het hoofdvenster naar het spel toe"},

            // Advanced UI Settings
            {"advanced_ui_settings", "Geavanceerde UI-instellingen"},
            {"no_advanced_ui_settings", "(Geen geavanceerde UI-instellingen beschikbaar)"},

            // Display Settings
            {"display_settings", "Weergave-instellingen"},
            {"show_item_icons", "Toon item-iconen"},
            {"show_item_icons_tooltip", "Toon item-iconen in de lijst"},
            {"show_rarity_borders", "Toon zeldzaamheidsranden"},
            {"show_rarity_borders_tooltip", "Toont gekleurde randen rond iconen op basis van zeldzaamheid"},
            {"enable_grid_view", "Schakel rasterweergave in"},
            {"enable_grid_view_tooltip", "Toon items in rasterlay-out in plaats van lijst"},
            {"grid_icon_size", "Raster-icongrootte"},
            {"grid_icon_size_tooltip", "Grootte van iconen in rasterweergave"},

            // Count Display Settings
            {"count_display_settings", "Aantal-weergave-instellingen"},
            {"count_text_color", "Aantal-tekstkleur"},
            {"count_text_color_tooltip", "Kleur van aantal-tekst"},
            {"count_background_color", "Aantal-achtergrondkleur"},
            {"count_background_color_tooltip", "Kleur van aantal-achtergrond"},
            {"count_font_size", "Aantal-lettergrootte"},
            {"count_font_size_tooltip", "Grootte van aantal-lettertype"},
            {"count_horizontal_alignment", "Aantal-horizontale uitlijning"},
            {"count_horizontal_alignment_tooltip", "Horizontale uitlijning van aantal-tekst"},

            // Gradient Background Settings
            {"gradient_background_settings", "Gradient-achtergrond-instellingen"},
            {"enable_gradient_backgrounds", "Schakel gradient-achtergrond in"},
            {"enable_gradient_backgrounds_tooltip", "Schakel gradient-achtergrond in voor vensters"},
            {"gradient_top_color", "Gradient-bovenkleur"},
            {"gradient_top_color_tooltip", "Bovenkleur van gradient-achtergrond"},
            {"gradient_bottom_color", "Gradient-onderkleur"},
            {"gradient_bottom_color_tooltip", "Onderkleur van gradient-achtergrond"},

            // Custom Profit System
            {"custom_profit_system", "Aangepast winstsysteem"},
            {"enable_custom_profit", "Schakel aangepaste winst in"},
            {"enable_custom_profit_tooltip", "Schakel aangepaste winstwaarden in voor items"},

            // Search
            {"search_settings", "Zoeken"},
            {"enable_search", "Schakel zoeken in"},
            {"enable_search_tooltip", "Schakel zoekfunctionaliteit in"},

            // Ignored Items
            {"ignored_items_settings", "Genegeerde items"},
            {"enable_ignored_items", "Schakel genegeerde items in"},
            {"enable_ignored_items_tooltip", "Schakel functionaliteit voor genegeerde items in"},

            // Auto Reset
            {"auto_reset_settings", "Auto-reset"},
            {"enable_auto_reset", "Schakel auto-reset in"},
            {"enable_auto_reset_tooltip", "Reset farming-sessie automatisch na een duur"},
            {"auto_reset_duration", "Auto-reset-duur (minuten)"},
            {"auto_reset_duration_tooltip", "Duur in minuten voor auto-reset"},

            // DRF Settings
            {"drf_settings", "DRF-instellingen"},
            {"drf_token", "DRF Token"},
            {"drf_token_label", "DRF Token:"},
            {"drf_token_tooltip", "Uw DRF-authenticatietoken"},
            {"edit_token", "Bewerk token"},
            {"save_token", "Sla token op"},

            // GW2 API Settings
            {"gw2_api_settings", "GW2 API-instellingen"},
            {"gw2_api_key", "GW2 API-sleutel"},
            {"gw2_api_key_tooltip", "Uw GW2 API-sleutel voor item-details"},
            {"edit_key", "Bewerk sleutel"},
            {"save_key", "Sla sleutel op"},

            // Language Settings
            {"language_settings", "Taal"},
            {"language_tooltip", "Selecteer interface-taal"},
            {"language_english", "Engels"},
            {"language_german", "Duits"},
            {"language_french", "Frans"},
            {"language_spanish", "Spaans"},
            {"language_chinese", "Chinees"},
            {"language_czech", "Tsjechisch"},
            {"language_italian", "Italiaans"},
            {"language_polish", "Pools"},
            {"language_portuguese", "Portugees"},
            {"language_russian", "Russisch"},

            // Additional hardcoded strings found in UI
            {"farming_tracker_title", "Farming Tracker"},
            {"no_accounts_configured", "Geen accounts geconfigureerd"},
            {"no_profiles_created", "Geen profielen aangemaakt"},
            {"count_label", "Aantal:"},
            {"profit_label", "Winst:"},
            {"no_profit", "Geen winst"},
            {"vendor_value_label", "Verkoper-waarde:"},
            {"tp_sell_gross_label", "TP Verkoop (Bruto):"},
            {"tp_sell_net_label", "TP Verkoop (Netto):"},
            {"tp_buy_gross_label", "TP Koop (Bruto):"},
            {"tp_buy_net_label", "TP Koop (Netto):"},
            {"ignored_items_label", "Genegeerde items:"},
            {"ignored_currencies_label", "Genegeerde valuta:"},
            {"total_items_label", "Totaal aantal items:"},
            {"total_currencies_label", "Totaal aantal valuta:"},
            {"total_profit_label", "Totaal winst:"},
            {"tp_sell_profit_label", "TP Verkoop Winst:"},
            {"tp_sell_profit_tooltip", "Totaal winst als alle items worden verkocht tegen de huidige TP-verkoopprijzen (minus 15% fee)"},
            {"vendor_profit_label", "Verkoper Winst:"},
            {"profit_per_hour_label", "Winst per uur:"},
            {"opportunity_cost_profit_label", "Kans-kosten Winst:"},
            {"opportunity_cost_profit_per_hour_label", "Kans-kosten Winst per uur:"},
            {"custom_profit_feature_placeholder", "Functie geïmplementeerd - UI volgt"},
            {"custom_profit_items_header", "Items met aangepaste winst"},
            {"custom_profit_currencies_header", "Valuta met aangepaste winst"},
            {"add_custom_profit_item", "Voeg aangepaste winst toe voor item"},
            {"add_custom_profit_currency", "Voeg aangepaste winst toe voor valuta"},
            {"custom_profit_set_profit", "Stel winst in"},
            {"custom_profit_remove", "Verwijder"},
            {"custom_profit_value", "Winstwaarde (Koper)"},
            {"custom_profit_set_tooltip", "Stel aangepaste winstwaarde in voor dit item"},
            {"custom_profit_remove_tooltip", "Verwijder aangepaste winstwaarde voor dit item"},
            {"no_custom_profit_items", "(Geen items met aangepaste winst)"},
            {"no_custom_profit_currencies", "(Geen valuta met aangepaste winst)"},
            {"clear_all_custom_profits", "Verwijder alle aangepaste winsten"},
            {"clear_all_custom_profits_tooltip", "Verwijder alle aangepaste winstwaarden"},
            {"tabs_settings", "Andere tabs"},
            {"tabs_description", "Toon of verberg andere tabs"},
            {"tab_settings", "Tab-instellingen"},
            {"tab_settings_description", "Tab-volgorde en gedrag"},
            {"enable_dashboard_tab", "Schakel dashboard-tab in"},
            {"enable_dashboard_tab_tooltip", "Toon de dashboard-tab"},
            {"enable_items_tab", "Schakel items-tab in"},
            {"enable_items_tab_tooltip", "Toon de items-tab"},
            {"enable_currencies_tab", "Schakel valuta-tab in"},
            {"enable_currencies_tab_tooltip", "Toon de valuta-tab"},
            {"enable_ignored_tab", "Schakel genegeerde-tab in"},
            {"enable_ignored_tab_tooltip", "Toon de genegeerde items-tab"},
            {"enable_session_history_tab", "Schakel sessiegeschiedenis-tab in"},
            {"enable_session_history_tab_tooltip", "Toon de sessiegeschiedenis-tab"},
            {"enable_timeline_tab", "Schakel tijdslijn-tab in"},
            {"enable_timeline_tab_tooltip", "Toon de tijdslijn-tab met gedetailleerde dropgeschiedenis"},
            {"enable_loot_log_tab", "Schakel Loot Log-tab in"},
            {"enable_loot_log_tab_tooltip", "Toon de Loot Log-tab"},
            {"enable_filter_tab", "Schakel filter-tab in"},
            {"enable_filter_tab_tooltip", "Toon de filter-tab"},
            {"lock_tab_order", "Vergrendel tab-volgorde"},
            {"lock_tab_order_tooltip", "Schakel het herschikken van tabs in het hoofdvenster uit"},
            {"enable_summaries_tab", "Schakel samenvattingen-tab in"},
            {"enable_summaries_tab_tooltip", "Toon de dagelijkse/wekelijkse/maandelijkse samenvattingen-tab in sessiegeschiedenis"},
            {"custom_profit_settings", "Aangepaste winst"},
            {"total_profit_label_simple", "Totaal winst"},
            {"total_items_label_simple", "Totaal aantal items"},
            {"total_currencies_label_simple", "Totaal aantal valuta"},
            {"profit_per_hour_label_simple", "Winst per uur"},
            {"session_duration_label_simple", "Sessieduur"},
            {"next_reset_label_simple", "Volgende reset"},
            {"export_label", "Exporteer:"},
            {"quick_actions", "Snelkeuzes:"},
            {"reset_confirm", "Weet u zeker dat u alle instellingen wilt resetten naar de standaardwaarden?"},
            {"reset_warning", "Deze actie kan niet ongedaan worden gemaakt."},
            {"hotkeys", "Sneltoetsen"},
            {"mini_window_toggle_hotkey", "Mini-venster toggle-sneltoets"},
            {"backup_restore", "Backup & herstel"},
            {"appearance_settings", "Uiterlijk"},
            {"enable_tooltips", "Tooltips inschakelen"},
            {"enable_tooltips_tooltip", "Tooltips weergeven bij het zweven over UI-elementen"},
            {"enable_grid_view_tooltip", "Items in rasterweergave tonen in plaats van een lijst"},
            {"favorites_first_tooltip", "Favoriete items bovenaan de lijst tonen"},
            {"group_by_rarity_tooltip", "Items groeperen op zeldzaamheid"},
            {"show_rarity_as_tabs_tooltip", "Elke zeldzaamheid als een apart tabblad tonen"},
            {"group_by_category_tooltip", "Items groeperen op categorie"},
            {"show_group_as_tabs_tooltip", "Elke categorie als een apart tabblad tonen"},
            {"mass_ignore_rarity_tooltip", "Negeer alle items van deze zeldzaamheid"},
            {"icons_borders", "Iconen & randen"},
            {"colors_gradients", "Kleuren & gradients"},
            {"window_opacity", "Venstertransparantie"},
            {"windows_settings", "Vensters"},
            {"advanced_settings", "Geavanceerd"},
            {"export_settings", "Exporteer instellingen naar bestand:"},
            {"import_settings", "Importeer instellingen van bestand:"},
            {"edit_account", "Bewerk account: %s"},
            {"account_name", "Accountnaam:"},
            {"gw2_api_key_label", "GW2 API-sleutel:"},
            {"reload_config", "Herlaad configuratie:"},
            {"auto_reset_label", "Automatische reset:"},
            {"next_reset_utc", "Volgende geplande reset (UTC): %s"},
            {"favorites_ui", "Favorieten UI:"},
            {"favorites_colors", "Favorieten kleuren:"},
            {"visual_enhancements", "Visuele verbeteringen:"},
            {"show_profit_sparkline", "Toon winst-sparlijn"},
            {"show_profit_sparkline_tooltip", "Toon een kleine lijngrafiek met winst per uur-trend"},
            {"mini_window_widget", "Mini-venster (overlay-widget):"},
            {"main_window_label", "Hoofdvenster:"},
            {"profiles_description", "Profielen stellen u in staat om verschillende configuraties op te slaan en snel tussen hen te schakelen."},
            {"create_new_profile", "Maak een nieuw profiel aan:"},
            {"current_profile", "Huidig profiel: %s"},
            {"auto_backup", "Maak automatisch een back-up van uw instellingen voordat u belangrijke wijzigingen aanbrengt"},
            {"notifications", "Configureer in-game meldingen voor belangrijke gebeurtenissen"},
            {"profit_goal", "Winstdoel:"},
            {"reset_warning_label", "Reset-waarschuwing:"},
            {"session_complete", "Sessie voltooid:"},
            {"manage_ignored_items", "Beheer genegeerde items"},
            {"manage_ignored_currencies", "Beheer genegeerde valuta"},
            {"rarity_label", "Zeldzaamheid: %s"},
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
            {"enable_profit_tab", "Enable Dashboard Tab"},
            {"enable_profit_tab_tooltip", "Show the Dashboard tab"},
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
            {"account_bound_label", "Account-gebonden: %s"},
            {"nosell_label", "NoSell: %s"},
            {"item_id_label", "Item-ID: %d"},
            {"currency_id_label", "Valuta-ID: %d"},
            {"context_menu_add_favorites", "Toevoegen aan favorieten"},
            {"context_menu_remove_favorites", "Verwijderen uit favorieten"},
            {"context_menu_ignore", "Negeer item"},
            {"context_menu_unignore", "Verwijderen uit genegeerd"},
            {"context_menu_copy_name", "Kopieer naam"},
            {"context_menu_copy_id", "Kopieer ID"},
            {"sell_method_filters_label", "Verkoopmethode-filters:"},
            {"api_knowledge_filters_label", "API-kennis-filters:"},
            {"additional_filters_label", "Aanvullende filters:"},
            {"item_type_filters_label", "Itemtype-filters:"},
            {"currency_filters_label", "Valuta-filters:"},
            {"price_range", "Prijsbereik (Koper):"},
            {"quantity_range", "Hoeveelheidsbereik:"},
            {"debug_info", "Debug-informatie"},
            {"drf_status", "DRF-status: %s"},
            {"drf_reconnect_count", "DRF-herverbindingsaantal: %d"},
            {"gw2_api_status", "GW2 API-status: %s"},
            {"gw2_api_reconnect_count", "GW2 API-herverbindingsaantal: %d"},
            {"session_duration_debug", "Sessieduur"},
            {"gw2_memory", "GW2-procesgeheugen: %zu MB"},
            {"gw2_api_request_count", "GW2 API-verzoekaantal: %d"},
            {"ignored_items_count", "Genegeerde items: %d"},
            {"ignored_currencies_count", "Genegeerde valuta: %d"},
            {"drf_logs", "DRF-logboek:"},
            {"last_100_entries", "(Laatste 100 vermeldingen)"},
            {"gw2_api_logs", "GW2 API-logboek:"},
            {"item_currency_details", "Item/Valuta-details (Eerste 5):"},
            {"item_label", "Item %d: %s (Aantal: "},
            {"loaded_label", ", Geladen: %s)"},
            {"currency_label", "Valuta %d: %s (Aantal: "},
            {"custom_profit_items", "Items met aangepaste winst (Eerste 5):"},
            {"custom_profit_item", "Item %d: %s (Aangepaste winst: "},
            {"no_custom_profit_items", "(Geen items met aangepaste winst)"},
            {"ignored_items_debug", "Genegeerde items (Eerste 5):"},
            {"yes_label", "Ja"},
            {"no_label", "Nee"},
            {"profits_label", "Winsten:"},
            {"profits_tooltip", "Totale winsten uit farming"},
            {"approx_profits_label", "Ongeveer winsten:"},
            {"approx_profits_tooltip", "Totale winst van MAX(Verkoper, TP Verkoop met 15% fee) of Aangepaste winst"},
            {"approx_gold_per_hour_label", "Ongeveer goud per uur:"},
            {"approx_gold_per_hour_tooltip", "Winst per uur gebaseerd op sessieduur"},
            {"trading_profits_label", "Handelswinsten:"},
            {"trading_profits_tooltip", "Winsten uit verkopen van items op Trading Post"},
            {"approx_trading_profits_listings_label", "Ongeveer handelswinsten (Lijsten):"},
            {"approx_trading_profits_listings_tooltip", "Totale winst als verkocht via TP-lijsten (15% fee afgetrokken)"},
            {"approx_trading_profits_instant_label", "Ongeveer handelswinsten (Directe verkoop):"},
            {"approx_trading_profits_instant_tooltip", "Totale winst als verkocht via TP directe kooporders (15% fee afgetrokken)"},
            {"trading_details_label", "Handelsdetails (Kans-kosten):"},
            {"trading_details_tooltip", "Winst verloren door niet te verkopen via TP-lijsten"},
            {"lost_profit_vs_tp_sell_label", "Verloren winst (vs TP Verkoop):"},
            {"lost_profit_vs_tp_sell_tooltip", "Kans-kosten: Winst verloren door niet te verkopen via TP (met 15% fee)"},
            {"lost_profit_per_hour_vs_tp_sell_label", "Verloren winst per uur (vs TP Verkoop):"},
            {"lost_profit_per_hour_vs_tp_sell_tooltip", "Kans-kosten per uur"},
            {"session_duration_debug_label", "Sessieduur: %s"},
            {"session_duration_debug_tooltip", "Huidige farming-sessieduur"},
            {"tab_items", "Items"},
            {"manage_ignored_items", "Beheer genegeerde items"},
            {"clear_all_ignored_items", "Verwijder alle genegeerde items"},
            {"unignore_item", "De-negeer item"},
            {"manage_favorite_items", "Beheer favoriete items"},
            {"favorite_items_label", "Favoriete items:"},
            {"clear_all_favorite_items", "Verwijder alle favoriete items"},
            {"tab_currencies", "Valuta's"},
            {"manage_ignored_currencies", "Beheer genegeerde valuta"},
            {"clear_all_ignored_currencies", "Verwijder alle genegeerde valuta"},
            {"unignore_currency", "De-negeer valuta"},
            {"manage_favorite_currencies", "Beheer favoriete valuta"},
            {"favorite_currencies_label", "Favoriete valuta:"},
            {"clear_all_favorite_currencies", "Verwijder alle favoriete valuta"},
            {"filter_active",   "Actief"},
            {"filter_inactive", "Inactief"},
            {"filter_all", "Alle"},
            {"filter_none", "Geen"},
            {"filter_reset_all", "Reset Alles"},
            {"filter_search_hint", "Zoek filter..."},
            {"filter_active_count", "%d filters actief"},
            {"sell_method_filters_label", "Verkoopmethode-filters:"},
            {"api_knowledge_filters_label", "API-kennis-filters:"},
            {"additional_filters_label", "Aanvullende filters:"},
            {"item_type_filters_label", "Itemtype-filters:"},
            {"currency_filters_label", "Valuta-filters:"},
            {"price_range", "Prijsbereik (Koper):"},
            {"quantity_range", "Hoeveelheidsbereik:"},
            {"debug_connection_status", "Verbindingsstatus"},
            {"debug_session_snapshot", "Sessie-snapshot"},
            {"debug_profit_breakdown", "Winstverdeling"},
            {"debug_data_state", "Gegevensstatus"},
            {"debug_logs", "Logboeken"},
            {"debug_favorites", "Favorieten"},
            {"debug_total_session", "totaal in deze sessie"},
            {"debug_after_tp_fee", "na 15% fee"},
            {"debug_direct_sell", "directe verkoop"},
            {"debug_rolling_avg", "rollend gemiddelde"},
            {"debug_vs_tp_sell", "vs TP verkoop"},
            {"debug_per_hour", "per uur"},
            {"settings_api_key", "API-sleutel"},
            {"settings_drf_token", "DRF Token"},
            {"debug_information", "Debug-informatie"},
            {"drf_status_label", "DRF-status: %s"},
            {"drf_status_tooltip", "Huidige DRF-verbindingsstatus"},
            {"drf_reconnect_count_label", "DRF-herverbindingsaantal: %d"},
            {"drf_reconnect_count_tooltip", "Aantal DRF-herverbindingspogingen"},
            {"gw2_api_status_label", "GW2 API-status: %s"},
            {"gw2_api_status_tooltip", "Huidige GW2 API-verbindingsstatus"},
            {"gw2_api_reconnect_count_label", "GW2 API-herverbindingsaantal: %d"},
            {"gw2_api_reconnect_count_tooltip", "Aantal GW2 API-herverbindingspogingen"},
            {"session_duration_debug", "Sessieduur"},
            {"session_duration_debug_tooltip", "Huidige farming-sessieduur"},
            {"gw2_process_memory_label", "GW2-procesgeheugen"},
            {"gw2_process_memory_tooltip", "Huidig GW2-procesgeheugengebruik"},
            {"gw2_api_request_count_label", "GW2 API-verzoekaantal"},
            {"gw2_api_request_count_tooltip", "Totaal aantal GW2 API-verzoeken gedaan"},
            {"ignored_items_debug_label", "Genegeerde items: %d"},
            {"ignored_items_debug_tooltip", "Aantal genegeerde items"},
            {"ignored_currencies_debug_label", "Genegeerde valuta: %d"},
            {"ignored_currencies_debug_tooltip", "Aantal genegeerde valuta"},
            {"drf_logs_label", "DRF-logboek:"},
            {"clear_drf_logs", "Wis DRF-logboek"},
            {"clear_drf_logs_tooltip", "Wis alle DRF-logboekvermeldingen"},
            {"last_100_entries", "(Laatste 100 vermeldingen)"},
            {"gw2_api_logs_label", "GW2 API-logboek:"},
            {"clear_gw2_logs", "Wis GW2-logboek"},
            {"clear_gw2_logs_tooltip", "Wis alle GW2 API-logboekvermeldingen"},
            {"settings_label", "Instellingen:"},
            {"api_key_tooltip", "GW2 API-sleutelstatus"},
            {"not_set", "Niet ingesteld"},
            {"set", "Ingesteld"},
            {"drf_token_tooltip", "DRF Token-status"},
            {"toggle_hotkey_label", "Toggle-sneltoets: %s"},
            {"toggle_hotkey_tooltip", "Hoofdvenster toggle-sneltoets"},
            {"auto_reset_mode_label", "Auto-reset-modus: %d"},
            {"auto_reset_mode_tooltip", "Huidige automatische reset-modus"},
            {"next_reset_label", "Volgende reset: %s"},
            {"next_reset_tooltip", "Volgende geplande reset-tijd (UTC)"},
            {"fake_drf_server_label", "Nep DRF-server:"},
            {"use_fake_drf_server", "Gebruik nep DRF-server"},
            {"use_fake_drf_server_tooltip", "Alleen voor testdoeleinden"},
            {"reset_all_data", "Reset alle data"},
            {"reset_all_data_tooltip", "Reset alle farming-data"},
            {"coin", "Munt"},
            {"info_button", "Info"},
            {"info_title", "FarmingTracker Info"},
            {"info_text", "Helptekst wordt hier later toegevoegd..."},
            {"close_button", "Sluiten"},
            {"rarity_label", "Zeldzaamheid: %s"},
            {"type_label", "Type: %d"},
            {"account_bound_label", "Account-gebonden: %s"},
            {"nosell_label", "NoSell: %s"},
            {"yes_label", "Ja"},
            {"no_label", "Nee"},
            {"sort_price_down", "Sorteer: Item-prijs omlaag"},
            {"sort_price_up", "Sorteer: Item-prijs omhoog"},
            {"sort_count_high", "Sorteer: |Aantal| hoog"},
            {"sort_count_low", "Sorteer: |Aantal| laag"},
            {"sort_name_az", "Sorteer: Naam A–Z"},
            {"sort_name_za", "Sorteer: Naam Z–A"},
            {"last_reset_label", "Reset"},
            {"last_reset_tooltip", "Tijd sinds laatste reset"},
            {"custom_profit_edit_tooltip",    "Bewerk winstwaarde"},
            {"custom_profit_confirm_tooltip", "Sla wijzigingen op"},
            {"accent_color", "Accent Kleur (Buttons, Tabblads, UI)"}
            {"accent_color_tooltip", "Accent color for buttons, tabs, and UI elements"}
            {"account_management", "Account Management"}
            {"account_prefix", "Account"}
            {"actions", "Actions"}
            {"add_account", "+ Toevoegen Account"}
            {"api_key_invalid_format", "(Invalid Format: 9 Blocks required)"}
            {"auto_reset_custom_days", "Aangepast (days)"}
            {"auto_reset_daily", "Daily reset (00:00 UTC)"}
            {"auto_reset_done_msg", "The tracker has been reset."}
            {"auto_reset_done_title", "Resetten Complete"}
            {"auto_reset_minutes_unload", "Minutes after last unload"}
            {"auto_reset_never", "Never (manual Resetten only)"}
            {"auto_reset_on_load", "Aan addon load"}
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
            {"backup_manual_only", "Handmatig only"}
            {"backup_weekly", "Weekly"}
            {"best_drop", "Beste Drop"}
            {"border_size", "Rand Grootte"}
            {"border_size_tooltip", "Adjust the thickness of rarity borders (1.0 - 10.0)"}
            {"bottom_gradient_color", "Bottom"}
            {"bottom_gradient_color_tooltip", "Bottom gradient color"}
            {"browse_for_file", "Browse for file..."}
            {"cancel", "Annuleren"}
            {"clear_all_custom_profits_warning", "Alle custom profit values will be deleted. This action cannot be undone."}
            {"clear_compare_selection", "Wissen selection"}
            {"clear_history", "Wissen Geschiedenis"}
            {"clear_history_confirm", "Wissen all session history?"}
            {"clear_history_tooltip", "Verwijderen all saved session history"}
            {"clear_history_warning", "This action cannot be undone!"}
            {"clear_search", "Wissen"}
            {"clear_search_favorites", "Wissen"}
            {"clear_search_tooltip", "Wissens the current search"}
            {"column_count", "Aantal"}
            {"column_currency", "Currency"}
            {"column_favorite", "Favorite"}
            {"column_icon", "Icoon"}
            {"column_ignore", "Ignore"}
            {"column_item", "Item"}
            {"column_label", "Label"}
            {"column_name", "Name"}
            {"column_profit", "Winst"}
            {"column_value", "Waarde"}
            {"comparison_previous_period", "Comparison with previous period:"}
            {"count_format", "Aantal: %lld"}
            {"create", "Create"}
            {"create_new_profile_tooltip", "Create a new profile with current settings"}
            {"create_tooltip", "Create a new profile with current settings"}
            {"currencies_header", "Valuta"}
            {"currency_cat_common", "Common"}
            {"currency_cat_fractal", "Fractals"}
            {"currency_cat_janthir", "Janthir Wilds"}
            {"currency_cat_map", "Map Valuta"}
            {"currency_cat_other", "Other"}
            {"currency_cat_pvp", "PvP"}
            {"currency_cat_raid_strike", "Raids & Strikes"}
            {"currency_cat_wvw", "WvW"}
            {"currency_group_by_category", "Groepeeren by category"}
            {"currency_group_by_category_tooltip", "Groepeeren currencies by category with collapsible sections or tabs"}
            {"currency_show_as_tabs", "Weergeven as tabs"}
            {"currency_show_as_tabs_tooltip", "Weergeven category groups as tabs instead of collapsible sections"}
            {"currency_table_favorite_tooltip", "Toevoegen/remove favorite. Favorites appear in the Favorites tab. Tip: Right-click the icon/name for more actions."}
            {"currency_table_ignore_tooltip", "Toevoegen/remove ignored. Ignored currencies appear in the Ignored tab. Tip: Right-click the icon/name for more actions."}
            {"date", "Datum"}
            {"debug_settings", "Debuggen Instellingen"}
            {"default_no_profile", "Standaard (Nee Profile)"}
            {"delete_profile", "Verwijderen Profile"}
            {"delete_profile_tooltip", "Verwijderen current profile"}
            {"details", "Details"}
            {"drops", "Drops"}
            {"duration", "Duur"}
            {"enable_automatic_backups", "Inschakelen automatic backups"}
            {"enable_automatic_backups_tooltip", "Automatically create backups before changes"}
            {"enable_best_drop_highlight", "Highlight Beste Drop"}
            {"enable_best_drop_highlight_tooltip", "Highlight the most valuable drop with a golden border in the Items tab"}
            {"enable_best_drop_in_mini_window", "Weergeven Beste Drop in Mini Venster"}
            {"enable_best_drop_in_mini_window_tooltip", "Weergeven the most valuable drop in the mini window overlay"}
            {"enable_debug_tab", "Inschakelen Debuggen Tabblad"}
            {"enable_debug_tab_tooltip", "Weergevens the debug tab with additional information"}
            {"enable_favorite_row_color", "Inschakelen favorite row color"}
            {"enable_favorite_row_color_tooltip", "Highlights favorite items/currencies with custom row background color"}
            {"enable_favorite_text_color", "Inschakelen favorite text color"}
            {"enable_favorite_text_color_tooltip", "Highlights favorite items/currencies with custom text color"}
            {"enable_favorites", "Inschakelen Favorites"}
            {"enable_favorites_tab", "Inschakelen Favorites Tabblad"}
            {"enable_favorites_tab_tooltip", "Weergevens a separate favorites tab"}
            {"enable_grid_view_currencies", "Inschakelen Grid View (Valuta)"}
            {"enable_grid_view_currencies_tooltip", "Toggle between list and grid view in Valuta tab"}
            {"enable_grid_view_items", "Inschakelen Grid View (Items)"}
            {"enable_grid_view_items_tooltip", "Toggle between list and grid view in Items tab"}
            {"enable_icon_cache", "Inschakelen Icoon Cache"}
            {"enable_icon_cache_tooltip", "Cache item icons on disk to speed up loading after the first session"}
            {"enable_notifications", "Inschakelen notifications"}
            {"enable_notifications_tooltip", "Inschakelen in-game notifications"}
            {"enable_session_history", "Inschakelen Sessie Geschiedenis"}
            {"enable_session_history_tooltip", "Opslaan farming session history for later viewing"}
            {"enable_session_timeline", "Inschakelen Sessie Tijdlijn"}
            {"enable_session_timeline_tooltip", "Opslaan detailed drop timeline with timestamps for session details"}
            {"export_history", "Exporteren Geschiedenis"}
            {"export_history_tooltip", "Exporteren session history to a JSON file"}
            {"export_logs", "Exporteren Logs"}
            {"favorite_items_header", "Favorite Items"}
            {"favorites_first", "Favorites First"}
            {"favorites_first_tooltip", "Weergevens favorites first in item/currency lists"}
            {"favorites_settings", "Favorites Instellingen"}
            {"filter_account_bound", "Account-bound"}
            {"filter_account_bound_tooltip", "Weergeven account-bound items"}
            {"filter_aether_rich_sap", "Aether-Rich Sap"}
            {"filter_aether_rich_sap_tooltip", "Weergeven aether-rich sap currency"}
            {"filter_airship_parts", "Airship Parts"}
            {"filter_airship_parts_tooltip", "Weergeven airship parts currency"}
            {"filter_ancient_coin", "Ancient Coin"}
            {"filter_ancient_coin_tooltip", "Weergeven ancient coin currency"}
            {"filter_antiquated_ducat", "Antiquated Ducat"}
            {"filter_antiquated_ducat_tooltip", "Weergeven antiquated ducat currency"}
            {"filter_ascended_shards_of_glory", "Ascended Shards of Glory"}
            {"filter_ascended_shards_of_glory_tooltip", "Weergeven ascended shards of glory currency"}
            {"filter_astral_acclaim", "Astral Acclaim"}
            {"filter_astral_acclaim_tooltip", "Weergeven astral acclaim currency"}
            {"filter_badge_of_honor", "Badge of Honor"}
            {"filter_badge_of_honor_tooltip", "Weergeven badge of honor currency"}
            {"filter_calcified_gasp", "Calcified Gasp"}
            {"filter_calcified_gasp_tooltip", "Weergeven calcified gasp currency"}
            {"filter_canach_coins", "Canach Coins"}
            {"filter_canach_coins_tooltip", "Weergeven canach coins currency"}
            {"filter_custom_profit", "Has custom profit"}
            {"filter_custom_profit_tooltip", "Weergeven items with custom profit set"}
            {"filter_elegy_mosaic", "Elegy Mosaic"}
            {"filter_elegy_mosaic_tooltip", "Weergeven elegy mosaic currency"}
            {"filter_favorite", "Favorite"}
            {"filter_favorite_tooltip", "Weergeven favorite items (outside Favorites tab)"}
            {"filter_fractal_relic", "Fractal Relic"}
            {"filter_fractal_relic_tooltip", "Weergeven fractal relic currency"}
            {"filter_gaeting_crystal_janthir", "Gaeting Crystal (Janthir)"}
            {"filter_gaeting_crystal_janthir_tooltip", "Weergeven gaeting crystal (janthir) currency"}
            {"filter_gaeting_crystals", "Gaeting Crystals"}
            {"filter_gaeting_crystals_tooltip", "Weergeven gaeting crystals currency"}
            {"filter_gem", "Gem"}
            {"filter_gem_tooltip", "Weergeven gem currency"}
            {"filter_geode", "Geode"}
            {"filter_geode_tooltip", "Weergeven geode currency"}
            {"filter_green_prophet_shards", "Green Prophet Shards"}
            {"filter_green_prophet_shards_tooltip", "Weergeven green prophet shards currency"}
            {"filter_guild_commendation", "Guild Commendation"}
            {"filter_guild_commendation_tooltip", "Weergeven guild commendation currency"}
            {"filter_ignored", "Ignored"}
            {"filter_ignored_tooltip", "Weergeven ignored items"}
            {"filter_imperial_favor", "Imperial Favor"}
            {"filter_imperial_favor_tooltip", "Weergeven imperial favor currency"}
            {"filter_items", "Filter Items"}
            {"filter_jade_sliver", "Jade Sliver"}
            {"filter_jade_sliver_tooltip", "Weergeven jade sliver currency"}
            {"filter_karma_tooltip", "Weergeven karma currency"}
            {"filter_known_by_api", "Known by API"}
            {"filter_known_by_api_tooltip", "Weergeven items known by GW2 API"}
            {"filter_laurel", "Laurel"}
            {"filter_laurel_tooltip", "Weergeven laurel currency"}
            {"filter_legendary_insight", "Legendary Insight"}
            {"filter_legendary_insight_tooltip", "Weergeven legendary insight currency"}
            {"filter_ley_line_crystals", "Ley-Line Crystals"}
            {"filter_ley_line_crystals_tooltip", "Weergeven ley-line crystals currency"}
            {"filter_magnetite_shards", "Magnetite Shards"}
            {"filter_magnetite_shards_tooltip", "Weergeven magnetite shards currency"}
            {"filter_max_price_tooltip", "Maximum price filter (0 = disabled)"}
            {"filter_max_quantity_tooltip", "Maximum quantity filter (0 = disabled)"}
            {"filter_min_price_tooltip", "Minimum price filter (0 = disabled)"}
            {"filter_min_quantity_tooltip", "Minimum quantity filter (0 = disabled)"}
            {"filter_nosell", "NeeSell"}
            {"filter_nosell_tooltip", "Weergeven NeeSell items"}
            {"filter_not_account_bound", "Neet Account-bound"}
            {"filter_not_account_bound_tooltip", "Weergeven non-account-bound items"}
            {"filter_not_favorite", "Neet Favorite"}
            {"filter_not_favorite_tooltip", "Weergeven items that are not marked as favorite"}
            {"filter_not_ignored", "Neet Ignored"}
            {"filter_not_ignored_tooltip", "Weergeven non-ignored items"}
            {"filter_not_nosell", "Neet NeeSell"}
            {"filter_not_nosell_tooltip", "Weergeven sellable items"}
            {"filter_pinch_of_stardust", "Pinch of Stardust"}
            {"filter_pinch_of_stardust_tooltip", "Weergeven pinch of stardust currency"}
            {"filter_pristine_fractal_relics", "Pristine Fractal Relics"}
            {"filter_pristine_fractal_relics_tooltip", "Weergeven pristine fractal relics currency"}
            {"filter_proofs_of_heroics", "Proofs of Heroics"}
            {"filter_proofs_of_heroics_tooltip", "Weergeven proofs of heroics currency"}
            {"filter_prophet_shards", "Prophet Shards"}
            {"filter_prophet_shards_tooltip", "Weergeven prophet shards currency"}
            {"filter_pvp_league_tickets", "PvP League Tickets"}
            {"filter_pvp_league_tickets_tooltip", "Weergeven PvP league tickets currency"}
            {"filter_rarity", "Filter Rarity"}
            {"filter_research_notes", "Research Neetes"}
            {"filter_research_notes_tooltip", "Weergeven research notes currency"}
            {"filter_sellable_on_tp", "Sellable on TP"}
            {"filter_sellable_on_tp_tooltip", "Weergeven items sellable on Trading Post"}
            {"filter_sellable_to_vendor", "Sellable to vendor"}
            {"filter_sellable_to_vendor_tooltip", "Weergeven items sellable to vendor"}
            {"filter_spirit_shards", "Spirit Shards"}
            {"filter_spirit_shards_tooltip", "Weergeven spirit shards currency"}
            {"filter_static_charge", "Static Charge"}
            {"filter_static_charge_tooltip", "Weergeven static charge currency"}
            {"filter_tales_of_dungeon_delving", "Tales of Dungeon Delving"}
            {"filter_tales_of_dungeon_delving_tooltip", "Weergeven tales of dungeon delving currency"}
            {"filter_testimony_of_castoran_heroics", "Testenimony of Castoran Heroics"}
            {"filter_testimony_of_castoran_heroics_tooltip", "Weergeven testimony of castoran heroics currency"}
            {"filter_testimony_of_desert_heroics", "Testenimony of Desert Heroics"}
            {"filter_testimony_of_desert_heroics_tooltip", "Weergeven testimony of desert heroics currency"}
            {"filter_testimony_of_jade_heroics", "Testenimony of Jade Heroics"}
            {"filter_testimony_of_jade_heroics_tooltip", "Weergeven testimony of jade heroics currency"}
            {"filter_trade_contracts", "Trade Contracts"}
            {"filter_trade_contracts_tooltip", "Weergeven trade contracts currency"}
            {"filter_transmutation_charge", "Transmutation Charge"}
            {"filter_transmutation_charge_tooltip", "Weergeven transmutation charge currency"}
            {"filter_type_armor", "Armor"}
            {"filter_type_armor_tooltip", "Weergeven armor items"}
            {"filter_type_backpack", "Backpack"}
            {"filter_type_backpack_tooltip", "Weergeven backpack items"}
            {"filter_type_bag", "Bag"}
            {"filter_type_bag_tooltip", "Weergeven bags"}
            {"filter_type_consumable", "Consumable"}
            {"filter_type_consumable_tooltip", "Weergeven consumable items"}
            {"filter_type_container", "Container"}
            {"filter_type_container_tooltip", "Weergeven containers"}
            {"filter_type_crafting_material", "Crafting Material"}
            {"filter_type_crafting_material_tooltip", "Weergeven crafting materials"}
            {"filter_type_gathering_tool", "Gathering Tool"}
            {"filter_type_gathering_tool_tooltip", "Weergeven gathering tools"}
            {"filter_type_gizmo", "Gizmo"}
            {"filter_type_gizmo_container", "Gizmo Container"}
            {"filter_type_gizmo_container_tooltip", "Weergeven gizmo container items"}
            {"filter_type_gizmo_tooltip", "Weergeven gizmo items"}
            {"filter_type_mini_pet", "Mini Pet"}
            {"filter_type_mini_pet_tooltip", "Weergeven mini pets"}
            {"filter_type_tool", "Tool"}
            {"filter_type_tool_tooltip", "Weergeven tool items"}
            {"filter_type_trinket", "Trinket"}
            {"filter_type_trinket_tooltip", "Weergeven trinket items"}
            {"filter_type_trophy", "Trophy"}
            {"filter_type_trophy_tooltip", "Weergeven trophy items"}
            {"filter_type_unlock", "Ontgrendelen"}
            {"filter_type_unlock_tooltip", "Weergeven unlock items"}
            {"filter_type_upgrade_component", "Upgrade Component"}
            {"filter_type_upgrade_component_tooltip", "Weergeven upgrade components"}
            {"filter_type_weapon", "Weapon"}
            {"filter_type_weapon_tooltip", "Weergeven weapon items"}
            {"filter_tyrian_defense_seal", "Tyrian Defense Seal"}
            {"filter_tyrian_defense_seal_tooltip", "Weergeven tyrian defense seal currency"}
            {"filter_unbound_magic", "Unbound Magic"}
            {"filter_unbound_magic_tooltip", "Weergeven unbound magic currency"}
            {"filter_uncommon_coins", "Uncommon Coins"}
            {"filter_uncommon_coins_tooltip", "Weergeven uncommon coins currency"}
            {"filter_unknown_by_api", "Unknown by API"}
            {"filter_unknown_by_api_tooltip", "Weergeven items not known by GW2 API"}
            {"filter_unstable_fractal_essence", "Unstable Fractal Essence"}
            {"filter_unstable_fractal_essence_tooltip", "Weergeven unstable fractal essence currency"}
            {"filter_unusual_coin", "Unusual Coin"}
            {"filter_unusual_coin_tooltip", "Weergeven unusual coin currency"}
            {"filter_ursus_oblige", "Ursus Oblige"}
            {"filter_ursus_oblige_tooltip", "Weergeven ursus oblige currency"}
            {"filter_volatile_magic", "Volatile Magic"}
            {"filter_volatile_magic_tooltip", "Weergeven volatile magic currency"}
            {"filter_wvw_skirmish_tickets", "WvW Skirmish Tickets"}
            {"filter_wvw_skirmish_tickets_tooltip", "Weergeven WvW skirmish tickets currency"}
            {"first_5_custom_profit", "First 5 items with custom profit set"}
            {"first_5_custom_profit_tooltip", "First 5 items with custom profit set"}
            {"first_5_ignored_items", "First 5 ignored items"}
            {"first_5_ignored_items_tooltip", "First 5 ignored items"}
            {"first_5_tracked_items", "First 5 tracked items and currencies with details"}
            {"first_5_tracked_items_tooltip", "First 5 tracked items and currencies with details"}
            {"full_backup", "Full Backup"}
            {"full_backup_tooltip", "Backup all data (settings, session history, favorites, ignored items, custom profit) to a JSON file"}
            {"full_restore", "Full Herstellen"}
            {"full_restore_tooltip", "Herstellen all data from a backup JSON file"}
            {"general_settings", "Algemeen Instellingen"}
            {"gold_format", "Gold: %lld"}
            {"gradient_backgrounds", "Gradient backgrounds"}
            {"gradient_backgrounds_tooltip", "Inschakelens smooth gradient backgrounds for a more modern look"}
            {"grid_icon_size_currencies", "Grid Icoon size (Valuta)"}
            {"grid_icon_size_currencies_tooltip", "Grootte of icons in grid view for Valuta (16-128)"}
            {"grid_icon_size_items", "Grid Icoon size (Items)"}
            {"grid_icon_size_items_tooltip", "Grootte of icons in grid view for Items (16-128)"}
            {"group_by_rarity", "Groepeeren by Rarity"}
            {"group_by_type", "Groepeeren by Category"}
            {"icon_cache_max_icons", "Max Cached Icoons"}
            {"icon_cache_max_icons_tooltip", "Maximum number of icons to keep in cache (older icons are deleted when limit is reached)"}
            {"icon_size", "Icoon size"}
            {"icon_size_tooltip", "Grootte of item icons in pixels (16-96)"}
            {"import", "Importeren"}
            {"import_history", "Importeren Geschiedenis"}
            {"import_history_tooltip", "Importeren session history from a JSON file"}
            {"import_tooltip", "Importeren settings from a JSON file"}
            {"infusion_drop_label", "Infusion Drop!"}
            {"item", "Item"}
            {"items_header", "Items"}
            {"magic_find_abbreviation", "MF: %d%%"}
            {"main_window_click_through", "Click through"}
            {"main_window_click_through_tooltip", "Alleows clicking through the main window to the game"}
            {"main_window_opacity", "Main Venster Transparency"}
            {"main_window_opacity_tooltip", "Main window background transparency (0-100%)"}
            {"main_window_settings", "Main Venster"}
            {"map", "Map"}
            {"mass_actions_clear_ignore", "Wissen ignore list"}
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
            {"max_session_history", "Max Sessies"}
            {"max_session_history_tooltip", "Maximum number of sessions to save (1-50). Oldest session is deleted when limit is reached if overwrite is enabled."}
            {"min_value", "Min Waarde"}
            {"mini_window_click_through", "Venster click through"}
            {"mini_window_click_through_tooltip", "Alleows clicking through the mini window to the game"}
            {"mini_window_hide_title_bar", "Verbergen Mini Venster Title Bar"}
            {"mini_window_hide_title_bar_tooltip", "Verbergen the title bar of the mini window"}
            {"mini_window_locked", "Vergrendelen Mini Venster"}
            {"mini_window_locked_tooltip", "Fix the mini window position and size (no longer movable or resizable)"}
            {"mini_window_opacity", "Mini Venster Transparency"}
            {"mini_window_opacity_tooltip", "Mini window background transparency (0-100%)"}
            {"mini_window_show_profit", "Weergeven Winst"}
            {"mini_window_show_profit_per_hour", "Weergeven Winst/Hour"}
            {"mini_window_show_profit_per_hour_tooltip", "Display profit per hour in mini window"}
            {"mini_window_show_profit_tooltip", "Display total profit in mini window"}
            {"mini_window_show_session_duration", "Weergeven Sessie Duur"}
            {"mini_window_show_session_duration_tooltip", "Display session duration in mini window"}
            {"mini_window_show_total_items", "Weergeven Totaal Items"}
            {"mini_window_show_total_items_tooltip", "Display total item count in mini window"}
            {"mini_window_show_tp_instant", "Weergeven TP Instant (Instant Sell)"}
            {"mini_window_show_tp_instant_tooltip", "Display TP instant sell profit in mini window"}
            {"mini_window_show_tp_sell", "Weergeven TP Sell (Listings)"}
            {"mini_window_show_tp_sell_tooltip", "Display TP sell profit (listings) in mini window"}
            {"minutes_after_unload_tooltip", "Minutes after addon unload before automatic reset"}
            {"no_cancel", "Nee, Annuleren"}
            {"no_items_in_session", "Nee items in this session"}
            {"no_sessions_recorded", "Nee sessions recorded yet."}
            {"notification_combine_logic", "Combine Filters (AND)"}
            {"notification_combine_logic_tooltip", "If enabled, BOTH conditions (Waarde AND Rarity) must be met. If disabled, ANY one of them is enough."}
            {"notification_duration", "Display Duur"}
            {"notification_duration_tooltip", "How long the notification stays visible (seconds)"}
            {"notification_general", "Algemeen Instellingen"}
            {"notification_include_agony", "Include Agony Infusions"}
            {"notification_include_agony_tooltip", "If enabled, Agony Infusions (+1 to +30) will also trigger an alert."}
            {"notification_include_non_profit", "Include Neen-Winst Items"}
            {"notification_include_non_profit_tooltip", "If enabled, items with no gold value (0c) will also trigger alerts if they meet the rarity requirement."}
            {"notification_infusion_alert", "Infusion Alert"}
            {"notification_infusion_alert_tooltip", "Always notify when an Infusion is found (ignores Waarde/Rarity filters)"}
            {"notification_item_alerts", "Item Alerts"}
            {"notification_min_rarity", "Min. Rarity"}
            {"notification_min_rarity_tooltip", "Trigger notification if item rarity is at least this level"}
            {"notification_min_value", "Min. Waarde (Gold)"}
            {"notification_min_value_tooltip", "Trigger notification if item value is at least this amount"}
            {"notification_play_sound", "Afspelen Geluid"}
            {"notification_play_sound_tooltip", "Afspelen a sound effect when a notification appears"}
            {"notification_precursor_alert", "Pre-Cursor Alert"}
            {"notification_precursor_alert_tooltip", "Always notify when a Pre-Cursor is found (ignores Waarde/Rarity filters)"}
            {"notification_session_alerts", "Progress & Tijd"}
            {"notification_settings", "Neetificatie Instellingen"}
            {"notification_setup_hint", "[Drag to reposition notifications]"}
            {"notification_stacking", "Stack Neetificaties"}
            {"notification_stacking_tooltip", "Weergeven multiple notifications at once instead of replacing the old one immediately"}
            {"notification_triggers", "Neetificatie Triggers"}
            {"notification_volume", "Master Volume"}
            {"notification_volume_tooltip", "Volume for notification sounds"}
            {"notify_profit_goal", "Neetify when profit goal reached"}
            {"notify_profit_goal_tooltip", "Neetify when you reach your profit goal"}
            {"notify_reset_warning", "Neetify before reset"}
            {"notify_reset_warning_tooltip", "Neetify before automatic reset occurs"}
            {"notify_session_complete", "Neetify after session duration"}
            {"notify_session_complete_tooltip", "Neetify after farming for a certain duration"}
            {"opportunity_cost_per_hour", "Opportunity cost per hour"}
            {"opportunity_cost_per_hour_tooltip", "Opportunity cost per hour"}
            {"opportunity_cost_vs_tp_sell", "Opportunity cost vs TP sell"}
            {"opportunity_cost_vs_tp_sell_tooltip", "Opportunity cost vs TP sell"}
            {"overwrite_session_history", "Overwrite Sessies"}
            {"overwrite_session_history_tooltip", "If enabled, oldest session is deleted when limit is reached"}
            {"performance_settings", "Performance Instellingen"}
            {"precursor_drop_label", "Pre-Cursor Drop!"}
            {"profit_change", "Winst Change"}
            {"profit_goal_amount", "Goal Amount (Gold)"}
            {"profit_goal_gold", "Winst goal (gold)"}
            {"profit_goal_gold_tooltip", "Winst goal in gold coins (1-1000)"}
            {"profit_goal_reached_msg", "You have reached your profit goal of %d gold!"}
            {"profit_goal_reached_title", "Winst Goal Reached"}
            {"profit_per_hour_calculation", "Winst per hour calculation"}
            {"profit_per_hour_calculation_tooltip", "Winst per hour calculation"}
            {"quantity", "Quantity"}
            {"range_filters_tooltip", "Weergeven price and quantity range filters"}
            {"rare_drop_label", "Rare Drop!"}
            {"rarity_border_thickness", "Rarity Rand Thickness"}
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
            {"remove_account", "- Verwijderen Account"}
            {"reset_all", "Resetten Alle"}
            {"reset_all_tooltip", "Resetten all settings to default values"}
            {"reset_interval_days", "Resetten interval (days)"}
            {"reset_interval_days_tooltip", "Aangepast reset interval in days (1-30 days)"}
            {"reset_settings", "Auto Resetten"}
            {"reset_warning_minutes", "Resetten Waarschuwing (Minutes)"}
            {"reset_warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"reset_warning_msg", "The tracker will reset in %d minutes!"}
            {"reset_warning_title", "Resetten Waarschuwing"}
            {"restore", "Herstellen"}
            {"row_color", "Row Kleur"}
            {"save", "Opslaan"}
            {"save_account", "Opslaan Account"}
            {"save_all_items_confirm", "Inschakelen session timeline?"}
            {"save_all_items_warning", "This will significantly increase file size!"}
            {"save_current_session", "Opslaan Current Sessie"}
            {"save_current_session_tooltip", "Opslaan the current farming session without resetting"}
            {"save_tooltip", "Opslaan current settings"}
            {"search_favorite_currencies_hint", "Zoeken favorite currencies..."}
            {"search_favorite_items_hint", "Zoeken favorite items..."}
            {"search_items", "Zoeken Items"}
            {"search_items_hint", "Zoeken items..."}
            {"select_profile", "Selecteren a profile to apply its settings"}
            {"select_profile_tooltip", "Selecteren a profile to apply its settings"}
            {"session_complete_hours", "Sessie Complete (Hours)"}
            {"session_complete_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_complete_msg", "You have been farming for %d hours!"}
            {"session_complete_title", "Sessie Complete"}
            {"session_count", "Sessie Aantal"}
            {"session_details", "Sessie Details"}
            {"session_history", "Sessie Geschiedenis"}
            {"session_hours", "Sessie hours"}
            {"session_hours_tooltip", "Hours of farming before notification (1-24)"}
            {"session_note", "Neete"}
            {"session_profit_trend", "Winst Trend"}
            {"session_search_hint", "Zoeken sessions, items, notes..."}
            {"sessions_selected", "sessions selected"}
            {"sessions_stored", "Sessies Stored"}
            {"settings_profiles", "Instellingen Profiles"}
            {"show_ignored_items", "Weergeven ignored items"}
            {"show_ignored_items_tooltip", "Weergeven ignored items/currencies in Items and Valuta tabs (disable to hide). Difference from 'Ignored' filter: This filter controls display in Items/Valuta tabs, the 'Ignored' filter controls display in Filter tab."}
            {"show_main_window", "Weergeven main window"}
            {"show_mini_window", "Weergeven mini window"}
            {"show_mini_window_tooltip", "Weergevens a small overlay widget with key statistics"}
            {"show_notification_setup", "Setup Mode (Positioning)"}
            {"show_notification_setup_tooltip", "Makes the notification window visible so you can move it"}
            {"show_rarity_as_tabs", "Weergeven as Tabblads"}
            {"show_summaries", "Weergeven Summaries"}
            {"show_summaries_tooltip", "Weergeven daily/weekly/monthly profit summaries"}
            {"show_type_as_tabs", "Weergeven as Tabblads"}
            {"showing", "Weergevening"}
            {"sort_profit_high", "Sorteren: Winst high"}
            {"sort_profit_low", "Sorteren: Winst low"}
            {"sort_rarity_high", "Sorteren: Rarity high to low"}
            {"sort_rarity_low", "Sorteren: Rarity low to high"}
            {"sound_alert", "Alert Geluid"}
            {"sound_infusion", "Infusion Geluid"}
            {"sound_path_hint", "Path to sound file (empty = default)"}
            {"sound_precursor", "Pre-Cursor Geluid"}
            {"sound_standard", "Standard Geluid"}
            {"sound_test", "Testen"}
            {"stat_avg_profit_per_hour", "Avg Winst/h"}
            {"stat_best_session", "Beste Sessie"}
            {"stat_total_profit", "Totaal Winst"}
            {"stat_total_time", "Totaal Tijd"}
            {"summaries_coming_soon", "Summaries feature coming soon..."}
            {"summaries_label", "Summaries"}
            {"summaries_tooltip", "Daily/Weekly/Monthly profit summaries"}
            {"summary_period", "Period:"}
            {"summary_this_month", "This Month"}
            {"summary_this_week", "This Week"}
            {"summary_today", "Today"}
            {"tab_session_history", "Sessie Geschiedenis"}
            {"tab_sessions", "Sessies"}
            {"tab_summaries", "Summaries"}
            {"test_item_label", "Testen Item"}
            {"text_color", "Text Kleur"}
            {"time", "Tijd"}
            {"time_ago_seconds", "%llds ago"}
            {"timeline_icon_size_currencies", "Tijdlijn Icoon Grootte (Valuta)"}
            {"timeline_icon_size_currencies_tooltip", "Grootte of currency icons in Tijdlijn tab (16-48)"}
            {"timeline_icon_size_items", "Tijdlijn Icoon Grootte (Items)"}
            {"timeline_icon_size_items_tooltip", "Grootte of item icons in Tijdlijn tab (16-96)"}
            {"toggle_favorite", "Toggle favorite"}
            {"toggle_favorite_tooltip", "Toggle favorite"}
            {"toggle_ignore", "Toggle ignore"}
            {"toggle_ignore_tooltip", "Toggle ignore"}
            {"top_currencies_count_header", "Top 5 Valuta (Aantal)"}
            {"top_currencies_count_tooltip", "Top 5 currencies by count"}
            {"top_drops", "Top Drops"}
            {"top_gradient_color", "Top"}
            {"top_gradient_color_tooltip", "Top gradient color"}
            {"top_items_count_header", "Top 5 Items (Aantal)"}
            {"top_items_profit_header", "Top 5 Items by Winst"}
            {"total_custom_profit", "Totaal custom profit"}
            {"total_custom_profit_tooltip", "Totaal custom profit"}
            {"total_drops", "Totaal Drops"}
            {"total_duration", "Totaal Duur"}
            {"total_tp_sell_profit", "Totaal TP sell profit"}
            {"total_tp_sell_profit_tooltip", "Totaal TP sell profit"}
            {"total_tracked_currencies", "Totaal number of tracked currencies"}
            {"total_tracked_currencies_tooltip", "Totaal number of tracked currencies"}
            {"total_tracked_items", "Totaal number of tracked items"}
            {"total_tracked_items_tooltip", "Totaal number of tracked items"}
            {"total_vendor_sell_profit", "Totaal vendor sell profit"}
            {"total_vendor_sell_profit_tooltip", "Totaal vendor sell profit"}
            {"tp_buy_gross_format", "TP Buy (Gross): %s"}
            {"tp_buy_net_format", "TP Buy (Net): %s"}
            {"tp_sell_gross_format", "TP Sell (Gross): %s"}
            {"tp_sell_net_format", "TP Sell (Net): %s"}
            {"trigger_drops", "Item Drops"}
            {"trigger_profit_goal", "Winst Goal"}
            {"trigger_time_reset", "Tijd & Resetten"}
            {"type_backpack", "Backpack"}
            {"type_gizmo_container", "Gizmo Container"}
            {"type_tool", "Tool"}
            {"type_trophy", "Trophy"}
            {"type_unlock", "Ontgrendelen"}
            {"type_upgrade_component", "Upgrade Component"}
            {"unknown_map", "Unknown"}
            {"update_profile", "Update Profile"}
            {"update_profile_tooltip", "Update current profile with current settings"}
            {"value", "Waarde"}
            {"vendor_value_format", "Vendor Waarde: %s"}
            {"visual_settings", "Visual Instellingen"}
            {"warning_minutes", "Waarschuwing minutes"}
            {"warning_minutes_tooltip", "Minutes before reset to show warning (1-60)"}
            {"yes_clear", "Ja, Wissen"}
            {"yes_enable", "Ja, Inschakelen"}
            {"yes_reset", "Ja, Resetten"},

            // Drops Tab
            {"settings_tab", "Instellingen"},
        
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
