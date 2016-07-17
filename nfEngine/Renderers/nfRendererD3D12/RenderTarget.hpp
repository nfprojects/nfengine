/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's render target
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"
#include "Common.hpp"

#include <vector>


namespace NFE {
namespace Renderer {

class Texture;

class RenderTarget : public IRenderTarget
{
    friend class CommandBuffer;

    // TODO: temporary
    std::vector<Texture*> mTextures;
    std::vector<uint32> mRTVs[2];

    Texture* mDepthTexture;
    uint32 mDSV;

    uint16 mWidth;
    uint16 mHeight;

public:
    RenderTarget();
    ~RenderTarget();
    void GetDimensions(int& width, int& height);
    bool Init(const RenderTargetDesc& desc);
};

} // namespace Renderer
} // namespace NFE
