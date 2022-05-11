/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan render's compute pipeline states.
 */

#pragma once

#include "BasePipelineState.hpp"


namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState, public BasePipelineState
{
public:
    ComputePipelineState();
    ~ComputePipelineState();

    bool Init(const ComputePipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
