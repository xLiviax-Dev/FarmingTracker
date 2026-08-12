#include "material_storage_manager.h"
#include "settings.h"
#include "shared.h"
#include "gw2_api.h"
#include "nlohmann/json.hpp"
#include "ui_common.h"

#include <fstream>
#include <filesystem>

namespace
{
    std::mutex s_Mutex;
    std::map<int, int> s_MaterialCounts; // itemId -> count
    std::map<int, int> s_WalletCounts; // currencyId -> count
    std::map<int, int> s_BankCounts; // itemId -> count (account bank)
    std::map<int, int> s_InventoryCounts; // itemId -> count (current char bags + shared slots)
    std::unordered_set<int> s_MaterialStorageItemIds; // Cache of all item IDs that can go into material storage
    std::chrono::system_clock::time_point s_LastFetchTime;
    constexpr std::chrono::minutes s_FetchCooldown{5}; // 5 minutes regular cooldown
    std::atomic<bool> s_ForceFetch{false}; // When true, bypass cooldown once (triggered by OnMapChange)
    std::string s_AddonDir;

    // Worker thread stuff
    std::atomic<bool> s_Shutdown{true};
    std::atomic<bool> s_WorkerFinished{false};
    std::thread s_WorkerThread;
    std::mutex s_CvMutex;
    std::condition_variable s_Cv;
    std::atomic<bool> s_WakeWorker{false};
    std::string s_PendingApiToken; // Token queued for fetching (protected by s_CvMutex)

    // Internal save without mutex (caller must already hold s_Mutex)
    void SaveInternal()
    {
        if (s_AddonDir.empty())
        {
            return;
        }
        
        std::filesystem::path filePath = std::filesystem::path(s_AddonDir) / "material_storage.json";
        
        try
        {
            nlohmann::json json;
            json["counts"] = nlohmann::json::object();
            json["wallet_counts"] = nlohmann::json::object();
            json["bank_counts"] = nlohmann::json::object();
            json["inventory_counts"] = nlohmann::json::object();
            
            for (const auto& [itemId, count] : s_MaterialCounts)
            {
                json["counts"][std::to_string(itemId)] = count;
            }
            
            for (const auto& [currencyId, count] : s_WalletCounts)
            {
                json["wallet_counts"][std::to_string(currencyId)] = count;
            }
            
            for (const auto& [itemId, count] : s_BankCounts)
            {
                json["bank_counts"][std::to_string(itemId)] = count;
            }
            
            for (const auto& [itemId, count] : s_InventoryCounts)
            {
                json["inventory_counts"][std::to_string(itemId)] = count;
            }
            
            auto now = std::chrono::system_clock::now();
            auto timeT = std::chrono::system_clock::to_time_t(now);
            json["last_fetch_utc"] = std::to_string(timeT);
            
            std::ofstream file(filePath);
            if (file.is_open())
            {
                file << json.dump(4);
            }
        }
        catch (...)
        {
            // Ignore errors
        }
    }

    void WorkerLoop()
    {
        while (!s_Shutdown.load())
        {
            std::unique_lock<std::mutex> lk(s_CvMutex);
            s_Cv.wait(lk, [] { return s_Shutdown.load() || s_WakeWorker.exchange(false); });
            lk.unlock();

            if (s_Shutdown.load()) break;

            std::string apiToken;
            {
                std::lock_guard<std::mutex> lk(s_CvMutex);
                apiToken = s_PendingApiToken;
            }

            if (apiToken.empty()) continue;

            // Check cooldown — but allow OnMapChange-triggered force fetches to bypass it
            bool shouldFetch = false;
            if (s_ForceFetch.exchange(false))
            {
                // Bypass timer when OnMapChange explicitly requested a refresh
                shouldFetch = true;
            }
            else
            {
                std::lock_guard<std::mutex> lock(s_Mutex);
                auto now = std::chrono::system_clock::now();
                if (now - s_LastFetchTime >= s_FetchCooldown)
                {
                    shouldFetch = true;
                }
            }

            if (!shouldFetch) continue;

            // Fetch from API
            nlohmann::json materialsJson;
            nlohmann::json walletJson;
            nlohmann::json bankJson;
            nlohmann::json inventoryJson;
            nlohmann::json sharedInventoryJson;
            nlohmann::json charactersJson;
            nlohmann::json materialsListJson;
            std::string error;
            
            // Fetch material storage item IDs list (one-time cache if empty)
            bool materialsListFetched = false;
            {
                std::lock_guard<std::mutex> lock(s_Mutex);
                if (s_MaterialStorageItemIds.empty())
                {
                    materialsListFetched = Gw2Api::FetchMaterials(apiToken, materialsListJson, error);
                }
            }
            
            bool materialsFetched = Gw2Api::FetchMaterialStorage(apiToken, materialsJson, error);
            bool walletFetched = Gw2Api::FetchWallet(apiToken, walletJson, error);
            bool bankFetched = Gw2Api::FetchBank(apiToken, bankJson, error);

            if (materialsListFetched) Gw2Api::Log("MatStorage: materials list fetched OK (" +
                std::to_string(materialsListJson.size()) + " item IDs)", "info");
            else if (!s_MaterialStorageItemIds.empty())
                Gw2Api::Log("MatStorage: materials list already cached (" +
                std::to_string(s_MaterialStorageItemIds.size()) + " item IDs)", "info");
            else Gw2Api::Log("MatStorage: Fetch materials list FAILED: " + error, "warning");
            
            if (materialsFetched) Gw2Api::Log("MatStorage: materials fetched OK (" +
                std::to_string(materialsJson.size()) + " entries)", "info");
            else Gw2Api::Log("MatStorage: Fetch materials FAILED: " + error, "warning");
            if (walletFetched) Gw2Api::Log("MatStorage: wallet fetched OK (" +
                std::to_string(walletJson.size()) + " currencies)", "info");
            else Gw2Api::Log("MatStorage: Fetch wallet FAILED: " + error, "warning");
            if (bankFetched) Gw2Api::Log("MatStorage: bank fetched OK (raw array size=" +
                std::to_string(bankJson.size()) + ")", "info");
            else Gw2Api::Log("MatStorage: Fetch bank FAILED: " + error, "warning");

            // 1. Preferred: character name from DRF drop event (s_AccountNameBuf)
            std::string currentChar;
            {
                std::lock_guard<std::mutex> accLock(UICommon::s_AccountNameMutex);
                currentChar = UICommon::s_AccountNameBuf;
            }

            // 2. Fallback: fetch /v2/characters and pick the FIRST character name.
            //    This guarantees we always get SOME inventory even when DRF has not
            //    sent a drop event yet since addon startup.
            if (currentChar.empty())
            {
                bool charsFetched = Gw2Api::FetchCharacters(apiToken, charactersJson, error);
                if (charsFetched && charactersJson.is_array() && !charactersJson.empty())
                {
                    for (const auto& c : charactersJson)
                    {
                        if (c.contains("name") && c["name"].is_string())
                        {
                            currentChar = c["name"].get<std::string>();
                            Gw2Api::Log("MatStorage: DRF char name empty — using fallback char '" + currentChar + "' from /v2/characters", "info");
                            break;
                        }
                    }
                }
                else
                {
                    if (!charsFetched)
                        Gw2Api::Log("MatStorage: Fetch characters FAILED (cannot load char inventory): " + error, "warning");
                    else
                        Gw2Api::Log("MatStorage: /v2/characters returned empty array — no char inventory loaded", "warning");
                }
            }

            // Fetch current character inventory — now guaranteed to run if we have a char name.
            bool inventoryFetched = false;
            if (!currentChar.empty())
            {
                inventoryFetched = Gw2Api::FetchInventory(apiToken, currentChar, inventoryJson, error);
                if (inventoryFetched)
                    Gw2Api::Log("MatStorage: character inventory fetched for '" + currentChar + "' (bags=" +
                        std::to_string(inventoryJson.value("bags", nlohmann::json::array()).size()) + ")", "info");
                else
                    Gw2Api::Log("MatStorage: Fetch inventory for '" + currentChar + "' FAILED: " + error, "warning");
            }

            // Fetch shared inventory slots (shared across all characters).
            bool sharedFetched = Gw2Api::FetchSharedInventory(apiToken, sharedInventoryJson, error);
            if (sharedFetched) Gw2Api::Log("MatStorage: shared inventory fetched OK (size=" +
                std::to_string(sharedInventoryJson.size()) + ")", "info");
            else Gw2Api::Log("MatStorage: Fetch shared inventory FAILED: " + error, "warning");

            // Update state
            {
                std::lock_guard<std::mutex> lock(s_Mutex);
                auto now = std::chrono::system_clock::now();

                // Parse materials list (cache of all item IDs that can go into material storage)
                if (materialsListFetched && s_MaterialStorageItemIds.empty())
                {
                    if (materialsListJson.is_array())
                    {
                        for (const auto& id : materialsListJson)
                        {
                            if (id.is_number())
                            {
                                s_MaterialStorageItemIds.insert(id.get<int>());
                            }
                        }
                    }
                    Gw2Api::Log("MatStorage: materials list cached with " + 
                        std::to_string(s_MaterialStorageItemIds.size()) + " item IDs", "debug");
                }

                if (materialsFetched)
                {
                    s_MaterialCounts.clear();
                    size_t parsed = 0;
                    if (materialsJson.is_array())
                    {
                        for (const auto& entry : materialsJson)
                        {
                            if (entry.contains("id") && entry.contains("count"))
                            {
                                int itemId = entry["id"].get<int>();
                                int count = entry["count"].get<int>();
                                s_MaterialCounts[itemId] = count;
                                ++parsed;
                            }
                        }
                    }
                    Gw2Api::Log("MatStorage: materials parsed into " + std::to_string(parsed) + " item entries", "debug");
                }
                
                if (walletFetched)
                {
                    s_WalletCounts.clear();
                    size_t parsed = 0;
                    if (walletJson.is_array())
                    {
                        for (const auto& entry : walletJson)
                        {
                            if (entry.contains("id") && entry.contains("value"))
                            {
                                int currencyId = entry["id"].get<int>();
                                int count = entry["value"].get<int>();
                                s_WalletCounts[currencyId] = count;
                                ++parsed;
                            }
                        }
                    }
                    Gw2Api::Log("MatStorage: wallet parsed into " + std::to_string(parsed) + " currency entries", "debug");
                }

                if (bankFetched)
                {
                    s_BankCounts.clear();
                    size_t parsed = 0;
                    if (bankJson.is_array())
                    {
                        for (const auto& slot : bankJson)
                        {
                            if (slot.is_object() && slot.contains("id") && slot.contains("count"))
                            {
                                int itemId = slot["id"].get<int>();
                                int count  = slot["count"].get<int>();
                                if (count > 0) {
                                    s_BankCounts[itemId] += count;
                                    ++parsed;
                                }
                            }
                        }
                    }
                    Gw2Api::Log("MatStorage: bank parsed into " + std::to_string(parsed) + " item entries", "debug");
                }

                size_t invParsed = 0;
                if (inventoryFetched || sharedFetched)
                {
                    s_InventoryCounts.clear();

                    // 1. Character inventory: /v2/characters/:name/inventory returns
                    //    { "bags": [ { "inventory": [ {id,count}, null, ... ] }, ... ] }
                    if (inventoryFetched && inventoryJson.contains("bags") && inventoryJson["bags"].is_array())
                    {
                        for (const auto& bag : inventoryJson["bags"])
                        {
                            if (bag.contains("inventory") && bag["inventory"].is_array())
                            {
                                for (const auto& slot : bag["inventory"])
                                {
                                    if (slot.is_object() && slot.contains("id") && slot.contains("count"))
                                    {
                                        int itemId = slot["id"].get<int>();
                                        int count  = slot["count"].get<int>();
                                        if (count > 0) {
                                            s_InventoryCounts[itemId] += count;
                                            ++invParsed;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 2. Shared inventory slots: /v2/account/inventory returns a flat array
                    //    [ {id,count}, null, ... ]
                    size_t sharedParsed = 0;
                    if (sharedFetched && sharedInventoryJson.is_array())
                    {
                        for (const auto& slot : sharedInventoryJson)
                        {
                            if (slot.is_object() && slot.contains("id") && slot.contains("count"))
                            {
                                int itemId = slot["id"].get<int>();
                                int count  = slot["count"].get<int>();
                                if (count > 0) {
                                    s_InventoryCounts[itemId] += count;
                                    ++sharedParsed;
                                    ++invParsed;
                                }
                            }
                        }
                    }
                    (void)sharedParsed;
                }
                if (inventoryFetched || sharedFetched)
                    Gw2Api::Log("MatStorage: inventory parsed into " + std::to_string(invParsed) +
                        " item entries (char bags + shared slots combined)", "debug");
                
                if (materialsFetched || walletFetched || bankFetched || inventoryFetched || sharedFetched)
                {
                    s_LastFetchTime = now;
                    SaveInternal();
                }
            }
        }

        s_WorkerFinished.store(true, std::memory_order_release);
    }
}

namespace MaterialStorageManager
{
    void Init(const char* addonDir)
    {
        if (addonDir)
        {
            s_AddonDir = addonDir;
        }
        Load();

        // Start worker thread
        if (!s_WorkerThread.joinable())
        {
            s_Shutdown.store(false);
            s_WorkerFinished.store(false, std::memory_order_release);
            s_WorkerThread = std::thread(WorkerLoop);
        }
    }

    void Shutdown()
    {
        s_Shutdown.store(true);
        s_WakeWorker.store(true);
        s_Cv.notify_all();

        if (s_WorkerThread.joinable())
        {
            // Wait a reasonable amount of time (max 2s)
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
            while (!s_WorkerFinished.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));

            if (s_WorkerFinished.load(std::memory_order_acquire))
            {
                s_WorkerThread.join();
            }
            else
            {
                Gw2Api::Log("MaterialStorageManager: worker thread didn't shutdown quickly — detaching", "warning");
                s_WorkerThread.detach();
            }
        }

        Save();
    }

    void OnMapChange(const std::string& apiToken)
    {
        if (apiToken.empty())
        {
            return;
        }

        // Queue the request
        {
            std::lock_guard<std::mutex> lk(s_CvMutex);
            s_PendingApiToken = apiToken;
        }

        // Signal force-refresh (bypasses the normal 5-minute cooldown)
        s_ForceFetch.store(true);

        // Wake the worker
        s_WakeWorker.store(true);
        s_Cv.notify_one();
    }

    int GetMaterialCount(int itemId)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        auto it = s_MaterialCounts.find(itemId);
        if (it != s_MaterialCounts.end())
        {
            return it->second;
        }
        return 0;
    }

    int GetWalletCount(int currencyId)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        auto it = s_WalletCounts.find(currencyId);
        if (it != s_WalletCounts.end())
        {
            return it->second;
        }
        return 0;
    }

    int GetBankCount(int itemId)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        auto it = s_BankCounts.find(itemId);
        if (it != s_BankCounts.end())
        {
            return it->second;
        }
        return 0;
    }

    int GetInventoryCount(int itemId)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        auto it = s_InventoryCounts.find(itemId);
        if (it != s_InventoryCounts.end())
        {
            return it->second;
        }
        return 0;
    }

    bool CanGoToMaterialStorage(int itemId)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        return s_MaterialStorageItemIds.count(itemId) > 0;
    }

    void Load()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        
        if (s_AddonDir.empty())
        {
            return;
        }
        
        std::filesystem::path filePath = std::filesystem::path(s_AddonDir) / "material_storage.json";
        
        if (!std::filesystem::exists(filePath))
        {
            return;
        }
        
        try
        {
            std::ifstream file(filePath);
            if (file.is_open())
            {
                nlohmann::json json;
                file >> json;
                
                if (json.contains("counts") && json["counts"].is_object())
                {
                    for (auto& [key, value] : json["counts"].items())
                    {
                        int itemId = std::stoi(key);
                        int count = value.get<int>();
                        s_MaterialCounts[itemId] = count;
                    }
                }
                
                if (json.contains("wallet_counts") && json["wallet_counts"].is_object())
                {
                    for (auto& [key, value] : json["wallet_counts"].items())
                    {
                        int currencyId = std::stoi(key);
                        int count = value.get<int>();
                        s_WalletCounts[currencyId] = count;
                    }
                }

                if (json.contains("bank_counts") && json["bank_counts"].is_object())
                {
                    for (auto& [key, value] : json["bank_counts"].items())
                    {
                        int itemId = std::stoi(key);
                        int count = value.get<int>();
                        s_BankCounts[itemId] = count;
                    }
                }

                if (json.contains("inventory_counts") && json["inventory_counts"].is_object())
                {
                    for (auto& [key, value] : json["inventory_counts"].items())
                    {
                        int itemId = std::stoi(key);
                        int count = value.get<int>();
                        s_InventoryCounts[itemId] = count;
                    }
                }
                
                if (json.contains("last_fetch_utc"))
                {
                    // We don't really need to load this, but just in case
                }
            }
        }
        catch (...)
        {
            // Ignore errors
        }
    }

    void Save()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        
        if (s_AddonDir.empty())
        {
            return;
        }
        
        std::filesystem::path filePath = std::filesystem::path(s_AddonDir) / "material_storage.json";
        
        try
        {
            nlohmann::json json;
            json["counts"] = nlohmann::json::object();
            json["wallet_counts"] = nlohmann::json::object();
            json["bank_counts"] = nlohmann::json::object();
            json["inventory_counts"] = nlohmann::json::object();
            
            for (const auto& [itemId, count] : s_MaterialCounts)
            {
                json["counts"][std::to_string(itemId)] = count;
            }
            
            for (const auto& [currencyId, count] : s_WalletCounts)
            {
                json["wallet_counts"][std::to_string(currencyId)] = count;
            }

            for (const auto& [itemId, count] : s_BankCounts)
            {
                json["bank_counts"][std::to_string(itemId)] = count;
            }

            for (const auto& [itemId, count] : s_InventoryCounts)
            {
                json["inventory_counts"][std::to_string(itemId)] = count;
            }
            
            auto now = std::chrono::system_clock::now();
            auto timeT = std::chrono::system_clock::to_time_t(now);
            json["last_fetch_utc"] = std::to_string(timeT);
            
            std::ofstream file(filePath);
            if (file.is_open())
            {
                file << json.dump(4);
            }
        }
        catch (...)
        {
            // Ignore errors
        }
    }
}
