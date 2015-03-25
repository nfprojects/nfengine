/**
 * @file   stdafx.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header.
 */

#pragma once

// TODO This block below is probably not needed in stdafx.hpp and should be included by every file
//      made by Windows platform.
#ifdef WIN32
// TODO seems reduntant, check and remove under Windows
#include "targetver.hpp"

#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#endif

#if defined(__LINUX__) | defined(__linux__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/// C lib
#include <stdlib.h>
#include <time.h>

/// STL
#include <vector>
#include <locale>
#include <stack>
