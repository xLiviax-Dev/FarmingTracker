#include "ui_settings.h"
#include "settings.h"
#include "item_tracker.h"
#include "drf_client.h"
#include "gw2_fetcher.h"
#include "localization.h"
#include "auto_reset.h"
#include "shared.h"
#include <algorithm>
#include <cstring>

namespace UISettings
{
void RenderOptions()
{
    ImGui::TextUnformatted(Localization::GetText("farming_tracker_title"));
    ImGui::Separator();
    if (ImGui::Checkbox(Localization::GetText("show_main_window"), &g_Settings.showMainWindow))
        SettingsManager::Save();
    if (ImGui::Checkbox(Localization::GetText("show_mini_window"), &g_Settings.showMiniWindow))
        SettingsManager::Save();

    ImGui::Spacing();
    ImGui::Separator();

    // === Quick Actions ===
    ImGui::Text("%s", Localization::GetText("quick_actions"));
    if (ImGui::Button(Localization::GetText("reset_all")))
    {
        ImGui::OpenPopup("Reset Confirm");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("reset_all_tooltip"));

    if (ImGui::BeginPopup("Reset Confirm"))
    {
        ImGui::Text("%s", Localization::GetText("reset_confirm"));
        ImGui::Text("%s", Localization::GetText("reset_warning"));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("yes_reset")))
        {
            SettingsManager::ResetToDefaults();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(Localization::GetText("export")))
    {
        ImGui::OpenPopup("Export Settings");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("export_tooltip"));

    if (ImGui::BeginPopup("Export Settings"))
    {
        static char exportPath[MAX_PATH] = "";
        if (exportPath[0] == '\0')
        {
            strcpy_s(exportPath, "farming_tracker_settings_export.json");
        }
        ImGui::Text("%s", Localization::GetText("export_settings"));
        ImGui::InputText("##ExportPath", exportPath, sizeof(exportPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("export")))
        {
            SettingsManager::ExportToFile(exportPath);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(Localization::GetText("import")))
    {
        ImGui::OpenPopup("Import Settings");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("import_tooltip"));

    if (ImGui::BeginPopup("Import Settings"))
    {
        static char importPath[MAX_PATH] = "";
        ImGui::Text("%s", Localization::GetText("import_settings"));
        ImGui::InputText("##ImportPath", importPath, sizeof(importPath));
        ImGui::Spacing();
        if (ImGui::Button(Localization::GetText("import")))
        {
            SettingsManager::ImportFromFile(importPath);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(Localization::GetText("save")))
    {
        SettingsManager::Save();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", Localization::GetText("save_tooltip"));

    ImGui::Spacing();
    ImGui::Separator();

    // === General Settings ===
    if (ImGui::CollapsingHeader(Localization::GetText("general_settings"), ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Language Selection
        ImGui::TextWrapped(Localization::GetText("language_settings"));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(Localization::GetText("language_tooltip"));

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
            Localization::GetText("language_russian")
        };

        int currentLangIndex = 0;
        if (g_Settings.language == "German") currentLangIndex = 1;
        else if (g_Settings.language == "French") currentLangIndex = 2;
        else if (g_Settings.language == "Spanish") currentLangIndex = 3;
        else if (g_Settings.language == "Chinese") currentLangIndex = 4;
        else if (g_Settings.language == "Czech") currentLangIndex = 5;
        else if (g_Settings.language == "Italian") currentLangIndex = 6;
        else if (g_Settings.language == "Polish") currentLangIndex = 7;
        else if (g_Settings.language == "Portuguese") currentLangIndex = 8;
        else if (g_Settings.language == "Russian") currentLangIndex = 9;

        if (ImGui::Combo("##Language", &currentLangIndex, languageItems, 10))
        {
            switch (currentLangIndex)
            {
                case 0: g_Settings.language = "English"; break;
                case 1: g_Settings.language = "German"; break;
                case 2: g_Settings.language = "French"; break;
                case 3: g_Settings.language = "Spanish"; break;
                case 4: g_Settings.language = "Chinese"; break;
                case 5: g_Settings.language = "Czech"; break;
                case 6: g_Settings.language = "Italian"; break;
                case 7: g_Settings.language = "Polish"; break;
                case 8: g_Settings.language = "Portuguese"; break;
                case 9: g_Settings.language = "Russian"; break;
            }
            Localization::SetLanguage(Localization::StringToLanguage(g_Settings.language));
            SettingsManager::Save();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Account Management ===
    if (ImGui::CollapsingHeader(Localization::GetText("account_management"), ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Account Selection Dropdown
        if (!g_Settings.accounts.empty())
        {
            std::vector<const char*> accountNames;
            for (const auto& acc : g_Settings.accounts)
                accountNames.push_back(acc.name.c_str());

            int currentAccountIndex = g_Settings.currentAccountIndex;
            if (currentAccountIndex < 0 || currentAccountIndex >= g_Settings.accounts.size())
                currentAccountIndex = 0;

            if (ImGui::Combo("##AccountSelect", &currentAccountIndex, accountNames.data(), static_cast<int>(accountNames.size())))
            {
                if (currentAccountIndex != g_Settings.currentAccountIndex)
                {
                    // Switch account logic
                    g_Settings.currentAccountIndex = currentAccountIndex;

                    // Load new token/keys from selected account
                    if (!g_Settings.accounts.empty() && g_Settings.currentAccountIndex >= 0 && g_Settings.currentAccountIndex < g_Settings.accounts.size())
                    {
                        g_Settings.drfToken = g_Settings.accounts[g_Settings.currentAccountIndex].drfToken;
                        g_Settings.gw2ApiKey = g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey;

                        // Connect with new token (automatically cancels any current connection)
                        if (!g_Settings.drfToken.empty() && SettingsManager::IsTokenValid(g_Settings.drfToken))
                        {
                            DrfClient::Connect(g_Settings.drfToken);
                        }

                        // Update GW2 API key
                        Gw2Fetcher::UpdateApiKey();
                    }

                    // Reset farming session
                    ItemTracker::Reset();
                    const char* addonDir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FarmingTracker") : nullptr;
                    ItemTracker::ClearPersistedData(addonDir);

                    SettingsManager::Save();
                }
            }
        }
        else
        {
            ImGui::TextDisabled("%s", Localization::GetText("no_accounts_configured"));
        }

        // Add/Remove Account Buttons
        if (ImGui::Button(Localization::GetText("add_account")))
        {
            // Add new account with default name
            Account newAccount;
            newAccount.name = std::string(Localization::GetText("account_prefix")) + " " + std::to_string(g_Settings.accounts.size() + 1);
            g_Settings.accounts.push_back(newAccount);
            g_Settings.currentAccountIndex = static_cast<int>(g_Settings.accounts.size() - 1);
            SettingsManager::Save();
        }
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("remove_account")) && !g_Settings.accounts.empty())
        {
            if (g_Settings.accounts.size() > 1)
            {
                g_Settings.accounts.erase(g_Settings.accounts.begin() + g_Settings.currentAccountIndex);
                if (g_Settings.currentAccountIndex >= static_cast<int>(g_Settings.accounts.size()))
                    g_Settings.currentAccountIndex = static_cast<int>(g_Settings.accounts.size() - 1);
                SettingsManager::Save();
            }
        }

        // Edit Current Account
        if (!g_Settings.accounts.empty() && g_Settings.currentAccountIndex >= 0 && g_Settings.currentAccountIndex < g_Settings.accounts.size())
        {
            ImGui::Spacing();
            ImGui::Separator();
            char editAccountLabel[256];
            snprintf(editAccountLabel, sizeof(editAccountLabel), Localization::GetText("edit_account"), g_Settings.accounts[g_Settings.currentAccountIndex].name.c_str());
            ImGui::Text("%s", editAccountLabel);

            // Initialize buffers with current account data
            if (ImGui::IsWindowAppearing())
            {
                strncpy_s(UICommon::s_AccountNameBuf, g_Settings.accounts[g_Settings.currentAccountIndex].name.c_str(), sizeof(UICommon::s_AccountNameBuf));
                strncpy_s(UICommon::s_AccountDrfBuf, g_Settings.accounts[g_Settings.currentAccountIndex].drfToken.c_str(), sizeof(UICommon::s_AccountDrfBuf));
                strncpy_s(UICommon::s_AccountGw2Buf, g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey.c_str(), sizeof(UICommon::s_AccountGw2Buf));
            }

            // Account Name
            ImGui::Text("%s", Localization::GetText("account_name"));
            if (ImGui::InputText("##AccountName", UICommon::s_AccountNameBuf, sizeof(UICommon::s_AccountNameBuf)))
            {
                g_Settings.accounts[g_Settings.currentAccountIndex].name = UICommon::s_AccountNameBuf;
            }

            // DRF Token
            ImGui::Text("%s", Localization::GetText("drf_token_label"));
            if (ImGui::InputText("##AccountDrfToken", UICommon::s_AccountDrfBuf, sizeof(UICommon::s_AccountDrfBuf)))
            {
                g_Settings.accounts[g_Settings.currentAccountIndex].drfToken = UICommon::s_AccountDrfBuf;
            }

            // GW2 API Key
            ImGui::Text("%s", Localization::GetText("gw2_api_key_label"));
            if (ImGui::InputText("##AccountGw2Key", UICommon::s_AccountGw2Buf, sizeof(UICommon::s_AccountGw2Buf)))
            {
                g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey = UICommon::s_AccountGw2Buf;
            }

            // Save Account Button
            if (ImGui::Button(Localization::GetText("save_account")))
            {
                // Update current account settings with input values
                g_Settings.accounts[g_Settings.currentAccountIndex].name = UICommon::s_AccountNameBuf;
                g_Settings.accounts[g_Settings.currentAccountIndex].drfToken = UICommon::s_AccountDrfBuf;
                g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey = UICommon::s_AccountGw2Buf;

                // Load new token/keys from current account
                g_Settings.drfToken = g_Settings.accounts[g_Settings.currentAccountIndex].drfToken;
                g_Settings.gw2ApiKey = g_Settings.accounts[g_Settings.currentAccountIndex].gw2ApiKey;

                // Connect with new token (automatically cancels any current connection)
                if (!g_Settings.drfToken.empty() && SettingsManager::IsTokenValid(g_Settings.drfToken))
                {
                    DrfClient::Connect(g_Settings.drfToken);
                }

                // Update GW2 API key
                Gw2Fetcher::UpdateApiKey();

                SettingsManager::Save();
            }

            ImGui::Spacing();
            ImGui::Separator();

            // Reload Configuration
            ImGui::Text("%s", Localization::GetText("reload_config"));
            if (ImGui::Button(Localization::GetText("reload_drf_token")))
            {
                DrfClient::Connect(g_Settings.drfToken);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("reconnect_drf_token"));
            ImGui::SameLine();
            if (ImGui::Button(Localization::GetText("reload_gw2_api_key")))
            {
                Gw2Fetcher::UpdateApiKey();
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("reload_gw2_api_key_tooltip"));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Reset Settings ===
    if (ImGui::CollapsingHeader(Localization::GetText("reset_settings")))
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
        ImGui::Text("%s", Localization::GetText("auto_reset_label"));
        if (ImGui::Combo("##AutoReset", &g_Settings.automaticResetMode, resetModes, 9))
        {
            SettingsManager::Save();
            AutoReset::RefreshSchedule();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("auto_reset_tooltip"));

        if (g_Settings.automaticResetMode == 7)
        {
            char minrstLabel[256];
            snprintf(minrstLabel, sizeof(minrstLabel), "%s##minrst", Localization::GetText("minutes_after_unload_tooltip"));
            if (ImGui::InputInt(minrstLabel, &g_Settings.minutesUntilResetAfterShutdown))
            {
                g_Settings.minutesUntilResetAfterShutdown =
                    std::clamp(g_Settings.minutesUntilResetAfterShutdown, 1, 24 * 60);
                SettingsManager::Save();
                AutoReset::RefreshSchedule();
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("minutes_after_unload_tooltip"));
        }

        if (g_Settings.automaticResetMode == 8)
        {
            int sliderDays = g_Settings.customResetDays;
            char customdaysLabel[256];
            snprintf(customdaysLabel, sizeof(customdaysLabel), "%s##customdays", Localization::GetText("reset_interval_days"));
            if (ImGui::SliderInt(customdaysLabel, &sliderDays, 1, 30))
            {
                g_Settings.customResetDays = sliderDays;
                SettingsManager::Save();
                AutoReset::RefreshSchedule();
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("reset_interval_days_tooltip"));
        }

        char nextResetLabel[256];
            snprintf(nextResetLabel, sizeof(nextResetLabel), Localization::GetText("next_reset_utc"), AutoReset::GetNextResetDisplayUtc().c_str());
            ImGui::Text("%s", nextResetLabel);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Separator();

    // === Favorites Settings ===
    if (ImGui::CollapsingHeader(Localization::GetText("favorites_settings")))
    {
        ImGui::Spacing();
        ImGui::Separator();

        // Favorites UI
        ImGui::Text("%s", Localization::GetText("favorites_ui"));

        if (ImGui::Checkbox(Localization::GetText("enable_favorites_tab"), &g_Settings.enableFavoritesTab))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_favorites_tab_tooltip"));

        if (ImGui::Checkbox(Localization::GetText("favorites_first"), &g_Settings.favoritesFirst))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("favorites_first_tooltip"));

        ImGui::Spacing();
        ImGui::Separator();

        // Favorites Colors
        ImGui::Text("%s", Localization::GetText("favorites_colors"));

        if (ImGui::Checkbox(Localization::GetText("enable_favorite_text_color"), &g_Settings.enableFavoriteTextColor))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_favorite_text_color_tooltip"));
        if (g_Settings.enableFavoriteTextColor)
        {
            ImGui::SameLine();
            char favoriteTextColorLabel[256];
            snprintf(favoriteTextColorLabel, sizeof(favoriteTextColorLabel), "%s##FavoriteTextColor", Localization::GetText("text_color"));
            if (ImGui::Button(favoriteTextColorLabel))
            {
                ImGui::OpenPopup("FavoriteTextColorPopup");
            }
            if (ImGui::BeginPopup("FavoriteTextColorPopup"))
            {
                if (ImGui::ColorPicker4("##FavoriteTextColorPicker", g_Settings.favoriteTextColor))
                {
                    SettingsManager::Save();
                }
                ImGui::EndPopup();
            }
        }

        if (ImGui::Checkbox(Localization::GetText("enable_favorite_row_color"), &g_Settings.enableFavoriteRowColor))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_favorite_row_color_tooltip"));
        if (g_Settings.enableFavoriteRowColor)
        {
            ImGui::SameLine();
            char favoriteRowColorLabel[256];
            snprintf(favoriteRowColorLabel, sizeof(favoriteRowColorLabel), "%s##FavoriteRowColor", Localization::GetText("row_color"));
            if (ImGui::Button(favoriteRowColorLabel))
            {
                ImGui::OpenPopup("FavoriteRowColorPopup");
            }
            if (ImGui::BeginPopup("FavoriteRowColorPopup"))
            {
                if (ImGui::ColorPicker4("##FavoriteRowColorPicker", g_Settings.favoriteRowColor))
                {
                    SettingsManager::Save();
                }
                ImGui::EndPopup();
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Other Tabs ===
    if (ImGui::CollapsingHeader(Localization::GetText("tabs_settings")))
    {
        ImGui::Text("%s", Localization::GetText("tabs_description"));

        ImGui::Spacing();
        ImGui::Separator();

        // Ignored Tab
        if (ImGui::Checkbox(Localization::GetText("enable_ignored_tab"), &g_Settings.enableIgnoredTab))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_ignored_tab_tooltip"));

        ImGui::Spacing();

        // Custom Profit
        ImGui::Text("%s", Localization::GetText("custom_profit_system"));

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Checkbox(Localization::GetText("enable_custom_profit"), &g_Settings.enableCustomProfit))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_custom_profit_tooltip"));
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Visual Settings ===
    if (ImGui::CollapsingHeader(Localization::GetText("visual_settings")))
    {
        // Visual Enhancement Settings
        ImGui::Text("%s", Localization::GetText("visual_enhancements"));
        if (ImGui::Checkbox(Localization::GetText("gradient_backgrounds"), &g_Settings.enableGradientBackgrounds))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("gradient_backgrounds_tooltip"));

        if (g_Settings.enableGradientBackgrounds)
        {
            ImGui::SameLine();
            ImGui::ColorEdit3(Localization::GetText("top_gradient_color"), g_Settings.gradientTopColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("top_gradient_color_tooltip"));
            ImGui::SameLine();
            ImGui::ColorEdit3(Localization::GetText("bottom_gradient_color"), g_Settings.gradientBottomColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("bottom_gradient_color_tooltip"));
            if (ImGui::IsItemDeactivatedAfterEdit())
                SettingsManager::Save();
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Basic Settings
        ImGui::Checkbox(Localization::GetText("show_item_icons"), &g_Settings.showItemIcons);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("show_item_icons_tooltip"));
        if (ImGui::SliderInt(Localization::GetText("icon_size"), &g_Settings.iconSize, 16, 96))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("icon_size_tooltip"));
        if (ImGui::Checkbox(Localization::GetText("show_rarity_borders"), &g_Settings.showRarityBorder))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("show_rarity_borders_tooltip"));
        if (g_Settings.showRarityBorder)
        {
            if (ImGui::SliderFloat(Localization::GetText("border_size"), &g_Settings.rarityBorderSize, 1.0f, 10.0f, "%.1f"))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("border_size_tooltip"));
        }
        if (ImGui::Checkbox(Localization::GetText("enable_grid_view_items"), &g_Settings.enableGridViewItems))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_grid_view_items_tooltip"));
        if (g_Settings.enableGridViewItems)
        {
            if (ImGui::SliderInt(Localization::GetText("grid_icon_size_items"), &g_Settings.gridIconSize, 16, 128))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("grid_icon_size_items_tooltip"));
        }
        if (ImGui::Checkbox(Localization::GetText("enable_grid_view_currencies"), &g_Settings.enableGridViewCurrencies))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_grid_view_currencies_tooltip"));
        if (g_Settings.enableGridViewCurrencies)
        {
            if (ImGui::SliderInt(Localization::GetText("grid_icon_size_currencies"), &g_Settings.gridIconSizeCurrencies, 16, 128))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("grid_icon_size_currencies_tooltip"));
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Mini Window
        ImGui::Text("%s", Localization::GetText("mini_window_widget"));
        if (ImGui::Checkbox(Localization::GetText("show_mini_window"), &g_Settings.showMiniWindow))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("show_mini_window_tooltip"));

        if (g_Settings.showMiniWindow)
        {
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_profit"), &g_Settings.miniWindowShowProfit))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_show_profit_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_profit_per_hour"), &g_Settings.miniWindowShowProfitPerHour))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_show_profit_per_hour_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_tp_sell"), &g_Settings.miniWindowShowTradingProfitSell))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_show_tp_sell_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_tp_instant"), &g_Settings.miniWindowShowTradingProfitInstant))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_show_tp_instant_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_total_items"), &g_Settings.miniWindowShowTotalItems))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_show_total_items_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("mini_window_show_session_duration"), &g_Settings.miniWindowShowSessionDuration))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_show_session_duration_tooltip"));
            if (ImGui::Checkbox(Localization::GetText("mini_window_click_through"), &g_Settings.miniWindowClickThrough))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("mini_window_click_through_tooltip"));
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Main Window
        ImGui::Text("%s", Localization::GetText("main_window_label"));
        if (ImGui::Checkbox(Localization::GetText("main_window_click_through"), &g_Settings.mainWindowClickThrough))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("main_window_click_through_tooltip"));
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Settings Profiles ===
    if (ImGui::CollapsingHeader(Localization::GetText("settings_profiles")))
    {
        ImGui::Text("%s", Localization::GetText("profiles_description"));

        ImGui::Spacing();

        // Profile Selection
        if (!g_Settings.settingsProfiles.empty())
        {
            std::vector<const char*> profileNames;
            profileNames.push_back(Localization::GetText("default_no_profile"));
            for (const auto& profile : g_Settings.settingsProfiles)
                profileNames.push_back(profile.name.c_str());

            int currentProfileDisplay = g_Settings.currentProfileIndex + 1;
            if (ImGui::Combo("##ProfileSelect", &currentProfileDisplay, profileNames.data(), static_cast<int>(profileNames.size())))
            {
                if (currentProfileDisplay == 0)
                {
                    g_Settings.currentProfileIndex = -1;
                }
                else
                {
                    SettingsManager::ApplyProfile(currentProfileDisplay - 1);
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("select_profile_tooltip"));
        }
        else
        {
            ImGui::TextDisabled("%s", Localization::GetText("no_profiles_created"));
        }

        ImGui::Spacing();

        // Profile Management Buttons
        ImGui::Text("%s", Localization::GetText("create_new_profile"));
        ImGui::InputText("##NewProfileName", UICommon::s_NewProfileNameBuf, sizeof(UICommon::s_NewProfileNameBuf));
        ImGui::SameLine();
        if (ImGui::Button(Localization::GetText("create")))
        {
            if (UICommon::s_NewProfileNameBuf[0] != '\0')
            {
                SettingsManager::CreateProfile(UICommon::s_NewProfileNameBuf);
                UICommon::s_NewProfileNameBuf[0] = '\0';
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("create_tooltip"));

        if (g_Settings.currentProfileIndex >= 0 && g_Settings.currentProfileIndex < static_cast<int>(g_Settings.settingsProfiles.size()))
        {
            ImGui::Spacing();
            char currentProfileLabel[256];
            snprintf(currentProfileLabel, sizeof(currentProfileLabel), Localization::GetText("current_profile"), g_Settings.settingsProfiles[g_Settings.currentProfileIndex].name.c_str());
            ImGui::Text("%s", currentProfileLabel);
            if (ImGui::Button(Localization::GetText("update_profile")))
            {
                SettingsManager::UpdateProfile(g_Settings.currentProfileIndex);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("update_profile_tooltip"));
            ImGui::SameLine();
            if (ImGui::Button(Localization::GetText("delete_profile")))
            {
                SettingsManager::DeleteProfile(g_Settings.currentProfileIndex);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("delete_profile_tooltip"));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Automatic Backups ===
    if (ImGui::CollapsingHeader(Localization::GetText("automatic_backups")))
    {
        ImGui::Text("%s", Localization::GetText("auto_backup"));

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Checkbox(Localization::GetText("enable_automatic_backups"), &g_Settings.enableAutoBackups))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_automatic_backups_tooltip"));

        if (g_Settings.enableAutoBackups)
        {
            ImGui::Spacing();

            const char* backupFreqItems[] = {Localization::GetText("backup_manual_only"), Localization::GetText("backup_daily"), Localization::GetText("backup_weekly")};
            if (ImGui::Combo("Backup frequency##BackupFreq", &g_Settings.backupFrequency, backupFreqItems, 3))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("backup_frequency_tooltip"));

            ImGui::Spacing();

            char backupCountLabel[256];
            snprintf(backupCountLabel, sizeof(backupCountLabel), "%s##BackupCount", Localization::GetText("max_backup_count"));
            if (ImGui::SliderInt(backupCountLabel, &g_Settings.maxBackupCount, 1, 20))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("max_backup_count_tooltip"));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Notification Settings ===
    if (ImGui::CollapsingHeader(Localization::GetText("notification_settings")))
    {
        ImGui::Text("%s", Localization::GetText("notifications"));

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Checkbox(Localization::GetText("enable_notifications"), &g_Settings.enableNotifications))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_notifications_tooltip"));

        if (g_Settings.enableNotifications)
        {
            ImGui::Spacing();

            // Profit Goal
            ImGui::Text("%s", Localization::GetText("profit_goal"));
            if (ImGui::Checkbox(Localization::GetText("notify_profit_goal"), &g_Settings.notifyProfitGoal))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("notify_profit_goal_tooltip"));

            if (g_Settings.notifyProfitGoal)
            {
                int profitGoalGold = g_Settings.profitGoalAmount / 10000;
                char profitGoalLabel[256];
                snprintf(profitGoalLabel, sizeof(profitGoalLabel), "%s##ProfitGoal", Localization::GetText("profit_goal_gold"));
                if (ImGui::SliderInt(profitGoalLabel, &profitGoalGold, 1, 1000))
                {
                    g_Settings.profitGoalAmount = profitGoalGold * 10000;
                    SettingsManager::Save();
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("profit_goal_gold_tooltip"));
            }

            ImGui::Spacing();

            // Reset Warning
            ImGui::Text("%s", Localization::GetText("reset_warning_label"));
            if (ImGui::Checkbox(Localization::GetText("notify_reset_warning"), &g_Settings.notifyResetWarning))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("notify_reset_warning_tooltip"));

            if (g_Settings.notifyResetWarning)
            {
                char resetWarningLabel[256];
                snprintf(resetWarningLabel, sizeof(resetWarningLabel), "%s##ResetWarning", Localization::GetText("warning_minutes"));
                if (ImGui::SliderInt(resetWarningLabel, &g_Settings.resetWarningMinutes, 1, 60))
                    SettingsManager::Save();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("warning_minutes_tooltip"));
            }

            ImGui::Spacing();

            // Session Complete
            ImGui::Text("%s", Localization::GetText("session_complete"));
            if (ImGui::Checkbox(Localization::GetText("notify_session_complete"), &g_Settings.notifySessionComplete))
                SettingsManager::Save();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", Localization::GetText("notify_session_complete_tooltip"));

            if (g_Settings.notifySessionComplete)
            {
                char sessionHoursLabel[256];
                snprintf(sessionHoursLabel, sizeof(sessionHoursLabel), "%s##SessionHours", Localization::GetText("session_hours"));
                if (ImGui::SliderInt(sessionHoursLabel, &g_Settings.sessionCompleteHours, 1, 24)) 
                    SettingsManager::Save();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Localization::GetText("session_hours_tooltip"));
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === Debug Settings ===
    if (ImGui::CollapsingHeader(Localization::GetText("debug_settings")))
    {
        ImGui::Text("%s", Localization::GetText("debug_settings"));

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Checkbox(Localization::GetText("enable_debug_tab"), &g_Settings.enableDebugTab))
            SettingsManager::Save();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Localization::GetText("enable_debug_tab_tooltip"));
    }
}
}
