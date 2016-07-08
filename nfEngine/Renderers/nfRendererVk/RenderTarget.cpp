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
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    if (desc.numTargets > 1)
        LOG_WARNING("MRTs are not supported now. Only the first RT will be used.");

    mTex = dynamic_cast<Texture*>(desc.targets[0].texture);
    if (!mTex)
    {
        LOG_ERROR("Incorrect render target texture pointer.");
        return false;
    }

    mDepthTex = dynamic_cast<Texture*>(desc.depthBuffer);

    std::vector<VkAttachmentDescription> atts;
    std::vector<VkAttachmentReference> colorRefs;

    VkAttachmentDescription att;
    VK_ZERO_MEMORY(att);
    att.format = mTex->mFormat;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    // we do not care about auto-clearing - this should be triggered by CommandBuffer::Clear()
    att.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    atts.push_back(att);

    VkAttachmentReference ref;
    VK_ZERO_MEMORY(ref);
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorRefs.push_back(ref);

    VkAttachmentReference depthRef;
    if (mDepthTex)
    {
        att.format = mDepthTex->mFormat;
        // only change storeOps - after rendering the depth buffer isn't relevant to us
        att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depthRef.attachment = 1;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass;
    VK_ZERO_MEMORY(subpass);
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32>(colorRefs.size());
    subpass.pColorAttachments = colorRefs.data();
    if (desc.depthBuffer)
        subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpInfo;
    VK_ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = static_cast<uint32>(atts.size());
    rpInfo.pAttachments = atts.data();
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(gDevice->GetDevice(), &rpInfo, nullptr, &mRenderPass);
    CHECK_VKRESULT(result, "Failed to create Render Pass");

    mAttachmentCount = static_cast<uint32>(atts.size());

    mFramebuffers.resize(mTex->mBuffersNum);

    // for each swapchain image create a framebuffer
    if (mTex->mFromSwapchain)
    {
        for (unsigned int i = 0; i < mTex->mBuffersNum; ++i)
        {
            std::vector<VkImageView> fbAtts;
            fbAtts.push_back(mTex->mBufferViews[i]);
            if (mDepthTex)
                fbAtts.push_back(mDepthTex->mBufferViews[0]);

            VkFramebufferCreateInfo fbInfo;
            VK_ZERO_MEMORY(fbInfo);
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.width = mTex->mWidth;
            fbInfo.height = mTex->mHeight;
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
        // TODO fill
    }

    LOG_INFO("Render Target created successfully");
    return true;
}

const VkFramebuffer& RenderTarget::GetCurrentFramebuffer() const
{
    return mFramebuffers[mTex->mCurrentBuffer];
}

} // namespace Renderer
} // namespace NFE
