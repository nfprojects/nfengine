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

#ifdef WIN32
/// glslang library duplicates strdup macro, which makes a collision when building with
/// _CRTDBG_MAP_ALLOC. For glslang headers, disable this warning at least until the
/// problem is fixed.
#pragma warning(push)
#pragma warning(disable: 4005)

/// Vulkan headers are C headers, so their enum types are unscoped. Disable that.
#pragma warning(disable: 26812)
#endif // WIN32

#include <vulkan/vulkan.h>

#define ENABLE_HLSL
#include <glslang/glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>
#include <glslang/StandAlone/ResourceLimits.h>

#ifdef WIN32
#pragma warning(pop)
#endif // WIN32


#include <string.h>
#include <memory>
#include <tuple>
#include <sstream>
#include <algorithm>
