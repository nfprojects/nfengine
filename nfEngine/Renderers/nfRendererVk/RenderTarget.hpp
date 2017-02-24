/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Render Target.
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"
#include "Texture.hpp"
#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
    friend class CommandRecorder;

    int mWidth;
    int mHeight;

    VkRenderPass mRenderPass;
    std::vector<Texture*> mTex;
    Texture* mDepthTex;

    std::vector<VkFramebuffer> mFramebuffers;

public:
    RenderTarget();
    ~RenderTarget();

    void GetDimensions(int& width, int& height);
    bool Init(const RenderTargetDesc& desc);

    NFE_INLINE const VkFramebuffer& GetCurrentFramebuffer() const
    {
        return mFramebuffers[mTex[0]->mCurrentBuffer];
    }

    NFE_INLINE bool HasDepthStencilAttachment() const
    {
        return (mDepthTex != nullptr);
    }
};

} // namespace Renderer
} // namepsace NFE
