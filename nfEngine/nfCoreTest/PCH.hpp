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

// TODO: remove this dependencies, engine should include everything what's needed
#define NOMINMAX
#include <Windows.h>
#include <set>
#include <map>
#include <vector>

#include "gtest\gtest.h"