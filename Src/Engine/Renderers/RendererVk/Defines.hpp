/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common defines for Vulkan Renderer
 */

#pragma once

// DLL import / export macro
#ifdef WIN32
#ifdef RENDERERVK_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else // RENDERERVK_EXPORTS
#define RENDERER_API __declspec(dllimport)
#endif // RENDERERVK_EXPORTS
#else // WIN32
#define RENDERER_API __attribute__((visibility("default")))
#endif // WIN32

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

#define NFE_VK_MAX_VOLATILE_BUFFERS 8
#define NFE_VK_SEMAPHORE_POOL_SIZE 16
#define NFE_VK_COMMAND_MEMORY_SPACE 2048
#define NFE_VK_MAX_COMMANDS_IN_BATCH 16
#define NFE_VK_MAX_VERTEX_BUFFERS 8
#define NFE_VK_MAX_BARRIERS 4
#define NFE_VK_MAX_RENDER_PASS_STATES 16
#define NFE_VK_INVALID_BATCH UINT32_MAX
