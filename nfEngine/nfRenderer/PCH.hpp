/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header.
 */

#pragma once

//suppress trivial warnings
#pragma warning (disable: 4251)
#pragma warning (disable: 4100)
#pragma warning (disable: 4127)
#pragma warning (disable: 4324)
#pragma warning (disable: 4099)

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

// WinAPI
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)


#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
#include <smmintrin.h>
#endif // NFE_USE_SSE4

#if defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)
#include <immintrin.h>
#endif // defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)

// C lib
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// STL lib
#include <memory>
#include <functional>

// FreeType library
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#ifdef GetWindowFont
#undef GetWindowFont  // ImGui workaround - GetWindowFont is both WinAPI macro and ImGui function
#endif

// ImGui
#include "imgui.h"