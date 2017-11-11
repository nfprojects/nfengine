/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "nfRenderer.hpp"
#include "../Renderers/RendererInterface/Device.hpp"


namespace NFE {
namespace Renderer {

class HighLevelRenderer;

/**
 * Represents a GBuffer used in deferred shading.
 */
class GeometryBuffer final
{
    friend class GeometryRenderer;
    friend class LightsRenderer;

    // number of buffers in the GBuffer
    static constexpr uint32 NumBuffers = 3;

    uint32 mWidth;
    uint32 mHeight;
    TexturePtr mDepthBuffer;
    TexturePtr mTextures[NumBuffers];
    RenderTargetPtr mRenderTarget;
    ResourceBindingInstancePtr mBindingInstance;

public:
    void Release();
    bool Resize(HighLevelRenderer& renderer, uint32 width, uint32 height);

    NFE_INLINE uint32 GetWidth() const
    {
        return mWidth;
    }

    NFE_INLINE uint32 GetHeight() const
    {
        return mHeight;
    }
};

} // namespace Renderer
} // namespace NFE
