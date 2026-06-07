#pragma once
#include <string>

// ---------------------------------------------------------------------------
// ui_tab_icons.h
// Loads and provides tab icon textures from data\icons\tabicons\
// ---------------------------------------------------------------------------

namespace UITabIcons
{
    // Call once at AddonLoad after APIDefs is set
    void Init(const char* addonDir);

    // Returns ImTextureID (as void*) for a given tab key, or nullptr if not loaded
    // key: e.g. "dashboard", "drops", "custom_profit", etc.
    void* GetIcon(const std::string& key);
}
