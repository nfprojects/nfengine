/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Vulkan render's compute pipeline states.
 */

#pragma once

#include "../RendererInterface/ComputePipelineState.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState
{
public:
    ComputePipelineState();
    bool Init(const ComputePipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
