#pragma once
#include "nfRendererD3D11.hpp"
#include "Multishader.hpp"
#include "../nfCore/Renderer.hpp"
#include "RendererResources.hpp"

namespace NFE {
namespace Render {

using namespace Math;

struct LightGlobalCBuffer
{
    Matrix cameraMatrix;
    Matrix viewMatrix;
    Matrix projMatrix;
    Matrix projInverseMatrix;
    Matrix viewProjMatrix;
    Matrix viewProjInverseMatrix;
    Vector viewportResInv;
    Vector screenScale;
};

struct AmbientLightCBuffer
{
    Float4 outResolution;
    Float4 ambientLight;
    Float4 backgroundColor;

    Float4 focalLen;
};

class LightsRendererD3D11 : public ILightsRenderer
{
    ID3D11BlendState* mAdditiveBlendState;
    ID3D11BlendState* mFogBlendState;
    ID3D11DepthStencilState* mDepthStencilState;
    ID3D11RasterizerState* mRasterizerState;
    ID3D11InputLayout* mInputLayout;

    ID3D11SamplerState* mShadowSampler;
    ID3D11SamplerState* mLightMapSampler;
    ID3D11SamplerState* mPointSampler;


    ID3D11Buffer* mGlobalCBuffer;
    ID3D11Buffer* mOmniLightCBuffer;
    ID3D11Buffer* mSpotLightCBuffer;
    ID3D11Buffer* mAmbientLightCBuffer;
    ID3D11Buffer* mDirLightCBuffer;

    ID3D11Buffer* mLightTileGlobalCBuffer;
    ID3D11Buffer* mLightTilePerPassCBuffer;
    ID3D11Buffer* mLightsBuffer;
    ID3D11ShaderResourceView* mLightsBufferSRV;

    ID3D11Buffer* mTilesFrustumsBuffer;
    ID3D11ShaderResourceView* mTilesFrustumsBufferSRV;


    ID3D11Buffer* mIcosahedronVB;
    ID3D11Buffer* mLightIB;
    ID3D11Buffer* mSpotLightVB;
    //ID3D11Buffer* mSpotLightVB;

    Multishader mGlobalFogPS;
    Multishader mAmbientLightPS;
    Multishader mAmbientLightCS;
    Multishader mDirLightCS;
    Multishader mSpotLightShaderVS;
    Multishader mSpotLightShaderPS;
    Multishader mOmniLightShaderVS;
    Multishader mOmniLightShaderPS;

    Multishader mLightTileCS;

    void DrawAmbientLight(NFE_CONTEXT_ARG, const Vector& ambientLightColor,
                          const Vector& backgroundColor);

    // TODO: move to per context data
    RenderTargetD3D11* mTarget;

public:
    LightsRendererD3D11();
    ~LightsRendererD3D11();

    Result Init(RendererD3D11* pRenderer);
    void Release();
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetUp(NFE_CONTEXT_ARG, IRenderTarget* pRT, const CameraRenderDesc* pCamera,
               const Vector& ambientLightColor, const Vector& backgroundColor);


    // draws max 1024 omnilights
    void TileBasedPass(NFE_CONTEXT_ARG, uint32 lightsCount, const TileOmniLightDesc* pLights);

    void DrawOmniLight(NFE_CONTEXT_ARG, const Vector& pos, float radius, const Vector& color,
                       IShadowMap* pShadowMap);
    void DrawSpotLight(NFE_CONTEXT_ARG, const SpotLightProperties& prop, const Frustum& frustum,
                       IShadowMap* pShadowMap, IRendererTexture* pLightMap);
    void DrawDirLight(NFE_CONTEXT_ARG, const DirLightProperties& prop, IShadowMap* pShadowMap);

    void DrawFog(NFE_CONTEXT_ARG);
};

} // namespace Render
} // namespace NFE
