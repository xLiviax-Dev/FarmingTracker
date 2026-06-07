#include "search_manager.h"
#include <algorithm>
#include <cctype>

bool SearchManager::MatchesSearch(const std::string& itemName, const std::string& searchTerm)
{
    return ContainsIgnoreCase(itemName, searchTerm);
}

bool SearchManager::MatchesSearchCurrency(const std::string& currencyName, const std::string& searchTerm)
{
    return ContainsIgnoreCase(currencyName, searchTerm);
}

std::string SearchManager::ToLowerCase(const std::string& str)
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lowerStr;
}

bool SearchManager::ContainsIgnoreCase(const std::string& text, const std::string& searchTerm)
{
    if (searchTerm.empty())
        return true;
    
    std::string lowerText = ToLowerCase(text);
    std::string lowerSearch = ToLowerCase(searchTerm);
    
    return lowerText.find(lowerSearch) != std::string::npos;
}
