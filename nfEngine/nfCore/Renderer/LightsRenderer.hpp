/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
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

class LightsRenderer : public RendererModule<LightsRenderer>
{
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<ISampler> mShadowMapSampler;

    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;

    std::unique_ptr<IResourceBindingSet> mGlobalBindingSet;
    std::unique_ptr<IResourceBindingSet> mGBufferBindingSet;
    std::unique_ptr<IResourceBindingSet> mShadowMapBindingSet;
    std::unique_ptr<IResourceBindingSet> mLightMapBindingSet;
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;

    Resource::MultiPipelineState mAmbientLightPipelineState;
    std::unique_ptr<IBuffer> mAmbientLightCBuffer;
    std::unique_ptr<IResourceBindingInstance> mAmbientLightBindingInstance;

    int mOmniLightUseShadowMap;
    Resource::MultiPipelineState mOmniLightPipelineState;
    std::unique_ptr<IBuffer> mOmniLightCBuffer;
    std::unique_ptr<IResourceBindingInstance> mOmniLightBindingInstance;

    int mSpotLightUseLightMap;
    int mSpotLightUseShadowMap;
    Resource::MultiPipelineState mSpotLightPipelineState;
    std::unique_ptr<IBuffer> mSpotLightCBuffer;
    std::unique_ptr<IResourceBindingInstance> mSpotLightBindingInstance;

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

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    void SetUp(RenderContext* context, IRenderTarget* target, GeometryBuffer *gbuffer,
               const CameraRenderDesc* camera);
    void DrawAmbientLight(RenderContext* context, const Vector& ambientLightColor,
                          const Vector& backgroundColor);
    void DrawOmniLight(RenderContext* context, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* shadowMap);
    void DrawSpotLight(RenderContext* context, const SpotLightProperties& prop,
                       ShadowMap* shadowMap, IResourceBindingInstance* pLightMap);
    void DrawDirLight(RenderContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(RenderContext* context);
};

} // namespace Renderer
} // namespace NFE
