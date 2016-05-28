/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header.
 */

#pragma once

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

// TODO This block below is probably not needed in PCH.hpp and should be included by every file
//      made by Windows platform.
#if defined(WIN32)
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#endif // defined(WIN32)

#if defined(__LINUX__) | defined(__linux__)
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif // defined(__LINUX__) | defined(__linux__)

/// C lib
#include <stdlib.h>
#include <time.h>

/// STL
#include <vector>
#include <locale>
#include <stack>
#include <string>
#include <sstream>