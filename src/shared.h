#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../include/nexus/Nexus.h"

// Global pointer to the Nexus API - set in AddonLoad, cleared in AddonUnload
extern AddonAPI_t* APIDefs;

#include <windows.h>
HMODULE GetModule();
int GetCurrentMapId();
