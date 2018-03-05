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

#include <nfCommon/nfCommon.hpp>
#include <nfCommon/Logger/Logger.hpp>
#include <nfCommon/Containers/SharedPtr.hpp>

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

#ifndef VK_RETURN_NULL_HANDLE_IF_FAILED
#define VK_RETURN_NULL_HANDLE_IF_FAILED(result, ...)                                \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("Vulkan runtime returned error %d (%s)", result, TranslateVkResultToString(result)); \
        NFE_LOG_ERROR(__VA_ARGS__);                                                 \
        return VK_NULL_HANDLE;                                                      \
    }
#endif // VK_RETURN_NULL_HANDLE_IF_FAILED


#ifndef VK_ZERO_MEMORY
#define VK_ZERO_MEMORY(x) memset(&x, 0, sizeof(x))
#endif // VK_ZERO_MEMORY

#define VK_MAX_VOLATILE_BUFFERS 8
#define VK_MAX_VERTEX_BUFFERS 4

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
using DevicePtr = Common::SharedPtr<Device>;

} // namespace Renderer
} // namespace NFE
