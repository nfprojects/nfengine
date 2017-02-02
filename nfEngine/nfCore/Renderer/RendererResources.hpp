/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "../Core.hpp"
#include "../../Renderers/RendererInterface/Device.hpp"

#include "nfCommon/Math/Matrix.hpp"


namespace NFE {
namespace Renderer {

struct MeshVertex
{
    Math::Float3 position;
    Math::Float2 texCoord;
    char normal[4];
    char tangent[4];
};

struct NFE_ALIGN(16) ShadowCameraRenderDesc
{
    Math::Matrix viewProjMatrix;
    Math::Vector lightPos;
};

struct NFE_ALIGN(16) CameraRenderDesc
{
    Math::Matrix matrix;        // global camera matrix (pos + orientation)
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;
    Math::Matrix secViewMatrix; // for motionblur

    Math::Vector velocity;
    Math::Vector angualrVelocity;

    Math::Vector screenScale;
    float fov;
};

/**
 *  Structure describing material layer information.
 *  Used by GBuffer renderer.
 */
struct RendererMaterialLayer
{
    ResourceBindingInstancePtr bindingInstance;

    Math::Float4 diffuseColor;
    Math::Float4 specularColor; // x - factor, w - power
    Math::Float4 emissionColor;
};

struct RendererMaterial
{
    RendererMaterialLayer layers[1];
    // TODO mutliple texture layers support
};

// Shadowmap resource (flat, cube and cascaded)
class ShadowMap
{
    friend class GeometryRenderer;
    friend class LightsRenderer;

public:
    const static uint8 MAX_CASCADE_SPLITS = 8;
    const static uint16 MIN_SHADOWMAP_SIZE = 16;
    const static uint16 MAX_SHADOWMAP_SIZE = 4096;

    enum class Type : uint8
    {
        None = 0,
        Flat,       // spot light
        Cube,       // omni light
        Cascaded,   // directional light
    };

    ShadowMap();
    ~ShadowMap();
    void Release();
    bool Resize(uint32 size, Type type, uint32 splits = 1);

    NFE_INLINE uint32 GetSize() const
    {
        return static_cast<uint32>(mSize);
    }

private:
    TexturePtr mTexture;
    TexturePtr mDepthBuffer;
    RenderTargetPtr mRenderTargets[MAX_CASCADE_SPLITS];
    ResourceBindingInstancePtr mBindingInstance;

    uint16 mSize;
    Type mType;
    uint8 mSplits;
};

class GeometryBuffer
{
    friend class GeometryRenderer;
    friend class LightsRenderer;

    static const int gLayers = 3;

    int mWidth;
    int mHeight;
    TexturePtr mDepthBuffer;
    TexturePtr mTextures[gLayers];
    RenderTargetPtr mRenderTarget;
    ResourceBindingInstancePtr mBindingInstance;

public:
    void Release();
    bool Resize(int width, int height);

    NFE_INLINE int GetWidth() const
    {
        return mWidth;
    }

    NFE_INLINE int GetHeight() const
    {
        return mHeight;
    }
};

} // namespace Renderer
} // namespace NFE
