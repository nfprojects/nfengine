/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan's Render Pass Manager.
 */

#include "PCH.hpp"
#include "RenderPassManager.hpp"
#include "Device.hpp"


namespace NFE {
namespace Renderer {

RenderPassManager::RenderPassManager(VkDevice device)
    : mDeviceRef(device)
{
}

RenderPassManager::~RenderPassManager()
{
    for (auto& rp : mRenderPasses)
        if (rp.second != VK_NULL_HANDLE)
            vkDestroyRenderPass(mDeviceRef, rp.second, nullptr);
}

VkRenderPass RenderPassManager::ConstructRenderPass(const RenderPassDesc& desc)
{
    VkAttachmentDescription atts[MAX_RENDER_TARGETS + 1];
    uint32 curAtt = 0;
    VkAttachmentReference colorRefs[MAX_RENDER_TARGETS];

    for (uint32 i = 0; i < desc.colorFormatCount; ++i)
    {
        VK_ZERO_MEMORY(atts[curAtt]);
        atts[curAtt].format = desc.colorFormats[i];
        atts[curAtt].samples = VK_SAMPLE_COUNT_1_BIT;
        // we do not care about auto-clearing - this should be triggered by CommandRecorder::Clear()
        atts[curAtt].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[curAtt].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        atts[curAtt].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[curAtt].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[curAtt].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        atts[curAtt].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VK_ZERO_MEMORY(colorRefs[i]);
        colorRefs[i].attachment = i;
        colorRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        curAtt++;
    }

    VkAttachmentReference depthRef;
    if (desc.depthFormat != VK_FORMAT_UNDEFINED)
    {
        VK_ZERO_MEMORY(atts[curAtt]);
        atts[curAtt].format = desc.depthFormat;
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
    subpass.colorAttachmentCount = desc.colorFormatCount;
    subpass.pColorAttachments = colorRefs;
    if (desc.depthFormat != VK_FORMAT_UNDEFINED)
        subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpInfo;
    VK_ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = curAtt;
    rpInfo.pAttachments = atts;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    VkRenderPass tempRenderPass = VK_NULL_HANDLE;
    VkResult result = vkCreateRenderPass(mDeviceRef, &rpInfo, nullptr, &tempRenderPass);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to construct new Render Pass");
        LOG_DEBUG("Formats requested are:");
        for (uint32 i = 0; i < desc.colorFormatCount; ++i)
            LOG_DEBUG("    color #%i: %d (%s)", i, desc.colorFormats[i], TranslateVkFormatToString(desc.colorFormats[i]));
        LOG_DEBUG("    depth:     %d (%s)", desc.depthFormat, TranslateVkFormatToString(desc.depthFormat));
        return VK_NULL_HANDLE;
    }

    return tempRenderPass;
}

VkRenderPass RenderPassManager::GetRenderPass(const RenderPassDesc& desc)
{
    // TODO uncomment when NFE_ASSERT is done right
    //NFE_ASSERT(desc.colorFormatCount <= MAX_RENDER_TARGETS, "Too many color formats requested (max is 8)");

    auto rp = mRenderPasses.find(desc);
    if (rp == mRenderPasses.end())
    {
        VkRenderPass newRp = ConstructRenderPass(desc);
        if (newRp != VK_NULL_HANDLE)
            mRenderPasses.insert(std::make_pair(desc, newRp));

        LOG_INFO("Created new Render Pass");
        return newRp;
    }

    LOG_INFO("Using existing Render Pass");
    return rp->second;
}

} // namespace Renderer
} // namespace NFE
