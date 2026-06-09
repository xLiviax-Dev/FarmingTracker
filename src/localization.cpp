// ---------------------------------------------------------------------------
// localization.cpp – Localization system implementation for Farming Tracker
// ---------------------------------------------------------------------------

#include "localization.h"
#include <unordered_map>
#include <shared_mutex>

namespace Localization
{
    static Language s_CurrentLanguage = Language::English;
    static std::unordered_map<std::string, const char*> s_Translations;
    // Protects s_CurrentLanguage and s_Translations.
    // SetLanguage() takes an exclusive write lock; GetText() takes a shared read lock
    // so multiple worker threads can read concurrently without blocking each other.
    static std::shared_mutex s_TranslationMutex;

    void Initialize(Language lang)
    {
        SetLanguage(lang);
    }

    void SetLanguage(Language lang)
    {
        std::unique_lock<std::shared_mutex> lock(s_TranslationMutex);
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
            case Language::Danish:
                s_Translations = GetDanishTranslations();
                break;
            case Language::Greek:
                s_Translations = GetGreekTranslations();
                break;
            case Language::Finnish:
                s_Translations = GetFinnishTranslations();
                break;
            case Language::Hungarian:
                s_Translations = GetHungarianTranslations();
                break;
            case Language::Dutch:
                s_Translations = GetDutchTranslations();
                break;
            case Language::Norwegian:
                s_Translations = GetNorwegianTranslations();
                break;
            case Language::Romanian:
                s_Translations = GetRomanianTranslations();
                break;
            case Language::Swedish:
                s_Translations = GetSwedishTranslations();
                break;
            default:
                s_Translations = GetEnglishTranslations();
                break;
        }
    }

    Language GetLanguage()
    {
        std::shared_lock<std::shared_mutex> lock(s_TranslationMutex);
        return s_CurrentLanguage;
    }

    const char* GetText(const char* key)
    {
        std::shared_lock<std::shared_mutex> lock(s_TranslationMutex);
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
            case Language::Danish: return "Danish";
            case Language::Greek: return "Greek";
            case Language::Finnish: return "Finnish";
            case Language::Hungarian: return "Hungarian";
            case Language::Dutch: return "Dutch";
            case Language::Norwegian: return "Norwegian";
            case Language::Romanian: return "Romanian";
            case Language::Swedish: return "Swedish";
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
        if (str == "Danish") return Language::Danish;
        if (str == "Greek") return Language::Greek;
        if (str == "Finnish") return Language::Finnish;
        if (str == "Hungarian") return Language::Hungarian;
        if (str == "Dutch") return Language::Dutch;
        if (str == "Norwegian") return Language::Norwegian;
        if (str == "Romanian") return Language::Romanian;
        if (str == "Swedish") return Language::Swedish;
        return Language::English; // Default
    }
}
