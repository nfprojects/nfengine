/**
 * @file   Renderer.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  This header contains definitions of abstract classes used to interact with
 *         a generic renderer unit.
 */

#pragma once

#include "Core.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Render {

struct MeshVertex
{
    Math::Float3 position;
    Math::Float2 texCoord;
    char normal[4];
    char tangent[4];
};

NFE_ALIGN(16)
struct CameraRenderDesc
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
    IRendererTexture* diffuseTex;
    IRendererTexture* normalTex;
    IRendererTexture* specularTex;

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

        debugEnable = false;
        debugLights = false;
        debugMeshes = false;
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


/*
    Interface for main rendering unit.
    Provides methods for resources (textures, buffers, etc.)
    and modules creation.
*/
class IRenderer
{
public:
    // global renderer settings
    RendererSettings settings;

    virtual ~IRenderer() {};

    virtual int Init() = 0;
    virtual void* GetDevice() const = 0; // obtain lower-level API object (ID3D11Device, etc.)

    /// Rendering modules
    virtual IGuiRenderer* GetGuiRenderer() const = 0;
    virtual IPostProcessRenderer* GetPostProcessRenderer() const = 0;
    virtual IDebugRenderer* GetDebugRenderer() const = 0;
    virtual IShadowRenderer* GetShadowRenderer() const = 0;
    virtual IGBufferRenderer* GetGBufferRenderer() const = 0;
    virtual ILightsRenderer* GetLightsRenderer() const = 0;

    virtual IRenderContext* GetImmediateContext() const = 0;
    virtual IRenderContext* CreateDeferredContext() = 0;


    virtual void Begin() = 0;

    // apply postprocess and present
    virtual void SwapBuffers(IRenderTarget* pRenderTarget, ViewSettings* pViewSettings, float dt) = 0;

    virtual void ExecuteDeferredContext(IRenderContext* pContext) = 0;

    /// Renderer resources creation fuctions, use 'delete' to free them
    virtual IRendererBuffer* CreateBuffer() = 0;
    virtual IRendererTexture* CreateTexture() = 0;
    virtual IRenderTarget* CreateRenderTarget() = 0;
    virtual IShadowMap* CreateShadowMap() = 0;
};

// Abstract class representing renderer buffer: vertex buffer or index buffer
class IRendererBuffer
{
public:
    enum class Type
    {
        Unknown = 0,
        Vertex,
        Index,
    };

    IRendererBuffer() {}
    virtual ~IRendererBuffer() {}
    virtual void Release() = 0;
    virtual bool Init(Type type, const void* pData, uint32 dataSize) = 0;
};

// Renderer 2D texture resource
class IRendererTexture
{
public:
    IRendererTexture() {}
    virtual ~IRendererTexture() {}
    virtual void Release() = 0;
    virtual Result FromImage(const Common::Image& image) = 0;
};

// Buffer for view from cameras, tone mapping, etc.
class IRenderTarget : virtual public IRendererTexture
{
public:
    IRenderTarget() {}
    virtual ~IRenderTarget() {}
    virtual void Release() = 0;
    virtual int OnResize() = 0; // used only for targets binded with a window
    virtual int Init(uint32 width, uint32 height, Common::Window* pWindow = NULL, bool HDR = false) = 0;
    virtual int Present() = 0;
    virtual Result FromImage(const Common::Image& image) = 0;
};

// Shadowmap resource (flat, cube and cascaded)
class IShadowMap
{
public:
    enum class Type : int
    {
        None = 0,
        Flat,       // spot light
        Cube,       // omni light
        Cascaded,   // directional light
    };

    IShadowMap() {}
    virtual ~IShadowMap() {}
    virtual void Release() = 0;
    virtual int Resize(uint32 size, Type type, uint32 splits = 1) = 0;
    virtual uint32 GetSize() const = 0;
};


class Font;

class IGuiRenderer
{
public:
    IGuiRenderer() {};
    virtual ~IGuiRenderer() {};

    // TODO: move to the engine's core
    virtual Font* MakeFont(const char* pPath, int height) = 0;

    virtual void Enter(NFE_CONTEXT_ARG) = 0;
    virtual void Leave(NFE_CONTEXT_ARG) = 0;
    virtual void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget) = 0;
    virtual void DrawQuad(NFE_CONTEXT_ARG, const Recti& rect, IRendererTexture* pTexture,
                          Rectf* pTexCoords, UINT color, bool alpha = false) = 0;
    virtual void PrintText(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect, uint32 Align = 0,
                           Font* pFont = 0, uint32 Color = 0xFFFFFFFF) = 0;
    virtual void PrintTextWithBorder(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect,
                                     uint32 Align , Font* pFont, uint32 Color, uint32 BorderColor) = 0;
};

class IDebugRenderer
{
public:
    virtual ~IDebugRenderer() {};

    virtual void Enter(NFE_CONTEXT_ARG) = 0;
    virtual void Leave(NFE_CONTEXT_ARG) = 0;

    virtual void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget) = 0;
    virtual void SetCamera(NFE_CONTEXT_ARG, const Math::Matrix& viewMatrix,
                           const Math::Matrix& projMatrix) = 0; // TODO: CameraRenderDesc
    virtual void DrawLine(NFE_CONTEXT_ARG, const Math::Vector& A, const Math::Vector& B,
                          const UINT color) = 0;
    virtual void DrawLine(NFE_CONTEXT_ARG, const Math::Float3& A, const Math::Float3& B,
                          const UINT color) = 0;
    virtual void DrawBox(NFE_CONTEXT_ARG, const Math::Box& box, const UINT color) = 0;
    virtual void DrawFilledBox(NFE_CONTEXT_ARG, const Math::Box& box, const UINT color) = 0;
    virtual void DrawFrustum(NFE_CONTEXT_ARG, const Math::Frustum& frustum, const UINT color) = 0;
    virtual void DrawQuad(NFE_CONTEXT_ARG, const Math::Vector& pos, IRendererTexture* pTexture,
                          const UINT color) = 0;
    //virtual void SetMaterial(NFE_CONTEXT_ARG, const Material* pMaterial) = 0; // TODO: remove dependencies
    //virtual void DrawMesh(NFE_CONTEXT_ARG, const Mesh* pMesh) = 0;
};

class IShadowRenderer
{
public:
    virtual ~IShadowRenderer() {};

    virtual void Enter(NFE_CONTEXT_ARG) = 0;
    virtual void Leave(NFE_CONTEXT_ARG) = 0;
    virtual void SetDestination(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera,
                                IShadowMap* pShadowMap, uint32 faceID) = 0;
    virtual void SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial) = 0;
    virtual void Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer) = 0;
};

struct DirLightProperties
{
    Math::Vector direction;
    Math::Vector color;
    uint32 cascadesCount[4];

    Math::Vector splitDistance[8];
    Math::Matrix viewProjMatrix[8];
};

struct OmniLightProperties
{
    Math::Vector position;
    Math::Vector radius;
    Math::Vector color;
    Math::Vector shadowMapResInv;
};

struct SpotLightProperties
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

class ILightsRenderer
{
public:
    ILightsRenderer() {}
    virtual ~ILightsRenderer() {}

    virtual void Enter(NFE_CONTEXT_ARG) = 0;
    virtual void Leave(NFE_CONTEXT_ARG) = 0;

    // TODO: reduce number of arguments
    virtual void SetUp(NFE_CONTEXT_ARG, IRenderTarget* pRT, const CameraRenderDesc* pCamera,
                       const Math::Vector& ambientLightColor, const Math::Vector& backgroundColor) = 0;
    virtual void TileBasedPass(NFE_CONTEXT_ARG, uint32 lightsCount,
                               const TileOmniLightDesc* pLights) = 0;
    virtual void DrawOmniLight(NFE_CONTEXT_ARG, const Math::Vector& pos, float radius,
                               const Math::Vector& color, IShadowMap* pShadowMap) = 0;
    virtual void DrawSpotLight(NFE_CONTEXT_ARG, const SpotLightProperties& prop,
                               const Math::Frustum& frustum, IShadowMap* pShadowMap, IRendererTexture* pLightMap) = 0;
    virtual void DrawDirLight(NFE_CONTEXT_ARG, const DirLightProperties& prop,
                              IShadowMap* pShadowMap) = 0;
    virtual void DrawFog(NFE_CONTEXT_ARG) = 0;
};

class IGBufferRenderer
{
public:
    virtual ~IGBufferRenderer() {};

    virtual void Enter(NFE_CONTEXT_ARG) = 0;
    virtual void Leave(NFE_CONTEXT_ARG) = 0;

    virtual void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget) = 0;
    virtual void SetCamera(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera) = 0;
    virtual void SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial) = 0;
    virtual void Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer) = 0;
};


class IRenderContext
{
public:
    InstanceData* instanceData; // used to store per-instance vertex buffers
    RenderCommandBuffer commandBuffer;

    virtual ~IRenderContext() {}
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual bool Execute(IRenderContext* pCtx, bool saveState = false) = 0;
};

} // namespace Render
} // namespace NFE
