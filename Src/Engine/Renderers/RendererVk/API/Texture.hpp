/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#pragma once

#include "../Interface/Texture.hpp"
#include "Defines.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class Texture : public ITexture
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
    VkImage mImage;
    VkImageView mImageView;
    VkImageLayout mImageLayout;
    VkDeviceMemory mImageMemory;

    // tempshit to support double-buffering
    uint32 mBuffersNum;
    uint32 mCurrentBuffer;
    Common::DynArray<VkImage> mBuffers;
    Common::DynArray<VkImageView> mBufferViews;
    bool mFromSwapchain;

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
