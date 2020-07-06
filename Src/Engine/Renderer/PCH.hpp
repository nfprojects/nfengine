#pragma once

//suppress trivial warnings
#pragma warning (disable: 4251)
#pragma warning (disable: 4100)
#pragma warning (disable: 4127)
#pragma warning (disable: 4324)
#pragma warning (disable: 4099)

// enable memory allocation tracking (Windows only)
#if defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)

// WinAPI
#if defined(NFE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(NFE_PLATFORM_WINDOWS)


#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#if defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)
#include <immintrin.h>
#endif // defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)


// C lib
#include <assert.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <setjmp.h>
#include <time.h>
#include <sys/stat.h>

// STL lib
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <sstream>
#include <tuple>
#include <atomic>
#include <unordered_map>
#include <algorithm>

// FreeType library
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#ifdef GetWindowFont
#undef GetWindowFont  // ImGui workaround - GetWindowFont is both WinAPI macro and ImGui function
#endif

// ImGui
#include "imgui.h"
