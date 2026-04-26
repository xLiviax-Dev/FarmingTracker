#include "ui_custom_profit.h"
#include "settings.h"
#include "custom_profit.h"
#include "localization.h"

namespace UICustomProfit
{
void RenderCustomProfitTab()
{
    ImGui::Text("Set custom profit values for items/currencies");
    ImGui::TextDisabled("%s", Localization::GetText("custom_profit_feature_placeholder"));

    ImGui::Spacing();
    if (ImGui::Button("Clear All Custom Profits"))
    {
        CustomProfitManager::ClearAll();
    }
}
}
