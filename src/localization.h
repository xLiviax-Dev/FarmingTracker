// ---------------------------------------------------------------------------
// localization.h – Localization system for Farming Tracker
// ---------------------------------------------------------------------------

#pragma once

#include <string>
#include <map>

namespace Localization
{
    enum class Language
    {
        English,
        German,
        French,
        Spanish,
        Chinese,
        Czech,
        Italian,
        Polish,
        Portuguese,
        Russian
    };

    // Initialize localization system
    void Initialize(Language lang = Language::English);

    // Set current language
    void SetLanguage(Language lang);

    // Get current language
    Language GetLanguage();

    // Get localized text by key
    const char* GetText(const char* key);

    // Convert language enum to string
    const char* LanguageToString(Language lang);

    // Convert string to language enum
    Language StringToLanguage(const std::string& str);
}
