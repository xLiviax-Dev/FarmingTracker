#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <functional>

// ---------------------------------------------------------------------------
// Search Manager - provides search functionality
// Provides search functionality for items and currencies
// ---------------------------------------------------------------------------

class SearchManager
{
public:
    // Check if item matches search term
    static bool MatchesSearch(const std::string& itemName, const std::string& searchTerm);
    
    // Check if currency matches search term
    static bool MatchesSearchCurrency(const std::string& currencyName, const std::string& searchTerm);
    
    private:
    // Convert string to lowercase for case-insensitive search
    static std::string ToLowerCase(const std::string& str);
    
    // Check if search term is contained in text (case-insensitive)
    static bool ContainsIgnoreCase(const std::string& text, const std::string& searchTerm);
};
