// ---------------------------------------------------------------------------
// localization.h – Localization system for Farming Tracker
// ---------------------------------------------------------------------------

#pragma once

#include <string>
#include <map>
#include <unordered_map>

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

    // Language translation functions (defined in separate files)
    const std::unordered_map<std::string, const char*> GetEnglishTranslations();
    const std::unordered_map<std::string, const char*> GetGermanTranslations();
    const std::unordered_map<std::string, const char*> GetFrenchTranslations();
    const std::unordered_map<std::string, const char*> GetSpanishTranslations();
    const std::unordered_map<std::string, const char*> GetChineseTranslations();
    const std::unordered_map<std::string, const char*> GetCzechTranslations();
    const std::unordered_map<std::string, const char*> GetItalianTranslations();
    const std::unordered_map<std::string, const char*> GetPolishTranslations();
    const std::unordered_map<std::string, const char*> GetPortugueseTranslations();
    const std::unordered_map<std::string, const char*> GetRussianTranslations();
}
