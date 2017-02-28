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
    std::vector<VkFormat> colorFormats;
    VkFormat depthFormat;

    RenderPassDesc()
        : colorFormats()
        , depthFormat(VK_FORMAT_UNDEFINED)
    {
    }

    RenderPassDesc(VkFormat* color, uint32 colorCount, VkFormat depth)
        : colorFormats(colorCount)
        , depthFormat(depth)
    {
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
            for (auto& format: x.colorFormats)
                colorHash ^= hash<VkFormat>()(format);
            return colorHash ^ hash<size_t>()(x.colorFormats.size()) ^ hash<VkFormat>()(x.depthFormat);
        }
    };
} // namespace std


