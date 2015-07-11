/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's render target
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Texture;

class RenderTarget : public IRenderTarget
{
    friend class CommandBuffer;

    std::vector<D3DPtr<ID3D11RenderTargetView>> mRTVs;
    Texture* mDepthBuffer;

public:
    RenderTarget();
    bool Init(const RenderTargetDesc& desc);
};

} // namespace Renderer
} // namespace NFE
