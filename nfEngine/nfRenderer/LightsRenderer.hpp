/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "LightsRendererContext.hpp"
#include "MultiPipelineState.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

class GeometryBuffer;
class ShadowMap;

struct NFE_ALIGN(16) DirLightProperties
{
    Math::Vector4 direction;
    Math::Vector4 color;
    uint32 cascadesCount[4];

    Math::Vector4 splitDistance[8];
    Math::Matrix4 viewProjMatrix[8];
};

struct NFE_ALIGN(16) SpotLightProperties
{
    Math::Vector4 position;
    Math::Vector4 direction;
    Math::Vector4 color;
    Math::Vector4 farDist;
    Math::Matrix4 viewProjMatrix;
    Math::Matrix4 viewProjMatrixInv;
    Math::Vector4 shadowMapProps;
};

class LightsRenderer : public RendererModule<LightsRenderer, LightsRendererContext>
{
    BufferPtr mGlobalCBuffer;
    SamplerPtr mShadowMapSampler;

    BufferPtr mVertexBuffer;
    BufferPtr mIndexBuffer;
    VertexLayoutPtr mVertexLayout;

    ResourceBindingSetPtr mGBufferBindingSet;
    ResourceBindingSetPtr mShadowMapBindingSet;
    ResourceBindingSetPtr mLightMapBindingSet;
    ResourceBindingLayoutPtr mResBindingLayout;

    MultiPipelineState mAmbientLightPipelineState;
    BufferPtr mAmbientLightCBuffer;

    int mOmniLightUseShadowMap;
    MultiPipelineState mOmniLightPipelineState;
    BufferPtr mOmniLightCBuffer;

    int mSpotLightUseLightMap;
    int mSpotLightUseShadowMap;
    MultiPipelineState mSpotLightPipelineState;
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

    void SetUp(LightsRendererContext* context, const RenderTargetPtr& target, GeometryBuffer *gbuffer, const CameraRenderDesc* camera);
    void DrawAmbientLight(LightsRendererContext* context, const Vector4& ambientLightColor, const Vector4& backgroundColor);
    void DrawOmniLight(LightsRendererContext* context, const Vector4& pos, float radius, const Vector4& color, ShadowMap* shadowMap);
    void DrawSpotLight(LightsRendererContext* context, const SpotLightProperties& prop, ShadowMap* shadowMap, const ResourceBindingInstancePtr& pLightMap);
    void DrawDirLight(LightsRendererContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(LightsRendererContext* context);
};

} // namespace Renderer
} // namespace NFE
