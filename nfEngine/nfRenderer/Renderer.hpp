#pragma once
#include "nfRenderer.hpp"
#include "RendererResources.hpp"
#include "RendererContext.hpp"
#include "../nfCore/Renderer.hpp"

namespace NFE {
namespace Render {

struct GPUStats
{
    uint32 IAVertices;
    uint32 IAPrimitives;
    uint32 VSInvocations;
    uint32 GSInvocations;
    uint32 GSPrimitives;
    uint32 CInvocations;
    uint32 CPrimitives;
    uint32 PSInvocations;
    uint32 HSInvocations;
    uint32 DSInvocations;
    uint32 CSInvocations;

    GPUStats();
};

struct GPUFeatures
{
    D3D11_FEATURE_DATA_DOUBLES doubles;
    D3D11_FEATURE_DATA_THREADING threading;
    D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS D3D10Options;
};


class RendererD3D11 : public IRenderer
{
    void InitDepthBuffer();

    RenderContextD3D11* mImmContext;
    PostProcessRendererD3D11* mPostProcessRenderer;
    GuiRendererD3D11* mGuiRenderer;
    LightsRendererD3D11* mLightRenderer;
    DebugRendererD3D11* mDebugRenderer;
    GBufferRendererD3D11* mGBufferRenderer;
    ShadowRendererD3D11* mShadowRenderer;

public:
    //main D3D objects
    ID3D11Device* D3DDevice;
    IDXGIFactory* DXGIFactory;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Query* statsQuery;
    GPUStats pipelineStats;

    ID3D11BlendState* defaultBlendState;
    ID3D11DepthStencilState* defaultDepthStencilState;
    ID3D11RasterizerState* defaultRasterizerState;
    ID3D11SamplerState* defaultSampler;

    //common full screen quad rendering (ambient light, postprocess)
    ID3D11Buffer* screenQuadVB;
    ID3D11Buffer* screenQuadIB;
    Multishader fullScreenQuadVS;
    ID3D11InputLayout* fullScreenQuadIL;

    //this buffers are common for all displays (to reduce VRAM usage)
    int bufferWidth;
    int bufferHeight;
    GeometryBuffer geomBuffer;
    RenderTargetD3D11 renderTarget;
    RenderTargetD3D11 postProcessed;
    RenderTargetD3D11 toneMapped;
    RenderTargetD3D11 bloomA; //downsapled render target and final blur (vertical blur of BloomA)
    RenderTargetD3D11 bloomB; //horizontal blur of BloomA

    //default material textures
    RendererTextureD3D11* defaultDiffuseTexture;
    RendererTextureD3D11* defaultNormalTexture;
    RendererTextureD3D11* defaultSpecularTexture;
    RendererTextureD3D11* noiseTexture;

    //default depth buffer
    ID3D11Texture2D* depthStencilBuffer;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11ShaderResourceView* depthStencilSRV;

    GPUFeatures gpuFeatures;


    RendererD3D11();
    ~RendererD3D11();
    void* GetDevice() const;
    static RendererD3D11*& GetInstance();
    int Init();
    int InitModules();
    void ReleaseModules();

    void Begin();
    void SwapBuffers(IRenderTarget* pRenderTarget, ViewSettings* pViewSettings, float dt);

    void ExecuteDeferredContext(IRenderContext* pContext);


    IGuiRenderer* GetGuiRenderer() const;
    IPostProcessRenderer* GetPostProcessRenderer() const;
    IDebugRenderer* GetDebugRenderer() const;
    IShadowRenderer* GetShadowRenderer() const;
    IGBufferRenderer* GetGBufferRenderer() const;
    ILightsRenderer* GetLightsRenderer() const;


    IRenderContext* GetImmediateContext() const;
    IRenderContext* CreateDeferredContext();

    /// Resources creation methods implementation
    IRendererBuffer* CreateBuffer();
    IRendererTexture* CreateTexture();
    IRenderTarget* CreateRenderTarget();
    IShadowMap* CreateShadowMap();
};

} // namespace Render
} // namespace NFE
