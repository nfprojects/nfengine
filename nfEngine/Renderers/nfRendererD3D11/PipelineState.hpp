/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 11 render's pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandBuffer;

    D3DPtr<ID3D11RasterizerState> mRS;
    D3DPtr<ID3D11BlendState> mBS;
    D3DPtr<ID3D11DepthStencilState> mDS;
    // TODO Blend, Rasterizer and DepthStencil states can be shared in D3D11

public:
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
