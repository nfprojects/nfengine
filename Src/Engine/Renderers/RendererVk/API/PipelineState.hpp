/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "../RendererCommon/PipelineState.hpp"
#include "Defines.hpp"
#include "ResourceBinding.hpp"

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
    VkPipeline mPipeline;

    VkShaderModule CreateShaderModule(const Common::DynArray<uint32>& shaderSpv);
    bool PrepareShaderStage(const ShaderPtr& shader);

public:
    PipelineState();
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
