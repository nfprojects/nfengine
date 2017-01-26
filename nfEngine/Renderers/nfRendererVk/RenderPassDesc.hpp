/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan's Render Pass description structure.
 */

#pragma once

namespace NFE {
namespace Renderer {

struct RenderPassDesc
{
    VkFormat* colorFormats;
    uint32 colorFormatCount;
    VkFormat depthFormat;

    RenderPassDesc()
        : colorFormats(nullptr)
        , colorFormatCount(0)
        , depthFormat(VK_FORMAT_UNDEFINED)
    {
    }

    RenderPassDesc(VkFormat* color, uint32 colorCount, VkFormat depth)
        : colorFormats(color)
        , colorFormatCount(colorCount)
        , depthFormat(depth)
    {
    }

    bool operator==(const RenderPassDesc& other) const
    {
        if (colorFormatCount != other.colorFormatCount)
            return false;
        if (depthFormat != other.depthFormat)
            return false;
        for (uint32 i = 0; i < colorFormatCount; ++i)
            if (colorFormats[i] != other.colorFormats[i])
                return false;

        return true;
    }
};

} // namespace Renderer
} // namespace NFE


// hash specialization for RenderPassDesc
namespace std
{
    template <> struct hash<VkFormat>
    {
        size_t operator()(const VkFormat format) const
        {
            return hash<unsigned int>()(static_cast<unsigned int>(format));
        }
    };

    template <> struct hash<NFE::Renderer::RenderPassDesc>
    {
        size_t operator()(const NFE::Renderer::RenderPassDesc& x) const
        {
            size_t colorHash = 0;
            for (NFE::uint32 i = 0; i < x.colorFormatCount; ++i)
                colorHash ^= hash<VkFormat>()(x.colorFormats[i]);
            return colorHash ^ hash<NFE::uint32>()(x.colorFormatCount) ^ hash<VkFormat>()(x.depthFormat);
        }
    };
} // namespace std


