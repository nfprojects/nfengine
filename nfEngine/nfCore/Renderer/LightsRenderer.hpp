/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "../Resources/MultiShaderProgram.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

struct NFE_ALIGN16 DirLightProperties
{
    Math::Vector direction;
    Math::Vector color;
    uint32 cascadesCount[4];

    Math::Vector splitDistance[8];
    Math::Matrix viewProjMatrix[8];
};

struct NFE_ALIGN16 SpotLightProperties
{
    Math::Vector position;
    Math::Vector direction;
    Math::Vector color;
    Math::Vector farDist;
    Math::Matrix viewProjMatrix;
    Math::Matrix viewProjMatrixInv;
    Math::Vector shadowMapProps;
};

class LightsRenderer : public RendererModule<LightsRenderer>
{
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<IDepthState> mLightsDepthState;
    std::unique_ptr<IRasterizerState> mLightsRasterizerState;
    std::unique_ptr<IBlendState> mLightsBlendState;
    std::unique_ptr<ISampler> mShadowMapSampler;

    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;

    std::unique_ptr<IVertexLayout> mVertexLayout;

    Resource::MultiShaderProgram mAmbientLightShaderProgram;
    std::unique_ptr<IBuffer> mAmbientLightCBuffer;

    int mOmniLightUseShadowMap;
    Resource::MultiShaderProgram mOmniLightShaderProgram;
    std::unique_ptr<IBuffer> mOmniLightCBuffer;

    int mSpotLightUseLightMap;
    int mSpotLightUseShadowMap;
    Resource::MultiShaderProgram mSpotLightShaderProgram;
    std::unique_ptr<IBuffer> mSpotLightCBuffer;

public:
    LightsRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    void SetUp(RenderContext* context, IRenderTarget* target, GeometryBuffer *gbuffer,
               const CameraRenderDesc* camera);
    void DrawAmbientLight(RenderContext* context, const Vector& ambientLightColor,
                          const Vector& backgroundColor);
    void DrawOmniLight(RenderContext* context, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* shadowMap);
    void DrawSpotLight(RenderContext* context, const SpotLightProperties& prop,
                       ShadowMap* shadowMap, ITexture* pLightMap);
    void DrawDirLight(RenderContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(RenderContext* context);
};

} // namespace Renderer
} // namespace NFE
