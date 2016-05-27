#pragma once

//ignore pointless warnings
#pragma warning (disable: 4251)
#pragma warning (disable: 4100)

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#include <cstdio>

// TODO: remove this dependencies. Engine should include everything what is needed
#include <string>
#include <map>
#include <vector>
#include <set>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#ifdef GetWindowFont
#undef GetWindowFont  // ImGui workaround - GetWindowFont is both WinAPI macro and ImGui function
#endif
#include "imgui.h"