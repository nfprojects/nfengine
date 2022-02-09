/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Vulkan render's compute pipeline states.
 */

#pragma once

#include "../RendererCommon/ComputePipelineState.hpp"

namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState
{
    friend class CommandRecorder;

    VkPipeline mPipeline;

public:
    ComputePipelineState();
    ~ComputePipelineState();

    bool Init(const ComputePipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
