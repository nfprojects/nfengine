/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Vulkan implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"
#include "Device.hpp"
#include "Shader.hpp"

#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
    : BasePipelineState(VK_PIPELINE_BIND_POINT_COMPUTE)
{
}

ComputePipelineState::~ComputePipelineState()
{
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    InitializeSettings(desc);

    if (!PrepareShaderStage(desc.computeShader))
    {
        NFE_LOG_ERROR("Failed to prepare Compute Shader Stage for Pipeline");
        return false;
    }

    if (!CreatePipelineLayout())
    {
        NFE_LOG_ERROR("Failed to create a Pipeline Layout for this PipelineState");
        return false;
    }

    if (!FormShaderModules())
    {
        NFE_LOG_ERROR("Failed to compile Shaders for PipelineState");
        return false;
    }

    VkComputePipelineCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    info.stage = mShaderStageDescs[0];
    info.layout = mPipelineLayout;
    VkResult result = vkCreateComputePipelines(gDevice->GetDevice(), gDevice->GetPipelineCache(), 1, &info, nullptr, &mPipeline);
    CHECK_VKRESULT(result, "Failed to create Compute Pipeline");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mPipeline), VK_OBJECT_TYPE_PIPELINE, desc.debugName);

    NFE_LOG_INFO("Compute Pipeline State created successfully");
    return true;
}

} // namespace Renderer
} // namespace NFE
