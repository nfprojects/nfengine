/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Vulkan implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"
#include "Device.hpp"
#include "Shader.hpp"
#include "ResourceBinding.hpp"

#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
    : mPipeline(VK_NULL_HANDLE)
{
}

ComputePipelineState::~ComputePipelineState()
{
    if (mPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(gDevice->GetDevice(), mPipeline, nullptr);
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    Shader* s = dynamic_cast<Shader*>(desc.computeShader.Get());
    if (s == nullptr)
    {
        NFE_LOG_ERROR("Invalid compute shader provided for compute pipeline state");
        return false;
    }

    ResourceBindingLayout* rbl = dynamic_cast<ResourceBindingLayout*>(desc.resBindingLayout.Get());
    if (rbl == nullptr)
    {
        NFE_LOG_ERROR("Invalid resource binding layout provided for compute pipeline state");
        return false;
    }

    VkComputePipelineCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    info.stage = s->mStageInfo;
    info.layout = rbl->mPipelineLayout;
    VkResult result = vkCreateComputePipelines(gDevice->GetDevice(), gDevice->GetPipelineCache(), 1, &info, nullptr, &mPipeline);
    CHECK_VKRESULT(result, "Failed to create Compute Pipeline");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mPipeline), VK_OBJECT_TYPE_PIPELINE, desc.debugName);

    NFE_LOG_INFO("Compute Pipeline State created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
