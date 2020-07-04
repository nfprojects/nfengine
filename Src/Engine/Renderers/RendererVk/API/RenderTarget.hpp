/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
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
};

} // namespace Renderer
} // namepsace NFE
