/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header
 */

#pragma once

 // enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)

#include <iostream>
#include <memory>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
