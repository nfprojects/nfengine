/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 12 render's pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

typedef std::tuple<IPipelineState*, IShaderProgram*> FullPipelineStateParts;

class PipelineState : public IPipelineState
{
    friend class CommandBuffer;
    friend class FullPipelineState;

    ResourceBindingLayout* mBindingLayout;
    D3D12_INPUT_LAYOUT_DESC mInputLayoutDesc;
    D3D12_RASTERIZER_DESC mRasterizerDesc;
    D3D12_DEPTH_STENCIL_DESC mDepthStencilDesc;
    D3D12_BLEND_DESC mBlendDesc;

public:
    bool Init(const PipelineStateDesc& desc);
};

class FullPipelineState
{
    friend class CommandBuffer;
    friend class Device;

    D3DPtr<ID3D12PipelineState> mPipelineState;

public:
    bool Init(const FullPipelineStateParts& parts);
};

} // namespace Renderer
} // namespace NFE
