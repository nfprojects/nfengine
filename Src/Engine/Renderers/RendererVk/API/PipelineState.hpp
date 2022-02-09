/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "../RendererCommon/PipelineState.hpp"
#include "Defines.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandRecorder;

    PipelineStateDesc mDesc;
    Common::StaticArray<VkPipelineShaderStageCreateInfo, 5> mShaderStageDescs;
    Common::StaticArray<VkShaderModule, 5> mShaderStages;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;

    VkShaderModule CreateShaderModule(const SpvReflectShaderModule& shaderSpv);
    bool PrepareShaderStage(const ShaderPtr& shader);
    bool CreatePipelineLayout();

public:
    PipelineState();
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
