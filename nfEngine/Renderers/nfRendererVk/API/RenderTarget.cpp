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

    if ((desc.numTargets == 0 || desc.targets == nullptr) && desc.depthBuffer == nullptr)
    {
        NFE_LOG_ERROR("Framebuffer requires textures to be created");
        return false;
    }

    if (desc.numTargets > 0)
    {
        Texture* t = dynamic_cast<Texture*>(desc.targets[0].texture.Get());
        mWidth = t->mWidth;
        mHeight = t->mHeight;

        // validate dimensions
        for (uint32 i = 1; i < desc.numTargets; ++i)
        {
            Texture* t2 = dynamic_cast<Texture*>(desc.targets[i].texture.Get());
            if (t2->mWidth != mWidth || t2->mHeight != mHeight)
            {
                NFE_LOG_ERROR("Color textures must have matching dimensions");
                return false;
            }
        }

        if (desc.depthBuffer)
        {
            Texture* d = dynamic_cast<Texture*>(desc.depthBuffer.Get());
            if (d->mWidth != mWidth || d->mHeight != mHeight)
            {
                NFE_LOG_ERROR("Depth Texture's dimensions must equal color texture's dimensions");
                return false;
            }
        }
    }
    else
    {
        Texture* t = dynamic_cast<Texture*>(desc.depthBuffer.Get());
        mWidth = t->mWidth;
        mHeight = t->mHeight;
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

    VkResult result = VK_SUCCESS;

    // go through Framebuffer's desc and see if we have any backbuffers within our collection
    int32 texFromSwapchain = -1;
    for (uint32 i = 0; i < mTex.Size(); ++i)
    {
        if (mTex[i]->mFromSwapchain)
        {
            if (texFromSwapchain == -1)
            {
                texFromSwapchain = i;
            }
            else
            {
                // TODO should this limitation should be resolved in the future?
                NFE_LOG_ERROR("Cannot create a framebuffer with two Backbuffers as attachments");
                return false;
            }
        }
    }

    // space for all color attachments + one depth attachment
    Common::StaticArray<VkImageView, MAX_RENDER_TARGETS + 1> fbAtts;
    if (texFromSwapchain != -1)
    {
        // create as many framebuffers as needed for backbuffer's multiple buffers
        mFramebuffers.Resize(mTex[texFromSwapchain]->mImages.Size());

        for (uint32 i = 0; i < mTex[texFromSwapchain]->mImages.Size(); ++i)
        {
            fbAtts.Clear();
            for (int32 t = 0; t < static_cast<int32>(mTex.Size()); ++t)
            {
                if (t == texFromSwapchain)
                    fbAtts.PushBack(mTex[t]->mImages[i].view);
                else
                    fbAtts.PushBack(mTex[t]->mImages[mTex[t]->mCurrentBuffer].view);
            }
            if (mDepthTex)
                    fbAtts.PushBack(mDepthTex->mImages[mDepthTex->mCurrentBuffer].view);

            VkFramebufferCreateInfo fbInfo;
            VK_ZERO_MEMORY(fbInfo);
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.width = mWidth;
            fbInfo.height = mHeight;
            fbInfo.renderPass = mRenderPass;
            fbInfo.attachmentCount = fbAtts.Size();
            fbInfo.pAttachments = fbAtts.Data();
            fbInfo.layers = 1;
            result = vkCreateFramebuffer(gDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffers[i]);
            VK_RETURN_FALSE_IF_FAILED(result, "Failed to create framebuffer");
        }
    }
    else
    {
        mFramebuffers.Resize(1);

        for (uint32 t = 0; t < mTex.Size(); ++t)
        {
            fbAtts.PushBack(mTex[t]->mImages[mTex[t]->mCurrentBuffer].view);
        }
        if (mDepthTex)
            fbAtts.PushBack(mDepthTex->mImages[mDepthTex->mCurrentBuffer].view);

        VkFramebufferCreateInfo fbInfo;
        VK_ZERO_MEMORY(fbInfo);
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.width = mWidth;
        fbInfo.height = mHeight;
        fbInfo.renderPass = mRenderPass;
        fbInfo.attachmentCount = fbAtts.Size();
        fbInfo.pAttachments = fbAtts.Data();
        fbInfo.layers = 1;
        result = vkCreateFramebuffer(gDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffers[0]);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to create framebuffer");
    }

    NFE_LOG_INFO("Render Target created successfully");
    return true;
}

const VkFramebuffer& RenderTarget::GetCurrentFramebuffer() const
{
    return mFramebuffers[mTex[0]->mCurrentBuffer];
}

} // namespace Renderer
} // namespace NFE
