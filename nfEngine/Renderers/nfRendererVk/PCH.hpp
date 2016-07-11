/**
* @file
* @author  LKostyra (costyrra.xl@gmail.com)
* @brief   Precompiled header for Vulkan renderer.
*/

#pragma once

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <vulkan/vulkan.h>
#include <memory>
#include <set>
