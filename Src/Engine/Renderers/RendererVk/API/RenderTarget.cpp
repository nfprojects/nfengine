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

#include "Internal/Debugger.hpp"
#include "Internal/Utilities.hpp"

#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mWidth(0)
    , mHeight(0)
    , mAttachments()
    , mDepthAttachment()
    , mRenderPass(VK_NULL_HANDLE)
    , mFramebuffer(VK_NULL_HANDLE)
{
}

RenderTarget::~RenderTarget()
{
    // Render pass is cleaned by Render Pass Manager
    if (mFramebuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(gDevice->GetDevice(), mFramebuffer, nullptr);
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = static_cast<int>(mWidth);
    height = static_cast<int>(mHeight);
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    VkResult result = VK_SUCCESS;

    const bool hasColorAttachments = !desc.targets.Empty();
    if (!hasColorAttachments && !desc.depthBuffer)
    {
        NFE_LOG_ERROR("Cannot create Render Target without any color or depth targets");
        return false;
    }

    if (hasColorAttachments)
    {
        for (uint32 i = 0; i < desc.targets.Size(); ++i)
        {
            mAttachments.PushBack(dynamic_cast<Texture*>(desc.targets[i].texture.Get()));
            if (!mAttachments[i])
            {
                NFE_LOG_ERROR("Not all Render Target targets have a Texture attached");
                return false;
            }
        }
    }

    mDepthAttachment = dynamic_cast<Texture*>(desc.depthBuffer.Get());

    if (hasColorAttachments)
    {
        mWidth = mAttachments[0]->mWidth;
        mHeight = mAttachments[0]->mHeight;

        // check if other attachments are compatible
        for (uint32 i = 0; i < desc.targets.Size(); ++i)
        {
            Texture* t = dynamic_cast<Texture*>(desc.targets[i].texture.Get());
            if ((t->mWidth != mWidth) || (t->mHeight != mHeight))
            {
                NFE_LOG_ERROR("Provided targets for Render Target are not compatible");
                return false;
            }
        }

        if (mDepthAttachment)
        {
            if ((mDepthAttachment->mWidth != mWidth) || (mDepthAttachment->mHeight != mHeight))
            {
                NFE_LOG_ERROR("Provided depth buffer for Render Target is not compatible");
                return false;
            }
        }
    }
    else
    {
        mWidth = mDepthAttachment->mWidth;
        mHeight = mDepthAttachment->mHeight;
    }


    // request a render pass from manager
    VkFormat colorFormats[MAX_RENDER_TARGETS];
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    if (hasColorAttachments)
    {
        for (uint32 i = 0; i < desc.targets.Size(); ++i)
        {
            if (desc.targets[i].format == Format::Unknown)
                colorFormats[i] = mAttachments[i]->mFormat;
            else
                colorFormats[i] = TranslateFormatToVkFormat(desc.targets[i].format);
        }
    }

    if (mDepthAttachment)
        depthFormat = mDepthAttachment->mFormat;

    RenderPassDesc rpDesc(colorFormats, desc.targets.Size(), depthFormat);
    mRenderPass = gDevice->GetRenderPassManager()->GetRenderPass(rpDesc);
    if (mRenderPass == VK_NULL_HANDLE)
        return false;

    Common::StaticArray<VkImageView, MAX_RENDER_TARGETS + 1> fbAtts;
    for (uint32 i = 0; i < desc.targets.Size(); ++i)
        fbAtts.PushBack(mAttachments[i]->mImageView);

    if (mDepthAttachment)
        fbAtts.PushBack(mDepthAttachment->mImageView);

    VkFramebufferCreateInfo fbInfo;
    VK_ZERO_MEMORY(fbInfo);
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.width = mWidth;
    fbInfo.height = mHeight;
    fbInfo.renderPass = mRenderPass;
    fbInfo.attachmentCount = fbAtts.Size();
    fbInfo.pAttachments = fbAtts.Data();
    fbInfo.layers = 1;
    result = vkCreateFramebuffer(gDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffer);
    CHECK_VKRESULT(result, "Failed to create Framebuffer");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mFramebuffer), VK_OBJECT_TYPE_FRAMEBUFFER, desc.debugName);

    NFE_LOG_INFO("Render Target created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
