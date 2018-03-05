/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's Render Target
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "Device.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mWidth(0)
    , mHeight(0)
{
}

RenderTarget::~RenderTarget()
{
    for (auto& fb : mFramebuffers)
        if (fb != VK_NULL_HANDLE)
            vkDestroyFramebuffer(gDevice->GetDevice(), fb, nullptr);

    // Render pass is cleaned by Render Pass Manager
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    if (desc.numTargets > 1)
    {
        NFE_LOG_ERROR("MRTs are not yet supported");
        return false;
    }

    mTex.Resize(desc.numTargets);
    mDepthTex = dynamic_cast<Texture*>(desc.depthBuffer.Get());

    // request a render pass from manager
    VkFormat colorFormats[MAX_RENDER_TARGETS];
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    for (uint32 i = 0; i < desc.numTargets; ++i)
    {
        if (desc.targets[i].format == ElementFormat::Unknown)
        {
            mTex[i] = dynamic_cast<Texture*>(desc.targets[i].texture.Get());
            colorFormats[i] = mTex[i]->mFormat;
        }
        else
            colorFormats[i] = TranslateElementFormatToVkFormat(desc.targets[i].format);
    }

    if (mDepthTex)
        depthFormat = mDepthTex->mFormat;

    RenderPassDesc rpDesc(colorFormats, desc.numTargets, depthFormat);
    mRenderPass = gDevice->GetRenderPassManager()->GetRenderPass(rpDesc);
    if (mRenderPass == VK_NULL_HANDLE)
        return false;

    // TODO something very temporary to resolve in the future
    mFramebuffers.Resize(mTex[0]->mBuffersNum);

    VkResult result = VK_SUCCESS;
    // for each swapchain image create a framebuffer
    if (mTex[0]->mFromSwapchain)
    {
        for (unsigned int i = 0; i < mTex[0]->mBuffersNum; ++i)
        {
            Common::StaticArray<VkImageView, 2> fbAtts;
            fbAtts.PushBack(mTex[0]->mBufferViews[i]);
            if (mDepthTex)
                fbAtts.PushBack(mDepthTex->mBufferViews[0]);

            VkFramebufferCreateInfo fbInfo;
            VK_ZERO_MEMORY(fbInfo);
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.width = mTex[0]->mWidth;
            fbInfo.height = mTex[0]->mHeight;
            fbInfo.renderPass = mRenderPass;
            fbInfo.attachmentCount = fbAtts.Size();
            fbInfo.pAttachments = fbAtts.Data();
            fbInfo.layers = 1;
            result = vkCreateFramebuffer(gDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffers[i]);
            VK_RETURN_FALSE_IF_FAILED(result, "Failed to create framebuffer");
        }
    }
    else // ...or, if we create a framebuffer from a single image
    {
        // TODO fill
    }

    Texture* tex = dynamic_cast<Texture*>(desc.targets[0].texture.Get());
    mWidth = tex->mWidth;
    mHeight = tex->mHeight;

    NFE_LOG_INFO("Render Target created successfully");
    return true;
}

const VkFramebuffer& RenderTarget::GetCurrentFramebuffer() const
{
    return mFramebuffers[mTex[0]->mCurrentBuffer];
}

} // namespace Renderer
} // namespace NFE
