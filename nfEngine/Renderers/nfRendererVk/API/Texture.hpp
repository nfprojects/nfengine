/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Defines.hpp"
#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class Texture : virtual public ITexture
{
    friend class CommandRecorder;
    friend class RenderTarget;
    friend class ResourceBindingInstance;

protected:
    struct Image
    {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
        VkImageLayout layout;

        Image()
            : image(VK_NULL_HANDLE)
            , view(VK_NULL_HANDLE)
            , memory(VK_NULL_HANDLE)
            , layout(VK_IMAGE_LAYOUT_UNDEFINED)
        {
        }
    };

    Common::SharedPtr<Device> mDevicePtr;

    TextureType mType;
    uint32 mWidth;
    uint32 mHeight;
    uint32 mDepth;
    VkFormat mFormat;
    VkImageSubresourceRange mSubresourceRange;

    // tempshit to support double-buffering
    uint32 mCurrentBuffer;
    Common::DynArray<Image> mImages;
    bool mFromSwapchain;

    void TransitionCurrentTexture(VkCommandBuffer cmdBuffer, VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED);

public:
    Texture();
    virtual ~Texture();
    bool Init(Common::SharedPtr<Device>& device, const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
