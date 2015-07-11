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

class BlendState : public IBlendState
{
    friend class CommandBuffer;
    D3DPtr<ID3D11BlendState> mBS;
public:
    bool Init(const BlendStateDesc& desc);
};

class RasterizerState : public IRasterizerState
{
    friend class CommandBuffer;
    D3DPtr<ID3D11RasterizerState> mRS;
public:
    bool Init(const RasterizerStateDesc& desc);
};

class DepthState : public IDepthState
{
    friend class CommandBuffer;
    D3DPtr<ID3D11DepthStencilState> mDS;
public:
    bool Init(const DepthStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
