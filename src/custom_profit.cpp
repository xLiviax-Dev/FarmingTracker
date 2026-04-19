#include "custom_profit.h"
#include "shared.h"
#include <algorithm>

std::mutex CustomProfitManager::s_Mutex;
std::map<int, CustomProfitEntry> CustomProfitManager::s_CustomProfits;

void CustomProfitManager::SetCustomProfit(int apiId, long long profitCopper)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits[apiId].customProfitCopper = profitCopper;
    s_CustomProfits[apiId].hasCustomProfit = true;
}

long long CustomProfitManager::GetCustomProfit(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_CustomProfits.find(apiId);
    return (it != s_CustomProfits.end() && it->second.hasCustomProfit) 
        ? it->second.customProfitCopper 
        : 0;
}

bool CustomProfitManager::HasCustomProfit(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_CustomProfits.find(apiId);
    return it != s_CustomProfits.end() && it->second.hasCustomProfit;
}

void CustomProfitManager::RemoveCustomProfit(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits.erase(apiId);
}

void CustomProfitManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits.clear();
}
