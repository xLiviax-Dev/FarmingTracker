// ---------------------------------------------------------------------------
// localization.cpp – Localization system implementation for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>

namespace Localization
{
    static Language s_CurrentLanguage = Language::English;
    static std::unordered_map<std::string, const char*> s_Translations;

    void Initialize(Language lang)
    {
        SetLanguage(lang);
    }

    void SetLanguage(Language lang)
    {
        s_CurrentLanguage = lang;
        s_Translations.clear();

        switch (lang)
        {
            case Language::English:
                s_Translations = GetEnglishTranslations();
                break;
            case Language::German:
                s_Translations = GetGermanTranslations();
                break;
            case Language::French:
                s_Translations = GetFrenchTranslations();
                break;
            case Language::Spanish:
                s_Translations = GetSpanishTranslations();
                break;
            case Language::Chinese:
                s_Translations = GetChineseTranslations();
                break;
            case Language::Czech:
                s_Translations = GetCzechTranslations();
                break;
            case Language::Italian:
                s_Translations = GetItalianTranslations();
                break;
            case Language::Polish:
                s_Translations = GetPolishTranslations();
                break;
            case Language::Portuguese:
                s_Translations = GetPortugueseTranslations();
                break;
            case Language::Russian:
                s_Translations = GetRussianTranslations();
                break;
            default:
                s_Translations = GetEnglishTranslations();
                break;
        }
    }

    Language GetLanguage()
    {
        return s_CurrentLanguage;
    }

    const char* GetText(const char* key)
    {
        auto it = s_Translations.find(key);
        if (it != s_Translations.end())
            return it->second;
        return key; // Return key if not found
    }

    const char* LanguageToString(Language lang)
    {
        switch (lang)
        {
            case Language::English: return "English";
            case Language::German: return "German";
            case Language::French: return "French";
            case Language::Spanish: return "Spanish";
            case Language::Chinese: return "Chinese";
            case Language::Czech: return "Czech";
            case Language::Italian: return "Italian";
            case Language::Polish: return "Polish";
            case Language::Portuguese: return "Portuguese";
            case Language::Russian: return "Russian";
            default: return "English";
        }
    }

    Language StringToLanguage(const std::string& str)
    {
        if (str == "German") return Language::German;
        if (str == "French") return Language::French;
        if (str == "Spanish") return Language::Spanish;
        if (str == "Chinese") return Language::Chinese;
        if (str == "Czech") return Language::Czech;
        if (str == "Italian") return Language::Italian;
        if (str == "Polish") return Language::Polish;
        if (str == "Portuguese") return Language::Portuguese;
        if (str == "Russian") return Language::Russian;
        return Language::English; // Default
    }
}
