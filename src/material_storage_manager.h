#pragma once
#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <unordered_set>

namespace MaterialStorageManager
{
    void Init(const char* addonDir);
    void Shutdown();
    
    // Call this on map change (like MagnetiteTracker)
    void OnMapChange(const std::string& apiToken);
    
    // Get the count for an item ID (returns 0 if not present)
    int GetMaterialCount(int itemId);
    
    // Get the count for a currency ID (returns 0 if not present)
    int GetWalletCount(int currencyId);
    
    // Get the count for an item ID in account bank
    int GetBankCount(int itemId);
    
    // Get the count for an item ID in current character's inventory
    // (all equipped bags + shared inventory slots combined)
    int GetInventoryCount(int itemId);
    
    // Check if an item can go into material storage (cached from /v2/materials)
    bool CanGoToMaterialStorage(int itemId);
    
    // Save/Load
    void Load();
    void Save();
}
