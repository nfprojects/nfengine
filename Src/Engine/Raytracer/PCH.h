#pragma once

#if defined(NFE_CONFIGURATION_DEBUG) && defined(NFE_PLATFORM_WINDOWS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // NFE_CONFIGURATION_DEBUG

#if defined(NFE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <VersionHelpers.h>
#endif // defined(NFE_PLATFORM_WINDOWS)

#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#if defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)
#include <immintrin.h>
#endif // defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)

#include <cassert>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <memory>
#include <initializer_list>
#include <functional>
#include <type_traits>
#include <sstream>
#include <algorithm>
#include <atomic>
