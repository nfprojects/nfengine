#pragma once

#include "Engine/Common/nfCommon.hpp"

#include "../RendererCommon/CommandQueue.hpp"


namespace NFE {
namespace Renderer {
namespace Util {

NFE_FORCE_INLINE uint32 CommandQueueTypeToIndex(CommandQueueType type)
{
    // Note that this DOES NOT have to mean queueFamilyIndex assigned by driver! Only an
    // internal index used by Renderer
    switch (type)
    {
    case CommandQueueType::Graphics: return 0;
    case CommandQueueType::Compute: return 1;
    case CommandQueueType::Copy: return 2;
    default:
        NFE_ASSERT(0, "Invalid command queue type");
        return UINT32_MAX;
    }
}

NFE_FORCE_INLINE const char* CommandQueueTypeToString(CommandQueueType type)
{
    switch (type)
    {
    case CommandQueueType::Graphics: return "Graphics";
    case CommandQueueType::Compute:  return "Compute";
    case CommandQueueType::Copy:     return "Copy";
    default:
        NFE_ASSERT(0, "Invalid command queue type");
        return "INVALID";
    }
}

NFE_FORCE_INLINE VkQueueFlags CommandQueueTypeToVkQueueFlags(CommandQueueType type)
{
    switch (type)
    {
    case CommandQueueType::Graphics: return VK_QUEUE_GRAPHICS_BIT;
    case CommandQueueType::Compute:  return VK_QUEUE_COMPUTE_BIT;
    case CommandQueueType::Copy:     return VK_QUEUE_TRANSFER_BIT;
    default:
        return 0;
    }
}

NFE_FORCE_INLINE bool FormatHasStencil(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

NFE_FORCE_INLINE bool IsDepthFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

NFE_FORCE_INLINE VkImageViewType TextureTypeToVkImageViewType(TextureType type)
{
    switch (type)
    {
    case TextureType::Texture1D:    return VK_IMAGE_VIEW_TYPE_1D;
    case TextureType::Texture2D:    return VK_IMAGE_VIEW_TYPE_2D;
    case TextureType::Texture3D:    return VK_IMAGE_VIEW_TYPE_3D;
    case TextureType::TextureCube:  return VK_IMAGE_VIEW_TYPE_CUBE;
    default: return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

} // namespace Util
} // namespace Renderer
} // namespace NFE
