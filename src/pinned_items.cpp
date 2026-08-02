#include "pinned_items.h"
#include "shared.h"

std::mutex PinnedItemsManager::s_Mutex;
std::vector<PinnedItemEntry> PinnedItemsManager::s_PinnedItems;

void PinnedItemsManager::PinItem(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    // Check if already pinned to avoid duplicates
    for (const auto& entry : s_PinnedItems)
    {
        if (entry.apiId == apiId && entry.type == StatType::Item)
            return; // Already pinned
    }
    PinnedItemEntry entry;
    entry.apiId = apiId;
    entry.type = StatType::Item;
    s_PinnedItems.push_back(entry);
}

void PinnedItemsManager::PinCurrency(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    // Check if already pinned to avoid duplicates
    for (const auto& entry : s_PinnedItems)
    {
        if (entry.apiId == apiId && entry.type == StatType::Currency)
            return; // Already pinned
    }
    PinnedItemEntry entry;
    entry.apiId = apiId;
    entry.type = StatType::Currency;
    s_PinnedItems.push_back(entry);
}

void PinnedItemsManager::Unpin(int apiId, StatType type)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_PinnedItems.erase(
        std::remove_if(s_PinnedItems.begin(), s_PinnedItems.end(),
            [apiId, type](const PinnedItemEntry& entry) {
                return entry.apiId == apiId && entry.type == type;
            }),
        s_PinnedItems.end());
}

bool PinnedItemsManager::IsItemPinned(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (const auto& entry : s_PinnedItems)
    {
        if (entry.apiId == apiId && entry.type == StatType::Item)
            return true;
    }
    return false;
}

bool PinnedItemsManager::IsCurrencyPinned(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (const auto& entry : s_PinnedItems)
    {
        if (entry.apiId == apiId && entry.type == StatType::Currency)
            return true;
    }
    return false;
}

std::vector<PinnedItemEntry> PinnedItemsManager::GetPinnedItems()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_PinnedItems;
}

void PinnedItemsManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_PinnedItems.clear();
}

void PinnedItemsManager::MoveUp(int apiId, StatType type)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (size_t i = 0; i < s_PinnedItems.size(); ++i)
    {
        if (s_PinnedItems[i].apiId == apiId && s_PinnedItems[i].type == type)
        {
            if (i > 0)
            {
                std::swap(s_PinnedItems[i], s_PinnedItems[i - 1]);
            }
            break;
        }
    }
}

void PinnedItemsManager::MoveDown(int apiId, StatType type)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    for (size_t i = 0; i < s_PinnedItems.size(); ++i)
    {
        if (s_PinnedItems[i].apiId == apiId && s_PinnedItems[i].type == type)
        {
            if (i < s_PinnedItems.size() - 1)
            {
                std::swap(s_PinnedItems[i], s_PinnedItems[i + 1]);
            }
            break;
        }
    }
}

void PinnedItemsManager::MoveToIndex(int apiId, StatType type, size_t newIndex)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    
    // Find current index
    size_t currentIndex = -1;
    for (size_t i = 0; i < s_PinnedItems.size(); ++i)
    {
        if (s_PinnedItems[i].apiId == apiId && s_PinnedItems[i].type == type)
        {
            currentIndex = i;
            break;
        }
    }
    
    if (currentIndex == -1 || currentIndex == newIndex || newIndex >= s_PinnedItems.size())
        return;
    
    // Remove from current position
    PinnedItemEntry entry = s_PinnedItems[currentIndex];
    s_PinnedItems.erase(s_PinnedItems.begin() + currentIndex);
    
    // Insert at new position
    if (newIndex > currentIndex)
        newIndex--; // Adjust index since we removed the element
    s_PinnedItems.insert(s_PinnedItems.begin() + newIndex, entry);
}

void PinnedItemsManager::ImportFromJson(const nlohmann::json& j)
{
    std::vector<PinnedItemEntry> newItems;

    const nlohmann::json* arrayToParse = nullptr;
    if (j.is_array())
    {
        arrayToParse = &j;
    }
    else if (j.is_object() && j.contains("pinnedItems") && j["pinnedItems"].is_array())
    {
        arrayToParse = &j["pinnedItems"];
    }

    if (arrayToParse)
    {
        for (const auto& v : *arrayToParse)
        {
            if (v.is_object())
            {
                PinnedItemEntry entry;
                if (v.contains("apiId") && v["apiId"].is_number_integer())
                    entry.apiId = v["apiId"].get<int>();
                if (v.contains("type") && v["type"].is_number_integer())
                    entry.type = static_cast<StatType>(v["type"].get<int>());
                
                if (entry.apiId != 0)
                    newItems.push_back(entry);
            }
        }
    }

    std::lock_guard<std::mutex> lock(s_Mutex);
    s_PinnedItems = newItems;
}

nlohmann::json PinnedItemsManager::ExportToJson()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    nlohmann::json pinnedArray = nlohmann::json::array();
    
    for (const auto& entry : s_PinnedItems)
    {
        nlohmann::json item;
        item["apiId"] = entry.apiId;
        item["type"] = static_cast<int>(entry.type);
        pinnedArray.push_back(item);
    }
    
    return pinnedArray;
}
