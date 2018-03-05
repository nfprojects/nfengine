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

#include "nfCommon/nfCommon.hpp"
#include "nfCommon/Logger/Logger.hpp"

#include "Internal/Extensions.hpp"
#include "Internal/Translations.hpp"

#ifndef VK_RETURN_FALSE_IF_FAILED
#define VK_RETURN_FALSE_IF_FAILED(result, errstr)                                   \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("%s: %d (%s)", errstr, result, TranslateVkResultToString(result));\
        return false;                                                               \
    }
#endif // VK_RETURN_FALSE_IF_FAILED

#ifndef VK_RETURN_EMPTY_VKRAII_IF_FAILED
#define VK_RETURN_EMPTY_VKRAII_IF_FAILED(t, result, errstr)                         \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("%s: %d (%s)", errstr, result, TranslateVkResultToString(result));\
        return VkRAII<t>();                                                         \
    }
#endif // VK_RETURN_EMPTY_VKRAII_IF_FAILED


#ifndef VK_ZERO_MEMORY
#define VK_ZERO_MEMORY(x) memset(&x, 0, sizeof(x))
#endif // VK_ZERO_MEMORY

#define VK_MAX_VOLATILE_BUFFERS 8

// some common structs/unions used throughout the project
namespace NFE {

union SetBindingPair
{
    struct
    {
        uint16 set;
        uint16 binding;
    } pair;

    int total;
};

} // namespace NFE
