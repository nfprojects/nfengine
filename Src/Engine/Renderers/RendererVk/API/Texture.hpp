/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#pragma once

#include "../RendererCommon/Texture.hpp"
#include "Defines.hpp"
#include "IResource.hpp"

#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class Texture : public ITexture, public IResource
{
    friend class CommandRecorder;
    friend class RenderTarget;
    friend class ResourceBindingInstance;
    friend class ResourceTracker;

protected:
    TextureType mType;
    uint32 mWidth;
    uint32 mHeight;
    uint32 mDepth;
    VkFormat mFormat;
    VkImage mImage;
    VkImageView mImageView;
    VkImageLayout mImageLayout;
    VkImageLayout mImageLayoutDefault;
    VkDeviceMemory mImageMemory;
    VkImageSubresourceRange mImageSubresRange;

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);

    const Internal::ResourceType GetType() const override
    {
        return Internal::ResourceType::Texture;
    }
};

} // namespace Renderer
} // namespace NFE
