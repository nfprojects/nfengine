/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Render Target.
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"
#include "Texture.hpp"
#include "Defines.hpp"
#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
    friend class CommandRecorder;

    Common::SharedPtr<Device> mDevicePtr;

    uint32 mWidth;
    uint32 mHeight;

    VkRenderPass mRenderPass;
    Common::DynArray<Texture*> mTex;
    Texture* mDepthTex;

    Common::DynArray<VkFramebuffer> mFramebuffers;

public:
    RenderTarget();
    ~RenderTarget();

    void GetDimensions(int& width, int& height);
    bool Init(Common::SharedPtr<Device>& device, const RenderTargetDesc& desc);

    const VkFramebuffer& GetCurrentFramebuffer() const;
};

} // namespace Renderer
} // namepsace NFE
