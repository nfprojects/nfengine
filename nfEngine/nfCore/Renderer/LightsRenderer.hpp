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
    BufferPtr mGlobalCBuffer;
    std::unique_ptr<ISampler> mShadowMapSampler;

    BufferPtr mVertexBuffer;
    BufferPtr mIndexBuffer;
    VertexLayoutPtr mVertexLayout;

    ResourceBindingSetPtr mGBufferBindingSet;
    ResourceBindingSetPtr mShadowMapBindingSet;
    ResourceBindingSetPtr mLightMapBindingSet;
    ResourceBindingLayoutPtr mResBindingLayout;

    Resource::MultiPipelineState mAmbientLightPipelineState;
    BufferPtr mAmbientLightCBuffer;

    int mOmniLightUseShadowMap;
    Resource::MultiPipelineState mOmniLightPipelineState;
    BufferPtr mOmniLightCBuffer;

    int mSpotLightUseLightMap;
    int mSpotLightUseShadowMap;
    Resource::MultiPipelineState mSpotLightPipelineState;
    BufferPtr mSpotLightCBuffer;

    bool CreateResourceBindingLayouts();

public:
    LightsRenderer();

    NFE_INLINE ResourceBindingSetPtr& GetGBufferBindingSet()
    {
        return mGBufferBindingSet;
    }

    NFE_INLINE ResourceBindingSetPtr& GetShadowMapBindingSet()
    {
        return mShadowMapBindingSet;
    }

    NFE_INLINE ResourceBindingSetPtr& GetLightMapBindingSet()
    {
        return mLightMapBindingSet;
    }

    void OnEnter(LightsRendererContext* context);
    void OnLeave(LightsRendererContext* context);

    void SetUp(LightsRendererContext* context, RenderTargetPtr target, GeometryBuffer *gbuffer,
               const CameraRenderDesc* camera);
    void DrawAmbientLight(LightsRendererContext* context, const Vector& ambientLightColor,
                          const Vector& backgroundColor);
    void DrawOmniLight(LightsRendererContext* context, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* shadowMap);
    void DrawSpotLight(LightsRendererContext* context, const SpotLightProperties& prop,
                       ShadowMap* shadowMap, ResourceBindingInstancePtr pLightMap);
    void DrawDirLight(LightsRendererContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(LightsRendererContext* context);
};

} // namespace Renderer
} // namespace NFE
