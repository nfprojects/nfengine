/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Render Target.
 */

#pragma once

#include "../RendererCommon/RenderTarget.hpp"
#include "Texture.hpp"
#include "Defines.hpp"

#include <Engine/Common/Containers/StaticArray.hpp>


namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
    friend class CommandRecorder;

    uint32 mWidth;
    uint32 mHeight;
    Common::StaticArray<Texture*, MAX_RENDER_TARGETS> mAttachments;
    Texture* mDepthAttachment;
    VkRenderPass mRenderPass;
    VkFramebuffer mFramebuffer;

public:
    RenderTarget();
    ~RenderTarget();

    void GetDimensions(int& width, int& height) override;
    bool Init(const RenderTargetDesc& desc) override;

    // TODO this has to go
    NFE_INLINE void TransitionColorAttachments(VkCommandBuffer cmd, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED)
    {
        for (const auto& a: mAttachments)
            a->Transition(cmd, dstLayout);
    }

    NFE_INLINE void TransitionDSAttachment(VkCommandBuffer cmd, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED)
    {
        if (mDepthAttachment)
            mDepthAttachment->Transition(cmd, dstLayout);
    }
};

} // namespace Renderer
} // namepsace NFE
