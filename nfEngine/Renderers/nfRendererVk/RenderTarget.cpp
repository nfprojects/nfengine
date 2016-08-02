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

    if (mRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(gDevice->GetDevice(), mRenderPass, nullptr);
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
        LOG_ERROR("MRTs are not yet supported");
        return false;
    }

    mTex.resize(desc.numTargets);
    mDepthTex = dynamic_cast<Texture*>(desc.depthBuffer);

    VkAttachmentDescription atts[MAX_RENDER_TARGETS + 2];
    uint32 curAtt = 0;
    VkAttachmentReference colorRefs[MAX_RENDER_TARGETS];

    for (uint32 i = 0; i < desc.numTargets; ++i)
    {
        VK_ZERO_MEMORY(atts[curAtt]);
        if (desc.targets[i].format == ElementFormat::Unknown)
        {
            mTex[i] = dynamic_cast<Texture*>(desc.targets[i].texture);
            atts[curAtt].format = mTex[i]->mFormat;
        }
        else
            atts[curAtt].format = TranslateElementFormatToVkFormat(desc.targets[i].format);

        atts[curAtt].samples = VK_SAMPLE_COUNT_1_BIT;
        // we do not care about auto-clearing - this should be triggered by CommandBuffer::Clear()
        atts[curAtt].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[curAtt].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        atts[curAtt].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[curAtt].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[curAtt].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        atts[curAtt].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VK_ZERO_MEMORY(colorRefs[curAtt]);
        colorRefs[curAtt].attachment = curAtt;
        colorRefs[curAtt].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        curAtt++;
    }

    VkAttachmentReference depthRef;
    if (desc.depthBuffer)
    {
        VK_ZERO_MEMORY(atts[curAtt]);
        atts[curAtt].format = mDepthTex->mFormat;
        atts[curAtt].samples = VK_SAMPLE_COUNT_1_BIT;
        atts[curAtt].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[curAtt].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[curAtt].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[curAtt].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[curAtt].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        atts[curAtt].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depthRef.attachment = curAtt;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass;
    VK_ZERO_MEMORY(subpass);
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = curAtt;
    subpass.pColorAttachments = colorRefs;
    if (desc.depthBuffer)
        subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpInfo;
    VK_ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = curAtt;
    rpInfo.pAttachments = atts;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(gDevice->GetDevice(), &rpInfo, nullptr, &mRenderPass);
    CHECK_VKRESULT(result, "Failed to create Render Pass");

    mAttachmentCount = curAtt;

    // TODO TEMPSHIT!!!!
    mFramebuffers.resize(mTex[0]->mBuffersNum);

    // for each swapchain image create a framebuffer
    if (mTex[0]->mFromSwapchain)
    {
        for (unsigned int i = 0; i < mTex[0]->mBuffersNum; ++i)
        {
            std::vector<VkImageView> fbAtts;
            fbAtts.push_back(mTex[0]->mBufferViews[i]);
            if (mDepthTex)
                fbAtts.push_back(mDepthTex->mBufferViews[0]);

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
        // TODO fill
    }

    Texture* tex = dynamic_cast<Texture*>(desc.targets[0].texture);
    mWidth = tex->mWidth;
    mHeight = tex->mHeight;

    LOG_INFO("Render Target created successfully");
    return true;
}

const VkFramebuffer& RenderTarget::GetCurrentFramebuffer() const
{
    return mFramebuffers[mTex[0]->mCurrentBuffer];
}

} // namespace Renderer
} // namespace NFE
