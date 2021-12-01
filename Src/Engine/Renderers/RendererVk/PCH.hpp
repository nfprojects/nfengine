/**
* @file
* @author  Lookey (costyrra.xl@gmail.com)
* @brief   Precompiled header for Vulkan renderer.
*/

#pragma once

// enable memory allocation tracking (Windows only)
#if defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)

#ifdef NFE_PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <vulkan/vulkan.h>

#ifdef NFE_PLATFORM_WINDOWS
/// glslang library duplicates strdup macro, which makes a collision when building with
/// _CRTDBG_MAP_ALLOC. For glslang headers, disable this warning at least until the
/// problem is fixed.
#pragma warning(push)
#pragma warning(disable: 4005)
#endif // NFE_PLATFORM_WINDOWS

#define ENABLE_HLSL
#include <glslang/glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>
#include <glslang/StandAlone/ResourceLimits.h>

#ifdef NFE_PLATFORM_WINDOWS
#pragma warning(pop)
#endif // NFE_PLATFORM_WINDOWS


// TODO Remove
#include <string.h>
#include <memory>
#include <tuple>
#include <sstream>
#include <algorithm>
