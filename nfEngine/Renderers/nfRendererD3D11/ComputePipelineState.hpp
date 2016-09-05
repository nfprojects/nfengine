/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 11 render's compute pipeline states.
 */

#pragma once

#include "../RendererInterface/ComputePipelineState.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState
{
    ID3D11ComputeShader* mComputeShader;
    ResourceBindingLayout* mResBindingLayout;

public:
    ComputePipelineState();
    bool Init(const ComputePipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
