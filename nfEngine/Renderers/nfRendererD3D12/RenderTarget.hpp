/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's render target
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

    Texture* mDepthBuffer;
    int mWidth;
    int mHeight;

public:
    RenderTarget();
    void GetDimensions(int& width, int& height);
    bool Init(const RenderTargetDesc& desc);
};

} // namespace Renderer
} // namespace NFE
