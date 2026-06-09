#include "ui_tooltips.h"

#include "localization.h"
#include "ui_common.h"

namespace UITooltips
{
    static ImVec4 RarityColor(const std::string& rarity)
    {
        if (rarity == "Junk") return ImVec4(0.7f, 0.7f, 0.7f, 1.f);
        if (rarity == "Basic") return ImVec4(1.f, 1.f, 1.f, 1.f);
        if (rarity == "Fine") return ImVec4(0.0f, 0.5f, 1.f, 1.f);
        if (rarity == "Masterwork") return ImVec4(0.2f, 0.8f, 0.2f, 1.f);
        if (rarity == "Rare") return ImVec4(1.f, 0.9f, 0.0f, 1.f);
        if (rarity == "Exotic") return ImVec4(1.f, 0.6f, 0.0f, 1.f);
        if (rarity == "Ascended") return ImVec4(0.9f, 0.3f, 0.9f, 1.f);
        if (rarity == "Legendary") return ImVec4(0.55f, 0.25f, 0.85f, 1.f);
        return ImVec4(1.f, 1.f, 1.f, 1.f);
    }

    static void RenderItemTooltipLoaded(const ApiDetails& d, int itemId, const ItemTooltipOptions& opt)
    {
        ImGui::BeginTooltip();
        if (!d.rarity.empty())
            ImGui::TextColored(RarityColor(d.rarity), "%s", d.name.c_str());
        else
            ImGui::Text("%s", d.name.c_str());
        ImGui::Separator();

        if (opt.showCount)
        {
            ImVec4 countColor = UICommon::ValueColor(opt.count);
            ImGui::TextColored(countColor, "%s %lld", Localization::GetText("count_label"), opt.count);
        }

        if (opt.showProfit)
        {
            ImVec4 profitColor = UICommon::ValueColor(opt.profit);
            ImGui::TextColored(profitColor, "%s %s", Localization::GetText("profit_label"), UICommon::FormatCoin(opt.profit).c_str());
        }

        if (opt.showValue)
        {
            ImVec4 valueColor = UICommon::ValueColor(opt.value);
            ImGui::TextColored(valueColor, "%s: %s", Localization::GetText(opt.valueLabelKey), UICommon::FormatCoin(opt.value).c_str());
        }

        char rarityLabel[256];
        snprintf(rarityLabel, sizeof(rarityLabel), Localization::GetText("rarity_label"), d.rarity.c_str());
        ImGui::TextColored(RarityColor(d.rarity), "%s", rarityLabel);

        char typeLabel[256];
        snprintf(typeLabel, sizeof(typeLabel), Localization::GetText("type_label"), static_cast<int>(d.itemType));
        ImGui::Text("%s", typeLabel);

        if (opt.showTrading)
        {
            if (ItemTracker::CanSellToVendor(d))
            {
                ImVec4 vendorColor = UICommon::ValueColor(d.vendorValue);
                ImGui::TextColored(vendorColor, Localization::GetText("vendor_value_format"), UICommon::FormatCoin(d.vendorValue).c_str());
            }

            ImVec4 tpSellGrossColor = UICommon::ValueColor(d.tpSellPrice);
            ImGui::TextColored(tpSellGrossColor, Localization::GetText("tp_sell_gross_format"), UICommon::FormatCoin(d.tpSellPrice).c_str());
            long long tpSellNet = static_cast<long long>(d.tpSellPrice * 85.0 / 100.0);
            ImVec4 tpSellNetColor = UICommon::ValueColor(tpSellNet);
            ImGui::TextColored(tpSellNetColor, Localization::GetText("tp_sell_net_format"), UICommon::FormatCoin(tpSellNet).c_str());

            ImVec4 tpBuyGrossColor = UICommon::ValueColor(d.tpBuyPrice);
            ImGui::TextColored(tpBuyGrossColor, Localization::GetText("tp_buy_gross_format"), UICommon::FormatCoin(d.tpBuyPrice).c_str());
            long long tpBuyNet = static_cast<long long>(d.tpBuyPrice * 85.0 / 100.0);
            ImVec4 tpBuyNetColor = UICommon::ValueColor(tpBuyNet);
            ImGui::TextColored(tpBuyNetColor, Localization::GetText("tp_buy_net_format"), UICommon::FormatCoin(tpBuyNet).c_str());
        }

        if (opt.showAccountFlags)
        {
            char accountBoundLabel[256];
            snprintf(accountBoundLabel, sizeof(accountBoundLabel), Localization::GetText("account_bound_label"), d.accountBound ? Localization::GetText("yes_label") : Localization::GetText("no_label"));
            ImGui::Text("%s", accountBoundLabel);

            char noSellLabel[256];
            snprintf(noSellLabel, sizeof(noSellLabel), Localization::GetText("nosell_label"), d.noSell ? Localization::GetText("yes_label") : Localization::GetText("no_label"));
            ImGui::Text("%s", noSellLabel);
        }

        if (opt.showId)
        {
            ImGui::Separator();
            char itemIdLabel[256];
            snprintf(itemIdLabel, sizeof(itemIdLabel), Localization::GetText("item_id_label"), itemId);
            ImGui::Text("%s", itemIdLabel);
        }

        ImGui::EndTooltip();
    }

    static void RenderItemTooltipFallbackImpl(const std::string& name, const std::string& rarity, int itemId, const ItemTooltipOptions& opt)
    {
        ImGui::BeginTooltip();
        if (!rarity.empty())
            ImGui::TextColored(RarityColor(rarity), "%s", name.c_str());
        else
            ImGui::Text("%s", name.c_str());
        ImGui::Separator();

        if (opt.showCount)
        {
            ImVec4 countColor = UICommon::ValueColor(opt.count);
            ImGui::TextColored(countColor, "%s %lld", Localization::GetText("count_label"), opt.count);
        }

        if (opt.showProfit)
        {
            ImVec4 profitColor = UICommon::ValueColor(opt.profit);
            ImGui::TextColored(profitColor, "%s %s", Localization::GetText("profit_label"), UICommon::FormatCoin(opt.profit).c_str());
        }

        if (opt.showValue)
        {
            ImVec4 valueColor = UICommon::ValueColor(opt.value);
            ImGui::TextColored(valueColor, "%s: %s", Localization::GetText(opt.valueLabelKey), UICommon::FormatCoin(opt.value).c_str());
        }

        if (!rarity.empty())
        {
            char rarityLabel[256];
            snprintf(rarityLabel, sizeof(rarityLabel), Localization::GetText("rarity_label"), rarity.c_str());
            ImGui::TextColored(RarityColor(rarity), "%s", rarityLabel);
        }

        if (opt.showId)
        {
            ImGui::Separator();
            char itemIdLabel[256];
            snprintf(itemIdLabel, sizeof(itemIdLabel), Localization::GetText("item_id_label"), itemId);
            ImGui::Text("%s", itemIdLabel);
        }

        ImGui::EndTooltip();
    }

    static void RenderCurrencyTooltipLoaded(const ApiDetails& d, int currencyId, const CurrencyTooltipOptions& opt)
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", d.name.c_str());
        ImGui::Separator();

        if (opt.showCount)
        {
            ImVec4 countColor = UICommon::ValueColor(opt.count);
            ImGui::TextColored(countColor, "%s %lld", Localization::GetText("count_label"), opt.count);
        }

        if (opt.showProfit)
        {
            ImVec4 profitColor = UICommon::ValueColor(opt.profit);
            ImGui::TextColored(profitColor, "%s %s", Localization::GetText("profit_label"), UICommon::FormatCoin(opt.profit).c_str());
        }

        if (opt.showRarity && !d.rarity.empty())
        {
            ImGui::TextColored(RarityColor(d.rarity), "%s", d.rarity.c_str());
        }

        if (opt.showId)
        {
            char currencyIdLabel[256];
            snprintf(currencyIdLabel, sizeof(currencyIdLabel), Localization::GetText("currency_id_label"), currencyId);
            ImGui::Text("%s", currencyIdLabel);
        }

        ImGui::EndTooltip();
    }

    static void RenderCurrencyTooltipFallbackImpl(const std::string& name, const std::string& rarity, int currencyId, const CurrencyTooltipOptions& opt)
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", name.c_str());
        ImGui::Separator();

        if (opt.showCount)
        {
            ImVec4 countColor = UICommon::ValueColor(opt.count);
            ImGui::TextColored(countColor, "%s %lld", Localization::GetText("count_label"), opt.count);
        }

        if (opt.showProfit)
        {
            ImVec4 profitColor = UICommon::ValueColor(opt.profit);
            ImGui::TextColored(profitColor, "%s %s", Localization::GetText("profit_label"), UICommon::FormatCoin(opt.profit).c_str());
        }

        if (opt.showRarity && !rarity.empty())
        {
            ImGui::TextColored(RarityColor(rarity), "%s", rarity.c_str());
        }

        if (opt.showId)
        {
            char currencyIdLabel[256];
            snprintf(currencyIdLabel, sizeof(currencyIdLabel), Localization::GetText("currency_id_label"), currencyId);
            ImGui::Text("%s", currencyIdLabel);
        }

        ImGui::EndTooltip();
    }

    void RenderItemTooltip(const ApiDetails& details, int itemId, const ItemTooltipOptions& opt)
    {
        RenderItemTooltipLoaded(details, itemId, opt);
    }

    void RenderItemTooltipFallback(const std::string& name, const std::string& rarity, int itemId, const ItemTooltipOptions& opt)
    {
        RenderItemTooltipFallbackImpl(name, rarity, itemId, opt);
    }

    void RenderCurrencyTooltip(const ApiDetails& details, int currencyId, const CurrencyTooltipOptions& opt)
    {
        RenderCurrencyTooltipLoaded(details, currencyId, opt);
    }

    void RenderCurrencyTooltipFallback(const std::string& name, const std::string& rarity, int currencyId, const CurrencyTooltipOptions& opt)
    {
        RenderCurrencyTooltipFallbackImpl(name, rarity, currencyId, opt);
    }
}
