#include "custom_profit.h"
#include "shared.h"
#include <algorithm>
#include <sstream>

std::mutex CustomProfitManager::s_Mutex;
std::map<int, CustomProfitEntry> CustomProfitManager::s_CustomProfits;

void CustomProfitManager::SetCustomProfit(int apiId, long long profitCopper, StatType type)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits[apiId].customProfitCopper = profitCopper;
    s_CustomProfits[apiId].hasCustomProfit = true;
    s_CustomProfits[apiId].type = type;
}

long long CustomProfitManager::GetCustomProfit(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_CustomProfits.find(apiId);
    return (it != s_CustomProfits.end() && it->second.hasCustomProfit) 
        ? it->second.customProfitCopper 
        : 0;
}

StatType CustomProfitManager::GetType(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_CustomProfits.find(apiId);
    return (it != s_CustomProfits.end()) ? it->second.type : StatType::Item;
}

bool CustomProfitManager::HasCustomProfit(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    auto it = s_CustomProfits.find(apiId);
    return it != s_CustomProfits.end() && it->second.hasCustomProfit;
}

void CustomProfitManager::RemoveCustomProfit(int apiId)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits.erase(apiId);
}

void CustomProfitManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits.clear();
}

std::map<int, long long> CustomProfitManager::GetAllCustomProfits()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::map<int, long long> result;
    for (std::map<int, CustomProfitEntry>::const_iterator it = s_CustomProfits.begin(); it != s_CustomProfits.end(); ++it)
    {
        if (it->second.hasCustomProfit)
        {
            result[it->first] = it->second.customProfitCopper;
        }
    }
    return result;
}

std::map<int, CustomProfitEntry> CustomProfitManager::GetAllCustomProfitsDetailed()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    return s_CustomProfits;
}

void CustomProfitManager::ImportFromJson(const nlohmann::json& j)
{
    const nlohmann::json* src = &j;
    if (j.is_object() && j.contains("customProfits"))
        src = &j["customProfits"];

    std::map<int, CustomProfitEntry> newMap;

    if (src->is_object())
    {
        for (auto it = src->begin(); it != src->end(); ++it)
        {
            int id = 0;
            try { id = std::stoi(it.key()); } catch (...) { continue; }

            CustomProfitEntry e;
            if (it.value().is_object())
            {
                e.customProfitCopper = it.value().value("profit", 0LL);
                e.type = static_cast<StatType>(it.value().value("type", static_cast<int>(StatType::Item)));
                e.hasCustomProfit = true;
                newMap[id] = e;
            }
            else if (it.value().is_number_integer() || it.value().is_number_unsigned() || it.value().is_number_float())
            {
                e.customProfitCopper = it.value().get<long long>();
                e.type = StatType::Item;
                e.hasCustomProfit = true;
                newMap[id] = e;
            }
        }
    }
    else if (src->is_array())
    {
        for (const auto& entry : *src)
        {
            if (!entry.is_object()) continue;
            int id = entry.value("apiId", 0);
            if (id == 0) id = entry.value("id", 0);
            if (id == 0) continue;

            CustomProfitEntry e;
            e.customProfitCopper = entry.value("profit", 0LL);
            e.type = static_cast<StatType>(entry.value("type", static_cast<int>(StatType::Item)));
            e.hasCustomProfit = true;
            newMap[id] = e;
        }
    }
    else
    {
        return;
    }

    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits = newMap;
}

std::string CustomProfitManager::ExportToJson()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    nlohmann::json j;
    nlohmann::json profits = nlohmann::json::object();
    for (const auto& [id, entry] : s_CustomProfits)
    {
        nlohmann::json e;
        e["profit"] = entry.customProfitCopper;
        e["type"]   = static_cast<int>(entry.type);
        profits[std::to_string(id)] = e;
    }
    j["customProfits"] = profits;
    return j.dump(2);
}

std::string CustomProfitManager::ExportToCsv()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    std::stringstream csv;
    csv << "API ID,Type,Profit (Copper)\n";
    for (const auto& [id, entry] : s_CustomProfits)
    {
        csv << id << "," << static_cast<int>(entry.type) << "," << entry.customProfitCopper << "\n";
    }
    return csv.str();
}

void CustomProfitManager::ImportFromCsv(const std::string& csv)
{
    std::map<int, CustomProfitEntry> newMap;
    std::stringstream ss(csv);
    std::string line;
    
    // Skip header
    std::getline(ss, line);
    
    while (std::getline(ss, line))
    {
        if (line.empty()) continue;
        
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> cells;
        
        while (std::getline(lineStream, cell, ','))
        {
            cells.push_back(cell);
        }
        
        if (cells.size() >= 3)
        {
            try
            {
                int id = std::stoi(cells[0]);
                int type = std::stoi(cells[1]);
                long long profit = std::stoll(cells[2]);
                
                CustomProfitEntry e;
                e.customProfitCopper = profit;
                e.type = static_cast<StatType>(type);
                e.hasCustomProfit = true;
                newMap[id] = e;
            }
            catch (...)
            {
                // Skip invalid lines
            }
        }
    }
    
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_CustomProfits = newMap;
}
