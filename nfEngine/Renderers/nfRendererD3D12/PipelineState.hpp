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

    ResourceBindingLayout* mBindingLayout;
    D3D12_INPUT_LAYOUT_DESC mInputLayoutDesc;
    D3D12_RASTERIZER_DESC mRasterizerDesc;
    D3D12_DEPTH_STENCIL_DESC mDepthStencilDesc;
    D3D12_BLEND_DESC mBlendDesc;

    PrimitiveType mPrimitiveType;
    uint32 mNumControlPoints;

    uint32 mNumRenderTargets;
    DXGI_FORMAT mRenderTargetFormats[MAX_RENDER_TARGETS];
    DXGI_FORMAT mDepthStencilFormat;

public:
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);

    static ID3D12PipelineState* CreateFullPipelineState(const FullPipelineStateParts& parts);
};

} // namespace Renderer
} // namespace NFE
