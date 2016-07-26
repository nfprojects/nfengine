/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "LightsRendererContext.hpp"
#include "../Resources/MultiPipelineState.hpp"

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

class LightsRenderer : public RendererModule<LightsRenderer, LightsRendererContext>
{
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<ISampler> mShadowMapSampler;

    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;

    std::unique_ptr<IResourceBindingSet> mGBufferBindingSet;
    std::unique_ptr<IResourceBindingSet> mShadowMapBindingSet;
    std::unique_ptr<IResourceBindingSet> mLightMapBindingSet;
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;

    Resource::MultiPipelineState mAmbientLightPipelineState;
    std::unique_ptr<IBuffer> mAmbientLightCBuffer;

    int mOmniLightUseShadowMap;
    Resource::MultiPipelineState mOmniLightPipelineState;
    std::unique_ptr<IBuffer> mOmniLightCBuffer;

    int mSpotLightUseLightMap;
    int mSpotLightUseShadowMap;
    Resource::MultiPipelineState mSpotLightPipelineState;
    std::unique_ptr<IBuffer> mSpotLightCBuffer;

    bool CreateResourceBindingLayouts();

public:
    LightsRenderer();

    NFE_INLINE std::unique_ptr<IResourceBindingSet>& GetGBufferBindingSet()
    {
        return mGBufferBindingSet;
    }

    NFE_INLINE std::unique_ptr<IResourceBindingSet>& GetShadowMapBindingSet()
    {
        return mShadowMapBindingSet;
    }

    NFE_INLINE std::unique_ptr<IResourceBindingSet>& GetLightMapBindingSet()
    {
        return mLightMapBindingSet;
    }

    void OnEnter(LightsRendererContext* context);
    void OnLeave(LightsRendererContext* context);

    void SetUp(LightsRendererContext* context, IRenderTarget* target, GeometryBuffer *gbuffer,
               const CameraRenderDesc* camera);
    void DrawAmbientLight(LightsRendererContext* context, const Vector& ambientLightColor,
                          const Vector& backgroundColor);
    void DrawOmniLight(LightsRendererContext* context, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* shadowMap);
    void DrawSpotLight(LightsRendererContext* context, const SpotLightProperties& prop,
                       ShadowMap* shadowMap, IResourceBindingInstance* pLightMap);
    void DrawDirLight(LightsRendererContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(LightsRendererContext* context);
};

} // namespace Renderer
} // namespace NFE
