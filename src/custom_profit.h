#pragma once
#include <map>
#include <string>
#include <mutex>

#include "item_tracker.h"

// ---------------------------------------------------------------------------
// Custom Profit System - provides custom profit functionality
// Allows users to set custom profit values for items/currencies
// ---------------------------------------------------------------------------

struct CustomProfitEntry
{
    long long customProfitCopper = 0;
    bool hasCustomProfit = false;
    StatType type = StatType::Item;
};

class CustomProfitManager
{
private:
    static std::mutex s_Mutex;
    static std::map<int, CustomProfitEntry> s_CustomProfits;

public:
    // Set custom profit for an item/currency
    static void SetCustomProfit(int apiId, long long profitCopper, StatType type = StatType::Item);
    
    // Get custom profit for an item/currency
    static long long GetCustomProfit(int apiId);

    // Get type for a custom profit
    static StatType GetType(int apiId);
    
    // Check if item has custom profit
    static bool HasCustomProfit(int apiId);
    
    // Remove custom profit for an item/currency
    static void RemoveCustomProfit(int apiId);
    
    // Clear all custom profits
    static void ClearAll();
    
    // Get all custom profits
    static std::map<int, long long> GetAllCustomProfits();

    // Get all custom profits with types
    static std::map<int, CustomProfitEntry> GetAllCustomProfitsDetailed();

    static void ImportFromJson(const nlohmann::json& j);
    static std::string ExportToJson();

    static void ImportFromCsv(const std::string& csv);
    static std::string ExportToCsv();
};
