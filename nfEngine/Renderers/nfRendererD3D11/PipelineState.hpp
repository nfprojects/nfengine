/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 11 render's pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Common.hpp"

#include "VertexLayout.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

class VertexLayout;

class PipelineState : public IPipelineState
{
    friend class CommandBuffer;

    ID3D11VertexShader* mVertexShader;
    ID3D11GeometryShader* mGeometryShader;
    ID3D11HullShader* mHullShader;
    ID3D11DomainShader* mDomainShader;
    ID3D11PixelShader* mPixelShader;

    D3DPtr<ID3D11RasterizerState> mRS;
    D3DPtr<ID3D11BlendState> mBS;
    D3DPtr<ID3D11DepthStencilState> mDS;
    PrimitiveType mPrimitiveType;
    uint32 mNumControlPoints;
    VertexLayout* mVertexLayout;
    ResourceBindingLayout* mResBindingLayout;

public:
    PipelineState();
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
