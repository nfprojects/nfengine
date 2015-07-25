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

struct NFE_ALIGN16 LightGlobalCBuffer
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
    Float4 ambientLight;
    Float4 backgroundColor;
};

class LightsRenderer : public RendererModule<LightsRenderer>
{
    Multishader mFullscreenQuadVS;
    std::unique_ptr<IBuffer> mFullscreenQuadVB;
    std::unique_ptr<IVertexLayout> mVertexLayout;

    Multishader mAmbientLightPS;
    std::unique_ptr<IBuffer> mAmbientLightCBuffer;

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
    void DrawSpotLight(RenderContext* context, const SpotLightProperties& prop, const Frustum& frustum,
                       ShadowMap* shadowMap, ITexture* pLightMap);
    void DrawDirLight(RenderContext* context, const DirLightProperties& prop, ShadowMap* shadowMap);

    void DrawFog(RenderContext* context);
};

} // namespace Renderer
} // namespace NFE
