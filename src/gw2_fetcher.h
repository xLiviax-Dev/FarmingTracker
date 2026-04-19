#pragma once

#include <string>

namespace Gw2Fetcher
{
    enum class Gw2Status
    {
        Disconnected,
        Connected,
        Error
    };

    void Init();
    void Shutdown();
    void NotifyDrfActivity();
    void UpdateApiKey();

    // Current status
    Gw2Status GetStatus();

    // For debug display: number of reconnect attempts since last success
    int GetReconnectCount();
}
