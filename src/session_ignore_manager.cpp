#include "session_ignore_manager.h"

std::mutex SessionIgnoreManager::s_Mutex;
std::set<int> SessionIgnoreManager::s_SessionIgnoreItems;
std::set<int> SessionIgnoreManager::s_SessionIgnoreCurrencies;

void SessionIgnoreManager::IgnoreItemForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreItems.insert(apiId);
}

void SessionIgnoreManager::IgnoreCurrencyForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreCurrencies.insert(apiId);
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
}

void SessionIgnoreManager::UnignoreCurrencyForSession(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreCurrencies.erase(apiId);
}

void SessionIgnoreManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_SessionIgnoreItems.clear();
    s_SessionIgnoreCurrencies.clear();
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
