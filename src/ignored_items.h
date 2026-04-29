#pragma once
#include <set>
#include <mutex>

// ---------------------------------------------------------------------------
// Ignored Items System - provides ignore functionality
// Allows users to ignore specific items/currencies from tracking
// ---------------------------------------------------------------------------

class IgnoredItemsManager
{
private:
    static std::mutex s_Mutex;
    static std::set<int> s_IgnoredItems;
    static std::set<int> s_IgnoredCurrencies;

public:
    // Ignore an item
    static void IgnoreItem(int apiId);
    
    // Ignore a currency
    static void IgnoreCurrency(int apiId);
    
    // Set ignored state for an item (alias for IgnoreItem/UnignoreItem)
    static void SetIgnored(int apiId, bool ignored);
    
    // Set ignored state for a currency (alias for IgnoreCurrency/UnignoreCurrency)
    static void SetIgnoredCurrency(int apiId, bool ignored);
    
    // Unignore an item
    static void UnignoreItem(int apiId);
    
    // Unignore a currency
    static void UnignoreCurrency(int apiId);
    
    // Check if item is ignored
    static bool IsItemIgnored(int apiId);
    
    // Check if currency is ignored
    static bool IsCurrencyIgnored(int apiId);
    
    // Get all ignored items
    static std::set<int> GetIgnoredItems();
    
    // Get all ignored currencies
    static std::set<int> GetIgnoredCurrencies();
    
    // Clear all ignored items
    static void ClearAll();
};
