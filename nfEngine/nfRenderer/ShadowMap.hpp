/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "nfRenderer.hpp"

#include "../Renderers/RendererInterface/Texture.hpp"


namespace NFE {
namespace Renderer {

class HighLevelRenderer;

/**
 * Shadowmap resource (flat, cube or cascaded)
 */
// TODO consider splitting into separate class for each type
class ShadowMap
{
    friend class GeometryRenderer;
    friend class LightsRenderer;

public:
    static constexpr uint8 MaxCascadeSplits = 4;
    static constexpr uint16 MinSize = 16;
    static constexpr uint16 MaxSize = 4096;

    enum class Type : uint8
    {
        None = 0,
        Flat,       // for spot light
        Cube,       // for omni light
        Cascaded,   // for directional light
    };

    ShadowMap();
    ~ShadowMap();

    void Release();
    bool Resize(HighLevelRenderer& renderer, uint16 size, Type type, uint8 splits = 1);

    NFE_INLINE uint16 GetSize() const
    {
        return mSize;
    }

private:
    TexturePtr mTexture;
    TexturePtr mDepthBuffer; // TODO not needed?
    RenderTargetPtr mRenderTargets[MaxCascadeSplits];
    ResourceBindingInstancePtr mBindingInstance;

    uint16 mSize;
    Type mType;
    uint8 mSplits;
};


} // namespace Renderer
} // namespace NFE
