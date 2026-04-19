#pragma once

namespace UI
{
    // Called from entry.cpp – registers all render callbacks with Nexus
    void Init();

    // Unregister all render callbacks
    void Shutdown();
}
