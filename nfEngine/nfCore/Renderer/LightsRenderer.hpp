/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RendererResources.hpp"
#include "Multishader.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

class LightsRenderer : public RendererModule<LightsRenderer>
{
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<IDepthState> mLightsDepthState;
    std::unique_ptr<IRasterizerState> mLightsRasterizerState;
    std::unique_ptr<IBlendState> mLightsBlendState;
    std::unique_ptr<ISampler> mShadowMapSampler;

    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;

    Multishader mFullscreenQuadVS;
    std::unique_ptr<IVertexLayout> mVertexLayout;

    Multishader mAmbientLightPS;
    std::unique_ptr<IBuffer> mAmbientLightCBuffer;

    Multishader mOmniLightVS;
    Multishader mOmniLightPS;
    std::unique_ptr<IBuffer> mOmniLightCBuffer;

    int mUseLightMapMacroId;
    int mUseShadowMapMacroId;
    Multishader mSpotLightVS;
    Multishader mSpotLightPS;
    std::unique_ptr<IBuffer> mSpotLightCBuffer;

public:
    LightsRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    void SetUp(RenderContext* context, IRenderTarget* target, GeometryBuffer *gbuffer,
               const CameraRenderDesc* camera);
    void DrawAmbientLight(RenderContext* context, const Vector& ambientLightColor,
                          const Vector& backgroundColor);
    void TileBasedPass(RenderContext* context, uint32 lightsCount, const TileOmniLightDesc* lights);
    void DrawOmniLight(RenderContext* context, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* shadowMap);
    void DrawSpotLight(RenderContext* context, const SpotLightProperties& prop,
                       ShadowMap* shadowMap, ITexture* pLightMap);
    void DrawDirLight(RenderContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(RenderContext* context);
};

} // namespace Renderer
} // namespace NFE
