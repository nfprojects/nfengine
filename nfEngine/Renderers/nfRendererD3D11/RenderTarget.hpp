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
    friend class CommandRecorder;

    std::vector<D3DPtr<ID3D11RenderTargetView>> mRTVs;
    Texture* mDepthBuffer;
    int mWidth;
    int mHeight;

public:
    RenderTarget();
    void GetDimensions(int& width, int& height) override;
    bool Init(const RenderTargetDesc& desc) override;
};

} // namespace Renderer
} // namespace NFE
