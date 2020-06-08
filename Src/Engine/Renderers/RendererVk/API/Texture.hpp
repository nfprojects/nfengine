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
    VkImageSubresourceRange mImageSubresRange;

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);

    void Transition(VkCommandBuffer cb, VkImageLayout dstLayout);
};

} // namespace Renderer
} // namespace NFE
