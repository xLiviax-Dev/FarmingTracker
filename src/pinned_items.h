#pragma once
#include <vector>
#include <mutex>
#include "item_tracker.h"
#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#else
#include "../include/nlohmann/json.hpp"
#endif

// ---------------------------------------------------------------------------
// Pinned Items System - provides pin functionality for Mini Window
// Allows users to pin specific items/currencies to show in the compact Mini Window
// ---------------------------------------------------------------------------

struct PinnedItemEntry
{
    int apiId;
    StatType type;
};

class PinnedItemsManager
{
private:
    static std::mutex s_Mutex;
    static std::vector<PinnedItemEntry> s_PinnedItems;

public:
    // Pin an item
    static void PinItem(int apiId);
    
    // Pin a currency
    static void PinCurrency(int apiId);
    
    // Unpin an item/currency by ID and type
    static void Unpin(int apiId, StatType type);
    
    // Check if item is pinned
    static bool IsItemPinned(int apiId);
    
    // Check if currency is pinned
    static bool IsCurrencyPinned(int apiId);
    
    // Get all pinned items
    static std::vector<PinnedItemEntry> GetPinnedItems();
    
    // Clear all pinned items
    static void ClearAll();

    // Move pinned item up in the list (for sorting)
    static void MoveUp(int apiId, StatType type);

    // Move pinned item down in the list (for sorting)
    static void MoveDown(int apiId, StatType type);

    // Move pinned item to specific index (for drag & drop)
    static void MoveToIndex(int apiId, StatType type, size_t newIndex);

    static void ImportFromJson(const nlohmann::json& j);
    static nlohmann::json ExportToJson();
};
