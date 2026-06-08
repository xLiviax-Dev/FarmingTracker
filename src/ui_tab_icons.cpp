// ---------------------------------------------------------------------------
// ui_tab_icons.cpp
// Loads tab PNG icons from DLL resources
// ---------------------------------------------------------------------------
#include "ui_tab_icons.h"
#include "resource.h"
#include "shared.h"
#include "settings.h"
#include "../include/nexus/Nexus.h"
#include "../include/imgui/imgui.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <windows.h>

namespace UITabIcons
{

// Map: tab key → Resource ID
static const std::unordered_map<std::string, int> kIconResources =
{
    // Main tabs
    { "dashboard",       IDB_ICON_DASHBOARD       },
    { "drops",           IDB_ICON_DROPS           },
    { "loot_filter",     IDB_ICON_FILTER          },
    { "loot_log",        IDB_ICON_LOOT_LOG        },
    { "session_history", IDB_ICON_SESSION_HISTORY },
    { "timeline",        IDB_ICON_TIMELINE        },
    { "custom_profit",   IDB_ICON_CUSTOM_PROFIT   },
    { "debug",           IDB_ICON_DEBUG           },
    // Sub-tabs
    { "items",           IDB_ICON_ITEMS           },
    { "currencies",      IDB_ICON_CURRENCIES      },
    { "general",         IDB_ICON_GENERAL         },
    { "favorites",       IDB_ICON_FAVORITES       },
    { "ignored",         IDB_ICON_IGNORED         },
    { "filter",          IDB_ICON_FILTER          },
    { "live_log",        IDB_ICON_LOOT_LOG        },
    { "settings",        IDB_ICON_GENERAL         },
    // Settings sidebar icons
    { "account_connection", IDB_ICON_ACCOUNT_CONNECTION },
    { "appearance",      IDB_ICON_APPEARANCE      },
    { "windows",         IDB_ICON_WINDOWS         },
    { "tabs",            IDB_ICON_TABS            },
    { "data_reset",      IDB_ICON_DATA_RESET      },
    { "notifications",   IDB_ICON_NOTIFICATIONS   },
    { "performance",     IDB_ICON_PERFORMANCE     },
    { "advanced",        IDB_ICON_ADVANCED        },
    // Drops settings sub-tab icons
    { "layout-grid",     IDB_ICON_LAYOUT_GRID     },
    { "color-swatch",    IDB_ICON_COLOR_SWATCH    },
    { "category",        IDB_ICON_CATEGORY        },
    { "groupcategory",   IDB_ICON_GROUP_CATEGORY  },
    { "layout-columns",  IDB_ICON_LAYOUT_COLUMNS  },
    { "folder",          IDB_ICON_FOLDER          },
    // Loot Log settings icons
    { "sword",           IDB_ICON_SWORD           },
    { "coin",            IDB_ICON_COIN            },
    { "file-description",IDB_ICON_FILE_DESCRIPTION},
    { "database",        IDB_ICON_DATABASE        },
    { "settings",        IDB_ICON_GENERAL         },
    { "folder-open",     IDB_ICON_FOLDER_OPEN     },
    { "toggle-left",     IDB_ICON_TOGGLE_LEFT     },
    { "star",            IDB_ICON_STAR            },
    { "filter",          IDB_ICON_FILTER          },
    { "currency-dollar", IDB_ICON_CURRENCY_DOLLAR },
    { "file-size",       IDB_ICON_FILE_SIZE       },
    { "trash",           IDB_ICON_TRASH           },
    { "history",         IDB_ICON_HISTORY         },
    { "map-pin",         IDB_ICON_MAP_PIN         },
    { "sparkles",        IDB_ICON_SPARKLES        },
    { "wand",            IDB_ICON_WAND            },
    { "fingerprint",     IDB_ICON_FINGERPRINT     },
    // Settings page section icons
    { "toggle",          IDB_ICON_TOGGLE          },
    { "icons_borders",   IDB_ICON_ICONS_BORDERS   },
    { "main_window",     IDB_ICON_MAIN_WINDOW     },
    { "mini_window",     IDB_ICON_MINI_WINDOW     },
    { "profit",          IDB_ICON_PROFIT          },
    { "magnetite",       IDB_ICON_MAGNETITE       },
    { "settings_profiles", IDB_ICON_SETTINGS_PROFILES },
    { "copy",            IDB_ICON_COPY            },
    { "reset",           IDB_ICON_RESET           },
    { "open_folder",     IDB_ICON_OPEN_FOLDER     },
    { "export",          IDB_ICON_EXPORT          },
    { "file_csv",        IDB_ICON_FILE_DESCRIPTION },
};

// Nexus texture ID prefix
static const std::string kTexPrefix = "FT_TABICON_";

// Helper function: Loads bytes directly from the DLL
static std::vector<unsigned char> GetResourceBytes(int resourceId)
{
    HMODULE hMod = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&GetResourceBytes, &hMod);
    
    if (!hMod) return {};

    HRSRC hRes = FindResourceA(hMod, MAKEINTRESOURCEA(resourceId), MAKEINTRESOURCEA(10)); // RT_RCDATA = 10
    if (!hRes) return {};

    DWORD size = SizeofResource(hMod, hRes);
    if (size == 0) return {};

    HGLOBAL hLoaded = LoadResource(hMod, hRes);
    if (!hLoaded) return {};

    void* pData = LockResource(hLoaded);
    if (!pData) return {};

    std::vector<unsigned char> bytes;
    bytes.resize(size);
    memcpy(bytes.data(), pData, size);

    return bytes;
}

void Init(const char* addonDir)
{
    if (!APIDefs) return;

    for (const auto& [key, resourceId] : kIconResources)
    {
        std::string texId = kTexPrefix + key;
        std::vector<unsigned char> iconBytes = GetResourceBytes(resourceId);
        
        if (!iconBytes.empty())
        {
            APIDefs->Textures_GetOrCreateFromMemory(texId.c_str(), iconBytes.data(), iconBytes.size());
        }
    }
}

void* GetIcon(const std::string& key)
{
    if (!APIDefs) return nullptr;

    std::string texId = kTexPrefix + key;
    Texture_t* tex = APIDefs->Textures_Get(texId.c_str());
    if (!tex) return nullptr;
    return tex->Resource;
}

bool RenderSubPillTabBar(const std::vector<SubTabDef>& tabs, int& activeIdx)
{
    if (tabs.empty()) return false;

    // Clamp activeIdx
    if (activeIdx < 0 || activeIdx >= (int)tabs.size())
        activeIdx = 0;

    const float acR = g_Settings.accentColorR;
    const float acG = g_Settings.accentColorG;
    const float acB = g_Settings.accentColorB;

    auto clampBright = [](float v) { return v < 0.15f ? 0.15f : v; };

    // HTML design colors
    const ImVec4 colBarBg        = ImVec4(0.055f, 0.055f, 0.055f, 1.f); // #0e0e0e
    const ImVec4 colInactiveBgTop   = ImVec4(0.706f, 0.373f, 0.216f, 0.28f); // rgba(180, 95, 55, 0.28)
    const ImVec4 colInactiveBgBottom = ImVec4(0.412f, 0.176f, 0.098f, 0.42f); // rgba(105, 45, 25, 0.42)
    const ImVec4 colInactiveBorder = ImVec4(0.627f, 0.314f, 0.176f, 0.55f); // rgba(160, 80, 45, 0.55)
    const ImVec4 colInactiveTxt  = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colInactiveIcon = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colHoverBgTop   = ImVec4(acR * 2.0f * 0.72f, acG * 2.0f * 0.72f, acB * 2.0f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverBgMid   = ImVec4(acR * 1.2f * 0.72f, acG * 1.2f * 0.72f, acB * 1.2f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverBgBottom = ImVec4(acR * 0.5f * 0.72f, acG * 0.5f * 0.72f, acB * 0.5f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverBorder = ImVec4(acR * 1.5f * 0.72f, acG * 1.5f * 0.72f, acB * 1.5f * 0.72f, 1.f); // Accent-based, 30% darker
    const ImVec4 colHoverTxt     = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colHoverIcon    = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colActiveBgTop   = ImVec4(acR * 2.0f, acG * 2.0f, acB * 2.0f, 1.f); // Accent-based
    const ImVec4 colActiveBgMid   = ImVec4(acR * 1.2f, acG * 1.2f, acB * 1.2f, 1.f); // Accent-based
    const ImVec4 colActiveBgBottom = ImVec4(acR * 0.5f, acG * 0.5f, acB * 0.5f, 1.f); // Accent-based
    const ImVec4 colActiveBorder = ImVec4(acR * 1.5f, acG * 1.5f, acB * 1.5f, 1.f); // Accent-based
    const ImVec4 colActiveTxt    = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff
    const ImVec4 colActiveIcon   = ImVec4(1.0f, 1.0f, 1.0f, 1.f); // #ffffff

    // Slightly smaller than main tabs
    const float barPad   = 3.f;
    const float tabPadX  = 8.f;
    const float tabPadY  = 3.f;
    const float iconSz   = 14.f;
    const float iconGap  = 4.f;
    const float tabGap   = 3.f;
    const float tabRound = 4.f;

    ImDrawList* dl  = ImGui::GetWindowDrawList();
    float lineH     = ImGui::GetTextLineHeight();
    float tabH      = std::max(iconSz, lineH) + tabPadY * 2.f;
    float barH      = tabH + barPad * 2.f;
    ImVec2 mousePos = ImGui::GetMousePos();

    // Pre-calculate tab widths and positions
    struct Entry { float btnW; float minX; };
    std::vector<Entry> entries(tabs.size());
    float barW = barPad * 2.f;
    for (size_t i = 0; i < tabs.size(); i++)
    {
        float textW = ImGui::CalcTextSize(tabs[i].label).x;
        entries[i].btnW = tabPadX * 2.f + iconSz + iconGap + textW;
        entries[i].minX = barW - barPad;
        barW += entries[i].btnW + (i + 1 < tabs.size() ? tabGap : 0.f);
    }
    barW += barPad;
    float avail = ImGui::GetContentRegionAvail().x;
    barW = std::min(barW, avail);

    ImVec2 barStart = ImGui::GetCursorScreenPos();
    for (auto& e : entries)
        e.minX += barStart.x + barPad;

    float cy = barStart.y + barPad;

    // Bar background
    dl->AddRectFilled(barStart,
                      ImVec2(barStart.x + barW, barStart.y + barH),
                      ImGui::ColorConvertFloat4ToU32(colBarBg),
                      tabRound + barPad);

    bool changed = false;

    for (int i = 0; i < (int)tabs.size(); i++)
    {
        float cx       = entries[i].minX;
        float btnW     = entries[i].btnW;
        bool  active   = (i == activeIdx);

        ImVec2 btnMin = ImVec2(cx, cy);
        ImVec2 btnMax = ImVec2(cx + btnW, cy + tabH);

        // Hover detection
        bool hovered = (mousePos.x >= btnMin.x && mousePos.x < btnMax.x &&
                        mousePos.y >= btnMin.y && mousePos.y < btnMax.y);

        // Background with gradient
        if (active)
        {
            // Active gradient: radial-gradient(ellipse at 50% 0%, #C07040 0%, #6B3318 55%, #2E1508 100%)
            ImU32 topColor = ImGui::ColorConvertFloat4ToU32(colActiveBgTop);
            ImU32 midColor = ImGui::ColorConvertFloat4ToU32(colActiveBgMid);
            ImU32 bottomColor = ImGui::ColorConvertFloat4ToU32(colActiveBgBottom);

            // Draw gradient background
            dl->AddRectFilledMultiColor(btnMin, btnMax, topColor, topColor, bottomColor, bottomColor);
            dl->AddRect(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colActiveBorder), tabRound, 0, 0.5f);
        }
        else if (hovered)
        {
            // Hover gradient: radial-gradient(ellipse at 50% 0%, #C07040 0%, #6B3318 55%, #2E1508 100%)
            ImU32 topColor = ImGui::ColorConvertFloat4ToU32(colHoverBgTop);
            ImU32 midColor = ImGui::ColorConvertFloat4ToU32(colHoverBgMid);
            ImU32 bottomColor = ImGui::ColorConvertFloat4ToU32(colHoverBgBottom);

            // Draw gradient background
            dl->AddRectFilledMultiColor(btnMin, btnMax, topColor, topColor, bottomColor, bottomColor);
            dl->AddRect(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colHoverBorder), tabRound, 0, 0.5f);
        }
        else
        {
            // Inactive gradient: radial-gradient(ellipse at 50% 0%, rgba(180, 95, 55, 0.28) 0%, rgba(105, 45, 25, 0.42) 100%)
            ImU32 topColor = ImGui::ColorConvertFloat4ToU32(colInactiveBgTop);
            ImU32 bottomColor = ImGui::ColorConvertFloat4ToU32(colInactiveBgBottom);

            // Draw gradient background
            dl->AddRectFilledMultiColor(btnMin, btnMax, topColor, topColor, bottomColor, bottomColor);
            dl->AddRect(btnMin, btnMax, ImGui::ColorConvertFloat4ToU32(colInactiveBorder), tabRound, 0, 0.5f);
        }

        // Icon
        float iconX = cx + tabPadX;
        float iconY = cy + (tabH - iconSz) * 0.5f;
        void* iconTex = GetIcon(tabs[i].key);
        if (iconTex)
        {
            ImVec4 tint = active  ? colActiveIcon
                        : hovered ? colHoverIcon
                        : colInactiveIcon;

            // Glow effect (draw icon multiple times with offset and transparency)
            const float glowOffset = 2.0f;
            const float glowAlpha = 0.3f;
            ImU32 glowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(tint.x, tint.y, tint.z, glowAlpha));

            // Draw glow layers
            for (int i = 1; i <= 2; i++)
            {
                float offset = glowOffset * i;
                dl->AddImage((ImTextureID)iconTex,
                             ImVec2(iconX - offset, iconY - offset),
                             ImVec2(iconX + iconSz + offset, iconY + iconSz + offset),
                             ImVec2(0,0), ImVec2(1,1),
                             glowColor);
            }

            // Main icon
            dl->AddImage((ImTextureID)iconTex,
                         ImVec2(iconX, iconY),
                         ImVec2(iconX + iconSz, iconY + iconSz),
                         ImVec2(0,0), ImVec2(1,1),
                         ImGui::ColorConvertFloat4ToU32(tint));
        }

        // Label
        const ImVec4& lblCol = active  ? colActiveTxt
                             : hovered ? colHoverTxt
                             : colInactiveTxt;
        float textX = iconX + iconSz + iconGap;
        float textY = cy + (tabH - lineH) * 0.5f;
        dl->AddText(ImVec2(textX, textY),
                    ImGui::ColorConvertFloat4ToU32(lblCol),
                    tabs[i].label);

        // InvisibleButton for click detection
        ImGui::SetCursorScreenPos(btnMin);
        ImGui::PushID(i);
        ImGui::InvisibleButton("##subtab", ImVec2(btnW, tabH));
        if (ImGui::IsItemClicked() && !active)
        {
            activeIdx = i;
            changed   = true;
        }
        ImGui::PopID();
    }

    // Advance cursor past bar
    ImGui::SetCursorScreenPos(ImVec2(barStart.x, barStart.y + barH + 4.f));
    ImGui::Spacing();

    return changed;
}

} // namespace UITabIcons
