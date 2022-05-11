/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
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

#include <functional>

#ifndef CHECK_VKRESULT
#define CHECK_VKRESULT(result, errstr)                                              \
    if (result != VK_SUCCESS)                                                       \
    {                                                                               \
        NFE_LOG_ERROR("%s: %d (%s)", errstr, result, TranslateVkResultToString(result));\
        return false;                                                               \
    }
#endif // CHECK_VKRESULT

#ifndef CHECK_SPVREFLECTRESULT
#define CHECK_SPVREFLECTRESULT(result, errstr)                                      \
    if (result != SPV_REFLECT_RESULT_SUCCESS)                                       \
    {                                                                               \
        NFE_LOG_ERROR("%s: %d (%s)", errstr, result, TranslateSpvReflectResultToString(result));\
        return false;                                                               \
    }
#endif // CHECK_SPVREFLECTRESULT

#ifndef VK_ZERO_MEMORY
#define VK_ZERO_MEMORY(x) \
    memset(&x, 0, sizeof(x))
#endif // VK_ZERO_MEMORY

#ifndef MS_TO_NS
#define MS_TO_NS(x) (x * 1'000'000)
#endif // MS_IN_NS

// RendererVk limits
#define VK_MAX_BINDINGS_PER_SET 16
#define VK_MAX_BOUND_DESCRIPTOR_SETS 16
#define VK_MAX_PENDING_RESOURCES 16
#define VK_MAX_VOLATILE_BUFFERS 8
#define VK_SEMAPHORE_POOL_SIZE 16
#define VK_COMMAND_BUFFER_POOL_SIZE 32
#define VK_MAX_BINDINGS_PER_STAGE (8)
#define VK_MAX_SHADER_STAGES (5)
#define VK_MAX_DESCRIPTOR_SETS (VK_MAX_BINDINGS_PER_STAGE * VK_MAX_SHADER_STAGES)


namespace NFE {
namespace Renderer {

// RendererVk common types
using DescriptorSetCollectionID = uint32;
using DescriptorSetCollection = Common::StaticArray<VkDescriptorSet, VK_MAX_DESCRIPTOR_SETS>;
using DescriptorSetLayoutCollection = Common::StaticArray<VkDescriptorSetLayout, VK_MAX_DESCRIPTOR_SETS>;
using UsedDescriptorSetsArray = Common::StaticArray<DescriptorSetCollectionID, 32>;

template <typename T>
using ForEachFunction = std::function<bool(const T&)>;

} // namespace Renderer
} // namespace NFE
