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
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

// stdcall macro to cover Vulkan callbacks on Win32
#ifdef WIN32
#define VK_STDCALL __stdcall
#else
#define VK_STDCALL
#endif


// Vulkan
#include <vulkan/vulkan.h>
#include <glslang/glslang/Public/ShaderLang.h>


// STL
#include <memory>
#include <tuple>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstddef>
