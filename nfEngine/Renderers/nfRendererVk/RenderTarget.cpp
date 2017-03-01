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
    mTex.resize(desc.numTargets);
    mDepthTex = dynamic_cast<Texture*>(desc.depthBuffer.get());

    // request a render pass from manager
    VkFormat colorFormats[MAX_RENDER_TARGETS];
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    for (uint32 i = 0; i < desc.numTargets; ++i)
    {
        mTex[i] = dynamic_cast<Texture*>(desc.targets[i].texture.get());

        if (desc.targets[i].format == ElementFormat::Unknown)
            colorFormats[i] = mTex[i]->mFormat;
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
    mFramebuffers.resize(mTex[0]->mBuffersNum);

    VkResult result = VK_SUCCESS;
    // for each swapchain image create a framebuffer
    if (mTex[0]->mFromSwapchain)
    {
        for (uint32 i = 0; i < mTex[0]->mBuffersNum; ++i)
        {
            std::vector<VkImageView> fbAtts;
            fbAtts.push_back(mTex[0]->mImages[i].view);
            if (mDepthTex)
                fbAtts.push_back(mDepthTex->mImages[0].view);

            VkFramebufferCreateInfo fbInfo;
            VK_ZERO_MEMORY(fbInfo);
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.width = mTex[0]->mWidth;
            fbInfo.height = mTex[0]->mHeight;
            fbInfo.renderPass = mRenderPass;
            fbInfo.attachmentCount = static_cast<uint32>(fbAtts.size());
            fbInfo.pAttachments = fbAtts.data();
            fbInfo.layers = 1;
            result = vkCreateFramebuffer(gDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffers[i]);
            CHECK_VKRESULT(result, "Failed to create framebuffer");
        }
    }
    else // ...or, if we create a framebuffer from a single image
    {
        // TODO this has lots of corner-cases which might be uncovered. Needs discussion & lots of thinking.
        std::vector<VkImageView> fbAtts;
        for (uint32 i = 0; i < desc.numTargets; ++i)
        {
            fbAtts.push_back(mTex[i]->mImages[0].view);
        }

        if (mDepthTex)
            fbAtts.push_back(mDepthTex->mImages[0].view);

        VkFramebufferCreateInfo fbInfo;
        VK_ZERO_MEMORY(fbInfo);
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.width = mTex[0]->mWidth;
        fbInfo.height = mTex[0]->mHeight;
        fbInfo.renderPass = mRenderPass;
        fbInfo.attachmentCount = static_cast<uint32>(fbAtts.size());
        fbInfo.pAttachments = fbAtts.data();
        fbInfo.layers = 1;
        result = vkCreateFramebuffer(gDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffers[0]);
        CHECK_VKRESULT(result, "Failed to create framebuffer");
    }

    mWidth = mTex[0]->mWidth;
    mHeight = mTex[0]->mHeight;

    LOG_INFO("Render Target created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
