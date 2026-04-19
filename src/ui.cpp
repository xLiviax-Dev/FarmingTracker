// ---------------------------------------------------------------------------
// ui.cpp – ImGui rendering for the Farming Tracker Nexus addon
// ---------------------------------------------------------------------------
#include "ui.h"
#include "shared.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "gw2_api.h"
#include "gw2_fetcher.h"
#include "auto_reset.h"
#include "custom_profit.h"
#include "ignored_items.h"
#include "search_manager.h"
#include "../include/nexus/Nexus.h"
#include "../include/imgui/imgui.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static long long AbsLL(long long x) { return x >= 0 ? x : -x; }

static bool s_ShowMainWindow   = true;
static char s_TokenBuf[128]    = {};
static char s_Gw2KeyBuf[280]   = {};

// Edit Mode for Token and Key
static bool s_TokenEditMode    = false;
static bool s_KeyEditMode      = false;

// Advanced Features
static char s_SearchBuf[256]   = {};

// Forward declaration
static void ProcessKeybind(const char* aIdentifier, bool aIsRelease);

static void EnsureItemIconTexture(int itemId, const std::string& url)
{
    if (!APIDefs || url.empty() || !g_Settings.showItemIcons) return;

    char texId[80];
    if (snprintf(texId, sizeof texId, "FTi_%d", itemId) < 0) return;
    if (APIDefs->Textures_Get(texId)) return;

    size_t p = url.find("://");
    if (p == std::string::npos) return;
    p += 3;
    size_t sl = url.find('/', p);
    if (sl == std::string::npos) return;
    std::string host = url.substr(0, sl);
    std::string path = url.substr(sl);
    
    // Use async API with callback
    APIDefs->Textures_LoadFromURL(texId, host.c_str(), path.c_str(), [](const char* aIdentifier, Texture_t* aTexture) {
        // Callback - texture loaded (or failed)
        // Check APIDefs to avoid use-after-free during unload
        if (APIDefs && aTexture)
        {
            APIDefs->Log(LOGL_INFO, "FarmingTracker", ("Icon loaded: " + std::string(aIdentifier)).c_str());
        }
    });
}

static void DrawItemIconCell(int itemId, const std::string& url, float sz, const std::string& rarity = "")
{
    if (!g_Settings.showItemIcons || url.empty())
    {
        ImGui::Dummy(ImVec2(sz, sz));
        return;
    }

    // Ensure texture is loaded
    EnsureItemIconTexture(itemId, url);
    char texId[80];
    if (snprintf(texId, sizeof texId, "FTi_%d", itemId) < 0) {
        ImGui::Dummy(ImVec2(sz, sz));
        return;
    }
    Texture_t* tex = APIDefs ? APIDefs->Textures_Get(texId) : nullptr;

    if (tex && tex->Resource)
    {
        // Icon loaded successfully
        if (g_Settings.showRarityBorder && !rarity.empty())
        {
            // Rarity color for border
            ImVec4 borderColor = ImVec4(1.f, 1.f, 1.f, 1.f);
            if (rarity == "Junk") borderColor = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
            else if (rarity == "Basic") borderColor = ImVec4(1.f, 1.f, 1.f, 1.f);
            else if (rarity == "Fine") borderColor = ImVec4(0.0f, 0.5f, 1.f, 1.f);
            else if (rarity == "Masterwork") borderColor = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
            else if (rarity == "Rare") borderColor = ImVec4(1.f, 0.9f, 0.0f, 1.f);
            else if (rarity == "Exotic") borderColor = ImVec4(1.f, 0.6f, 0.0f, 1.f);
            else if (rarity == "Ascended") borderColor = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
            else if (rarity == "Legendary") borderColor = ImVec4(1.0f, 0.5f, 0.8f, 1.f);

            // Draw border
            ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::Image((ImTextureID)tex->Resource, ImVec2(sz, sz));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Image((ImTextureID)tex->Resource, ImVec2(sz, sz));
        }
    }
    else
    {
        // Icon not loaded yet - show placeholder
        ImGui::Dummy(ImVec2(sz, sz));
    }
}

static const char* StatusText(DrfStatus s)
{
    switch (s)
    {
        case DrfStatus::Disconnected: return "Disconnected";
        case DrfStatus::Connecting:   return "Connecting...";
        case DrfStatus::Connected:    return "Connected";
        case DrfStatus::AuthFailed:   return "Auth Failed – check token";
        case DrfStatus::Reconnecting: return "Reconnecting...";
        case DrfStatus::Error:        return "Error";
        default:                      return "Unknown";
    }
}

static ImVec4 StatusColor(DrfStatus s)
{
    switch (s)
    {
        case DrfStatus::Connected:    return ImVec4(0.2f, 0.9f, 0.2f, 1.f);
        case DrfStatus::AuthFailed:
        case DrfStatus::Error:        return ImVec4(0.9f, 0.2f, 0.2f, 1.f);
        case DrfStatus::Reconnecting:
        case DrfStatus::Connecting:   return ImVec4(0.9f, 0.8f, 0.1f, 1.f);
        default:                      return ImVec4(0.6f, 0.6f, 0.6f, 1.f);
    }
}

static std::string FormatCoin(long long copper)
{
    bool neg     = copper < 0;
    long long ac = copper < 0 ? -copper : copper;
    int g        = (int)(ac / 10000);
    int s        = (int)((ac % 10000) / 100);
    int c        = (int)(ac % 100);
    std::ostringstream oss;
    if (neg) oss << "-";
    if (g > 0)       oss << g << "g ";
    if (s > 0 || g > 0) oss << s << "s ";
    oss << c << "c";
    return oss.str();
}

static void DrawCoinDisplay(long long copper)
{
    bool neg     = copper < 0;
    long long ac = copper < 0 ? -copper : copper;
    int g        = (int)(ac / 10000);
    int s        = (int)((ac % 10000) / 100);
    int c        = (int)(ac % 100);
    
    if (neg) ImGui::Text("-");
    else if (copper > 0) ImGui::Text("+");
    ImGui::SameLine(0, 0);
    
    if (g > 0)
    {
        ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%d", g);
        ImGui::SameLine(0, 2);
        ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "g");
        ImGui::SameLine(0, 4);
    }
    
    if (s > 0 || g > 0)
    {
        ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.f, 1.f), "%d", s);
        ImGui::SameLine(0, 2);
        ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.f, 1.f), "s");
        ImGui::SameLine(0, 4);
    }
    
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%d", c);
    ImGui::SameLine(0, 2);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "c");
}

static std::string FormatDuration(long long seconds)
{
    long long h = seconds / 3600;
    long long m = (seconds % 3600) / 60;
    long long s = seconds % 60;
    std::ostringstream oss;
    if (h > 0) oss << h << "h ";
    if (m > 0 || h > 0) oss << m << "m ";
    oss << s << "s";
    return oss.str();
}

static void TextWithTooltip(const char* text, float maxWidth = 200.0f, const ImVec4& color = ImVec4(1.f, 1.f, 1.f, 1.f))
{
    ImVec2 textSize = ImGui::CalcTextSize(text);
    if (textSize.x > maxWidth)
    {
        ImGui::TextColored(color, "%s", text);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextColored(color, "%s", text);
            ImGui::EndTooltip();
        }
    }
    else
    {
        ImGui::TextColored(color, "%s", text);
    }
}

static bool PassesRarityFilter(const Stat& st)
{
    if (g_Settings.itemRarityFilterMin <= 0) return true;
    if (!st.details.loaded) return true;
    return ItemTracker::RarityRank(st.details.rarity) >= g_Settings.itemRarityFilterMin;
}

static void SortVisible(std::vector<std::pair<int, Stat>>& v)
{
    int mode = g_Settings.itemSortMode;
    std::sort(v.begin(), v.end(), [mode](const auto& a, const auto& b) {
        const Stat& sa = a.second;
        const Stat& sb = b.second;
        switch (mode)
        {
        default:
        case 0: return AbsLL(sa.count) > AbsLL(sb.count);
        case 1: return AbsLL(sa.count) < AbsLL(sb.count);
        case 2: return a.first < b.first;
        case 3: return a.first > b.first;
        case 4:
        {
            std::string na = sa.details.loaded ? sa.details.name : "";
            std::string nb = sb.details.loaded ? sb.details.name : "";
            if (na.empty() && nb.empty()) return a.first < b.first;
            if (na.empty()) return false;
            if (nb.empty()) return true;
            if (na != nb) return na < nb;
            return a.first < b.first;
        }
        }
    });
}

static void RenderMiniWindow()
{
    if (!g_Settings.showMiniWindow) return;

    ImGui::SetNextWindowPos(ImVec2(g_Settings.miniWindowPosX, g_Settings.miniWindowPosY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(g_Settings.miniWindowWidth, g_Settings.miniWindowHeight), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

    if (g_Settings.miniWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Farming Tracker Mini##FT_Mini", &g_Settings.showMiniWindow, flags))
    {
        // Save position
        ImVec2 pos = ImGui::GetWindowPos();
        g_Settings.miniWindowPosX = pos.x;
        g_Settings.miniWindowPosY = pos.y;

        // Save size
        ImVec2 size = ImGui::GetWindowSize();
        g_Settings.miniWindowWidth = size.x;
        g_Settings.miniWindowHeight = size.y;

        // Display selected metrics
        if (g_Settings.miniWindowShowProfit)
        {
            long long totalProfit = ItemTracker::CalcTotalCustomProfit();
            ImGui::Text("Profit: %s", FormatCoin(totalProfit).c_str());
        }

        if (g_Settings.miniWindowShowProfitPerHour)
        {
            auto duration = ItemTracker::GetSessionDuration();
            long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
            ImGui::Text("Profit/Hour: %s", FormatCoin(profitPerHour).c_str());
        }

        if (g_Settings.miniWindowShowTradingProfitSell)
        {
            long long tpSell = ItemTracker::CalcTotalTpSellProfit();
            ImGui::Text("TP Sell: %s", FormatCoin(tpSell).c_str());
        }

        if (g_Settings.miniWindowShowTradingProfitInstant)
        {
            long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
            ImGui::Text("TP Instant: %s", FormatCoin(tpInstant).c_str());
        }

        if (g_Settings.miniWindowShowTotalItems)
        {
            auto items = ItemTracker::GetItemsCopy();
            ImGui::Text("Total Items: %zu", items.size());
        }

        if (g_Settings.miniWindowShowSessionDuration)
        {
            auto duration = ItemTracker::GetSessionDuration();
            ImGui::Text("Session: %s", FormatDuration(duration.count()).c_str());
        }
    }

    ImGui::End();
}

static void RenderMainWindow()
{
    if (!s_ShowMainWindow) return;

    AutoReset::Tick();

    ImGui::SetNextWindowSize(ImVec2(520, 640), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 220), ImVec2(1200, 1200));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (g_Settings.mainWindowClickThrough)
        flags |= ImGuiWindowFlags_NoInputs;

    if (!ImGui::Begin("Farming Tracker##FT_Main", &s_ShowMainWindow, flags))
    {
        ImGui::End();
        return;
    }

    DrfStatus status = DrfClient::GetStatus();
    ImGui::TextColored(StatusColor(status), "DRF: %s", StatusText(status));
    ImGui::SameLine();

    const char* gw2StatusText = "Unknown";
    ImVec4 gw2StatusColor = ImVec4(1.f, 1.f, 1.f, 1.f);
    switch (Gw2Fetcher::GetStatus())
    {
        case Gw2Fetcher::Gw2Status::Disconnected: gw2StatusText = "Disconnected"; gw2StatusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.f); break;
        case Gw2Fetcher::Gw2Status::Connected: gw2StatusText = "Connected"; gw2StatusColor = ImVec4(0.1f, 0.8f, 0.1f, 1.f); break;
        case Gw2Fetcher::Gw2Status::Error: gw2StatusText = "Error"; gw2StatusColor = ImVec4(0.8f, 0.1f, 0.1f, 1.f); break;
    }
    ImGui::TextColored(gw2StatusColor, "| GW2 API: %s", gw2StatusText);
    ImGui::SameLine();

    auto dur = ItemTracker::GetSessionDuration();
    ImGui::Text("| Session Time: %s", FormatDuration((long long)dur.count()).c_str());
    ImGui::SameLine();

    if (ImGui::Button("Reset##FT_Reset"))
    {
        ItemTracker::Reset();
        AutoReset::OnManualReset();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Reset all farming counters (manual reset)");

    ImGui::Separator();

    if (ImGui::BeginTabBar("FT_Tabs"))
    {
        // === Summary Tab ===
        if (ImGui::BeginTabItem("Summary"))
        {
            // DRF Status Warning
            DrfStatus status = DrfClient::GetStatus();
            if (status == DrfStatus::Disconnected)
            {
                ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "⚠️ DRF not connected");
                ImGui::TextDisabled("This plugin requires DRF for data transmission.");
                ImGui::TextDisabled("Install DRF via Nexus Addon Manager or https://drf.rs/");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }
            else if (status == DrfStatus::AuthFailed)
            {
                ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "⚠️ DRF Token invalid");
                ImGui::TextDisabled("Please check your DRF Token in Settings.");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }

            // API Keys Warning
            if (g_Settings.gw2ApiKey.empty())
            {
                ImGui::TextColored(ImVec4(1.f, 0.6f, 0.0f, 1.f), "⚠️ GW2 API Key not set");
                ImGui::TextDisabled("Please set your GW2 API Key in Settings for item details.");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }

            // Overview
            ImGui::Text("Overview:");
            ImGui::Separator();
            long long totalProfit = ItemTracker::CalcTotalCustomProfit();
            auto duration = ItemTracker::GetSessionDuration();
            long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
            auto items = ItemTracker::GetItemsCopy();
            auto currencies = ItemTracker::GetCurrenciesCopy();

            ImGui::Text("Total Profit:          ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", FormatCoin(totalProfit).c_str());

            ImGui::Text("Total Items:           ");
            ImGui::SameLine();
            ImGui::Text("%zu", items.size());

            ImGui::Text("Total Currencies:      ");
            ImGui::SameLine();
            ImGui::Text("%zu", currencies.size());

            ImGui::Text("Profit Per Hour:       ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", FormatCoin(profitPerHour).c_str());

            ImGui::Text("Session Duration:      ");
            ImGui::SameLine();
            ImGui::Text("%s", FormatDuration(duration.count()).c_str());

            ImGui::Spacing();
            ImGui::Spacing();

            // Top Items
            ImGui::Text("Top Items (Profit):");
            ImGui::Separator();
            auto sortedByProfit = ItemTracker::GetSortedItems(ItemTracker::SortMode::ProfitDesc);
            int count = 0;
            for (auto& [id, st] : sortedByProfit)
            {
                if (count >= 5) break;
                if (st.count == 0) continue;
                long long profit = ItemTracker::GetStatProfit(st);
                if (profit <= 0) continue;
                std::string name = st.details.loaded ? st.details.name : "Loading...";
                ImGui::Text("%d. %s (%s)", count + 1, name.c_str(), FormatCoin(profit).c_str());
                count++;
            }

            ImGui::Spacing();
            ImGui::Text("Top Items (Count):");
            ImGui::Separator();
            auto sortedByCount = ItemTracker::GetSortedItems(ItemTracker::SortMode::CountDesc);
            count = 0;
            for (auto& [id, st] : sortedByCount)
            {
                if (count >= 5) break;
                if (st.count == 0) continue;
                std::string name = st.details.loaded ? st.details.name : "Loading...";
                ImGui::Text("%d. %s (%lld)", count + 1, name.c_str(), st.count);
                count++;
            }

            ImGui::Spacing();
            ImGui::Text("Top Currencies:");
            ImGui::Separator();
            auto sortedCurrencies = ItemTracker::GetSortedCurrencies(ItemTracker::SortMode::CountDesc);
            count = 0;
            for (auto& [id, st] : sortedCurrencies)
            {
                if (count >= 5) break;
                if (st.count == 0) continue;
                std::string name = st.details.loaded ? st.details.name : (id == 1 ? "Coin" : "Loading...");
                ImGui::Text("%d. %s (%lld)", count + 1, name.c_str(), st.count);
                count++;
            }

            ImGui::Spacing();
            ImGui::Spacing();

            // Quick Statistics
            ImGui::Text("Quick Statistics:");
            ImGui::Separator();
            long long totalItemValue = 0;
            int itemCount = 0;
            for (auto& [id, st] : items)
            {
                if (st.count > 0 && st.details.loaded)
                {
                    totalItemValue += ItemTracker::GetStatProfit(st);
                    itemCount++;
                }
            }
            if (itemCount > 0)
            {
                double avgValue = static_cast<double>(totalItemValue) / itemCount;
                ImGui::Text("Average Item Value:  %s", FormatCoin(static_cast<long long>(avgValue)).c_str());
            }
            else
            {
                ImGui::Text("Average Item Value:  N/A");
            }

            ImGui::Text("Total Unique Items:  %zu", items.size());

            // No Data Warning
            if (items.empty() && currencies.empty())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.f, 0.6f, 0.0f, 1.f), "⚠️ No data loaded");
                ImGui::TextDisabled("Waiting for DRF data...");
            }

            // Export Buttons
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Export:");
            if (ImGui::Button("Export as JSON##ExportJson"))
            {
                std::string jsonData = ItemTracker::ExportToJson();
                const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
                std::string filename = std::string(addonDir ? addonDir : "") + "\\FarmingTracker_Export_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json";

                // Use standard file operations to save
                FILE* file = nullptr;
                errno_t err = fopen_s(&file, filename.c_str(), "w");
                if (err == 0 && file)
                {
                    if (fprintf(file, "%s", jsonData.c_str()) < 0)
                    {
                        // Handle write error
                    }
                    fclose(file);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Export as CSV##ExportCsv"))
            {
                std::string csvData = ItemTracker::ExportToCsv();
                const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
                std::string filename = std::string(addonDir ? addonDir : "") + "\\FarmingTracker_Export_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".csv";

                // Use standard file operations to save
                FILE* file = nullptr;
                errno_t err = fopen_s(&file, filename.c_str(), "w");
                if (err == 0 && file)
                {
                    if (fprintf(file, "%s", csvData.c_str()) < 0)
                    {
                        // Handle write error
                    }
                    fclose(file);
                }
            }

            ImGui::EndTabItem();
        }

        // === Items Tab ===
        if (ImGui::BeginTabItem("Items"))
        {
            // Search bar
            if (g_Settings.enableSearch)
            {
                if (ImGui::InputTextWithHint("##Search", "Search items...", s_SearchBuf, sizeof(s_SearchBuf)))
                {
                    g_Settings.searchTerm = s_SearchBuf;
                    SettingsManager::Save();
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear##ClearSearch"))
                {
                    memset(s_SearchBuf, 0, sizeof(s_SearchBuf));
                    g_Settings.searchTerm = "";
                    SettingsManager::Save();
                }
                ImGui::Spacing();
            }

            const char* sortLabels[] = {
                "Sort: |Count| high",
                "Sort: |Count| low",
                "Sort: Item ID up",
                "Sort: Item ID down",
                "Sort: Name A–Z"
            };
            if (ImGui::Combo("##SortItems", &g_Settings.itemSortMode, sortLabels, 5))
                SettingsManager::Save();

            ImGui::SameLine();

            const char* rarityLabels[] = {
                "Rarity: all",
                "Rarity: Basic+",
                "Rarity: Fine+",
                "Rarity: Masterwork+",
                "Rarity: Rare+",
                "Rarity: Exotic+",
                "Rarity: Ascended+",
                "Rarity: Legendary only"
            };
            int rarityCombo = std::clamp(g_Settings.itemRarityFilterMin, 0, 7);
            if (ImGui::Combo("##RarityF", &rarityCombo, rarityLabels, 8))
            {
                g_Settings.itemRarityFilterMin = rarityCombo;
                SettingsManager::Save();
            }

            ImGui::Spacing();

            auto sortedItems = ItemTracker::GetSortedItems(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));

            // Items Table with enhanced features
            int itemTableColumnCount = g_Settings.enableFavorites ? 6 : 5;
            if (ImGui::BeginTable("##ItemsTable", itemTableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable))
            {
                float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f; // iconSize + padding
                ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 80.0f);
                ImGui::TableSetupColumn("Profit", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 120.0f);
                if (g_Settings.enableFavorites)
                    ImGui::TableSetupColumn("Favorite", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 60.0f);
                ImGui::TableSetupColumn("Ignore", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 60.0f);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedItems)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    DrawItemIconCell(id, st.details.iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");

                    ImGui::TableSetColumnIndex(1);
                    std::string name = st.details.loaded ? st.details.name : "Loading...";
                    ImVec4 col = ImVec4(1.f, 1.f, 1.f, 1.f);
                    if (st.details.loaded && !st.details.rarity.empty())
                    {
                        // Rarity color (enhanced) - according to GW2 Wiki
                        if (st.details.rarity == "Junk") col = ImVec4(0.7f, 0.7f, 0.7f, 1.f);
                        else if (st.details.rarity == "Basic") col = ImVec4(1.f, 1.f, 1.f, 1.f);
                        else if (st.details.rarity == "Fine") col = ImVec4(0.0f, 0.5f, 1.f, 1.f);
                        else if (st.details.rarity == "Masterwork") col = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                        else if (st.details.rarity == "Rare") col = ImVec4(1.f, 0.9f, 0.0f, 1.f);
                        else if (st.details.rarity == "Exotic") col = ImVec4(1.f, 0.6f, 0.0f, 1.f);
                        else if (st.details.rarity == "Ascended") col = ImVec4(0.9f, 0.3f, 0.9f, 1.f);
                        else if (st.details.rarity == "Legendary") col = ImVec4(1.0f, 0.5f, 0.8f, 1.f);
                    }
                    TextWithTooltip(name.c_str(), 200.0f, col);
                    if (ImGui::IsItemHovered() && st.details.loaded)
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Rarity: %s", st.details.rarity.c_str());
                        ImGui::Text("Type: %d", static_cast<int>(st.details.itemType));
                        ImGui::Text("Vendor Value: %s", FormatCoin(st.details.vendorValue).c_str());
                        ImGui::Text("TP Sell (Gross): %s", FormatCoin(st.details.tpSellPrice).c_str());
                        ImGui::Text("TP Sell (Net): %s", FormatCoin(static_cast<long long>(st.details.tpSellPrice * 85.0 / 100.0)).c_str());
                        ImGui::Text("TP Buy (Gross): %s", FormatCoin(st.details.tpBuyPrice).c_str());
                        ImGui::Text("TP Buy (Net): %s", FormatCoin(static_cast<long long>(st.details.tpBuyPrice * 85.0 / 100.0)).c_str());
                        ImGui::Text("Account-bound: %s", st.details.accountBound ? "Yes" : "No");
                        ImGui::Text("NoSell: %s", st.details.noSell ? "Yes" : "No");
                        ImGui::EndTooltip();
                    }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lld", st.count);

                    ImGui::TableSetColumnIndex(3);
                    long long profit = ItemTracker::GetStatProfit(st);
                    if (profit > 0)
                        ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", FormatCoin(profit).c_str());
                    else if (profit < 0)
                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.f), "%s", FormatCoin(profit).c_str());
                    else
                        ImGui::TextUnformatted("No profit");

                    int currentColumn = 4;
                    if (g_Settings.enableFavorites)
                    {
                        ImGui::TableSetColumnIndex(currentColumn++);
                        bool isFavorite = st.isFavorite;
                        if (ImGui::Checkbox(("##fav_" + std::to_string(id)).c_str(), &isFavorite))
                        {
                            ItemTracker::SetFavorite(id, isFavorite);
                        }
                    }

                    ImGui::TableSetColumnIndex(currentColumn);
                    if (g_Settings.enableIgnoredItems)
                    {
                        bool isIgnored = st.isIgnored;
                        if (ImGui::Checkbox(("##ign_" + std::to_string(id)).c_str(), &isIgnored))
                        {
                            if (isIgnored)
                                IgnoredItemsManager::IgnoreItem(id);
                            else
                                IgnoredItemsManager::UnignoreItem(id);
                        }
                    }
                }

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }

        // === Currencies Tab ===
        if (ImGui::BeginTabItem("Currencies"))
        {
            // Search bar
            if (g_Settings.enableSearch)
            {
                if (ImGui::InputTextWithHint("##SearchCurrencies", "Search currencies...", s_SearchBuf, sizeof(s_SearchBuf)))
                {
                    g_Settings.searchTerm = s_SearchBuf;
                    SettingsManager::Save();
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear##ClearSearchCurrencies"))
                {
                    memset(s_SearchBuf, 0, sizeof(s_SearchBuf));
                    g_Settings.searchTerm = "";
                    SettingsManager::Save();
                }
                ImGui::Spacing();
            }

            auto sortedCurrencies = ItemTracker::GetSortedCurrencies(static_cast<ItemTracker::SortMode>(g_Settings.itemSortMode));

            if (ImGui::BeginTable("##CurrenciesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
            {
                float iconColumnWidth = static_cast<float>(g_Settings.iconSize) + 10.0f; // iconSize + padding
                ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, iconColumnWidth);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 80.0f);
                ImGui::TableSetupColumn("Ignore", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 60.0f);
                ImGui::TableHeadersRow();

                for (auto& [id, st] : sortedCurrencies)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    // Fallback for Coin (ID 1) - use copper coin icon from GW2 Wiki
                    std::string iconUrl = st.details.iconUrl;
                    if (id == 1 && iconUrl.empty())
                        iconUrl = "https://wiki.guildwars2.com/images/e/eb/Copper_coin.png"; // Copper coin icon
                    DrawItemIconCell(id, iconUrl, static_cast<float>(g_Settings.iconSize), st.details.loaded ? st.details.rarity : "");

                    ImGui::TableSetColumnIndex(1);
                    std::string name = st.details.loaded ? st.details.name : (id == 1 ? "Coin" : "Loading...");
                    TextWithTooltip(name.c_str(), 200.0f);
                    if (ImGui::IsItemHovered() && st.details.loaded)
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("API ID: %d", id);
                        ImGui::Text("Count: %lld", st.count);
                        ImGui::EndTooltip();
                    }

                    ImGui::TableSetColumnIndex(2);
                    // Special display for Coin (ID 1) - show Gold/Silver/Copper with colored icons
                    if (id == 1)
                    {
                        DrawCoinDisplay(st.count);
                    }
                    else
                    {
                        ImGui::Text("%lld", st.count);
                    }

                    ImGui::TableSetColumnIndex(3);
                    if (g_Settings.enableIgnoredItems)
                    {
                        bool isIgnored = IgnoredItemsManager::IsCurrencyIgnored(id);
                        if (ImGui::Checkbox(("##ign_cur_" + std::to_string(id)).c_str(), &isIgnored))
                        {
                            if (isIgnored)
                                IgnoredItemsManager::IgnoreCurrency(id);
                            else
                                IgnoredItemsManager::UnignoreCurrency(id);
                        }
                    }
                }

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }

        // === Profit Tab ===
        if (ImGui::BeginTabItem("Profit"))
        {
            long long tpSell = ItemTracker::CalcTotalTpSellProfit();
            long long tpInstant = ItemTracker::CalcTotalTpInstantProfit();
            long long custom = ItemTracker::CalcTotalCustomProfit();

            ImGui::Text("Approx. Profits:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", FormatCoin(custom).c_str());

            ImGui::Spacing();
            ImGui::Text("Approx. Gold Per Hour:");
            ImGui::SameLine();
            auto duration = ItemTracker::GetSessionDuration();
            long long seconds = duration.count();
            long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s",
                FormatCoin(profitPerHour).c_str());

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Approx. Trading Profits (Listings):");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", FormatCoin(tpSell).c_str());

            ImGui::Spacing();
            ImGui::Text("Approx. Trading Profits (Instant Sell):");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "%s", FormatCoin(tpInstant).c_str());

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextDisabled("Trading Details (Opportunity Cost):");

            ImGui::Spacing();
            ImGui::Text("Lost Profit (vs TP Sell):");
            ImGui::SameLine();
            long long opportunityCost = ItemTracker::GetOpportunityCostProfit();
            ImVec4 ocColor = opportunityCost > 0 ? ImVec4(1.f, 0.3f, 0.3f, 1.f) : ImVec4(0.3f, 1.f, 0.3f, 1.f);
            ImGui::TextColored(ocColor, "%s", FormatCoin(opportunityCost).c_str());

            ImGui::Spacing();
            ImGui::Text("Lost Profit Per Hour (vs TP Sell):");
            ImGui::SameLine();
            long long opportunityCostPerHour = ItemTracker::GetOpportunityCostProfitPerHour(duration);
            ImGui::TextColored(ocColor, "%s", FormatCoin(opportunityCostPerHour).c_str());

            if (seconds > 0)
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("Session duration: %s", FormatDuration(seconds).c_str());
            }

            ImGui::EndTabItem();
        }

        // === Filter Tab ===
        if (ImGui::BeginTabItem("Filter"))
        {
            ImGui::Text("Count Filters:");
            ImGui::Checkbox("Positive count", &g_Settings.filterPositiveCount);
            ImGui::SameLine();
            ImGui::Checkbox("Negative count", &g_Settings.filterNegativeCount);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Sell Method Filters:");
            ImGui::Checkbox("Sellable to vendor", &g_Settings.filterSellableToVendor);
            ImGui::Checkbox("Sellable on TP", &g_Settings.filterSellableOnTp);
            ImGui::Checkbox("Has custom profit", &g_Settings.filterCustomProfit);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("API Knowledge Filters:");
            ImGui::Checkbox("Known by API", &g_Settings.filterKnownByApi);
            ImGui::SameLine();
            ImGui::Checkbox("Unknown by API", &g_Settings.filterUnknownByApi);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Item Type Filters:");
            ImGui::Checkbox("Armor", &g_Settings.filterTypeArmor);
            ImGui::SameLine();
            ImGui::Checkbox("Weapon", &g_Settings.filterTypeWeapon);
            ImGui::Checkbox("Trinket", &g_Settings.filterTypeTrinket);
            ImGui::SameLine();
            ImGui::Checkbox("Gizmo", &g_Settings.filterTypeGizmo);
            ImGui::Checkbox("Crafting Material", &g_Settings.filterTypeCraftingMaterial);
            ImGui::SameLine();
            ImGui::Checkbox("Consumable", &g_Settings.filterTypeConsumable);
            ImGui::Checkbox("Gathering Tool", &g_Settings.filterTypeGatheringTool);
            ImGui::SameLine();
            ImGui::Checkbox("Bag", &g_Settings.filterTypeBag);
            ImGui::Checkbox("Container", &g_Settings.filterTypeContainer);
            ImGui::SameLine();
            ImGui::Checkbox("Mini Pet", &g_Settings.filterTypeMiniPet);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Currency Filters:");
            ImGui::Checkbox("Karma", &g_Settings.filterKarma);
            ImGui::SameLine();
            ImGui::Checkbox("Laurel", &g_Settings.filterLaurel);
            ImGui::Checkbox("Gem", &g_Settings.filterGem);
            ImGui::SameLine();
            ImGui::Checkbox("Fractal Relic", &g_Settings.filterFractalRelic);
            ImGui::Checkbox("Badge of Honor", &g_Settings.filterBadgeOfHonor);
            ImGui::SameLine();
            ImGui::Checkbox("Guild Commendation", &g_Settings.filterGuildCommendation);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Additional Filters:");
            if (ImGui::Checkbox("Account-bound", &g_Settings.filterAccountBound))
                SettingsManager::Save();
            ImGui::SameLine();
            if (ImGui::Checkbox("Not Account-bound", &g_Settings.filterNotAccountBound))
                SettingsManager::Save();
            if (ImGui::Checkbox("NoSell", &g_Settings.filterNoSell))
                SettingsManager::Save();
            ImGui::SameLine();
            if (ImGui::Checkbox("Not NoSell", &g_Settings.filterNotNoSell))
                SettingsManager::Save();
            if (ImGui::Checkbox("Favorite", &g_Settings.filterFavorite))
                SettingsManager::Save();
            ImGui::SameLine();
            if (ImGui::Checkbox("Not Favorite", &g_Settings.filterNotFavorite))
                SettingsManager::Save();
            if (ImGui::Checkbox("Ignored", &g_Settings.filterIgnored))
                SettingsManager::Save();
            ImGui::SameLine();
            if (ImGui::Checkbox("Not Ignored", &g_Settings.filterNotIgnored))
                SettingsManager::Save();

            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Checkbox("Range Filters", &g_Settings.showRangeFilters))
                SettingsManager::Save();

            if (g_Settings.showRangeFilters)
            {
                ImGui::Text("Price Range (Copper):");
                if (ImGui::InputInt("Min Price", &g_Settings.filterMinPrice))
                    SettingsManager::Save();
                ImGui::SameLine();
                if (ImGui::InputInt("Max Price", &g_Settings.filterMaxPrice))
                    SettingsManager::Save();
                ImGui::Text("Quantity Range:");
                if (ImGui::InputInt("Min Quantity", &g_Settings.filterMinQuantity))
                    SettingsManager::Save();
                ImGui::SameLine();
                if (ImGui::InputInt("Max Quantity", &g_Settings.filterMaxQuantity))
                    SettingsManager::Save();
            }

            ImGui::EndTabItem();
        }

        // === Custom Profit Tab ===
        if (g_Settings.enableCustomProfit && ImGui::BeginTabItem("Custom Profit"))
        {
            ImGui::Text("Set custom profit values for items/currencies");
            ImGui::TextDisabled("Feature implemented - UI to be added");

            ImGui::Spacing();
            if (ImGui::Button("Clear All Custom Profits"))
            {
                CustomProfitManager::ClearAll();
            }

            ImGui::EndTabItem();
        }

        // === Ignored Items Tab ===
        if (g_Settings.enableIgnoredItems && ImGui::BeginTabItem("Ignored"))
        {
            ImGui::Text("Manage ignored items and currencies");

            auto ignoredItems = IgnoredItemsManager::GetIgnoredItems();
            auto ignoredCurrencies = IgnoredItemsManager::GetIgnoredCurrencies();

            ImGui::Text("Ignored Items: %zu", ignoredItems.size());
            ImGui::Text("Ignored Currencies: %zu", ignoredCurrencies.size());

            ImGui::Spacing();
            if (ImGui::Button("Clear All Ignored"))
            {
                IgnoredItemsManager::ClearAll();
            }

            ImGui::EndTabItem();
        }

        // === Debug Tab ===
        if (g_Settings.enableDebugTab && ImGui::BeginTabItem("Debug"))
        {
            ImGui::Text("Debug Information");
            ImGui::Separator();
            
            // DRF Status
            ImGui::Text("DRF Status: %s", StatusText(DrfClient::GetStatus()));
            ImGui::Text("DRF Reconnect Count: %d", DrfClient::GetReconnectCount());

            // GW2 API Status
            const char* gw2StatusText = "Unknown";
            switch (Gw2Fetcher::GetStatus())
            {
                case Gw2Fetcher::Gw2Status::Disconnected: gw2StatusText = "Disconnected"; break;
                case Gw2Fetcher::Gw2Status::Connected: gw2StatusText = "Connected"; break;
                case Gw2Fetcher::Gw2Status::Error: gw2StatusText = "Error"; break;
            }
            ImGui::Text("GW2 API Status: %s", gw2StatusText);
            ImGui::Text("GW2 API Reconnect Count: %d", Gw2Fetcher::GetReconnectCount());

            // Session Info
            ImGui::Text("Session Duration: %s", FormatDuration(ItemTracker::GetSessionDuration().count()).c_str());
            
            // Items/Currencies
            auto items = ItemTracker::GetItemsCopy();
            auto currencies = ItemTracker::GetCurrenciesCopy();
            ImGui::Text("Total Items: %zu", items.size());
            ImGui::Text("Total Currencies: %zu", currencies.size());
            
            // Memory Usage
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
            {
                SIZE_T memUsageMB = pmc.WorkingSetSize / (1024 * 1024);
                ImGui::Text("GW2 Process Memory: %zu MB", memUsageMB);
            }

            // API Request Count
            ImGui::Text("GW2 API Request Count: %d", Gw2Api::GetRequestCount());

            // Profit Info
            long long totalProfit = ItemTracker::CalcTotalCustomProfit();
            long long tpSellProfit = ItemTracker::CalcTotalTpSellProfit();
            long long vendorProfit = ItemTracker::CalcTotalVendorProfit();
            ImGui::Text("Total Profit: %s", FormatCoin(totalProfit).c_str());
            ImGui::Text("TP Sell Profit: %s", FormatCoin(tpSellProfit).c_str());
            ImGui::Text("Vendor Profit: %s", FormatCoin(vendorProfit).c_str());

            // Profit per Hour
            auto duration = ItemTracker::GetSessionDuration();
            long long profitPerHour = ItemTracker::GetTotalProfitPerHour(duration);
            ImGui::Text("Profit Per Hour: %s", FormatCoin(profitPerHour).c_str());

            // Opportunity Cost
            long long opportunityCostProfit = ItemTracker::GetOpportunityCostProfit();
            long long opportunityCostProfitPerHour = ItemTracker::GetOpportunityCostProfitPerHour(duration);
            ImGui::Text("Opportunity Cost Profit: %s", FormatCoin(opportunityCostProfit).c_str());
            ImGui::Text("Opportunity Cost Profit/Hour: %s", FormatCoin(opportunityCostProfitPerHour).c_str());

            // Filter Status
            int ignoredItemsCount = 0;
            int ignoredCurrenciesCount = 0;
            for (auto& [id, st] : items)
                if (st.isIgnored) ignoredItemsCount++;
            for (auto& [id, st] : currencies)
                if (st.isIgnored) ignoredCurrenciesCount++;
            ImGui::Text("Ignored Items: %d", ignoredItemsCount);
            ImGui::Text("Ignored Currencies: %d", ignoredCurrenciesCount);

            ImGui::Spacing();
            ImGui::Separator();

            // DRF Logs
            ImGui::Text("DRF Logs:");
            if (ImGui::Button("Clear DRF Logs"))
                DrfClient::ClearLogs();
            ImGui::SameLine();
            ImGui::Text("(Last 100 entries)");

            if (ImGui::BeginChild("DRFLogs", ImVec2(0, 150), true))
            {
                auto logs = DrfClient::GetLogs();
                for (auto& log : logs)
                {
                    auto time_t = std::chrono::system_clock::to_time_t(log.timestamp);
                    char timeStr[64];
                    struct tm timeinfo;
                    localtime_s(&timeinfo, &time_t);
                    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

                    ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f);
                    if (log.type == "error") color = ImVec4(1.f, 0.2f, 0.2f, 1.f);
                    else if (log.type == "data") color = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                    else if (log.type == "info") color = ImVec4(0.2f, 0.6f, 1.f, 1.f);

                    ImGui::TextColored(color, "[%s] [%s] %s", timeStr, log.type.c_str(), log.message.c_str());
                }
                ImGui::EndChild();
            }

            ImGui::Spacing();
            ImGui::Separator();

            // GW2 API Logs
            ImGui::Text("GW2 API Logs:");
            if (ImGui::Button("Clear GW2 Logs"))
                Gw2Api::ClearLogs();
            ImGui::SameLine();
            ImGui::Text("(Last 100 entries)");

            if (ImGui::BeginChild("GW2Logs", ImVec2(0, 150), true))
            {
                auto logs = Gw2Api::GetLogs();
                for (auto& log : logs)
                {
                    auto time_t = std::chrono::system_clock::to_time_t(log.timestamp);
                    char timeStr[64];
                    struct tm timeinfo;
                    localtime_s(&timeinfo, &time_t);
                    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

                    ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f);
                    if (log.type == "error") color = ImVec4(1.f, 0.2f, 0.2f, 1.f);
                    else if (log.type == "request") color = ImVec4(0.2f, 0.8f, 0.2f, 1.f);
                    else if (log.type == "info") color = ImVec4(0.2f, 0.6f, 1.f, 1.f);

                    ImGui::TextColored(color, "[%s] [%s] %s", timeStr, log.type.c_str(), log.message.c_str());
                }
                ImGui::EndChild();
            }

            ImGui::Spacing();
            ImGui::Separator();

            // Item/Currency Details
            ImGui::Text("Item/Currency Details (First 5):");
            int itemCount = 0;
            for (auto& [id, st] : items)
            {
                if (itemCount >= 5) break;
                ImGui::Text("Item %d: %s (Count: %lld, Loaded: %s)", id, st.details.loaded ? st.details.name.c_str() : "Loading...", st.count, st.details.loaded ? "Yes" : "No");
                itemCount++;
            }

            int currencyCount = 0;
            for (auto& [id, st] : currencies)
            {
                if (currencyCount >= 5) break;
                std::string name = st.details.loaded ? st.details.name : (id == 1 ? "Coin" : "Loading...");
                ImGui::Text("Currency %d: %s (Count: %lld, Loaded: %s)", id, name.c_str(), st.count, st.details.loaded ? "Yes" : "No");
                currencyCount++;
            }

            ImGui::Spacing();
            ImGui::Separator();

            // Custom Profit List
            ImGui::Text("Custom Profit Items (First 5):");
            int customProfitCount = 0;
            for (auto& [id, st] : items)
            {
                if (customProfitCount >= 5) break;
                if (st.HasCustomProfit())
                {
                    ImGui::Text("Item %d: %s (Custom Profit: %s)", id, st.details.loaded ? st.details.name.c_str() : "Loading...", FormatCoin(st.GetCustomProfit()).c_str());
                    customProfitCount++;
                }
            }
            if (customProfitCount == 0)
                ImGui::Text("(No custom profit items)");

            ImGui::Spacing();
            ImGui::Separator();

            // Ignored Items List
            ImGui::Text("Ignored Items (First 5):");
            int ignoredCount = 0;
            for (auto& [id, st] : items)
            {
                if (ignoredCount >= 5) break;
                if (st.isIgnored)
                {
                    ImGui::Text("Item %d: %s", id, st.details.loaded ? st.details.name.c_str() : "Loading...");
                    ignoredCount++;
                }
            }
            if (ignoredCount == 0)
                ImGui::Text("(No ignored items)");

            ImGui::Spacing();
            ImGui::Separator();

            // Settings
            ImGui::Text("Settings:");
            ImGui::Text("API Key: %s", g_Settings.gw2ApiKey.empty() ? "Not set" : "Set");
            ImGui::Text("DRF Token: %s", g_Settings.drfToken.empty() ? "Not set" : "Set");
            ImGui::Text("Toggle Hotkey: %s", g_Settings.toggleHotkey.c_str());
            ImGui::Text("Auto-Reset Mode: %d", g_Settings.automaticResetMode);
            ImGui::Text("Next Reset: %s", g_Settings.nextResetDateTimeUtc.c_str());

            ImGui::Spacing();
            ImGui::Separator();

            // Fake DRF Server
            ImGui::Text("Fake DRF Server:");
            ImGui::Checkbox("Use fake DRF server", &g_Settings.useFakeDrfServer);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("For testing purposes only");
            
            ImGui::Spacing();
            ImGui::Separator();

            // Reset
            if (ImGui::Button("Reset All Data"))
            {
                ItemTracker::Reset();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

static void RenderShortcut()
{
    ImGui::Checkbox("Show Farming Tracker", &s_ShowMainWindow);
}

static void RenderOptions()
{
    ImGui::TextUnformatted("Farming Tracker");
    ImGui::Separator();
    ImGui::Checkbox("Show main window", &s_ShowMainWindow);

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextWrapped("DRF Token");
    ImGui::TextDisabled("https://drf.rs/ -> Regenerate Token");
    if (s_TokenBuf[0] == '\0' && !g_Settings.drfToken.empty())
        if (strcpy_s(s_TokenBuf, sizeof(s_TokenBuf), g_Settings.drfToken.c_str()) != 0)
            s_TokenBuf[0] = '\0'; // Failed to copy, clear buffer

    bool tokenValid = SettingsManager::IsTokenValid(std::string(s_TokenBuf));
    ImVec4 frameBg = s_TokenBuf[0] == '\0'
        ? ImGui::GetStyleColorVec4(ImGuiCol_FrameBg)
        : tokenValid ? ImVec4(0.1f, 0.3f, 0.1f, 1.f) : ImVec4(0.4f, 0.1f, 0.1f, 1.f);
    if (!s_TokenEditMode && s_TokenBuf[0] == '\0')
        frameBg = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, frameBg);
    ImGuiInputTextFlags tokenFlags = s_TokenEditMode ? ImGuiInputTextFlags_None : ImGuiInputTextFlags_ReadOnly;
    bool tokCh = ImGui::InputText("##DRFToken", s_TokenBuf, sizeof(s_TokenBuf), tokenFlags);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushID("DRFTokenEdit");
    if (ImGui::Button(s_TokenEditMode ? "Save" : "Edit"))
    {
        if (s_TokenEditMode)
        {
            // Save
            g_Settings.drfToken = s_TokenBuf;
            SettingsManager::Save();
            s_TokenEditMode = false;
            if (tokenValid)
                DrfClient::Connect(g_Settings.drfToken);
        }
        else
        {
            // Edit
            s_TokenEditMode = true;
        }
    }
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextWrapped("Guild Wars 2 API key");
    ImGui::TextDisabled("https://account.arena.net/applications -- requires account + commerce + progression permissions.");
    if (s_Gw2KeyBuf[0] == '\0' && !g_Settings.gw2ApiKey.empty())
        if (strcpy_s(s_Gw2KeyBuf, sizeof(s_Gw2KeyBuf), g_Settings.gw2ApiKey.c_str()) != 0)
            s_Gw2KeyBuf[0] = '\0'; // Failed to copy, clear buffer

    bool keyOk = SettingsManager::IsGw2ApiKeyPlausible(std::string(s_Gw2KeyBuf));
    ImVec4 keyFrameBg = s_Gw2KeyBuf[0] == '\0'
        ? ImGui::GetStyleColorVec4(ImGuiCol_FrameBg)
        : keyOk ? ImVec4(0.1f, 0.25f, 0.15f, 1.f) : ImVec4(0.35f, 0.1f, 0.1f, 1.f);
    if (!s_KeyEditMode && s_Gw2KeyBuf[0] == '\0')
        keyFrameBg = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, keyFrameBg);
    ImGuiInputTextFlags keyFlags = s_KeyEditMode ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_Password | ImGuiInputTextFlags_ReadOnly;
    bool keyCh = ImGui::InputText("##GW2KEY", s_Gw2KeyBuf, sizeof(s_Gw2KeyBuf), keyFlags);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushID("GW2KeyEdit");
    if (ImGui::Button(s_KeyEditMode ? "Save" : "Edit"))
    {
        if (s_KeyEditMode)
        {
            // Save
            g_Settings.gw2ApiKey = s_Gw2KeyBuf;
            SettingsManager::Save();
            s_KeyEditMode = false;
            if (keyOk)
                Gw2Fetcher::UpdateApiKey();
        }
        else
        {
            // Edit
            s_KeyEditMode = true;
        }
    }
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Separator();

    // Reload Buttons
    ImGui::Text("Reload Configuration:");
    if (ImGui::Button("Reload DRF Token"))
    {
        DrfClient::Connect(g_Settings.drfToken);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload GW2 API Key"))
    {
        Gw2Fetcher::UpdateApiKey();
    }
    ImGui::Separator();

    const char* resetModes[] = {
        "Never (manual Reset only)",
        "On addon load",
        "Daily reset (00:00 UTC)",
        "Weekly (Mon 07:30 UTC)",
        "Weekly NA WvW (Sat 02:00 UTC)",
        "Weekly EU WvW (Fri 18:00 UTC)",
        "Weekly map bonus (Thu 20:00 UTC)",
        "Minutes after last unload"
    };
    ImGui::Text("Automatic reset:");
    if (ImGui::Combo("##AutoReset", &g_Settings.automaticResetMode, resetModes, 8))
    {
        SettingsManager::Save();
        AutoReset::RefreshSchedule();
    }

    if (g_Settings.automaticResetMode == 7)
    {
        if (ImGui::InputInt("Minutes after unload##minrst", &g_Settings.minutesUntilResetAfterShutdown))
        {
            g_Settings.minutesUntilResetAfterShutdown =
                std::clamp(g_Settings.minutesUntilResetAfterShutdown, 1, 24 * 60);
            SettingsManager::Save();
            AutoReset::RefreshSchedule();
        }
    }

    ImGui::Text("Next scheduled reset (UTC): %s", AutoReset::GetNextResetDisplayUtc().c_str());

    ImGui::Spacing();
    ImGui::Separator();

    // Basic Settings
    ImGui::Checkbox("Show item icons (GW2 render)", &g_Settings.showItemIcons);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Loads icons from render.guildwars2.com via Nexus.");
    if (ImGui::SliderInt("Icon size", &g_Settings.iconSize, 16, 96))
        SettingsManager::Save();
    if (ImGui::Checkbox("Show rarity borders", &g_Settings.showRarityBorder))
        SettingsManager::Save();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Mini Window (Overlay Widget):");
    if (ImGui::Checkbox("Show mini window", &g_Settings.showMiniWindow))
        SettingsManager::Save();

    if (g_Settings.showMiniWindow)
    {
        if (ImGui::Checkbox("Show Profit", &g_Settings.miniWindowShowProfit))
            SettingsManager::Save();
        if (ImGui::Checkbox("Show Profit/Hour", &g_Settings.miniWindowShowProfitPerHour))
            SettingsManager::Save();
        if (ImGui::Checkbox("Show TP Sell (Listings)", &g_Settings.miniWindowShowTradingProfitSell))
            SettingsManager::Save();
        if (ImGui::Checkbox("Show TP Instant (Instant Sell)", &g_Settings.miniWindowShowTradingProfitInstant))
            SettingsManager::Save();
        if (ImGui::Checkbox("Show Total Items", &g_Settings.miniWindowShowTotalItems))
            SettingsManager::Save();
        if (ImGui::Checkbox("Show Session Duration", &g_Settings.miniWindowShowSessionDuration))
            SettingsManager::Save();
        if (ImGui::Checkbox("Window click through", &g_Settings.miniWindowClickThrough))
            SettingsManager::Save();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Advanced Features
    ImGui::Text("Advanced Features:");
    if (ImGui::Checkbox("Enable search", &g_Settings.enableSearch))
        SettingsManager::Save();
    if (ImGui::Checkbox("Enable ignored items", &g_Settings.enableIgnoredItems))
        SettingsManager::Save();
    if (ImGui::Checkbox("Enable custom profit", &g_Settings.enableCustomProfit))
        SettingsManager::Save();
    if (ImGui::Checkbox("Enable favorites", &g_Settings.enableFavorites))
        SettingsManager::Save();
    if (ImGui::Checkbox("Enable debug tab", &g_Settings.enableDebugTab))
        SettingsManager::Save();

    if (ImGui::Checkbox("Show advanced settings", &g_Settings.showAdvancedSettings))
    {
        SettingsManager::Save();
    }

    if (g_Settings.showAdvancedSettings)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Advanced UI Settings:");

        if (ImGui::SliderInt("Negative count icon opacity", &g_Settings.negativeCountIconOpacity, 0, 255))
            SettingsManager::Save();
        if (ImGui::SliderInt("Count background opacity", &g_Settings.countBackgroundOpacity, 0, 255))
            SettingsManager::Save();
        if (ImGui::Checkbox("Main window click through", &g_Settings.mainWindowClickThrough))
            SettingsManager::Save();
    }
}

static void ProcessKeybind(const char* aIdentifier, bool aIsRelease)
{
    if (!aIsRelease && strcmp(aIdentifier, "KB_FT_TOGGLE") == 0)
        s_ShowMainWindow = !s_ShowMainWindow;
    if (!aIsRelease && strcmp(aIdentifier, "KB_FT_MINI_TOGGLE") == 0)
        g_Settings.showMiniWindow = !g_Settings.showMiniWindow;
}

void UI::Init()
{
    if (!APIDefs) return;

    const char* dir = APIDefs->Paths_GetAddonDirectory("FarmingTracker");
    if (dir)
    {
        std::string qa  = std::string(dir) + "\\qa_icon.png";
        std::string qah = std::string(dir) + "\\qa_icon_hover.png";
        if (GetFileAttributesA(qa.c_str()) != INVALID_FILE_ATTRIBUTES)
            APIDefs->Textures_GetOrCreateFromFile("ICON_FT", qa.c_str());
        if (GetFileAttributesA(qah.c_str()) != INVALID_FILE_ATTRIBUTES)
            APIDefs->Textures_GetOrCreateFromFile("ICON_FT_HOVER", qah.c_str());
    }

    APIDefs->GUI_Register(RT_Render, RenderMainWindow);
    APIDefs->GUI_Register(RT_Render, RenderMiniWindow);
    APIDefs->GUI_Register(RT_OptionsRender, RenderOptions);

    APIDefs->QuickAccess_Add(
        "QA_FT",
        "ICON_FT",
        "ICON_FT_HOVER",
        "KB_FT_TOGGLE",
        "Farming Tracker");

    APIDefs->QuickAccess_AddContextMenu("QAS_FT", "QA_FT", RenderShortcut);

    APIDefs->InputBinds_RegisterWithString("KB_FT_TOGGLE", ProcessKeybind, g_Settings.toggleHotkey.c_str());
    APIDefs->InputBinds_RegisterWithString("KB_FT_MINI_TOGGLE", ProcessKeybind, g_Settings.miniWindowToggleHotkey.c_str());
}

void UI::Shutdown()
{
    if (!APIDefs) return;

    APIDefs->GUI_Deregister(RenderMainWindow);
    APIDefs->GUI_Deregister(RenderMiniWindow);
    APIDefs->GUI_Deregister(RenderOptions);
    APIDefs->QuickAccess_Remove("QA_FT");
    APIDefs->QuickAccess_RemoveContextMenu("QAS_FT");
    APIDefs->InputBinds_Deregister("KB_FT_TOGGLE");
    APIDefs->InputBinds_Deregister("KB_FT_MINI_TOGGLE");
}
