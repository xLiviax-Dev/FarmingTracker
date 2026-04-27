#include "ui_filter.h"
#include "settings.h"
#include "localization.h"

namespace UIFilter
{
void RenderFilterTab()
{
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("sell_method_filters_label"));
    if (ImGui::Checkbox(Localization::GetText("filter_sellable_to_vendor"), &g_Settings.filterSellableToVendor))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_sellable_to_vendor_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_sellable_on_tp"), &g_Settings.filterSellableOnTp))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_sellable_on_tp_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_custom_profit"), &g_Settings.filterCustomProfit))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_custom_profit_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("api_knowledge_filters_label"));
    if (ImGui::Checkbox(Localization::GetText("filter_known_by_api"), &g_Settings.filterKnownByApi))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_known_by_api_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_unknown_by_api"), &g_Settings.filterUnknownByApi))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_unknown_by_api_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("additional_filters_label"));
    if (ImGui::Checkbox(Localization::GetText("show_ignored_items"), &g_Settings.showIgnoredItems))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("show_ignored_items_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("item_type_filters_label"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_armor"), &g_Settings.filterTypeArmor))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_armor_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_backpack"), &g_Settings.filterTypeBackpack))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_backpack_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_bag"), &g_Settings.filterTypeBag))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_bag_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_container"), &g_Settings.filterTypeContainer))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_container_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_consumable"), &g_Settings.filterTypeConsumable))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_consumable_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_crafting_material"), &g_Settings.filterTypeCraftingMaterial))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_crafting_material_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_gathering_tool"), &g_Settings.filterTypeGatheringTool))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_gathering_tool_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_gizmo"), &g_Settings.filterTypeGizmo))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_gizmo_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_gizmo_container"), &g_Settings.filterTypeGizmoContainer))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_gizmo_container_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_mini_pet"), &g_Settings.filterTypeMiniPet))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_mini_pet_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_tool"), &g_Settings.filterTypeTool))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_tool_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_trinket"), &g_Settings.filterTypeTrinket))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_trinket_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_trophy"), &g_Settings.filterTypeTrophy))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_trophy_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_unlock"), &g_Settings.filterTypeUnlock))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_unlock_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_type_upgrade_component"), &g_Settings.filterTypeUpgradeComponent))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_upgrade_component_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_type_weapon"), &g_Settings.filterTypeWeapon))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_type_weapon_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("currency_filters_label"));

    // Main Currencies (General)
    if (ImGui::CollapsingHeader(Localization::GetText("currency_main")))
    {
        if (ImGui::Checkbox(Localization::GetText("filter_gem"), &g_Settings.filterGem))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_gem_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_karma"), &g_Settings.filterKarma))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_karma_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_laurel"), &g_Settings.filterLaurel))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_laurel_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_spirit_shards"), &g_Settings.filterSpiritShards))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_spirit_shards_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_transmutation_charge"), &g_Settings.filterTransmutationCharge))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_transmutation_charge_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_research_notes"), &g_Settings.filterResearchNotes))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_research_notes_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_astral_acclaim"), &g_Settings.filterAstralAcclaim))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_astral_acclaim_tooltip"));
    }

    // WvW/PvP Currencies (Competitive)
    if (ImGui::CollapsingHeader(Localization::GetText("currency_wvw_pvp")))
    {
        if (ImGui::Checkbox(Localization::GetText("filter_ascended_shards_of_glory"), &g_Settings.filterAscendedShardsOfGlory))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_ascended_shards_of_glory_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_badge_of_honor"), &g_Settings.filterBadgeOfHonor))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_badge_of_honor_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_proofs_of_heroics"), &g_Settings.filterProofsOfHeroics))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_proofs_of_heroics_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_pvp_league_tickets"), &g_Settings.filterPvpLeagueTickets))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_pvp_league_tickets_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_wvw_skirmish_tickets"), &g_Settings.filterWvWSkirmishTickets))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_wvw_skirmish_tickets_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_testimony_of_castoran_heroics"), &g_Settings.filterTestimonyOfCastoranHeroics))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_testimony_of_castoran_heroics_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_testimony_of_desert_heroics"), &g_Settings.filterTestimonyOfDesertHeroics))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_testimony_of_desert_heroics_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_testimony_of_jade_heroics"), &g_Settings.filterTestimonyOfJadeHeroics))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_testimony_of_jade_heroics_tooltip"));
    }

    // Fractals/Raids/Dungeons Currencies (Instanced Group Content)
    if (ImGui::CollapsingHeader(Localization::GetText("currency_fractal")))
    {
        if (ImGui::Checkbox(Localization::GetText("filter_fractal_relic"), &g_Settings.filterFractalRelic))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_fractal_relic_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_pristine_fractal_relics"), &g_Settings.filterPristineFractalRelics))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_pristine_fractal_relics_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_unstable_fractal_essence"), &g_Settings.filterUnstableFractalEssence))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_unstable_fractal_essence_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_magnetite_shards"), &g_Settings.filterMagnetiteShards))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_magnetite_shards_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_gaeting_crystals"), &g_Settings.filterGaetingCrystals))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_gaeting_crystals_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_legendary_insight"), &g_Settings.filterLegendaryInsight))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_legendary_insight_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_tales_of_dungeon_delving"), &g_Settings.filterTalesOfDungeonDelving))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_tales_of_dungeon_delving_tooltip"));
    }

    // Map-specific Currencies
    if (ImGui::CollapsingHeader(Localization::GetText("currency_map")))
    {
        if (ImGui::Checkbox(Localization::GetText("filter_airship_parts"), &g_Settings.filterAirshipParts))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_airship_parts_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_aether_rich_sap"), &g_Settings.filterAetherRichSap))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_aether_rich_sap_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_antiquated_ducat"), &g_Settings.filterAntiquatedDucat))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_antiquated_ducat_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_calcified_gasp"), &g_Settings.filterCalcifiedGasp))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_calcified_gasp_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_elegy_mosaic"), &g_Settings.filterElegyMosaic))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_elegy_mosaic_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_gaeting_crystal_janthir"), &g_Settings.filterGaetingCrystalJanthir))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_gaeting_crystal_janthir_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_geode"), &g_Settings.filterGeode))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_geode_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_guild_commendation"), &g_Settings.filterGuildCommendation))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_guild_commendation_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_green_prophet_shards"), &g_Settings.filterGreenProphetShards))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_green_prophet_shards_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_imperial_favor"), &g_Settings.filterImperialFavor))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_imperial_favor_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_jade_sliver"), &g_Settings.filterJadeSliver))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_jade_sliver_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_ley_line_crystals"), &g_Settings.filterLeyLineCrystals))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_ley_line_crystals_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_pinch_of_stardust"), &g_Settings.filterPinchOfStardust))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_pinch_of_stardust_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_prophet_shards"), &g_Settings.filterProphetShards))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_prophet_shards_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_static_charge"), &g_Settings.filterStaticCharge))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_static_charge_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_trade_contracts"), &g_Settings.filterTradeContracts))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_trade_contracts_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_unbound_magic"), &g_Settings.filterUnboundMagic))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_unbound_magic_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_uncommon_coins"), &g_Settings.filterUncommonCoins))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_uncommon_coins_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("filter_ursus_oblige"), &g_Settings.filterUrsusOblige))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_ursus_oblige_tooltip"));
        ImGui::SameLine();
        if (ImGui::Checkbox(Localization::GetText("filter_volatile_magic"), &g_Settings.filterVolatileMagic))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_volatile_magic_tooltip"));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", Localization::GetText("additional_filters_label"));
    if (ImGui::Checkbox(Localization::GetText("filter_account_bound"), &g_Settings.filterAccountBound))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_account_bound_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_not_account_bound"), &g_Settings.filterNotAccountBound))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_not_account_bound_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_nosell"), &g_Settings.filterNoSell))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_nosell_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_not_nosell"), &g_Settings.filterNotNoSell))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_not_nosell_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_favorite"), &g_Settings.filterFavorite))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_favorite_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_not_favorite"), &g_Settings.filterNotFavorite))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_not_favorite_tooltip"));
    if (ImGui::Checkbox(Localization::GetText("filter_ignored"), &g_Settings.filterIgnored))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_ignored_tooltip"));
    ImGui::SameLine();
    if (ImGui::Checkbox(Localization::GetText("filter_not_ignored"), &g_Settings.filterNotIgnored))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("filter_not_ignored_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Checkbox(Localization::GetText("range_filters"), &g_Settings.showRangeFilters))
        SettingsManager::Save();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("range_filters_tooltip"));

    if (g_Settings.showRangeFilters)
    {
        ImGui::Text("%s", Localization::GetText("price_range"));
        
        // Min Price
        ImGui::Text("%s", Localization::GetText("filter_min_price"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("gold"), &g_Settings.filterMinPriceGold))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_min_price_tooltip"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("silver"), &g_Settings.filterMinPriceSilver))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_min_price_tooltip"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("copper"), &g_Settings.filterMinPriceCopper))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_min_price_tooltip"));
        
        // Max Price
        ImGui::Text("%s", Localization::GetText("filter_max_price"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("gold"), &g_Settings.filterMaxPriceGold))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_max_price_tooltip"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("silver"), &g_Settings.filterMaxPriceSilver))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_max_price_tooltip"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("copper"), &g_Settings.filterMaxPriceCopper))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_max_price_tooltip"));
        
        ImGui::Text("%s", Localization::GetText("quantity_range"));
        if (ImGui::InputInt(Localization::GetText("filter_min_quantity"), &g_Settings.filterMinQuantity))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_min_quantity_tooltip"));
        ImGui::SameLine();
        if (ImGui::InputInt(Localization::GetText("filter_max_quantity"), &g_Settings.filterMaxQuantity))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("filter_max_quantity_tooltip"));
    }
}
}
