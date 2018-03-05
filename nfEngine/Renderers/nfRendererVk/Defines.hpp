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
#define VK_RETURN_FALSE_IF_FAILED(result, ...)                                      \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("Vulkan runtime returned error: %d (%s)", result, TranslateVkResultToString(result)); \
        NFE_LOG_ERROR(__VA_ARGS__);                                                 \
        return false;                                                               \
    }
#endif // VK_RETURN_FALSE_IF_FAILED

#ifndef VK_RETURN_EMPTY_VKHANDLE_IF_FAILED
#define VK_RETURN_EMPTY_VKHANDLE_IF_FAILED(t, result, ...)                          \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("Vulkan runtime returned error %d (%s)", result, TranslateVkResultToString(result)); \
        NFE_LOG_ERROR(__VA_ARGS__);                                                 \
        return VkHandle<t>();                                                       \
    }
#endif // VK_RETURN_EMPTY_VKHANDLE_IF_FAILED


#ifndef VK_ZERO_MEMORY
#define VK_ZERO_MEMORY(x) memset(&x, 0, sizeof(x))
#endif // VK_ZERO_MEMORY

#define VK_MAX_VOLATILE_BUFFERS 8

// some common structs/unions used throughout the project
namespace NFE {
namespace Renderer {

union SetBindingPair
{
    struct
    {
        uint16 set;
        uint16 binding;
    } pair;

    int total;
};

class Device;
extern Common::UniquePtr<Device> gDevice;

} // namespace Renderer
} // namespace NFE
