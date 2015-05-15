#pragma once

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <tchar.h>

#define NOMINMAX
#include <Windows.h>
#endif // WIN32

#include <stdlib.h>
#include <stdio.h>


// TODO: remove this dependencies, engine should include everything what's needed
#include <set>
#include <map>
#include <vector>

#include "gtest/gtest.h"

// workaround for Windows/Linux mkdir difference
#if defined(__LINUX__) | defined(__linux__)
#define _mkdir(x) mkdir(x, 0777)
#endif // defined(__LINUX__) | defined(__linux__)
