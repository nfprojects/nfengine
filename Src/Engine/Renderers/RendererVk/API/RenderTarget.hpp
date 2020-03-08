/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Render Target.
 */

#pragma once

#include "../Interface/RenderTarget.hpp"
#include "Texture.hpp"
#include "Defines.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
    friend class CommandRecorder;

    int mWidth;
    int mHeight;

    VkRenderPass mRenderPass;
    Common::DynArray<Texture*> mTex;
    Texture* mDepthTex;

    Common::DynArray<VkFramebuffer> mFramebuffers;

public:
    RenderTarget();
    ~RenderTarget();

    void GetDimensions(int& width, int& height);
    bool Init(const RenderTargetDesc& desc);

    const VkFramebuffer& GetCurrentFramebuffer() const;
};

} // namespace Renderer
} // namepsace NFE
