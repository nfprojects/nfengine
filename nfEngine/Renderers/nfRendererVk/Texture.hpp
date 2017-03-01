/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Texture : virtual public ITexture
{
    friend class CommandRecorder;
    friend class RenderTarget;
    friend class ResourceBindingInstance;

protected:
    struct ImageDataSet
    {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
    };

    TextureType mType;
    uint32 mWidth;
    uint32 mHeight;
    uint32 mDepth;
    VkFormat mFormat;
    VkImageLayout mImageLayout;
    VkImageLayout mCurrentImageLayout;
    VkSampleCountFlagBits mSamplesNum;
    VkImageSubresourceRange mSubresRange;

    // Below vectors are needed to support Backbuffer and it's double-buffering
    uint32 mBuffersNum;
    uint32 mCurrentBuffer;
    std::vector<ImageDataSet> mImages;
    bool mFromSwapchain;
    bool mDepthTexture;

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);

    // Perform resource transition on bind/unbind. VK_IMAGE_LAYOUT_UNDEFINED reverts to original layout.ITexture
    // NOTE: this function must be called only on active command buffer
    void Transition(VkCommandBuffer cmdBuffer, VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED);
};

} // namespace Renderer
} // namespace NFE
