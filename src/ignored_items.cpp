#include "ignored_items.h"
#include "shared.h"

std::mutex IgnoredItemsManager::s_Mutex;
std::set<int> IgnoredItemsManager::s_IgnoredItems;
std::set<int> IgnoredItemsManager::s_IgnoredCurrencies;

void IgnoredItemsManager::IgnoreItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredItems.insert(apiId);
}

void IgnoredItemsManager::IgnoreCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredCurrencies.insert(apiId);
}

void IgnoredItemsManager::UnignoreItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredItems.erase(apiId);
}

void IgnoredItemsManager::UnignoreCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredCurrencies.erase(apiId);
}

bool IgnoredItemsManager::IsItemIgnored(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_IgnoredItems.find(apiId) != s_IgnoredItems.end();
}

bool IgnoredItemsManager::IsCurrencyIgnored(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_IgnoredCurrencies.find(apiId) != s_IgnoredCurrencies.end();
}

std::set<int> IgnoredItemsManager::GetIgnoredItems()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_IgnoredItems;
}

std::set<int> IgnoredItemsManager::GetIgnoredCurrencies()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_IgnoredCurrencies;
}

void IgnoredItemsManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredItems.clear();
    s_IgnoredCurrencies.clear();
}
