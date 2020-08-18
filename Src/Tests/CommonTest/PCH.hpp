#pragma once

// enable memory allocation tracking (Windows only)
#if defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)

#ifdef NFE_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // NFE_PLATFORM_WINDOWS


#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#if defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)
#include <immintrin.h>
#endif // defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)


#include <stdlib.h>
#include <stdio.h>

#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <array>
#include <memory>
#include <atomic>
#include <string>
#include <iostream>
#include <type_traits>
#include <limits>
#include <fstream>
#include <functional>
#include <chrono>

#include "gtest/gtest.h"
#include "gtest/gtest-typed-test.h"

#include "Engine/Common/nfCommon.hpp"

// disable some Visual Studio specific warnings
#ifdef _MSC_VER
#pragma warning(disable: 4324) // "structure was padded due to alignment specifier"
#endif

// workaround for Windows/Linux mkdir difference
#if defined(NFE_PLATFORM_LINUX)
#define _mkdir(x) mkdir(x, 0777)
#endif // defined(NFE_PLATFORM_LINUX)
