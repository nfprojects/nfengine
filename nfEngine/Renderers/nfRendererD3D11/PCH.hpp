/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Precompiled header
 */

#pragma once

 // enable memory allocation tracking
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(_DEBUG)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <new>
#include <memory>
#include <assert.h>
#include <set>
#include <map>
#include <vector>
#include <sstream>