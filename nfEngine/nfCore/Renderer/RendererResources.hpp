/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "../Core.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

namespace NFE {
namespace Renderer {

struct MeshVertex
{
    Math::Float3 position;
    Math::Float2 texCoord;
    char normal[4];
    char tangent[4];
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

/*
    Structure describing material layer information.
    Used by GBuffer renderer.
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


// Global renderer settings
struct RendererSettings
{
    bool VSync;


    bool tileBasedDeferredShading;

    bool pipelineStats;

    bool motionBlur;
    float motionBlurFactor;

    bool bloom;
    int bloomSize;
    float bloomVariance;
    float bloomFactor;

    bool autoExposure;
    bool gammaCorrection;
    float saturation;
    float noiseLevel;
    bool noiseEnabled;

    bool antialiasing;
    float antialiasingSpanMax;
    float antialiasingReduceMul;
    float antialiasingReduceMin;
    bool antialiasingLumaOpt;

    bool debugEnable;
    bool debugLights;
    bool debugMeshes;

    RendererSettings()
    {
        VSync = false;

        tileBasedDeferredShading = false;
        autoExposure = true;

        bloom = true;
        bloomSize = 16;
        bloomVariance = 4.0f;
        bloomFactor = 0.4f;

        motionBlur = true;
        motionBlurFactor = 0.2f;

        pipelineStats = false;
        gammaCorrection = true;
        noiseLevel = 0.02f;
        noiseEnabled = true;
        saturation = 1.0f;

        antialiasing = true;
        antialiasingLumaOpt = true;
        antialiasingSpanMax = 8.0;
        antialiasingReduceMul = 16.0;
        antialiasingReduceMin = 128.0;

        debugEnable = true;
        debugLights = true;
        debugMeshes = true;
    }
};


// Contains rendering preferences, etc.
struct ViewSettings
{
    // this parameter is modified by the engine if auto-exposure is ON
    float exposure;
    float clampedExposure;

    // TODO: move RendererSettings here (most of them)

    ViewSettings()
    {
        exposure = 1.0f;
        clampedExposure = 1.0f;
    }
};

// Shadowmap resource (flat, cube and cascaded)
class ShadowMap
{
public:
    enum class Type : int
    {
        None = 0,
        Flat,       // spot light
        Cube,       // omni light
        Cascaded,   // directional light
    };

    ShadowMap();
    void Release();
    int Resize(uint32 size, Type type, uint32 splits = 1);
    uint32 GetSize() const;
};

struct NFE_ALIGN16 DirLightProperties
{
    Math::Vector direction;
    Math::Vector color;
    uint32 cascadesCount[4];

    Math::Vector splitDistance[8];
    Math::Matrix viewProjMatrix[8];
};

struct NFE_ALIGN16 OmniLightProperties
{
    Math::Vector position;
    Math::Vector radius;
    Math::Vector color;
    Math::Vector shadowMapResInv;
};

struct NFE_ALIGN16 SpotLightProperties
{
    Math::Vector position;
    Math::Vector direction;
    Math::Vector color;
    Math::Vector farDist;
    Math::Matrix viewProjMatrix;
    Math::Matrix viewMatrix;
    float shadowMapResInv;
};

struct TileOmniLightDesc
{
    Math::Float3 pos;
    float radius;
    float radiusInv;
    Math::Float3 color;
};

struct ToneMappingDesc
{
    ITexture* source;
    ITexture* bloom;
    IRenderTarget* dest;

    float exposure;
    float bloomFactor;
    float noiseLevel;
    bool  noiseEnabled;
    float saturation;
};

struct FXAADesc
{
    ITexture* source;
    IRenderTarget* dest;

    float noiseLevel;
    bool  noiseEnabled;

    bool lumaOpt;
    float spanMax;
    float reduceMul;
    float reduceMin;
};

} // namespace Renderer
} // namespace NFE
