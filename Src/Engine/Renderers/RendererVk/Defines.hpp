/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common defines for Vulkan Renderer
 */

#pragma once

// DLL import / export macro
#if defined(NFE_PLATFORM_WINDOWS)
    #ifdef RENDERERVK_EXPORTS
        #define RENDERER_API __declspec(dllexport)
    #else // RENDERERVK_EXPORTS
        #define RENDERER_API __declspec(dllimport)
    #endif // RENDERERVK_EXPORTS
#else // NFE_PLATFORM_WINDOWS
    #define RENDERER_API __attribute__((visibility("default")))
#endif // NFE_PLATFORM_WINDOWS

#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Logger/Logger.hpp"

#include "Internal/Extensions.hpp"
#include "Internal/Translations.hpp"

#ifndef CHECK_VKRESULT
#define CHECK_VKRESULT(result, errstr)                                              \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("%s: %d (%s)", errstr, result, TranslateVkResultToString(result));\
        return false;                                                               \
    }
#endif // LOG_VKRESULT

#ifndef VK_ZERO_MEMORY
#define VK_ZERO_MEMORY(x) \
    memset(&x, 0, sizeof(x))
#endif // VK_ZERO_MEMORY

#ifndef MS_TO_NS
#define MS_TO_NS(x) (x * 1'000'000)
#endif // S_IN_NS

#define VK_MAX_VOLATILE_BUFFERS 8
#define VK_SEMAPHORE_POOL_SIZE 16
#define VK_COMMAND_BUFFER_POOL_SIZE 32
