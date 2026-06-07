#pragma once

// ---------------------------------------------------------------------------
// ui_info.h
// Info / About tab for FarmingTracker
// ---------------------------------------------------------------------------

namespace UIInfo
{
    // Call once at AddonLoad — loads all infobutton icons from disk
    void Init(const char* addonDir);

    // Render the full Info / About panel (call inside your ImGui window)
    void Render();
}
