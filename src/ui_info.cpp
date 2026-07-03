// ---------------------------------------------------------------------------
// ui_info.cpp
// Info / About tab for FarmingTracker
// by x Livia x
// ---------------------------------------------------------------------------
#include "ui_info.h"
#include "resource.h"
#include "shared.h"
#include "../include/nexus/Nexus.h"
#include "../include/imgui/imgui.h"

#include <string>
#include <vector>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

namespace UIInfo
{

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr const char* VERSION       = "2.0.0.7";
static constexpr const char* AUTHOR        = "x Livia x";
static constexpr const char* GITHUB_URL    = "https://github.com/xLiviax-Dev/FarmingTracker";
static constexpr const char* GITHUB_ISSUES = "https://github.com/xLiviax-Dev/FarmingTracker/issues";

// ---------------------------------------------------------------------------
// Resource ID map  (key → Win32 resource ID)
// Matches the defines in resource.h and entries in resources.rc
// ---------------------------------------------------------------------------
static const std::unordered_map<std::string, int> kIconResources =
{
    { "Emoji wheat 32", IDB_INFO_WHEAT        },
    { "ti-rocket",      IDB_INFO_ROCKET       },
    { "ti-keyboard",    IDB_INFO_KEYBOARD     },
    { "ti-history",     IDB_INFO_HISTORY      },
    { "ti-link",        IDB_INFO_LINK         },
    { "ti-brand-github",IDB_INFO_GITHUB       },
    { "ti-bug",         IDB_INFO_BUG          },
    { "ti-heart",       IDB_INFO_HEART        },
    { "ti-heart-filled",IDB_INFO_HEART_FILLED },
    { "ti-tag",         IDB_INFO_TAG          },
    { "ti-info-circle", IDB_INFO_INFO_CIRCLE  },
};

static const std::string kTexPrefix = "FT_INFO_";

// ---------------------------------------------------------------------------
// Load raw bytes from DLL resource (identical pattern to ui_tab_icons.cpp)
// ---------------------------------------------------------------------------
static std::vector<unsigned char> GetResourceBytes(int resourceId)
{
    HMODULE hMod = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&GetResourceBytes), &hMod);
    if (!hMod) return {};

    HRSRC hRes = FindResourceA(hMod, MAKEINTRESOURCEA(resourceId), MAKEINTRESOURCEA(10)); // RT_RCDATA
    if (!hRes) return {};

    DWORD size = SizeofResource(hMod, hRes);
    if (!size) return {};

    HGLOBAL hLoaded = LoadResource(hMod, hRes);
    if (!hLoaded) return {};

    void* pData = LockResource(hLoaded);
    if (!pData) return {};

    std::vector<unsigned char> bytes(size);
    memcpy(bytes.data(), pData, size);
    return bytes;
}

// ---------------------------------------------------------------------------
// Get texture pointer (via Nexus cache)
// ---------------------------------------------------------------------------
static void* GetIcon(const std::string& key)
{
    if (!APIDefs) return nullptr;
    Texture_t* tex = APIDefs->Textures_Get((kTexPrefix + key).c_str());
    return tex ? tex->Resource : nullptr;
}

// ---------------------------------------------------------------------------
// Render icon via DrawList (no cursor movement)
// ---------------------------------------------------------------------------
static void DrawListIcon(ImDrawList* dl, const std::string& key,
                          float x, float y, float size,
                          ImVec4 tint = {1,1,1,1})
{
    void* tex = GetIcon(key);
    if (tex)
        dl->AddImage(reinterpret_cast<ImTextureID>(tex),
                     {x, y}, {x + size, y + size},
                     {0,0}, {1,1},
                     ImGui::ColorConvertFloat4ToU32(tint));
}

// ---------------------------------------------------------------------------
// Render icon via ImGui::Image (moves cursor)
// ---------------------------------------------------------------------------
static void ImageIcon(const std::string& key, float size, ImVec4 tint = {1,1,1,1})
{
    void* tex = GetIcon(key);
    if (tex)
        ImGui::Image(reinterpret_cast<ImTextureID>(tex),
                     {size, size}, {0,0}, {1,1}, tint);
    else
        ImGui::Dummy({size, size});
}

static void OpenUrl(const char* url)
{
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

// ---------------------------------------------------------------------------
// Colors
// ---------------------------------------------------------------------------
static constexpr ImU32 COL_SECTION_HDR_BG  = IM_COL32( 36,  32,  20, 255);
static constexpr ImU32 COL_SECTION_BORDER   = IM_COL32( 58,  48,  24, 255);
static constexpr ImU32 COL_DIVIDER          = IM_COL32( 52,  46,  28, 220);
static constexpr ImU32 COL_STEP_NUM_BG      = IM_COL32( 42,  30,   8, 255);
static constexpr ImU32 COL_STEP_NUM_BORDER  = IM_COL32( 74,  48,  16, 255);
static constexpr ImU32 COL_LINK_BORDER      = IM_COL32( 46,  46,  46, 255);
static constexpr ImU32 COL_LINK_HOVER_BORDER= IM_COL32( 74,  48,  16, 255);
static constexpr ImU32 COL_VERSION_BG       = IM_COL32( 26,  42,  26, 255);
static constexpr ImU32 COL_VERSION_BORDER   = IM_COL32( 42,  90,  42, 255);

static const ImVec4 COL_ACCENT         = {0.851f, 0.549f, 0.227f, 1.f};
static const ImVec4 COL_TEXT_PRIMARY   = {0.800f, 0.800f, 0.800f, 1.f};
static const ImVec4 COL_TEXT_SECONDARY = {0.600f, 0.580f, 0.520f, 1.f};
static const ImVec4 COL_TEXT_DIM       = {0.380f, 0.360f, 0.300f, 1.f};
static const ImVec4 COL_GREEN          = {0.267f, 0.733f, 0.400f, 1.f};
static const ImVec4 COL_BLUE           = {0.290f, 0.604f, 0.933f, 1.f};
static const ImVec4 COL_RED            = {0.867f, 0.333f, 0.333f, 1.f};
static const ImVec4 COL_PURPLE         = {0.733f, 0.400f, 0.733f, 1.f};

// ---------------------------------------------------------------------------
// Section header helper
// ---------------------------------------------------------------------------
static void SectionHeader(const std::string& iconKey, const char* title, ImVec4 iconTint)
{
    ImDrawList* dl    = ImGui::GetWindowDrawList();
    float       w     = ImGui::GetContentRegionAvail().x;
    float       lineH = ImGui::GetTextLineHeight();
    float       iconSz= 14.f;
    float       h     = std::max(lineH, iconSz) + 10.f;
    ImVec2      pos   = ImGui::GetCursorScreenPos();

    dl->AddRectFilled(pos, {pos.x + w, pos.y + h}, COL_SECTION_HDR_BG, 4.f);
    dl->AddRect      (pos, {pos.x + w, pos.y + h}, COL_SECTION_BORDER, 4.f, 0, 0.5f);

    float cy = pos.y + (h - iconSz) * 0.5f;
    DrawListIcon(dl, iconKey, pos.x + 8.f, cy, iconSz, iconTint);

    float ty = pos.y + (h - lineH) * 0.5f;
    dl->AddText({pos.x + 8.f + iconSz + 5.f, ty},
                ImGui::ColorConvertFloat4ToU32({0.780f, 0.720f, 0.620f, 1.f}),
                title);

    ImGui::Dummy({w, h});
}

// ---------------------------------------------------------------------------
// Init — load all icons from DLL resources into Nexus texture cache
// ---------------------------------------------------------------------------
void Init(const char* /*addonDir*/)
{
    if (!APIDefs) return;

    for (const auto& [key, resourceId] : kIconResources)
    {
        std::string texId = kTexPrefix + key;
        auto bytes = GetResourceBytes(resourceId);
        if (!bytes.empty())
            APIDefs->Textures_GetOrCreateFromMemory(
                texId.c_str(),
                bytes.data(),
                static_cast<uint32_t>(bytes.size()));
    }
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------
void Render()
{
    const float panelW = ImGui::GetContentRegionAvail().x;
    const float iconSz = 16.f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  {6.f, 5.f});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {8.f, 4.f});

    // =========================================================================
    // HERO CARD
    // =========================================================================
    {
        ImDrawList* dl    = ImGui::GetWindowDrawList();
        ImVec2      pos   = ImGui::GetCursorScreenPos();
        float       lineH = ImGui::GetTextLineHeight();
        float       boxSz = 48.f;
        float textBlockH  = lineH * 1.15f + 4.f + lineH + 4.f + (lineH + 4.f);
        float h           = std::max(boxSz + 16.f, textBlockH + 16.f);

        dl->AddRectFilled(pos, {pos.x + panelW, pos.y + h}, IM_COL32(34,28,14,255), 6.f);
        dl->AddRect      (pos, {pos.x + panelW, pos.y + h}, COL_SECTION_BORDER, 6.f, 0, 0.5f);

        // Wheat icon box
        float boxX = pos.x + 10.f;
        float boxY = pos.y + (h - boxSz) * 0.5f;
        dl->AddRectFilled({boxX, boxY}, {boxX+boxSz, boxY+boxSz}, COL_STEP_NUM_BG, 6.f);
        dl->AddRect      ({boxX, boxY}, {boxX+boxSz, boxY+boxSz}, COL_STEP_NUM_BORDER, 6.f, 0, 0.5f);

        // Wheat icon (32px inside 48px box)
        float iconOff = (boxSz - 32.f) * 0.5f;
        ImGui::SetCursorScreenPos({boxX + iconOff, boxY + iconOff});
        ImageIcon("Emoji wheat 32", 32.f);

        // Title / subtitle / version badge via DrawList
        float textX = boxX + boxSz + 12.f;
        float textY = pos.y + 8.f;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 1.15f,
                    {textX, textY},
                    ImGui::ColorConvertFloat4ToU32({0.878f, 0.627f, 0.314f, 1.f}),
                    "FarmingTracker");
        float titleH = ImGui::GetFontSize() * 1.15f;
        dl->AddText({textX, textY + titleH + 3.f},
                    ImGui::ColorConvertFloat4ToU32(COL_TEXT_DIM),
                    "Guild Wars 2  \xc2\xb7  Nexus Addon");

        float badgeY = textY + titleH + 3.f + lineH + 5.f;
        std::string vstr = std::string("v") + VERSION;
        ImVec2 vtsz  = ImGui::CalcTextSize(vstr.c_str());
        float  badgeW = vtsz.x + 14.f, badgeH = lineH + 4.f;
        dl->AddRectFilled({textX, badgeY}, {textX+badgeW, badgeY+badgeH}, COL_VERSION_BG, 4.f);
        dl->AddRect      ({textX, badgeY}, {textX+badgeW, badgeY+badgeH}, COL_VERSION_BORDER, 4.f, 0, 0.5f);
        dl->AddText({textX + 7.f, badgeY + 2.f},
                    ImGui::ColorConvertFloat4ToU32(COL_GREEN), vstr.c_str());

        ImGui::SetCursorScreenPos({pos.x, pos.y + h + 6.f});
    }

    // =========================================================================
    // GETTING STARTED
    // =========================================================================
    SectionHeader("ti-rocket", "Getting Started", COL_ACCENT);
    ImGui::Separator();

    struct Step { const char* title; const char* desc; };
    static const Step steps[] =
    {
        { "Add your account",
          "Go to Settings -> Accounts and enter your GW2 API key.\n"
          "Permissions needed: inventories, wallet & tradingpost.\n"
          "Also add your DRF Token for real-time inventory tracking." },
        { "Start a session",
          "Press Alt+F to open the main window (reassignable in Settings).\n"
          "Click Reset session to begin tracking from scratch." },
        { "Track your drops",
          "Play normally - FarmingTracker auto-detects inventory changes via DRF\n"
          "and prices everything via the GW2 Trading Post." },
        { "Review results",
          "Check Overview for totals, Drops for item breakdown,\n"
          "or Timeline to see value over time." },
    };

    ImGui::Spacing();
    for (int i = 0; i < 4; i++)
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float       numSz = 20.f;

        dl->AddCircleFilled({pos.x + numSz * 0.5f, pos.y + numSz * 0.5f},
                            numSz * 0.5f, COL_STEP_NUM_BG);
        dl->AddCircle      ({pos.x + numSz * 0.5f, pos.y + numSz * 0.5f},
                            numSz * 0.5f, COL_STEP_NUM_BORDER, 0, 0.5f);
        char num[3]; snprintf(num, sizeof(num), "%d", i + 1);
        ImVec2 nsz = ImGui::CalcTextSize(num);
        dl->AddText({pos.x + numSz * 0.5f - nsz.x * 0.5f,
                     pos.y + numSz * 0.5f - nsz.y * 0.5f},
                    ImGui::ColorConvertFloat4ToU32(COL_ACCENT), num);

        ImGui::Dummy({numSz, 1.f});
        ImGui::SameLine(0, 8.f);
        ImGui::BeginGroup();
        ImGui::TextColored({0.800f, 0.780f, 0.700f, 1.f}, "%s", steps[i].title);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(COL_TEXT_DIM));
        ImGui::TextUnformatted(steps[i].desc);
        ImGui::PopStyleColor();
        ImGui::EndGroup();
        
        // Add buttons for GW2 API Key and DRF Token after the first step
        if (i == 0)
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
            
            if (ImGui::Button("Get GW2 API Key"))
            {
                OpenUrl("https://www.guildwars2.com/");
            }
            ImGui::SameLine(0, 8.0f);
            if (ImGui::Button("Get DRF Token"))
            {
                OpenUrl("https://drf.rs/");
            }
            
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
        }
        
        if (i < 3) ImGui::Spacing();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // =========================================================================
    // DEFAULT KEYBINDS
    // =========================================================================
    SectionHeader("ti-keyboard", "Default Keybinds", COL_PURPLE);
    ImGui::Separator();
    ImGui::Spacing();

    struct Bind { const char* action; const char* keys; };
    static const Bind binds[] =
    {
        { "Toggle main window", "CTRL + F"         },
        { "Toggle mini window", "CTRL + SHIFT + M" },
        { "Reset session",      "CTRL + SHIFT + R" },
    };

    for (auto& b : binds)
    {
        ImGui::TextColored(COL_TEXT_SECONDARY, "%s", b.action);
        ImGui::SameLine(panelW * 0.55f);

        bool notSet = (std::string(b.keys) == "not set");
        if (notSet)
        {
            ImGui::TextColored(COL_TEXT_DIM, "-- not set --");
        }
        else
        {
            ImDrawList* dl   = ImGui::GetWindowDrawList();
            ImVec2      bpos = ImGui::GetCursorScreenPos();
            std::string ks   = b.keys;
            float cx = bpos.x, cy = bpos.y;
            float bh = ImGui::GetTextLineHeight() + 4.f;
            float pad = 6.f;

            auto DrawKey = [&](const char* k)
            {
                ImVec2 tsz = ImGui::CalcTextSize(k);
                float  bw  = tsz.x + pad * 2.f;
                dl->AddRectFilled({cx,cy},{cx+bw,cy+bh},IM_COL32(26,26,26,255),3.f);
                dl->AddRect      ({cx,cy},{cx+bw,cy+bh},IM_COL32(56,56,56,255),3.f,0,0.5f);
                dl->AddText({cx+pad,cy+2.f},IM_COL32(136,136,136,255),k);
                cx += bw;
            };

            size_t plus = ks.find(" + ");
            if (plus != std::string::npos)
            {
                DrawKey(ks.substr(0, plus).c_str());
                dl->AddText({cx+3.f,cy+2.f},IM_COL32(68,68,68,255),"+");
                cx += ImGui::CalcTextSize("+").x + 6.f;
                DrawKey(ks.substr(plus+3).c_str());
            }
            else DrawKey(ks.c_str());

            ImGui::Dummy({cx - bpos.x, bh});
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // =========================================================================
    // CHANGELOG
    // =========================================================================
    SectionHeader("ti-history", "Changelog", COL_BLUE);
    ImGui::Separator();
    ImGui::Spacing();

    // v2.0.0.7
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        std::string vLabel = "v2.0.0.7";
        float bw = ImGui::CalcTextSize(vLabel.c_str()).x + 10.f, bh = ImGui::GetTextLineHeight() + 4.f;
        dl->AddRectFilled(pos, { pos.x + bw, pos.y + bh }, COL_VERSION_BG, 3.f);
        dl->AddRect(pos, { pos.x + bw, pos.y + bh }, COL_VERSION_BORDER, 3.f, 0, 0.5f);
        dl->AddText({ pos.x + 5.f, pos.y + 2.f },
                    ImGui::ColorConvertFloat4ToU32(COL_GREEN),
                    vLabel.c_str());
        ImGui::Dummy({ bw, bh });
        ImGui::SameLine(0, 8.f);
        ImGui::TextColored(COL_TEXT_DIM, "latest");
    }

    static const char* notes_v2007[] =
    {
        "Added new sorting options for the Drops tab.",
        "Fixed several bugs.",
        "Updated the options in Settings -> Drops.",
    };
    ImGui::Spacing();
    for (auto& n : notes_v2007)
    {
        ImGui::TextColored(COL_TEXT_DIM, "  \xe2\x80\xa2");
        ImGui::SameLine(0, 5.f);
        ImGui::TextColored(COL_TEXT_SECONDARY, "%s", n);
    }

    ImGui::Spacing();

    // v2.0.0.6
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        std::string vLabel = "v2.0.0.6";
        float bw = ImGui::CalcTextSize(vLabel.c_str()).x + 10.f, bh = ImGui::GetTextLineHeight() + 4.f;
        dl->AddRectFilled(pos, { pos.x + bw, pos.y + bh }, COL_STEP_NUM_BG, 3.f);
        dl->AddRect(pos, { pos.x + bw, pos.y + bh }, COL_STEP_NUM_BORDER, 3.f, 0, 0.5f);
        dl->AddText({ pos.x + 5.f, pos.y + 2.f },
                    ImGui::ColorConvertFloat4ToU32(COL_ACCENT),
                    vLabel.c_str());
        ImGui::Dummy({ bw, bh });
    }

    static const char* notes_v2006[] =
    {
        "Languages improved",
        "New options for icon and font sizes",
        "Improved visibility of numbers on icons",
    };
    ImGui::Spacing();
    for (auto& n : notes_v2006)
    {
        ImGui::TextColored(COL_TEXT_DIM, "  \xe2\x80\xa2");
        ImGui::SameLine(0, 5.f);
        ImGui::TextColored(COL_TEXT_SECONDARY, "%s", n);
    }

    ImGui::Spacing();

    // v2.0.0.5
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        std::string vLabel = "v2.0.0.5";
        float bw = ImGui::CalcTextSize(vLabel.c_str()).x + 10.f, bh = ImGui::GetTextLineHeight() + 4.f;
        dl->AddRectFilled(pos, { pos.x + bw, pos.y + bh }, COL_STEP_NUM_BG, 3.f);
        dl->AddRect(pos, { pos.x + bw, pos.y + bh }, COL_STEP_NUM_BORDER, 3.f, 0, 0.5f);
        dl->AddText({ pos.x + 5.f, pos.y + 2.f },
                    ImGui::ColorConvertFloat4ToU32(COL_ACCENT),
                    vLabel.c_str());
        ImGui::Dummy({ bw, bh });
    }

    static const char* notes_v2005[] =
    {
        "Fixes & Improvements: General bug fixes and performance improvements.",
        "Overview Tab: Added a new Overview tab (inspired by Nyx).",
        "Drops Settings: Added new configuration options to the Drops settings tab.",
        "Item Ignoring: Added the ability to ignore items either once or until the next reset (inspired by Nyx).",
        "Delete Option: Added a Delete option to the context menu to remove items/currencies from tracking.",
    };
    ImGui::Spacing();
    for (auto& n : notes_v2005)
    {
        ImGui::TextColored(COL_TEXT_DIM, "  \xe2\x80\xa2");
        ImGui::SameLine(0, 5.f);
        ImGui::TextColored(COL_TEXT_SECONDARY, "%s", n);
    }

    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Spacing();

    // =========================================================================
    // LINKS
    // =========================================================================
    SectionHeader("ti-link","Links",COL_GREEN);
    ImGui::Separator();
    ImGui::Spacing();

    struct LinkDef { const char* iconKey; const char* label; const char* sub; const char* url; };
    static const LinkDef links[] =
    {
        { "ti-brand-github", "GitHub Repository",
          "github.com/xLiviax-Dev/FarmingTracker", GITHUB_URL },
        { "ti-bug", "Report a Bug",
          "github.com/xLiviax-Dev/FarmingTracker/issues", GITHUB_ISSUES },
    };

    for (auto& lk : links)
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float       lineH = ImGui::GetTextLineHeight();
        float       h   = lineH * 2.f + 14.f;
        bool hovered = ImGui::IsMouseHoveringRect(pos,{pos.x+panelW,pos.y+h});
        ImU32 borderCol = hovered ? COL_LINK_HOVER_BORDER : COL_LINK_BORDER;

        dl->AddRectFilled(pos,{pos.x+panelW,pos.y+h},IM_COL32(30,30,30,255),4.f);
        dl->AddRect      (pos,{pos.x+panelW,pos.y+h},borderCol,4.f,0,0.5f);

        ImVec4 iconTintV = hovered ? COL_ACCENT : ImVec4{0.6f,0.6f,0.6f,1.f};
        float  iconX = pos.x + 10.f;
        float  iconY = pos.y + (h - iconSz) * 0.5f;
        DrawListIcon(dl, lk.iconKey, iconX, iconY, iconSz, iconTintV);

        float lc = hovered ? 1.f : 0.78f;
        float tx = iconX + iconSz + 8.f;
        float ty1 = pos.y + h * 0.5f - lineH - 1.f;
        float ty2 = pos.y + h * 0.5f + 1.f;
        dl->AddText({tx,ty1},IM_COL32((int)(lc*255),(int)(lc*200),(int)(lc*140),255),lk.label);
        dl->AddText({tx,ty2},ImGui::ColorConvertFloat4ToU32(COL_TEXT_DIM),lk.sub);

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushID(lk.url);
        if (ImGui::InvisibleButton("##link",{panelW,h})) OpenUrl(lk.url);
        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // =========================================================================
    // CREDITS
    // =========================================================================
    SectionHeader("ti-heart","Credits",COL_RED);
    ImGui::Separator();
    ImGui::Spacing();

    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float       avSz = 28.f;

        dl->AddRectFilled(pos,{pos.x+avSz,pos.y+avSz},COL_STEP_NUM_BG,5.f);
        dl->AddRect      (pos,{pos.x+avSz,pos.y+avSz},COL_STEP_NUM_BORDER,5.f,0,0.5f);
        ImVec2 initSz = ImGui::CalcTextSize("xL");
        dl->AddText({pos.x+(avSz-initSz.x)*0.5f, pos.y+(avSz-initSz.y)*0.5f},
                    ImGui::ColorConvertFloat4ToU32(COL_ACCENT),"xL");

        ImGui::Dummy({avSz,1.f});
        ImGui::SameLine(0,8.f);
        ImGui::BeginGroup();
        ImGui::TextColored(COL_ACCENT,"%s",AUTHOR);
        ImGui::TextColored(COL_TEXT_DIM,"Developer & Designer");
        ImGui::EndGroup();
    }

    ImGui::Spacing();

    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        dl->AddLine(pos,{pos.x+panelW,pos.y},COL_DIVIDER,0.5f);
        ImGui::Dummy({0,4.f});
    }

    struct AckDef { const char* iconKey; const char* text; ImVec4 tint; };
    static const AckDef acks[] =
    {
        { "ti-heart-filled", "Built with Nexus Addon Framework",                COL_RED },
        { "ti-heart-filled", "Thanks to Raidcore and drf.rs for the tools and",  COL_RED },
        { "ti-heart-filled", "services that made this project possible.",         COL_RED },
    };
    for (auto& a : acks)
    {
        ImageIcon(a.iconKey, 13.f, a.tint);
        ImGui::SameLine(0,6.f);
        ImGui::TextColored(COL_TEXT_DIM,"%s",a.text);
    }

    ImGui::Spacing();
    ImGui::PopStyleVar(2);
}

} // namespace UIInfo
