/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan's Render Pass description structure.
 */

#pragma once

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/Hash.hpp"


namespace NFE {
namespace Renderer {

struct RenderPassDesc
{
    Common::DynArray<VkFormat> colorFormats;
    VkFormat depthFormat;

    RenderPassDesc()
        : colorFormats()
        , depthFormat(VK_FORMAT_UNDEFINED)
    {
    }

    RenderPassDesc(VkFormat* color, uint32 colorCount, VkFormat depth)
        : depthFormat(depth)
    {
        colorFormats.Resize(colorCount);
        for (uint32 i = 0; i < colorCount; ++i)
            colorFormats[i] = color[i];
    }

    bool operator==(const RenderPassDesc& other) const
    {
        if (depthFormat != other.depthFormat)
            return false;

        return (colorFormats == other.colorFormats);
    }
};

} // namespace Renderer
} // namespace NFE
