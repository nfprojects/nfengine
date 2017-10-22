#pragma once

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#include <stdio.h>
#include <tchar.h>


// disable some Visual Studio specific warnings
#ifdef _MSC_VER
// "structure was padded due to __declspec(align())"
#pragma warning(disable: 4324)
#endif

// WinAPI
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)

#include <set>
#include <map>
#include <vector>
#include <thread>
#include <chrono>

#include "gtest/gtest.h"