#pragma once
#include <set>
#include <mutex>

class SessionIgnoreManager
{
private:
    static std::mutex s_Mutex;
    static std::set<int> s_SessionIgnoreItems;
    static std::set<int> s_SessionIgnoreCurrencies;

public:
    static void IgnoreItemForSession(int apiId);
    static void IgnoreCurrencyForSession(int apiId);
    static bool IsItemIgnoredForSession(int apiId);
    static bool IsCurrencyIgnoredForSession(int apiId);
    static void UnignoreItemForSession(int apiId);
    static void UnignoreCurrencyForSession(int apiId);
    static void ClearAll();
    static std::set<int> GetIgnoredItems();
    static std::set<int> GetIgnoredCurrencies();
};
