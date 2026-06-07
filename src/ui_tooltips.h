#pragma once

#include <string>

#include "item_tracker.h"

namespace UITooltips
{
    struct ItemTooltipOptions
    {
        bool showCount = false;
        long long count = 0;

        bool showProfit = false;
        long long profit = 0;

        bool showValue = false;
        long long value = 0;
        const char* valueLabelKey = "column_value";

        bool showTrading = true;
        bool showAccountFlags = true;
        bool showId = true;
    };

    struct CurrencyTooltipOptions
    {
        bool showCount = false;
        long long count = 0;

        bool showRarity = true;
        bool showId = true;
    };

    void RenderItemTooltip(const ApiDetails& details, int itemId, const ItemTooltipOptions& opt);
    void RenderItemTooltipFallback(const std::string& name, const std::string& rarity, int itemId, const ItemTooltipOptions& opt);

    void RenderCurrencyTooltip(const ApiDetails& details, int currencyId, const CurrencyTooltipOptions& opt);
    void RenderCurrencyTooltipFallback(const std::string& name, const std::string& rarity, int currencyId, const CurrencyTooltipOptions& opt);
}
