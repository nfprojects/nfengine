/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "../Core.hpp"
#include "../Prerequisites.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

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

struct NFE_ALIGN16 ShadowCameraRenderDesc
{
    Math::Matrix viewProjMatrix;
    Math::Vector lightPos;
};

struct NFE_ALIGN16 CameraRenderDesc
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
    ITexture* diffuseTex;
    ITexture* normalTex;
    ITexture* specularTex;

    Math::Float4 diffuseColor;
    Math::Float4 specularColor; // x - factor, w - power
    Math::Float4 emissionColor;
};

struct RendererMaterial
{
    RendererMaterialLayer* layers;
    uint32 layersNum;

    // TODO: layers mixing

    RendererMaterial()
    {
        layers = NULL;
        layersNum = 0;
    }

    ~RendererMaterial()
    {
        if (layers)
        {
            delete[] layers;
            layers = 0;
        }
    }
};

// Shadowmap resource (flat, cube and cascaded)
class ShadowMap
{
    friend class GeometryRenderer;
    friend class LightsRenderer;

public:
    const static uchar MAX_CASCADE_SPLITS = 8;
    const static uint16 MIN_SHADOWMAP_SIZE = 16;
    const static uint16 MAX_SHADOWMAP_SIZE = 4096;

    enum class Type : uchar
    {
        None = 0,
        Flat,       // spot light
        Cube,       // omni light
        Cascaded,   // directional light
    };

    ShadowMap();
    void Release();
    bool Resize(uint32 size, Type type, uint32 splits = 1);

    NFE_INLINE uint32 GetSize() const
    {
        return static_cast<uint32>(mSize);
    }

private:
    std::unique_ptr<ITexture> mTexture;
    std::unique_ptr<ITexture> mDepthBuffer;
    std::unique_ptr<IRenderTarget> mRenderTargets[MAX_CASCADE_SPLITS];

    uint16 mSize;
    Type mType;
    uchar mSplits;
};

class GeometryBuffer
{
    friend class GeometryRenderer;
    friend class LightsRenderer;

    static const int gLayers = 4;

    int mWidth;
    int mHeight;
    std::unique_ptr<ITexture> mDepthBuffer;
    std::unique_ptr<ITexture> mTextures[gLayers];
    std::unique_ptr<IRenderTarget> mRenderTarget;

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
