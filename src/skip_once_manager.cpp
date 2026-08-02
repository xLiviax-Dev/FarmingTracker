#include "skip_once_manager.h"
#include "item_tracker.h"

std::mutex SkipOnceManager::s_Mutex;
std::set<int> SkipOnceManager::s_SkipOnceItems;
std::set<int> SkipOnceManager::s_SkipOnceCurrencies;

void SkipOnceManager::SkipOnceItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceItems.insert(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Items to ensure UI reflects the change immediately
    ItemTracker::UpdateItemIgnoredFlag(apiId, true);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
}

void SkipOnceManager::SkipOnceCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceCurrencies.insert(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Currencies to ensure UI reflects the change immediately
    ItemTracker::UpdateCurrencyIgnoredFlag(apiId, true);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
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
    ItemTracker::BumpItemsStateVersion();
}

void SkipOnceManager::UnskipOnceCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceCurrencies.erase(apiId);
    ItemTracker::BumpItemsStateVersion();
}

void SkipOnceManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SkipOnceItems.clear();
    s_SkipOnceCurrencies.clear();
    ItemTracker::BumpItemsStateVersion();
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
