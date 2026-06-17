#pragma once
#include <set>
#include <mutex>

class SkipOnceManager
{
private:
    static std::mutex s_Mutex;
    static std::set<int> s_SkipOnceItems;
    static std::set<int> s_SkipOnceCurrencies;

public:
    static void SkipOnceItem(int apiId);
    static void SkipOnceCurrency(int apiId);
    static bool IsItemSkippedOnce(int apiId);
    static bool IsCurrencySkippedOnce(int apiId);
    static void UnskipOnceItem(int apiId);
    static void UnskipOnceCurrency(int apiId);
    static void ClearAll();
    static std::set<int> GetSkippedItems();
    static std::set<int> GetSkippedCurrencies();
};
