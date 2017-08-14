#pragma once

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif // WIN32


#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
#include <smmintrin.h>
#endif // NFE_USE_SSE4

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
#include <thread>
#include <limits>
#include <fstream>
#include <functional>
#include <chrono>

#include "gtest/gtest.h"

#include "nfCommon/nfCommon.hpp"

// workaround for Windows/Linux mkdir difference
#if defined(__LINUX__) | defined(__linux__)
#define _mkdir(x) mkdir(x, 0777)
#endif // defined(__LINUX__) | defined(__linux__)
