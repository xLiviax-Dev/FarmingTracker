#include "skip_once_manager.h"

std::mutex SkipOnceManager::s_Mutex;
std::set<int> SkipOnceManager::s_SkipOnceItems;
std::set<int> SkipOnceManager::s_SkipOnceCurrencies;

void SkipOnceManager::SkipOnceItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceItems.insert(apiId);
}

void SkipOnceManager::SkipOnceCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceCurrencies.insert(apiId);
}

bool SkipOnceManager::IsItemSkippedOnce(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SkipOnceItems.find(apiId) != s_SkipOnceItems.end();
}

bool SkipOnceManager::IsCurrencySkippedOnce(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SkipOnceCurrencies.find(apiId) != s_SkipOnceCurrencies.end();
}

void SkipOnceManager::UnskipOnceItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceItems.erase(apiId);
}

void SkipOnceManager::UnskipOnceCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceCurrencies.erase(apiId);
}

void SkipOnceManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceItems.clear();
    s_SkipOnceCurrencies.clear();
}

std::set<int> SkipOnceManager::GetSkippedItems()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SkipOnceItems;
}

std::set<int> SkipOnceManager::GetSkippedCurrencies()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SkipOnceCurrencies;
}
