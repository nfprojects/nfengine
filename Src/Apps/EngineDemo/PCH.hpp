#pragma once

//ignore pointless warnings
#pragma warning (disable: 4251)
#pragma warning (disable: 4100)

// enable memory allocation tracking (Windows only)
#if defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)

#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#if defined(NFE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(NFE_PLATFORM_WINDOWS)

#ifdef GetWindowFont
#undef GetWindowFont  // ImGui workaround - GetWindowFont is both WinAPI macro and ImGui function
#endif
#include "imgui.h"