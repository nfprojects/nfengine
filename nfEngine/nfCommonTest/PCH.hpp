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

#include <stdlib.h>
#include <stdio.h>

#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include "gtest/gtest.h"

// workaround for Windows/Linux mkdir difference
#if defined(__LINUX__) | defined(__linux__)
#define _mkdir(x) mkdir(x, 0777)
#endif // defined(__LINUX__) | defined(__linux__)