/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "../Interface/PipelineState.hpp"
#include "Defines.hpp"
#include "ResourceBinding.hpp"


namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandRecorder;

    PipelineStateDesc mDesc;
    VkPipeline mPipeline;

public:
    PipelineState();
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
