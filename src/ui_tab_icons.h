#pragma once
#include <string>
#include <vector>
#include <functional>

// ---------------------------------------------------------------------------
// ui_tab_icons.h
// Loads and provides tab icon textures from data\icons\tabicons\
// Also provides a reusable pill sub-tab bar renderer.
// ---------------------------------------------------------------------------

namespace UITabIcons
{
    // Call once at AddonLoad after APIDefs is set
    void Init(const char* addonDir);

    // Returns ImTextureID (as void*) for a given icon key, or nullptr if not loaded
    void* GetIcon(const std::string& key);

    // ---------------------------------------------------------------------------
    // Sub-tab bar — renders a compact pill tab strip for sub-tabs.
    // tabs: list of {key, labelText} pairs
    // activeIdx: current active tab index (read/write)
    // Returns true if the active tab changed this frame.
    // Usage:
    //   struct SubTab { std::string key; const char* label; };
    //   static int s_Sub = 0;
    //   UITabIcons::RenderSubPillTabBar({...}, s_Sub);
    //   switch(s_Sub) { ... }
    // ---------------------------------------------------------------------------
    struct SubTabDef
    {
        std::string key;    // icon key (matches GetIcon key)
        const char* label;  // display label
    };

    bool RenderSubPillTabBar(const std::vector<SubTabDef>& tabs, int& activeIdx);
}
