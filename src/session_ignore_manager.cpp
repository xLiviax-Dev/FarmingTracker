#include "session_ignore_manager.h"
#include "item_tracker.h"

std::mutex SessionIgnoreManager::s_Mutex;
std::set<int> SessionIgnoreManager::s_SessionIgnoreItems;
std::set<int> SessionIgnoreManager::s_SessionIgnoreCurrencies;

void SessionIgnoreManager::IgnoreItemForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreItems.insert(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Items to ensure UI reflects the change immediately
    ItemTracker::UpdateItemIgnoredFlag(apiId, true);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
}

void SessionIgnoreManager::IgnoreCurrencyForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreCurrencies.insert(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Currencies to ensure UI reflects the change immediately
    ItemTracker::UpdateCurrencyIgnoredFlag(apiId, true);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
}

bool SessionIgnoreManager::IsItemIgnoredForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SessionIgnoreItems.find(apiId) != s_SessionIgnoreItems.end();
}

bool SessionIgnoreManager::IsCurrencyIgnoredForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SessionIgnoreCurrencies.find(apiId) != s_SessionIgnoreCurrencies.end();
}

void SessionIgnoreManager::UnignoreItemForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreItems.erase(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Items to ensure UI reflects the change immediately
    ItemTracker::UpdateItemIgnoredFlag(apiId, false);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
}

void SessionIgnoreManager::UnignoreCurrencyForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreCurrencies.erase(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Currencies to ensure UI reflects the change immediately
    ItemTracker::UpdateCurrencyIgnoredFlag(apiId, false);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
}

void SessionIgnoreManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreItems.clear();
    s_SessionIgnoreCurrencies.clear();
    ItemTracker::BumpItemsStateVersion();
}

std::set<int> SessionIgnoreManager::GetIgnoredItems()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SessionIgnoreItems;
}

std::set<int> SessionIgnoreManager::GetIgnoredCurrencies()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_SessionIgnoreCurrencies;
}
