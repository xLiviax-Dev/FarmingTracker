#include "ignored_items.h"
#include "shared.h"
#include "item_tracker.h"

std::mutex IgnoredItemsManager::s_Mutex;
std::set<int> IgnoredItemsManager::s_IgnoredItems;
std::set<int> IgnoredItemsManager::s_IgnoredCurrencies;

void IgnoredItemsManager::IgnoreItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredItems.insert(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Items to ensure UI reflects the change immediately
    ItemTracker::UpdateItemIgnoredFlag(apiId, true);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
    // Note: favorites and ignored are independent persistent stores.
    // Removing from favorites here would require calling back into ItemTracker
    // which risks circular lock ordering. The UI enforces mutual exclusivity instead.
}

void IgnoredItemsManager::IgnoreCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredCurrencies.insert(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Currencies to ensure UI reflects the change immediately
    ItemTracker::UpdateCurrencyIgnoredFlag(apiId, true);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
    // Same reasoning as IgnoreItem above.
}

void IgnoredItemsManager::SetIgnored(int apiId, bool ignored)
{
    if (ignored)
    {
        IgnoreItem(apiId);
    }
    else
    {
        UnignoreItem(apiId);
    }
}

void IgnoredItemsManager::SetIgnoredCurrency(int apiId, bool ignored)
{
    if (ignored)
    {
        IgnoreCurrency(apiId);
    }
    else
    {
        UnignoreCurrency(apiId);
    }
}

void IgnoredItemsManager::UnignoreItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredItems.erase(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Items to ensure UI reflects the change immediately
    ItemTracker::UpdateItemIgnoredFlag(apiId, false);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
}

void IgnoredItemsManager::UnignoreCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredCurrencies.erase(apiId);
    ItemTracker::BumpItemsStateVersion();
    // Update isIgnored flag in s_Currencies to ensure UI reflects the change immediately
    ItemTracker::UpdateCurrencyIgnoredFlag(apiId, false);
    // Force cache invalidation to ensure sorted views are refreshed
    ItemTracker::ForceCacheInvalidate();
    // Bump session drops version to invalidate Timeline cache
    ItemTracker::BumpSessionDropsVersion();
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
    ItemTracker::BumpItemsStateVersion();
}

void IgnoredItemsManager::ImportFromJson(const nlohmann::json& j)
{
    std::set<int> items;
    std::set<int> currencies;

    if (j.is_object())
    {
        if (j.contains("ignoredItems") && j["ignoredItems"].is_array())
        {
            for (const auto& v : j["ignoredItems"])
            {
                if (v.is_number_integer())
                    items.insert(v.get<int>());
            }
        }
        if (j.contains("ignoredCurrencies") && j["ignoredCurrencies"].is_array())
        {
            for (const auto& v : j["ignoredCurrencies"])
            {
                if (v.is_number_integer())
                    currencies.insert(v.get<int>());
            }
        }

        if (items.empty() && currencies.empty() && j.contains("ignored") && j["ignored"].is_array())
        {
            for (const auto& v : j["ignored"])
            {
                if (v.is_number_integer())
                    items.insert(v.get<int>());
            }
        }
    }
    else if (j.is_array())
    {
        for (const auto& v : j)
        {
            if (v.is_number_integer())
                items.insert(v.get<int>());
        }
    }
    else
    {
        return;
    }

    std::lock_guard<std::mutex> lock(s_Mutex);
    s_IgnoredItems = items;
    s_IgnoredCurrencies = currencies;
}
