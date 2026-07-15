#include "shared.h"

AddonAPI_t* APIDefs = nullptr;

#pragma pack(push, 1)
struct Gw2MumbleContext
{
    uint8_t  serverAddress[28];
    uint32_t mapId;
    uint32_t mapType;
    uint32_t shardId;
    uint32_t instance;
    uint32_t buildId;
    uint32_t uiState;
    uint16_t compassWidth;
    uint16_t compassHeight;
    float    compassRotation;
    float    playerX;
    float    playerY;
    float    mapCenterX;
    float    mapCenterY;
    float    mapScale;
    uint32_t processId;
    uint8_t  mountIndex;
};
#pragma pack(pop)

struct LinkedMem
{
    uint32_t        uiVersion;
    uint32_t        uiTick;
    float           fAvatarPosition[3];
    float           fAvatarFront[3];
    float           fAvatarTop[3];
    wchar_t         name[256];
    float           fCameraPosition[3];
    float           fCameraFront[3];
    float           fCameraTop[3];
    wchar_t         identity[256];
    uint32_t        contextLen;
    Gw2MumbleContext context;
    wchar_t         description[2048];
};

bool IsInCombat()
{
    if (!APIDefs || !APIDefs->DataLink_Get) return false;
    auto* mumble = static_cast<LinkedMem*>(APIDefs->DataLink_Get(DL_MUMBLE_LINK));
    if (!mumble) return false;
    return (mumble->context.uiState & 0x40) != 0;
}
