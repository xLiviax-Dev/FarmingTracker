#pragma once

namespace UI
{
    // Called from entry.cpp – registers all render callbacks with Nexus
    void Init();

    // Unregister all render callbacks
    void Shutdown();

    // Main render callback called by Nexus every frame
    void Render();
}

// Helper functions for accent color (used by main window and mini window)
void PushAccentColor();
void PopAccentColor();
