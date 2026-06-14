#include "ui_settings.h"
#include "ui_tab_icons.h"
#include "ui_notifications.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "gw2_fetcher.h"
#include "auto_reset.h"
#include "session_history.h"
#include "backup_restore.h"
#include "localization.h"
#include "magnetite_tracker.h"
#include "shared.h"
#include <imgui/imgui_internal.h>
#include <algorithm>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <thread>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")

namespace UISettings
{

// =============================================================================
// Sidebar navigation state
// =============================================================================
enum class SettingsPage
{
    General      = 0,
    Account      = 1,
    Appearance   = 2,
    Windows      = 3,
    Tabs         = 4,
    DataReset    = 5,
    Favorites    = 6,
    Notifications= 7,
    Performance  = 8,
    Advanced     = 9,
    Export       = 10,
    COUNT
};

static SettingsPage s_CurrentPage = SettingsPage::General;

// =============================================================================
// Colors
// =============================================================================
static constexpr ImU32 COL_SIDEBAR_BG        = IM_COL32( 17,  17,  19, 255);
static constexpr ImU32 COL_SIDEBAR_BORDER     = IM_COL32( 42,  40,  32, 255);
static constexpr ImU32 COL_NAV_ACTIVE_BG      = IM_COL32( 42,  32,  16, 255);
static constexpr ImU32 COL_NAV_ACTIVE_STRIPE  = IM_COL32(192,  96,  32, 255);
static constexpr ImU32 COL_NAV_ACTIVE_TEXT    = IM_COL32(232, 160,  96, 255);
static constexpr ImU32 COL_NAV_HOVER_BG       = IM_COL32( 37,  34,  24, 255);
static constexpr ImU32 COL_NAV_TEXT           = IM_COL32(160, 144, 112, 255);
static constexpr ImU32 COL_NAV_SEP            = IM_COL32( 42,  40,  32, 255);
static constexpr ImU32 COL_SECTION_HDR_BG     = IM_COL32( 36,  32,  20, 255);
static constexpr ImU32 COL_SECTION_HDR_BORDER = IM_COL32( 58,  48,  24, 255);
static constexpr ImU32 COL_SECTION_HDR_TEXT   = IM_COL32(224, 160,  96, 255);
static constexpr ImU32 COL_SECTION_ARROW      = IM_COL32(200, 122,  48, 255);
static constexpr ImU32 COL_ROW_BORDER         = IM_COL32( 38,  36,  24, 255);
static constexpr ImU32 COL_ROW_HOVER          = IM_COL32( 37,  35,  22, 255);
static constexpr ImU32 COL_LABEL_TEXT         = IM_COL32(184, 168, 136, 255);
static constexpr ImU32 COL_DIM_TEXT           = IM_COL32(136, 120,  80, 255);
static constexpr ImU32 COL_SUBHDR_TEXT        = IM_COL32(136, 112,  64, 255);
static constexpr ImU32 COL_SEP_LINE           = IM_COL32( 46,  42,  26, 255);
static constexpr ImU32 COL_STATUS_ON          = IM_COL32( 64, 176,  48, 255);
static constexpr ImU32 COL_STATUS_OFF         = IM_COL32(176,  48,  32, 255);
static constexpr ImU32 COL_ACC_SEL_BG         = IM_COL32( 42,  30,  12, 255);
static constexpr ImU32 COL_ACC_SEL_BORDER     = IM_COL32(192,  96,  32, 255);

// =============================================================================
// Helper: styled label text
// =============================================================================
static void LabelText(const char* text, ImU32 col = COL_LABEL_TEXT)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(col));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

// =============================================================================
// Helper: sub-header
// =============================================================================
static void SubHeader(const char* text, const char* iconKey = nullptr)
{
    ImGui::Spacing();
    
    if (iconKey)
    {
        void* iconTex = UITabIcons::GetIcon(iconKey);
        if (iconTex)
        {
            float sz = 16.0f;
            float ty = ImGui::GetCursorPosY() + (ImGui::GetTextLineHeight() - sz) * 0.5f;
            ImGui::SetCursorPosY(ty);
            ImGui::Image(reinterpret_cast<ImTextureID>(iconTex), ImVec2(sz, sz), ImVec2(0, 0), ImVec2(1, 1), ImGui::ColorConvertU32ToFloat4(COL_SUBHDR_TEXT));
            ImGui::SameLine(0, 5.0f);
            ImGui::SetCursorPosY(ty - (ImGui::GetTextLineHeight() - sz) * 0.5f); // Reset Y slightly for text
        }
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(COL_SUBHDR_TEXT));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2      pos = ImGui::GetCursorScreenPos();
    float       w   = ImGui::GetContentRegionAvail().x;
    dl->AddLine(pos, ImVec2(pos.x + w, pos.y), COL_SEP_LINE, 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
}

// =============================================================================
// Helper: separator with centred label
// =============================================================================
static void SepLabel(const char* text)
{
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    float       avW = ImGui::GetContentRegionAvail().x;
    ImVec2      p   = ImGui::GetCursorScreenPos();
    float       th  = ImGui::GetTextLineHeight();
    float       ty  = p.y + th * 0.5f;
    float       tw  = ImGui::CalcTextSize(text).x;
    float       lx  = p.x + (avW - tw) * 0.5f;
    dl->AddLine(ImVec2(p.x,           ty), ImVec2(lx - 4.0f,       ty), COL_SEP_LINE, 0.5f);
    dl->AddLine(ImVec2(lx + tw + 4.0f, ty), ImVec2(p.x + avW,      ty), COL_SEP_LINE, 0.5f);
    dl->AddText(ImVec2(lx, p.y), COL_SUBHDR_TEXT, text);
    ImGui::SetCursorPosY(p.y + th + 4.0f);
}

// =============================================================================
// Helper: collapsible section header
// =============================================================================
static bool BeginSection(const char* id, const char* title, bool defaultOpen = true, const char* iconKey = nullptr)
{
    static bool s_Open[64];
    static bool s_Init[64];
    ImGuiID hash = ImHashStr(id);
    int     idx  = static_cast<int>(hash % 64u);
    if (!s_Init[idx]) { s_Open[idx] = defaultOpen; s_Init[idx] = true; }

    ImDrawList* dl  = ImGui::GetWindowDrawList();
    float       avW = ImGui::GetContentRegionAvail().x;
    const float H   = 24.0f;
    ImVec2      pos = ImGui::GetCursorScreenPos();

    dl->AddRectFilled(pos, ImVec2(pos.x + avW, pos.y + H), COL_SECTION_HDR_BG);
    dl->AddRect      (pos, ImVec2(pos.x + avW, pos.y + H), COL_SECTION_HDR_BORDER, 0.0f, 0, 0.5f);

    ImGui::PushID(id);
    ImGui::InvisibleButton("##sechdr", ImVec2(avW, H));
    if (ImGui::IsItemClicked()) s_Open[idx] = !s_Open[idx];
    ImGui::PopID();

    const char* arrow = s_Open[idx] ? "v" : ">";
    float ay = pos.y + (H - ImGui::GetTextLineHeight()) * 0.5f;
    dl->AddText(ImVec2(pos.x + 8.0f,  ay), COL_SECTION_ARROW,   arrow);

    // Optional icon before title
    float titleX = pos.x + 22.0f;
    if (iconKey)
    {
        const float iconSz = 14.0f;
        void* iconTex = UITabIcons::GetIcon(iconKey);
        if (iconTex)
        {
            float iconY = pos.y + (H - iconSz) * 0.5f;
            dl->AddImage(reinterpret_cast<ImTextureID>(iconTex),
                         ImVec2(titleX, iconY),
                         ImVec2(titleX + iconSz, iconY + iconSz),
                         ImVec2(0,0), ImVec2(1,1), COL_SECTION_HDR_TEXT);
            titleX += iconSz + 5.0f;
        }
    }
    dl->AddText(ImVec2(titleX, ay), COL_SECTION_HDR_TEXT, title);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + H));
    if (s_Open[idx])
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg,  ImVec4(0.11f, 0.11f, 0.09f, 1.0f));
        ImGui::PushStyleVar (ImGuiStyleVar_ChildBorderSize, 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Border,   ImGui::ColorConvertU32ToFloat4(COL_ROW_BORDER));
        ImGui::BeginChild((std::string("##secbody_") + id).c_str(),
            ImVec2(avW, 0.0f), true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Spacing();
        ImGui::Indent(8.0f);
    }
    return s_Open[idx];
}

static void EndSection()
{
    ImGui::Unindent(8.0f);
    ImGui::Spacing();
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::Spacing();
}

// =============================================================================
// Helper: sidebar nav item
// =============================================================================
static void DrawNavItem(const char* label, SettingsPage page, float itemW)
{
    // Icon key per settings page
    static const char* kPageIcons[] = {
        "general",         // General
        "account_connection", // Account  — mapped via tabicons key below
        "appearance",      // Appearance
        "windows",         // Windows  — mapped via tabicons key below
        "tabs",            // Tabs
        "data_reset",      // DataReset — mapped via tabicons key below
        "favorites",       // Favorites
        "notifications",   // Notifications — mapped via tabicons key below
        "performance",     // Performance
        "advanced",        // Advanced
        "export",          // Export
    };
    // UITabIcons keys that match the tabicons folder names
    // (must be registered in UITabIcons::kIconResources)
    static const char* kTabIconKeys[] = {
        "general",         // General
        "account_connection", // Account
        "appearance",      // Appearance  
        "windows",         // Windows
        "tabs",            // Tabs
        "data_reset",      // DataReset
        "favorites",       // Favorites
        "notifications",   // Notifications
        "performance",     // Performance
        "advanced",        // Advanced
        "export",          // Export
    };

    ImDrawList* dl     = ImGui::GetWindowDrawList();
    const float H      = 26.0f;
    const float iconSz = 14.0f;
    ImVec2      pos    = ImGui::GetCursorScreenPos();
    bool        active = (s_CurrentPage == page);

    ImGui::PushID(static_cast<int>(page));
    ImGui::InvisibleButton("##nav", ImVec2(itemW, H));
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();
    ImGui::PopID();

    if (clicked) s_CurrentPage = page;

    if (active)
    {
        dl->AddRectFilled(pos, ImVec2(pos.x + itemW, pos.y + H), COL_NAV_ACTIVE_BG);
        dl->AddRectFilled(pos, ImVec2(pos.x + 2.0f,  pos.y + H), COL_NAV_ACTIVE_STRIPE);
    }
    else if (hovered)
    {
        dl->AddRectFilled(pos, ImVec2(pos.x + itemW, pos.y + H), COL_NAV_HOVER_BG);
    }

    // Icon
    int pageIdx = static_cast<int>(page);
    if (pageIdx >= 0 && pageIdx < static_cast<int>(SettingsPage::COUNT))
    {
        void* iconTex = UITabIcons::GetIcon(kTabIconKeys[pageIdx]);
        if (iconTex)
        {
            float iconY = pos.y + (H - iconSz) * 0.5f;
            ImU32 tint  = active  ? COL_NAV_ACTIVE_TEXT
                        : hovered ? IM_COL32(200, 180, 140, 255)
                        :           COL_NAV_TEXT;
            dl->AddImage(reinterpret_cast<ImTextureID>(iconTex),
                         ImVec2(pos.x + 8.0f, iconY),
                         ImVec2(pos.x + 8.0f + iconSz, iconY + iconSz),
                         ImVec2(0,0), ImVec2(1,1), tint);
        }
    }

    // Label (shifted right to make room for icon)
    float ty = pos.y + (H - ImGui::GetTextLineHeight()) * 0.5f;
    ImU32 tc = active ? COL_NAV_ACTIVE_TEXT : COL_NAV_TEXT;
    dl->AddText(ImVec2(pos.x + 8.0f + iconSz + 6.0f, ty), tc, label);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + H));
}

static void DrawNavSep(float itemW)
{
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2      pos = ImGui::GetCursorScreenPos();
    dl->AddLine(ImVec2(pos.x + 6.0f, pos.y + 3.0f),
                ImVec2(pos.x + itemW - 6.0f, pos.y + 3.0f),
                COL_NAV_SEP, 0.5f);
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 6.0f));
}

// =============================================================================
// Page: General
// =============================================================================
static void RenderPage_General()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("lang", Localization::GetText("general_settings"), false, "general"))
    {
        LabelText(Localization::GetText("language_settings"));
        const char* languageItems[] = {
            Localization::GetText("language_english"),
            Localization::GetText("language_german"),
            Localization::GetText("language_french"),
            Localization::GetText("language_spanish"),
            Localization::GetText("language_chinese"),
            Localization::GetText("language_czech"),
            Localization::GetText("language_italian"),
            Localization::GetText("language_polish"),
            Localization::GetText("language_portuguese"),
            Localization::GetText("language_russian"),
            Localization::GetText("language_danish"),
            Localization::GetText("language_greek"),
            Localization::GetText("language_finnish"),
            Localization::GetText("language_hungarian"),
            Localization::GetText("language_dutch"),
            Localization::GetText("language_norwegian"),
            Localization::GetText("language_romanian"),
            Localization::GetText("language_swedish")
        };
        int langIdx = 0;
        if      (g_Settings.language == "German")     langIdx = 1;
        else if (g_Settings.language == "French")     langIdx = 2;
        else if (g_Settings.language == "Spanish")    langIdx = 3;
        else if (g_Settings.language == "Chinese")    langIdx = 4;
        else if (g_Settings.language == "Czech")      langIdx = 5;
        else if (g_Settings.language == "Italian")    langIdx = 6;
        else if (g_Settings.language == "Polish")     langIdx = 7;
        else if (g_Settings.language == "Portuguese") langIdx = 8;
        else if (g_Settings.language == "Russian")    langIdx = 9;
        else if (g_Settings.language == "Danish")     langIdx = 10;
        else if (g_Settings.language == "Greek")      langIdx = 11;
        else if (g_Settings.language == "Finnish")    langIdx = 12;
        else if (g_Settings.language == "Hungarian")  langIdx = 13;
        else if (g_Settings.language == "Dutch")      langIdx = 14;
        else if (g_Settings.language == "Norwegian")  langIdx = 15;
        else if (g_Settings.language == "Romanian")   langIdx = 16;
        else if (g_Settings.language == "Swedish")    langIdx = 17;

        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::Combo("##Language", &langIdx, languageItems, 18))
        {
            {
                std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                switch (langIdx)
                {
                    case 0: g_Settings.language = "English";    break;
                    case 1: g_Settings.language = "German";     break;
                    case 2: g_Settings.language = "French";     break;
                    case 3: g_Settings.language = "Spanish";    break;
                    case 4: g_Settings.language = "Chinese";    break;
                    case 5: g_Settings.language = "Czech";      break;
                    case 6: g_Settings.language = "Italian";    break;
                    case 7: g_Settings.language = "Polish";     break;
                    case 8: g_Settings.language = "Portuguese"; break;
                    case 9: g_Settings.language = "Russian";    break;
                    case 10: g_Settings.language = "Danish";    break;
                    case 11: g_Settings.language = "Greek";     break;
                    case 12: g_Settings.language = "Finnish";    break;
                    case 13: g_Settings.language = "Hungarian"; break;
                    case 14: g_Settings.language = "Dutch";     break;
                    case 15: g_Settings.language = "Norwegian"; break;
                    case 16: g_Settings.language = "Romanian";  break;
                    case 17: g_Settings.language = "Swedish";   break;
                }
            }
            Localization::SetLanguage(Localization::StringToLanguage(g_Settings.language));
            ItemTracker::ClearItemDetails();
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(Localization::GetText("language_tooltip"));

        ImGui::Spacing();
        if (ImGui::Checkbox(Localization::GetText("show_main_window"), &g_Settings.showMainWindow)) SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("show_mini_window"), &g_Settings.showMiniWindow)) SettingsManager::Save();

        EndSection();
    }

    if (BeginSection("magnetite", Localization::GetText("magnetite_tracker"), false, "magnetite"))
    {
        if (ImGui::Checkbox(Localization::GetText("enable_magnetite_tracker"), &g_Settings.enableMagnetiteTracker)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_magnetite_tracker_tooltip"));

        if (g_Settings.enableMagnetiteTracker)
        {
            ImGui::Spacing();
            LabelText(Localization::GetText("magnetite_weekly_earned"));
            int earned = MagnetiteTracker::GetWeeklyEarned();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputInt("##MagnetiteManual", &earned))
            {
                MagnetiteTracker::SetWeeklyEarned(earned);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("magnetite_manual_edit_tooltip"));
            ImGui::SameLine();
            ImGui::Text("/ %d", MagnetiteTracker::WEEKLY_CAP);

            ImGui::Spacing();
            ImGui::Text("%s", Localization::GetText("magnetite_api_check_cooldown")); ImGui::SameLine();
            ImGui::SetNextItemWidth(85.0f);
            if (ImGui::InputInt("##MagCooldown", &g_Settings.magnetiteApiCheckCooldownMin, 1, 5))
            { g_Settings.magnetiteApiCheckCooldownMin = std::max(1, std::min(60, g_Settings.magnetiteApiCheckCooldownMin)); SettingsManager::Save(); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Minimum time between wallet API checks. Default: 10 min.");
            
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f,0.10f,0.10f,0.40f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f,0.13f,0.13f,0.70f));
            ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.00f,1.00f,1.00f,1.00f));
            if (ImGui::Button("Reset weekly counter##MagReset")) ImGui::OpenPopup("MagResetConfirm");
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Manually resets the weekly counter to 0.");
            if (ImGui::BeginPopup("MagResetConfirm")) {
                ImGui::TextUnformatted(Localization::GetText("reset_confirm"));
                ImGui::TextUnformatted("This will reset the weekly Magnetite Shard counter to 0.");
                ImGui::Spacing();
                if (ImGui::Button(Localization::GetText("yes_reset"))) { MagnetiteTracker::OnWeeklyReset(); ImGui::CloseCurrentPopup(); }
                ImGui::SameLine();
                if (ImGui::Button(Localization::GetText("no_cancel"))) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::Spacing();
            std::string lastCheck;
            { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex); lastCheck = g_Settings.magnetiteLastApiCheckUtc; }
            if (!lastCheck.empty()) ImGui::Text(Localization::GetText("magnetite_last_wallet_check"), MagnetiteTracker::UtcToLocal(lastCheck).c_str());
            else ImGui::TextDisabled("%s", Localization::GetText("magnetite_wallet_not_queried"));
        }
        EndSection();
    }

    if (BeginSection("seshist", Localization::GetText("session_history"), false, "session_history"))
    {
        if (ImGui::Checkbox(Localization::GetText("enable_session_history"), &g_Settings.enableSessionHistory))
        { SettingsManager::Save(); SessionHistory::SetEnabled(g_Settings.enableSessionHistory); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_session_history_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("overwrite_session_history"), &g_Settings.overwriteSessionHistory))
        { SettingsManager::Save(); SessionHistory::SetOverwrite(g_Settings.overwriteSessionHistory); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("overwrite_session_history_tooltip"));
        LabelText(Localization::GetText("max_session_history"));
        if (ImGui::SliderInt("##MaxSessHist", &g_Settings.maxSessionHistory, 1, 50, "%d"))
        { SettingsManager::Save(); SessionHistory::SetMaxSessions(g_Settings.maxSessionHistory); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("max_session_history_tooltip"));
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Account
// =============================================================================
static void RenderPage_Account()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("accounts", Localization::GetText("account_management"), false, "account_connection"))
    {
        if (g_Settings.accounts.empty())
        {
            LabelText(Localization::GetText("no_accounts_configured"), COL_DIM_TEXT);
        }
        else
        {
            ImDrawList* dl  = ImGui::GetWindowDrawList();
            float       avW = ImGui::GetContentRegionAvail().x;
            for (int i = 0; i < (int)g_Settings.accounts.size(); ++i)
            {
                const auto& acc   = g_Settings.accounts[i];
                bool        isSel = (i == g_Settings.currentAccountIndex);
                ImVec2      pos   = ImGui::GetCursorScreenPos();
                const float H     = 27.0f;

                ImGui::PushID(i);
                ImGui::InvisibleButton("##accrow", ImVec2(avW, H));
                if (ImGui::IsItemClicked() && i != g_Settings.currentAccountIndex)
                {
                    {
                        std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                        g_Settings.currentAccountIndex = i;
                        g_Settings.drfToken  = acc.drfToken;
                        g_Settings.gw2ApiKey = acc.gw2ApiKey;
                    }
                    { std::lock_guard<std::mutex> lock(UICommon::s_AccountNameMutex);
                      strncpy_s(UICommon::s_AccountNameBuf, acc.name.c_str(), sizeof(UICommon::s_AccountNameBuf)); }
                    strncpy_s(UICommon::s_AccountDrfBuf, acc.drfToken.c_str(),  sizeof(UICommon::s_AccountDrfBuf));
                    strncpy_s(UICommon::s_AccountGw2Buf, acc.gw2ApiKey.c_str(), sizeof(UICommon::s_AccountGw2Buf));
                    if (!g_Settings.drfToken.empty() && SettingsManager::IsTokenValid(g_Settings.drfToken))
                        DrfClient::Connect(g_Settings.drfToken);
                    Gw2Fetcher::UpdateApiKey();
                    ItemTracker::SafeReset();
                    const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
                    ItemTracker::SaveData(dir);
                    SettingsManager::Save();
                }
                ImGui::PopID();

                ImU32 rowBg = isSel ? COL_ACC_SEL_BG : (ImGui::IsItemHovered() ? COL_ROW_HOVER : IM_COL32(0,0,0,0));
                dl->AddRectFilled(pos, ImVec2(pos.x + avW, pos.y + H), rowBg);
                if (isSel) dl->AddRect(pos, ImVec2(pos.x + avW, pos.y + H), COL_ACC_SEL_BORDER, 0.0f, 0, 0.5f);

                bool hasToken = !acc.drfToken.empty();
                dl->AddCircleFilled(ImVec2(pos.x + 10.0f, pos.y + H * 0.5f), 4.0f,
                    hasToken ? COL_STATUS_ON : COL_STATUS_OFF);

                float ty = pos.y + (H - ImGui::GetTextLineHeight()) * 0.5f;
                dl->AddText(ImVec2(pos.x + 20.0f, ty), isSel ? COL_NAV_ACTIVE_TEXT : COL_LABEL_TEXT, acc.name.c_str());

                const char* tag    = hasToken ? "active" : "no token";
                ImU32       tagCol = hasToken ? IM_COL32(64,176,48,255) : IM_COL32(176,80,48,255);
                float       tagW   = ImGui::CalcTextSize(tag).x + 8.0f;
                const float tagH   = 19.0f;
                float       tagX   = pos.x + avW - tagW - 4.0f;
                float       tagY   = pos.y + (H - tagH) * 0.5f;
                dl->AddRectFilled(ImVec2(tagX,tagY), ImVec2(tagX+tagW,tagY+tagH),
                    hasToken ? IM_COL32(20,58,16,255) : IM_COL32(58,20,16,255), 2.0f);
                dl->AddRect(ImVec2(tagX,tagY), ImVec2(tagX+tagW,tagY+tagH), tagCol, 2.0f, 0, 0.5f);
                dl->AddText(ImVec2(tagX+4.0f, tagY+(tagH-ImGui::GetTextLineHeight())*0.5f), tagCol, tag);

                ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + H + 2.0f));
            }
        }

        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("add_account")))
        {
            Account newAcc;
            newAcc.name = std::string(Localization::GetText("account_prefix")) + " " + std::to_string(g_Settings.accounts.size() + 1);
            { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
              g_Settings.accounts.push_back(newAcc);
              g_Settings.currentAccountIndex = (int)g_Settings.accounts.size() - 1; }
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("add_account_tooltip"));
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.65f, 0.10f, 0.10f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.15f, 0.15f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.50f, 0.08f, 0.08f, 1.00f));
        if (ImGui::Button(Localization::GetText("remove_account")) && g_Settings.accounts.size() > 1)
        {
            { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
              g_Settings.accounts.erase(g_Settings.accounts.begin() + g_Settings.currentAccountIndex);
              if (g_Settings.currentAccountIndex >= (int)g_Settings.accounts.size())
                  g_Settings.currentAccountIndex = (int)g_Settings.accounts.size() - 1; }
            SettingsManager::Save();
        }
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("remove_account_tooltip"));

        // Edit Account section — integrated into Account Management
        if (!g_Settings.accounts.empty() && g_Settings.currentAccountIndex >= 0 &&
            g_Settings.currentAccountIndex < (int)g_Settings.accounts.size())
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            char editAccountLabel[256];
            snprintf(editAccountLabel, sizeof(editAccountLabel), Localization::GetText("edit_account"), g_Settings.accounts[g_Settings.currentAccountIndex].name.c_str());
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", editAccountLabel);
            
            static bool s_AccInit = false;
            if (!s_AccInit) {
                { std::lock_guard<std::mutex> lock(UICommon::s_AccountNameMutex);
                  strncpy_s(UICommon::s_AccountNameBuf, g_Settings.accounts[g_Settings.currentAccountIndex].name.c_str(), sizeof(UICommon::s_AccountNameBuf)); }
                strncpy_s(UICommon::s_AccountDrfBuf, g_Settings.accounts[g_Settings.currentAccountIndex].drfToken.c_str(),  sizeof(UICommon::s_AccountDrfBuf));
                strncpy_s(UICommon::s_AccountGw2Buf, g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey.c_str(), sizeof(UICommon::s_AccountGw2Buf));
                s_AccInit = true;
            }

            LabelText(Localization::GetText("account_name"));
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputText("##AccountName", UICommon::s_AccountNameBuf, sizeof(UICommon::s_AccountNameBuf)))
                g_Settings.accounts[g_Settings.currentAccountIndex].name = UICommon::s_AccountNameBuf;

            LabelText(Localization::GetText("drf_token_label"));
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputText("##AccountDrfToken", UICommon::s_AccountDrfBuf, sizeof(UICommon::s_AccountDrfBuf)))
                g_Settings.accounts[g_Settings.currentAccountIndex].drfToken = UICommon::s_AccountDrfBuf;

            LabelText(Localization::GetText("gw2_api_key_label"));
            if (!SettingsManager::IsGw2ApiKeyPlausible(UICommon::s_AccountGw2Buf) && UICommon::s_AccountGw2Buf[0] != '\0')
            { ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), Localization::GetText("api_key_invalid_format")); }
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputText("##AccountGw2Key", UICommon::s_AccountGw2Buf, sizeof(UICommon::s_AccountGw2Buf)))
                g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey = UICommon::s_AccountGw2Buf;

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.50f, 0.15f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.65f, 0.20f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.08f, 0.40f, 0.12f, 1.00f));
            if (ImGui::Button(Localization::GetText("save_account")))
            {
                { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                  g_Settings.accounts[g_Settings.currentAccountIndex].name     = UICommon::s_AccountNameBuf;
                  g_Settings.accounts[g_Settings.currentAccountIndex].drfToken = UICommon::s_AccountDrfBuf;
                  g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey= UICommon::s_AccountGw2Buf;
                  g_Settings.drfToken  = UICommon::s_AccountDrfBuf;
                  g_Settings.gw2ApiKey = UICommon::s_AccountGw2Buf; }
                if (!g_Settings.drfToken.empty() && SettingsManager::IsTokenValid(g_Settings.drfToken))
                    DrfClient::Connect(g_Settings.drfToken);
                Gw2Fetcher::UpdateApiKey();
                SettingsManager::Save();
            }
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("save_account_tooltip"));
            ImGui::SameLine();
            if (ImGui::Button(Localization::GetText("reload_drf_token")))
                DrfClient::Connect(g_Settings.drfToken);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("reconnect_drf_token"));
            ImGui::SameLine();
            if (ImGui::Button(Localization::GetText("reload_gw2_api_key")))
                Gw2Fetcher::UpdateApiKey();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("reload_gw2_api_key_tooltip"));

            ImGui::Spacing();
            const float acR = g_Settings.accentColorR;
            const float acG = g_Settings.accentColorG;
            const float acB = g_Settings.accentColorB;
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(acR, acG, acB, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(acR * 1.2f, acG * 1.2f, acB * 1.2f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(acR * 0.8f, acG * 0.8f, acB * 0.8f, 1.00f));
            if (ImGui::Button(Localization::GetText("get_drf_token")))
            {
                ShellExecuteA(nullptr, "open", "https://drf.rs/", nullptr, nullptr, SW_SHOWNORMAL);
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(acR, acG, acB, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(acR * 1.2f, acG * 1.2f, acB * 1.2f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(acR * 0.8f, acG * 0.8f, acB * 0.8f, 1.00f));
            if (ImGui::Button(Localization::GetText("get_gw2_api_key")))
            {
                ShellExecuteA(nullptr, "open", "https://www.guildwars2.com/", nullptr, nullptr, SW_SHOWNORMAL);
            }
            ImGui::PopStyleColor(3);
        }

        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Appearance
// =============================================================================
static void RenderPage_Appearance()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("icons", Localization::GetText("icons_borders"), false, "icons_borders"))
    {
        if (ImGui::Checkbox(Localization::GetText("show_item_icons"), &g_Settings.showItemIcons)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_item_icons_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("history_icon_size"), &g_Settings.historyIconSize, 16, 96)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("history_icon_size_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("profit_icon_size"), &g_Settings.profitIconSize, 16, 96)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("profit_icon_size_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("items_icon_size"), &g_Settings.itemsIconSize, 16, 96)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("items_icon_size_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("timeline_icon_size_items"), &g_Settings.timelineIconSizeItems, 16, 96)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("timeline_icon_size_items_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("timeline_icon_size_currencies"), &g_Settings.timelineIconSizeCurrencies, 16, 48)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("timeline_icon_size_currencies_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("grid_icon_size_items"), &g_Settings.gridIconSize, 16, 96)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("grid_icon_size_items_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("grid_icon_size_currencies"), &g_Settings.gridIconSizeCurrencies, 16, 96)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("grid_icon_size_currencies_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("show_rarity_borders"), &g_Settings.showRarityBorder)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_rarity_borders_tooltip"));
        if (g_Settings.showRarityBorder)
        {
            if (ImGui::SliderFloat(Localization::GetText("border_size"), &g_Settings.rarityBorderSize, 0.0f, 10.0f)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("border_size_tooltip"));
        }
        EndSection();
    }

    if (BeginSection("colors", Localization::GetText("colors_gradients"), false, "color-swatch"))
    {
        ImGui::Text("%s:", Localization::GetText("accent_color")); ImGui::SameLine();
        ImVec4 accentColor(g_Settings.accentColorR, g_Settings.accentColorG, g_Settings.accentColorB, 1.0f);
        if (ImGui::ColorEdit3("##AccentColor", (float*)&accentColor, ImGuiColorEditFlags_NoInputs))
        { g_Settings.accentColorR = accentColor.x; g_Settings.accentColorG = accentColor.y;
          g_Settings.accentColorB = accentColor.z; SettingsManager::Save(); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("accent_color_tooltip"));

        ImGui::Spacing();
        if (ImGui::Checkbox(Localization::GetText("gradient_backgrounds"), &g_Settings.enableGradientBackgrounds)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("gradient_backgrounds_tooltip"));
        if (g_Settings.enableGradientBackgrounds)
        {
            ImGui::SameLine();
            ImGui::Text("%s:", Localization::GetText("top_gradient_color")); ImGui::SameLine();
            ImGui::ColorEdit3("##TopGrad", g_Settings.gradientTopColor, ImGuiColorEditFlags_NoInputs);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("top_gradient_color_tooltip"));
            ImGui::SameLine();
            ImGui::Text("%s:", Localization::GetText("bottom_gradient_color")); ImGui::SameLine();
            ImGui::ColorEdit3("##BotGrad", g_Settings.gradientBottomColor, ImGuiColorEditFlags_NoInputs);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("bottom_gradient_color_tooltip"));
            if (ImGui::IsItemDeactivatedAfterEdit()) SettingsManager::Save();
        }
        if (ImGui::Checkbox(Localization::GetText("show_profit_sparkline"), &g_Settings.showProfitSparkline)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_profit_sparkline_tooltip"));
        
        ImGui::Separator();
        ImGui::Spacing();
        
        LabelText(Localization::GetText("sparkline_color"));
        float sparklineColor[3] = {
            ((g_Settings.sparklineColor >> 16) & 0xFF) / 255.0f,
            ((g_Settings.sparklineColor >> 8) & 0xFF) / 255.0f,
            (g_Settings.sparklineColor & 0xFF) / 255.0f
        };
        if (ImGui::ColorEdit3("##SparklineColor", sparklineColor, ImGuiColorEditFlags_NoInputs))
        {
            g_Settings.sparklineColor = 
                ((int)(sparklineColor[0] * 255.0f) << 16) |
                ((int)(sparklineColor[1] * 255.0f) << 8) |
                (int)(sparklineColor[2] * 255.0f);
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("sparkline_color_tooltip"));
        
        ImGui::Separator();
        ImGui::Spacing();
        
        EndSection();
    }


    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Windows
// =============================================================================
static void RenderPage_Windows()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("mainwin", Localization::GetText("main_window_settings"), false, "main_window"))
    {
        if (ImGui::Checkbox(Localization::GetText("main_window_click_through"), &g_Settings.mainWindowClickThrough)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("main_window_click_through_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("main_window_hide_title_bar"), &g_Settings.mainWindowHideTitleBar)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("main_window_hide_title_bar_tooltip"));
        ImGui::Spacing();
        LabelText(Localization::GetText("main_window_font_size"));
        if (ImGui::SliderFloat("##MainFontSize", &g_Settings.mainWindowFontSize, 0.5f, 2.0f, "%.2f"))
        { SettingsManager::Save(); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("main_window_font_size_tooltip"));
        EndSection();
    }

    if (BeginSection("miniwin", Localization::GetText("mini_window_settings"), false, "mini_window"))
    {
        if (ImGui::Checkbox(Localization::GetText("mini_window_click_through"),  &g_Settings.miniWindowClickThrough))  SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_click_through_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("mini_window_locked"),          &g_Settings.miniWindowLocked))        SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_locked_tooltip"));

        SubHeader(Localization::GetText("mini_window_widget"));
        if (ImGui::Checkbox(Localization::GetText("mini_window_show_profit"),             &g_Settings.miniWindowShowProfit))              SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("mini_window_show_profit_per_hour"),    &g_Settings.miniWindowShowProfitPerHour))       SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("mini_window_show_tp_sell"),            &g_Settings.miniWindowShowTradingProfitSell))   SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("mini_window_show_tp_instant"),         &g_Settings.miniWindowShowTradingProfitInstant))SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("mini_window_show_total_items"),        &g_Settings.miniWindowShowTotalItems))          SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("mini_window_show_session_duration"),   &g_Settings.miniWindowShowSessionDuration))     SettingsManager::Save();
        if (ImGui::Checkbox(Localization::GetText("enable_best_drop_in_mini_window"),     &g_Settings.enableBestDropInMiniWindow))        SettingsManager::Save();
        if (g_Settings.enableBestDropInMiniWindow || g_Settings.miniWindowShowBestDropTotalValue)
        {
            ImGui::Indent();
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_best_drop_single"), &g_Settings.enableBestDropInMiniWindow)) SettingsManager::Save();
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_best_drop_total"),  &g_Settings.miniWindowShowBestDropTotalValue)) SettingsManager::Save();
            ImGui::Unindent();
        }
        
        SubHeader("");
        if (ImGui::Checkbox(Localization::GetText("mini_window_hide_title_bar"), &g_Settings.miniWindowHideTitleBar)) 
        {
            g_Settings.miniWindowHideBorder = g_Settings.miniWindowHideTitleBar;
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_hide_title_bar_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("mini_window_enable_text_shadow"), &g_Settings.miniWindowEnableTextShadow)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_enable_text_shadow_tooltip"));
        
        LabelText(Localization::GetText("mini_window_font_size"));
        if (ImGui::SliderFloat("##MiniFontSize", &g_Settings.miniWindowFontSize, 10.0f, 30.0f, "%.1f"))
        {
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_font_size_tooltip"));
        
        LabelText(Localization::GetText("mini_window_text_color"));
        float textColor[3] = {
            ((g_Settings.miniWindowTextColor >> 16) & 0xFF) / 255.0f,
            ((g_Settings.miniWindowTextColor >> 8) & 0xFF) / 255.0f,
            (g_Settings.miniWindowTextColor & 0xFF) / 255.0f
        };
        if (ImGui::ColorEdit3("##MiniTextColor", textColor, ImGuiColorEditFlags_NoInputs))
        {
            g_Settings.miniWindowTextColor = 
                ((int)(textColor[0] * 255.0f) << 16) |
                ((int)(textColor[1] * 255.0f) << 8) |
                (int)(textColor[2] * 255.0f);
            SettingsManager::Save();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_text_color_tooltip"));
        
        SubHeader("");
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", Localization::GetText("mini_window_element_order"));
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_element_order_tooltip"));
        
        // Element order UI with table for alignment
        if (ImGui::BeginTable("ElementOrderTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Element", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableHeadersRow();
            
            for (int i = 0; i < static_cast<int>(g_Settings.miniWindowElementOrder.size()); ++i)
            {
                ImGui::PushID(i);
                ImGui::TableNextRow();
                
                // Element name
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", g_Settings.miniWindowElementOrder[i].c_str());
                
                // Action buttons
                ImGui::TableSetColumnIndex(1);
                if (i > 0)
                {
                    if (ImGui::Button("^"))
                    {
                        std::swap(g_Settings.miniWindowElementOrder[i], g_Settings.miniWindowElementOrder[i - 1]);
                        SettingsManager::Save();
                    }
                    ImGui::SameLine();
                }
                
                if (i < static_cast<int>(g_Settings.miniWindowElementOrder.size()) - 1)
                {
                    if (ImGui::Button("v"))
                    {
                        std::swap(g_Settings.miniWindowElementOrder[i], g_Settings.miniWindowElementOrder[i + 1]);
                        SettingsManager::Save();
                    }
                }
                
                ImGui::PopID();
            }
            
            ImGui::EndTable();
        }
        
        EndSection();
    }

    if (BeginSection("opacity", Localization::GetText("window_opacity"), false, "main_window"))
    {
        LabelText(Localization::GetText("main_window_opacity"));
        float mainPct = (1.0f - g_Settings.mainWindowOpacity) * 100.0f;
        if (ImGui::SliderFloat("##MainOpacity", &mainPct, 0.0f, 100.0f, "%.0f%%"))
        { g_Settings.mainWindowOpacity = 1.0f - mainPct / 100.0f; SettingsManager::Save(); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("main_window_opacity_tooltip"));
        LabelText(Localization::GetText("mini_window_opacity"));
        float miniPct = (1.0f - g_Settings.miniWindowOpacity) * 100.0f;
        if (ImGui::SliderFloat("##MiniOpacity", &miniPct, 0.0f, 100.0f, "%.0f%%"))
        { g_Settings.miniWindowOpacity = 1.0f - miniPct / 100.0f; SettingsManager::Save(); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("mini_window_opacity_tooltip"));
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Tabs
// =============================================================================
static void RenderPage_Tabs()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("tabs", Localization::GetText("tabs_settings"), false, "tabs"))
    {
        if (ImGui::Checkbox(Localization::GetText("lock_tab_order"), &g_Settings.lockTabOrder)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("lock_tab_order_tooltip"));

        SubHeader(Localization::GetText("tabs_description"));
        if (ImGui::Checkbox(Localization::GetText("enable_drops_tab"),           &g_Settings.enableDropsTab))          SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_drops_tab_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("enable_session_history_tab"), &g_Settings.enableSessionHistoryTab)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_session_history_tab_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("enable_timeline_tab"),        &g_Settings.enableTimelineTab))       SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_timeline_tab_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("enable_custom_profit"),       &g_Settings.enableCustomProfit))      SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_custom_profit_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("enable_loot_log_tab"),        &g_Settings.enableLootLog))           SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_loot_log_tab_tooltip"));
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Data & Reset
// =============================================================================
static void RenderPage_DataReset()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("autoreset", Localization::GetText("reset_settings"), false, "reset"))
    {
        const char* resetModes[] = {
            Localization::GetText("auto_reset_never"),
            Localization::GetText("auto_reset_on_load"),
            Localization::GetText("auto_reset_daily"),
            Localization::GetText("auto_reset_weekly"),
            Localization::GetText("auto_reset_weekly_na_wvw"),
            Localization::GetText("auto_reset_weekly_eu_wvw"),
            Localization::GetText("auto_reset_weekly_map_bonus"),
            Localization::GetText("auto_reset_minutes_unload"),
            Localization::GetText("auto_reset_custom_days")
        };
        LabelText(Localization::GetText("auto_reset_label"));
        ImGui::SetNextItemWidth(240.0f);
        if (ImGui::Combo("##AutoReset", &g_Settings.automaticResetMode, resetModes, 9))
        { SettingsManager::Save(); AutoReset::RefreshSchedule(); }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("auto_reset_tooltip"));

        if (g_Settings.automaticResetMode == 7)
        {
            char lbl[256]; snprintf(lbl, sizeof(lbl), "%s##minrst", Localization::GetText("minutes_after_unload_tooltip"));
            if (ImGui::InputInt(lbl, &g_Settings.minutesUntilResetAfterShutdown))
            { g_Settings.minutesUntilResetAfterShutdown = std::clamp(g_Settings.minutesUntilResetAfterShutdown, 1, 24*60);
              SettingsManager::Save(); AutoReset::RefreshSchedule(); }
        }
        if (g_Settings.automaticResetMode == 8)
        {
            int sd = g_Settings.customResetDays;
            char lbl[256]; snprintf(lbl, sizeof(lbl), "%s##customdays", Localization::GetText("reset_interval_days"));
            if (ImGui::SliderInt(lbl, &sd, 1, 30))
            { g_Settings.customResetDays = sd; SettingsManager::Save(); AutoReset::RefreshSchedule(); }
        }
        char nrLbl[256];
        snprintf(nrLbl, sizeof(nrLbl), Localization::GetText("next_reset_utc"), AutoReset::GetNextResetDisplayUtc().c_str());
        LabelText(nrLbl, COL_DIM_TEXT);
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Favorites
// =============================================================================
static void RenderPage_Favorites()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("favcol", Localization::GetText("favorites_settings"), false, "favorites"))
    {
        LabelText(Localization::GetText("favorites_colors"));
        if (ImGui::Checkbox(Localization::GetText("enable_favorite_text_color"), &g_Settings.enableFavoriteTextColor)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_favorite_text_color_tooltip"));
        if (g_Settings.enableFavoriteTextColor) {
            ImGui::SameLine();
            if (ImGui::ColorEdit3("##FavTextCol", g_Settings.favoriteTextColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("text_color"));
        }
        if (ImGui::Checkbox(Localization::GetText("enable_favorite_row_color"), &g_Settings.enableFavoriteRowColor)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_favorite_row_color_tooltip"));
        if (g_Settings.enableFavoriteRowColor) {
            ImGui::SameLine();
            if (ImGui::ColorEdit3("##FavRowCol", g_Settings.favoriteRowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("row_color"));
        }
        ImGui::Spacing();
        if (ImGui::Checkbox(Localization::GetText("enable_best_drop_highlight"), &g_Settings.enableBestDropHighlight)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_best_drop_highlight_tooltip"));
        if (g_Settings.enableBestDropHighlight) {
            ImGui::SameLine();
            if (ImGui::ColorEdit3("##BestDropCol", g_Settings.bestDropHighlightColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("row_color"));
        }
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Notifications
// =============================================================================
static void RenderPage_Notifications()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("notifgen", Localization::GetText("notification_settings"), false, "notifications"))
    {
        if (ImGui::Checkbox(Localization::GetText("enable_notifications"), &g_Settings.enableNotifications)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_notifications_tooltip"));

        if (g_Settings.enableNotifications)
        {
            if (ImGui::Checkbox(Localization::GetText("show_notification_setup"), &g_Settings.showNotificationSetup)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("show_notification_setup_tooltip"));
            if (ImGui::SliderFloat(Localization::GetText("notification_duration"), &g_Settings.notificationDuration, 1.0f, 20.0f, "%.1f s")) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_duration_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("notification_stacking"), &g_Settings.notificationStacking)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_stacking_tooltip"));
            ImGui::Spacing();
            LabelText(Localization::GetText("notification_font_size"));
            if (ImGui::SliderFloat("##NotificationFontSize", &g_Settings.notificationFontSize, 0.5f, 2.0f, "%.2f"))
            { SettingsManager::Save(); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_font_size_tooltip"));

            SubHeader(Localization::GetText("notification_play_sound"));
            if (ImGui::Checkbox("##PlaySound", &g_Settings.notificationPlaySound)) SettingsManager::Save();
            ImGui::SameLine(); LabelText(Localization::GetText("notification_play_sound"));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_play_sound_tooltip"));

            if (g_Settings.notificationPlaySound)
            {
                int volPct = static_cast<int>(g_Settings.notificationVolume * 100.0f);
                if (ImGui::SliderInt(Localization::GetText("notification_volume"), &volPct, 1, 100, "%d%%"))
                { g_Settings.notificationVolume = volPct / 100.0f; UINotifications::SetVolume(g_Settings.notificationVolume); SettingsManager::Save(); }

                auto drawSoundRow = [](const char* labelKey, std::string& path, float& volume, bool isPrecursor, bool isInfusion, bool isAlert) {
                    ImGui::PushID(labelKey);
                    ImGui::Text("%s:", Localization::GetText(labelKey));
                    char buf[512]; strncpy_s(buf, sizeof(buf), path.c_str(), _TRUNCATE);
                    ImGui::SetNextItemWidth(250.0f);
                    if (ImGui::InputTextWithHint("##path", Localization::GetText("sound_path_hint"), buf, sizeof(buf)))
                    { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex); path = buf; SettingsManager::Save(); }
                    ImGui::SameLine();
                    if (ImGui::Button("...")) {
                        OPENFILENAMEA ofn; char szFile[512] = {}; ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn); ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
                        ofn.lpstrFilter = "Audio Files\0*.wav;*.mp3;*.flac\0All Files\0*.*\0";
                        ofn.nFilterIndex = 1; ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
                        if (GetOpenFileNameA(&ofn)) { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex); path = szFile; SettingsManager::Save(); }
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("browse_for_file"));
                    ImGui::SameLine(); ImGui::SetNextItemWidth(80.0f);
                    int vp = static_cast<int>(volume * 100.0f);
                    if (ImGui::SliderInt("##vol", &vp, 0, 100, "%d%%"))
                    { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex); volume = vp / 100.0f; SettingsManager::Save(); }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("individual_volume"));
                    ImGui::SameLine();
                    if (ImGui::Button(Localization::GetText("sound_test"))) UINotifications::PlayNotificationSound(isPrecursor, isInfusion, isAlert);
                    ImGui::PopID();
                };
                drawSoundRow("sound_standard", g_Settings.soundPathStandard,  g_Settings.notificationVolumeStandard,  false, false, false);
                drawSoundRow("sound_precursor",g_Settings.soundPathPrecursor, g_Settings.notificationVolumePrecursor, true,  false, false);
                drawSoundRow("sound_infusion", g_Settings.soundPathInfusion,  g_Settings.notificationVolumeInfusion,  false, true,  false);
                drawSoundRow("sound_alert",    g_Settings.soundPathAlert,     g_Settings.notificationVolumeAlert,     false, false, true);
            }
        }
        EndSection();
    }

    if (g_Settings.enableNotifications)
    {
        if (BeginSection("itemalerts", Localization::GetText("notification_item_alerts"), false, "drops"))
        {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", Localization::GetText("trigger_drops"));
            ImGui::Checkbox("##EnableValue", &g_Settings.notificationEnableMinValue);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_value_filter_tooltip"));
            ImGui::SameLine();
            bool disVal = !g_Settings.notificationEnableMinValue;
            if (disVal) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f); }
            if (ImGui::InputFloat(Localization::GetText("notification_min_value"), &g_Settings.notificationMinValueGold, 0.1f, 1.0f, "%.2f g"))
            { if (g_Settings.notificationMinValueGold < 0) g_Settings.notificationMinValueGold = 0; SettingsManager::Save(); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_min_value_tooltip"));
            if (disVal) { ImGui::PopStyleVar(); ImGui::PopItemFlag(); }

            ImGui::Checkbox("##EnableRarity", &g_Settings.notificationEnableMinRarity);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_rarity_filter_tooltip"));
            ImGui::SameLine();
            bool disRar = !g_Settings.notificationEnableMinRarity;
            if (disRar) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f); }
            const char* rarityLabels[] = { Localization::GetText("rarity_all"), Localization::GetText("rarity_basic"), Localization::GetText("rarity_fine"), Localization::GetText("rarity_masterwork"), Localization::GetText("rarity_rare"), Localization::GetText("rarity_exotic"), Localization::GetText("rarity_ascended"), Localization::GetText("rarity_legendary") };
            if (ImGui::Combo(Localization::GetText("notification_min_rarity"), &g_Settings.notificationMinRarity, rarityLabels, 8)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_min_rarity_tooltip"));
            if (disRar) { ImGui::PopStyleVar(); ImGui::PopItemFlag(); }

            bool disCom = !g_Settings.notificationEnableMinValue || !g_Settings.notificationEnableMinRarity;
            if (disCom) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f); }
            if (ImGui::Checkbox(Localization::GetText("notification_combine_logic"), &g_Settings.notificationCombineValueAndRarity)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_combine_logic_tooltip"));
            if (disCom) { ImGui::PopStyleVar(); ImGui::PopItemFlag(); }

            if (ImGui::Checkbox(Localization::GetText("notification_include_non_profit"), &g_Settings.notificationIncludeNonProfit)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_include_non_profit_tooltip"));

            SubHeader("");
            if (ImGui::Checkbox(Localization::GetText("notification_precursor_alert"), &g_Settings.notificationPrecursorAlert)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_precursor_alert_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("notification_infusion_alert"),  &g_Settings.notificationInfusionAlert))  SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_infusion_alert_tooltip"));
            if (g_Settings.notificationInfusionAlert) {
                ImGui::Indent();
                if (ImGui::Checkbox(Localization::GetText("notification_include_agony"), &g_Settings.notificationIncludeAgonyInfusions)) SettingsManager::Save();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notification_include_agony_tooltip"));
                ImGui::Unindent();
            }
            EndSection();
        }

        if (BeginSection("blacklist", Localization::GetText("notification_blacklist"), true, "ignored"))
        {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", Localization::GetText("blacklist_add_item"));
            
            // Input field for adding item by ID
            static int blacklistItemId = 0;
            ImGui::InputInt(Localization::GetText("blacklist_item_id"), &blacklistItemId);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("blacklist_item_id_tooltip"));
            
            ImGui::SameLine();
            if (ImGui::Button(Localization::GetText("add")))
            {
                if (blacklistItemId > 0)
                {
                    // Check if item is already in blacklist
                    if (std::find(g_Settings.notificationBlacklist.begin(), g_Settings.notificationBlacklist.end(), blacklistItemId) == g_Settings.notificationBlacklist.end())
                    {
                        g_Settings.notificationBlacklist.push_back(blacklistItemId);
                        SettingsManager::Save();
                    }
                    blacklistItemId = 0;
                }
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Display blacklist items
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", Localization::GetText("notification_blacklist_tooltip"));
            
            if (!g_Settings.notificationBlacklist.empty())
            {
                for (int i = 0; i < static_cast<int>(g_Settings.notificationBlacklist.size()); ++i)
                {
                    int itemId = g_Settings.notificationBlacklist[i];
                    
                    // Get item details
                    auto st = ItemTracker::GetItemStat(itemId);
                    std::string itemName = st.details.loaded ? st.details.name : ("Item " + std::to_string(itemId));
                    std::string iconUrl = st.details.loaded ? st.details.iconUrl : "";
                    
                    // Display item with icon and name
                    ImGui::PushID(i);
                    
                    // Icon
                    if (!iconUrl.empty())
                    {
                        UICommon::EnsureItemIconTexture(itemId, iconUrl);
                        // Try to get the texture from the icon cache
                        // Note: GetIcon doesn't exist, so we'll skip the icon display for now
                        // TODO: Implement proper icon display when icon cache is accessible
                    }
                    
                    // Name
                    ImGui::Text("%s", itemName.c_str());
                    
                    // Remove button (red X)
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                    if (ImGui::Button("X", ImVec2(20, 20)))
                    {
                        g_Settings.notificationBlacklist.erase(g_Settings.notificationBlacklist.begin() + i);
                        SettingsManager::Save();
                        ImGui::PopStyleColor(2);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopStyleColor(2);
                    
                    ImGui::PopID();
                }
            }
            else
            {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Blacklist is empty)");
            }
            
            EndSection();
        }

        if (BeginSection("sessalerts", Localization::GetText("notification_session_alerts"), false, "profit"))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", Localization::GetText("trigger_profit_goal"));
            if (ImGui::Checkbox(Localization::GetText("notify_profit_goal"), &g_Settings.notifyProfitGoal)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notify_profit_goal_tooltip"));
            if (g_Settings.notifyProfitGoal) {
                float gold = g_Settings.profitGoalAmount / 10000.0f;
                if (ImGui::InputFloat(Localization::GetText("profit_goal_amount"), &gold, 1.0f, 10.0f, "%.2f g"))
                { g_Settings.profitGoalAmount = static_cast<int>(gold * 10000); SettingsManager::Save(); }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("profit_goal_amount_tooltip"));
            }
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", Localization::GetText("trigger_time_reset"));
            if (ImGui::Checkbox(Localization::GetText("notify_reset_warning"), &g_Settings.notifyResetWarning)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notify_reset_warning_tooltip"));
            if (g_Settings.notifyResetWarning) {
                if (ImGui::SliderInt(Localization::GetText("reset_warning_minutes"), &g_Settings.resetWarningMinutes, 1, 60)) SettingsManager::Save();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("reset_warning_minutes_tooltip"));
            }
            if (ImGui::Checkbox(Localization::GetText("notify_session_complete"), &g_Settings.notifySessionComplete)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("notify_session_complete_tooltip"));
            if (g_Settings.notifySessionComplete) {
                if (ImGui::SliderInt(Localization::GetText("session_complete_hours"), &g_Settings.sessionCompleteHours, 1, 24)) SettingsManager::Save();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("session_complete_hours_tooltip"));
            }
            EndSection();
        }
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Performance
// =============================================================================
static void RenderPage_Performance()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    if (BeginSection("perf", Localization::GetText("performance_settings"), false, "performance"))
    {
        if (ImGui::Checkbox(Localization::GetText("disable_complex_visuals"), &g_Settings.disableComplexVisualsOnLowPerf)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("disable_complex_visuals_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("enable_icon_cache"), &g_Settings.enableIconCache)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_icon_cache_tooltip"));
        if (g_Settings.enableIconCache) {
            ImGui::Indent();
            if (ImGui::InputInt(Localization::GetText("icon_cache_max_icons"), &g_Settings.iconCacheMaxIcons, 10, 50))
            { g_Settings.iconCacheMaxIcons = std::clamp(g_Settings.iconCacheMaxIcons, 10, 2000); SettingsManager::Save(); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("icon_cache_max_icons_tooltip"));
            ImGui::Unindent();
        }
        ImGui::Spacing();
        if (ImGui::SliderInt(Localization::GetText("max_history_items_limit"), &g_Settings.maxHistoryItems, 50, 2000, "%d Items")) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("max_history_items_limit_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("api_update_interval"), &g_Settings.priceUpdateIntervalMin, 5, 15, "%d Min")) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("api_update_interval_tooltip"));
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Export & Backup
// =============================================================================
static void RenderPage_Export()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    // --- Export / Import & Full Backup Section ---
    if (BeginSection("export_backup_combined", Localization::GetText("export_settings"), false, "export"))
    {
        // Settings File
        SubHeader(Localization::GetText("export_settings"), "export");
        LabelText(Localization::GetText("export_tooltip"), COL_DIM_TEXT);
        if (ImGui::Button(Localization::GetText("export"), ImVec2(120, 0))) ImGui::OpenPopup("Export Settings");
        
        ImGui::Spacing();
        LabelText(Localization::GetText("import_tooltip"), COL_DIM_TEXT);
        if (ImGui::Button(Localization::GetText("import"), ImVec2(120, 0))) ImGui::OpenPopup("Import Settings");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Full Backup
        SubHeader(Localization::GetText("full_backup"), "file_csv");
        LabelText(Localization::GetText("full_backup_tooltip"), COL_DIM_TEXT);
        if (ImGui::Button(Localization::GetText("full_backup"), ImVec2(120, 0))) ImGui::OpenPopup("FullBackupConfirm");

        ImGui::Spacing();
        LabelText(Localization::GetText("full_restore_tooltip"), COL_DIM_TEXT);
        if (ImGui::Button(Localization::GetText("full_restore"), ImVec2(120, 0))) ImGui::OpenPopup("FullRestoreConfirm");
        
        EndSection();
    }

    // --- Auto Backup Section ---
    if (BeginSection("auto_backup_section", Localization::GetText("backup_restore"), false, "open_folder"))
    {
        SubHeader(Localization::GetText("backup_restore"), "open_folder");
        if (ImGui::Checkbox(Localization::GetText("enable_automatic_backups"), &g_Settings.enableAutoBackups)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_automatic_backups_tooltip"));

        if (g_Settings.enableAutoBackups)
        {
            const char* freqItems[] = { Localization::GetText("backup_manual_only"), Localization::GetText("backup_daily"), Localization::GetText("backup_weekly") };
            ImGui::SetNextItemWidth(160.0f);
            if (ImGui::Combo("##BackupFreq", &g_Settings.backupFrequency, freqItems, 3)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("backup_frequency_tooltip"));
            char cntLbl[256]; snprintf(cntLbl, sizeof(cntLbl), "%s##BackupCount", Localization::GetText("max_backup_count"));
            if (ImGui::SliderInt(cntLbl, &g_Settings.maxBackupCount, 1, 20)) SettingsManager::Save();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("max_backup_count_tooltip"));

            ImGui::Spacing();
            LabelText(Localization::GetText("backup_path_label"));
            static char s_BackupPathBuf[MAX_PATH] = "";
            static std::string s_LastAutoBackupPath = "___INIT___"; // force first update
            if (s_LastAutoBackupPath != g_Settings.autoBackupPath) {
                strncpy_s(s_BackupPathBuf, g_Settings.autoBackupPath.c_str(), sizeof(s_BackupPathBuf)-1);
                s_LastAutoBackupPath = g_Settings.autoBackupPath;
            }
            float browseW = 28.0f, openW = 28.0f, gap = 4.0f;
            float inputW  = ImGui::GetContentRegionAvail().x - browseW - openW - gap * 2.0f;
            ImGui::SetNextItemWidth(inputW);
            if (ImGui::InputText("##BackupFolder", s_BackupPathBuf, sizeof(s_BackupPathBuf)))
            { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex); g_Settings.autoBackupPath = s_BackupPathBuf; SettingsManager::Save(); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", g_Settings.autoBackupPath.empty() ? Localization::GetText("backup_path_default_tooltip") : g_Settings.autoBackupPath.c_str());
            ImGui::SameLine(0, gap);
            if (ImGui::Button("...##BackupBrowse", ImVec2(browseW, 0)))
            {
                std::string initialPath = g_Settings.autoBackupPath.empty() ? (APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "") : g_Settings.autoBackupPath;
                
                std::thread([initialPath]() {
                    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                    
                    BROWSEINFOA bi = {};
                    bi.lpszTitle = "Select backup folder";
                    // Removed BIF_NEWDIALOGSTYLE to prevent crashes in some environments
                    bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
                    
                    struct BD { const char* p; } bd; 
                    bd.p = initialPath.c_str();
                    bi.lParam = reinterpret_cast<LPARAM>(&bd);
                    bi.lpfn   = [](HWND hwnd, UINT msg, LPARAM, LPARAM lp) -> int {
                        if (msg == BFFM_INITIALIZED) { 
                            auto* d = reinterpret_cast<BD*>(lp);
                            if (d && d->p && d->p[0]) 
                                SendMessageA(hwnd, BFFM_SETSELECTIONA, TRUE, reinterpret_cast<LPARAM>(d->p)); 
                        } 
                        return 0; 
                    };

                    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
                    if (pidl)
                    {
                        char fp[MAX_PATH] = {};
                        if (SHGetPathFromIDListA(pidl, fp))
                        {
                            std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex);
                            g_Settings.autoBackupPath = fp;
                            // Note: s_BackupPathBuf cannot be updated easily from here as it's static in RenderPage_Export
                            // But it will be updated on the next frame because of g_Settings.autoBackupPath
                            SettingsManager::Save();
                        }
                        CoTaskMemFree(pidl);
                    }
                    CoUninitialize();
                }).detach();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("browse_for_folder_tooltip"));
            ImGui::SameLine(0, gap);
            if (ImGui::Button("->##BackupOpen", ImVec2(openW, 0)))
            {
                std::string p = g_Settings.autoBackupPath.empty() ? (APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "") : g_Settings.autoBackupPath;
                if (!p.empty()) ShellExecuteA(NULL, "explore", p.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("open_folder_tooltip"));

            ImGui::Spacing();
            if (UICommon::OrangeGradientButton(Localization::GetText("backup_now_button"), "##ManualBackupBtn"))
            {
                if (BackupRestore::CreateManualBackup())
                {
                    UINotifications::AddGenericNotification(Localization::GetText("backup_success_title"), Localization::GetText("backup_success_msg"), "", "Fine", false, 500.0f);
                }
                else
                {
                    UINotifications::AddGenericNotification(Localization::GetText("backup_failed_title"), Localization::GetText("backup_failed_msg"), "", "Junk", true, 500.0f);
                }
            }
            ImGui::SameLine();
            if (UICommon::OrangeGradientButton(Localization::GetText("restore_backup_button"), "##ManualRestoreBtn"))
            {
                std::string initialPath = g_Settings.autoBackupPath.empty() ? (APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "") : g_Settings.autoBackupPath;
                
                std::thread([initialPath]() {
                    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                    
                    char szFile[MAX_PATH] = { 0 };
                    OPENFILENAMEA ofn = { 0 };
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = NULL;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
                    ofn.nFilterIndex = 1;
                    ofn.lpstrInitialDir = initialPath.c_str();
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

                    if (GetOpenFileNameA(&ofn))
                    {
                        // We use a flag to trigger the actual restore on the main thread next frame
                        // because RestoreFromBackup reloads settings which might affect the current UI state.
                        // For simplicity here, we call it directly but with a notification.
                        if (BackupRestore::LoadBackupFromFile(szFile))
                        {
                            UINotifications::AddGenericNotification(Localization::GetText("restore_success_title"), Localization::GetText("restore_success_msg"), "", "Fine", false, 500.0f);
                        }
                        else
                        {
                            UINotifications::AddGenericNotification(Localization::GetText("restore_failed_title"), Localization::GetText("restore_failed_msg"), "", "Junk", true, 500.0f);
                        }
                    }
                    CoUninitialize();
                }).detach();
            }
        }
        
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Page: Advanced
// =============================================================================
static void RenderPage_Advanced()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark,     ImVec4(0.20f, 0.85f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.10f, 0.50f, 0.15f, 0.80f));

    // Settings Profiles section
    {
        const char* profileTitle = Localization::GetText("settings_profiles");
        static bool s_ProfilesOpen = false;
        ImDrawList* dlp = ImGui::GetWindowDrawList();
        float avWp = ImGui::GetContentRegionAvail().x;
        const float Hp = 24.0f;
        ImVec2 posp = ImGui::GetCursorScreenPos();
        dlp->AddRectFilled(posp, ImVec2(posp.x + avWp, posp.y + Hp), COL_SECTION_HDR_BG);
        dlp->AddRect      (posp, ImVec2(posp.x + avWp, posp.y + Hp), COL_SECTION_HDR_BORDER, 0.0f, 0, 0.5f);
        ImGui::PushID("profiles_hdr");
        ImGui::InvisibleButton("##sechdr", ImVec2(avWp, Hp));
        if (ImGui::IsItemClicked()) s_ProfilesOpen = !s_ProfilesOpen;
        ImGui::PopID();
        const char* arrowp = s_ProfilesOpen ? "v" : ">";
        float ayp = posp.y + (Hp - ImGui::GetTextLineHeight()) * 0.5f;
        dlp->AddText(ImVec2(posp.x + 8.0f,  ayp), COL_SECTION_ARROW,    arrowp);
        // Settings Profiles icon
        {
            const float iconSz = 14.0f;
            float iconX = posp.x + 22.0f;
            void* iconTex = UITabIcons::GetIcon("settings_profiles");
            if (iconTex)
            {
                float iconY = posp.y + (Hp - iconSz) * 0.5f;
                dlp->AddImage(reinterpret_cast<ImTextureID>(iconTex),
                              ImVec2(iconX, iconY),
                              ImVec2(iconX + iconSz, iconY + iconSz),
                              ImVec2(0,0), ImVec2(1,1), COL_SECTION_HDR_TEXT);
                iconX += iconSz + 5.0f;
            }
            dlp->AddText(ImVec2(iconX, ayp), COL_SECTION_HDR_TEXT, profileTitle);
        }
        ImGui::SetCursorScreenPos(ImVec2(posp.x, posp.y + Hp));

        if (s_ProfilesOpen)
        {
            ImGui::Spacing();
            ImGui::Indent(8.0f);

            LabelText(Localization::GetText("profiles_description"), COL_DIM_TEXT);
            ImGui::Spacing();
            if (!g_Settings.settingsProfiles.empty()) {
                std::vector<const char*> names;
                names.push_back(Localization::GetText("default_no_profile"));
                for (const auto& p : g_Settings.settingsProfiles) names.push_back(p.name.c_str());
                int cur = g_Settings.currentProfileIndex + 1;
                ImGui::SetNextItemWidth(200.0f);
                if (ImGui::Combo("##ProfileSel", &cur, names.data(), (int)names.size())) {
                    if (cur == 0) { std::lock_guard<std::recursive_mutex> lock(Settings::s_SettingsMutex); g_Settings.currentProfileIndex = -1; }
                    else SettingsManager::ApplyProfile(cur - 1);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("select_profile_tooltip"));
            } else {
                LabelText(Localization::GetText("no_profiles_created"), COL_DIM_TEXT);
            }
            if (g_Settings.currentProfileIndex >= 0 && g_Settings.currentProfileIndex < (int)g_Settings.settingsProfiles.size()) {
                char lbl[256]; snprintf(lbl, sizeof(lbl), Localization::GetText("current_profile"), g_Settings.settingsProfiles[g_Settings.currentProfileIndex].name.c_str());
                LabelText(lbl, COL_DIM_TEXT);
                if (ImGui::Button(Localization::GetText("update_profile"))) SettingsManager::UpdateProfile(g_Settings.currentProfileIndex);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("update_profile_tooltip"));
                ImGui::SameLine();
                if (ImGui::Button(Localization::GetText("delete_profile"))) SettingsManager::DeleteProfile(g_Settings.currentProfileIndex);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("delete_profile_tooltip"));
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("%s", Localization::GetText("create_new_profile"));
            ImGui::SetNextItemWidth(180.0f);
            ImGui::InputText("##NewProfileName", UICommon::s_NewProfileNameBuf, sizeof(UICommon::s_NewProfileNameBuf));
            ImGui::SameLine(0, 4.0f);
            if (ImGui::Button(Localization::GetText("create")) && UICommon::s_NewProfileNameBuf[0] != '\0')
            { SettingsManager::CreateProfile(UICommon::s_NewProfileNameBuf); UICommon::s_NewProfileNameBuf[0] = '\0'; }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("create_tooltip"));
            ImGui::Spacing();
        }
        ImGui::Spacing();
    }

    if (BeginSection("debug", Localization::GetText("debug_settings"), false, "debug"))
    {
        if (ImGui::Checkbox(Localization::GetText("enable_debug_tab"), &g_Settings.enableDebugTab)) SettingsManager::Save();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("enable_debug_tab_tooltip"));
        EndSection();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Main entry point called by Nexus
// =============================================================================
void RenderOptions()
{
    const float SIDEBAR_W   = 230.0f;
    const float CONTENT_PAD =   8.0f;
    const float TOPBAR_H    =  35.0f;

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    float       winW = ImGui::GetContentRegionAvail().x;
    float       winH = ImGui::GetContentRegionAvail().y;
    ImVec2      orig = ImGui::GetCursorScreenPos();

    // Sidebar background
    dl->AddRectFilled(orig, ImVec2(orig.x + SIDEBAR_W, orig.y + winH), COL_SIDEBAR_BG);
    dl->AddLine(ImVec2(orig.x + SIDEBAR_W, orig.y),
                ImVec2(orig.x + SIDEBAR_W, orig.y + winH),
                COL_SIDEBAR_BORDER, 0.5f);

    // Nav items
    ImGui::SetCursorScreenPos(ImVec2(orig.x, orig.y + 4.0f));
    DrawNavItem(Localization::GetText("general_settings"),      SettingsPage::General,       SIDEBAR_W);
    DrawNavItem(Localization::GetText("account_management"),    SettingsPage::Account,       SIDEBAR_W);
    DrawNavItem(Localization::GetText("appearance_settings"),   SettingsPage::Appearance,    SIDEBAR_W);
    DrawNavItem(Localization::GetText("windows_settings"),      SettingsPage::Windows,       SIDEBAR_W);
    DrawNavItem(Localization::GetText("tabs_settings"),         SettingsPage::Tabs,          SIDEBAR_W);
    DrawNavSep(SIDEBAR_W);
    DrawNavItem(Localization::GetText("reset_settings"),        SettingsPage::DataReset,     SIDEBAR_W);
    DrawNavItem(Localization::GetText("favorites_settings"),    SettingsPage::Favorites,     SIDEBAR_W);
    DrawNavItem(Localization::GetText("notification_settings"), SettingsPage::Notifications, SIDEBAR_W);
    DrawNavItem(Localization::GetText("performance_settings"),  SettingsPage::Performance,   SIDEBAR_W);
    DrawNavSep(SIDEBAR_W);
    DrawNavItem(Localization::GetText("advanced_settings"),     SettingsPage::Advanced,      SIDEBAR_W);
    DrawNavItem(Localization::GetText("export_backup_settings"), SettingsPage::Export,        SIDEBAR_W);

    // Bottom sidebar buttons
    {
        float btnW = SIDEBAR_W - 16.0f;
        float btnX = orig.x + 8.0f;
        float btnY = orig.y + winH - (22.0f * 1.0f + 8.0f * 2.0f); // Adjusted for only 1 button (Reset All)

        // FramePadding sorgt fuer vertikale Textzentrierung in allen Sidebar-Buttons
        float btnH = 22.0f;
        float textH = ImGui::GetTextLineHeight();
        float padY  = (btnH - textH) * 0.5f;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, padY));

        ImGui::SetCursorScreenPos(ImVec2(btnX, btnY));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.50f, 0.08f, 0.08f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.12f, 0.12f, 1.00f));
        if (ImGui::Button(Localization::GetText("reset_all"), ImVec2(btnW, btnH))) ImGui::OpenPopup("Reset Confirm");
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("reset_all_tooltip"));

        ImGui::PopStyleVar(); // FramePadding
    }

    // Popups
    if (ImGui::BeginPopup("Reset Confirm")) {
        ImGui::Text("%s", Localization::GetText("reset_confirm"));
        ImGui::Text("%s", Localization::GetText("reset_warning"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("yes_reset"))) { SettingsManager::ResetToDefaults(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("Export Settings")) {
        static char exportPath[MAX_PATH] = "farming_tracker_settings_export.json";
        ImGui::Text("%s", Localization::GetText("export_settings"));
        ImGui::InputText("##ExportPath", exportPath, sizeof(exportPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export"))) { SettingsManager::ExportToFile(exportPath); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("Import Settings")) {
        static char importPath[MAX_PATH] = "";
        ImGui::Text("%s", Localization::GetText("import_settings"));
        ImGui::InputText("##ImportPath", importPath, sizeof(importPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import"))) { SettingsManager::ImportFromFile(importPath); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("FullBackupConfirm")) {
        static char backupPath[MAX_PATH] = "farming_tracker_full_backup.json";
        ImGui::Text("%s", Localization::GetText("full_backup"));
        ImGui::InputText("##BackupPath", backupPath, sizeof(backupPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("backup"))) {
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string filename = std::string(addonDir ? addonDir : "") + "\\" + backupPath;
            BackupRestore::SaveBackupToFile(filename);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("FullRestoreConfirm")) {
        static char restorePath[MAX_PATH] = "farming_tracker_full_backup.json";
        ImGui::Text("%s", Localization::GetText("full_restore"));
        ImGui::InputText("##RestorePath", restorePath, sizeof(restorePath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("restore"))) {
            const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : "";
            std::string filename = std::string(addonDir ? addonDir : "") + "\\" + restorePath;
            BackupRestore::LoadBackupFromFile(filename);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // Top bar: page title + Save button
    float contentX = orig.x + SIDEBAR_W + CONTENT_PAD;
    float contentW = winW - SIDEBAR_W - CONTENT_PAD * 2.0f;

    const char* pageTitles[] = {
        Localization::GetText("general_settings"),
        Localization::GetText("account_management"),
        Localization::GetText("appearance_settings"),
        Localization::GetText("windows_settings"),
        Localization::GetText("tabs_settings"),
        Localization::GetText("reset_settings"),
        Localization::GetText("favorites_settings"),
        Localization::GetText("notification_settings"),
        Localization::GetText("performance_settings"),
        Localization::GetText("advanced_settings"),
        Localization::GetText("export_backup_settings"),
    };
    const char* pageTitle = pageTitles[static_cast<int>(s_CurrentPage)];
    dl->AddText(ImVec2(contentX, orig.y + (TOPBAR_H - ImGui::GetTextLineHeight()) * 0.5f),
                COL_SECTION_HDR_TEXT, pageTitle);

    float saveBtnW = ImGui::CalcTextSize(Localization::GetText("save")).x + 16.0f;
    ImGui::SetCursorScreenPos(ImVec2(contentX + contentW - saveBtnW, orig.y + (TOPBAR_H - 20.0f) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.50f, 0.15f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.65f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.08f, 0.40f, 0.12f, 1.00f));
    if (ImGui::Button(Localization::GetText("save"), ImVec2(saveBtnW, 25.0f))) SettingsManager::Save();
    ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Localization::GetText("save_tooltip"));

    dl->AddLine(ImVec2(contentX, orig.y + TOPBAR_H),
                ImVec2(contentX + contentW, orig.y + TOPBAR_H),
                COL_SIDEBAR_BORDER, 0.5f);

    // Scrollable content area
    ImGui::SetCursorScreenPos(ImVec2(contentX, orig.y + TOPBAR_H + 4.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::BeginChild("##settings_content", ImVec2(contentW, 0.0f), false, ImGuiWindowFlags_None);

    switch (s_CurrentPage)
    {
        case SettingsPage::General:       RenderPage_General();       break;
        case SettingsPage::Account:       RenderPage_Account();       break;
        case SettingsPage::Appearance:    RenderPage_Appearance();    break;
        case SettingsPage::Windows:       RenderPage_Windows();       break;
        case SettingsPage::Tabs:          RenderPage_Tabs();          break;
        case SettingsPage::DataReset:     RenderPage_DataReset();     break;
        case SettingsPage::Favorites:     RenderPage_Favorites();     break;
        case SettingsPage::Notifications: RenderPage_Notifications(); break;
        case SettingsPage::Performance:   RenderPage_Performance();   break;
        case SettingsPage::Export:        RenderPage_Export();        break;
        case SettingsPage::Advanced:      RenderPage_Advanced();      break;
        default: break;
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

} // namespace UISettings
