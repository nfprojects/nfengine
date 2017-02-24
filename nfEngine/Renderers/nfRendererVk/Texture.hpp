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
    TextureType mType;
    uint32 mWidth;
    uint32 mHeight;
    uint32 mDepth;
    VkFormat mFormat;
    VkImageLayout mImageLayout;

    // Below vectors are needed to support Backbuffer and it's double-buffering
    uint32 mBuffersNum;
    uint32 mCurrentBuffer;
    std::vector<VkImage> mImages;
    std::vector<VkImageView> mImageViews;
    std::vector<VkDeviceMemory> mImageMemories;
    bool mFromSwapchain;
    bool mDepthTexture;

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
