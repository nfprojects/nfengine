/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RendererResources.hpp"

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
    Float4 outResolution;
    Float4 ambientLight;
    Float4 backgroundColor;

    Float4 focalLen;
};

class LightsRenderer : public RendererModule<LightsRenderer>
{
    void DrawAmbientLight(RenderContext *pContext, const Vector& ambientLightColor,
                          const Vector& backgroundColor);

public:
    void SetUp(RenderContext *pContext, IRenderTarget* pRT, const CameraRenderDesc* pCamera,
               const Vector& ambientLightColor, const Vector& backgroundColor);

    // draws max 1024 omnilights
    void TileBasedPass(RenderContext *pContext, uint32 lightsCount, const TileOmniLightDesc* pLights);

    void DrawOmniLight(RenderContext *pContext, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* pShadowMap);
    void DrawSpotLight(RenderContext *pContext, const SpotLightProperties& prop, const Frustum& frustum,
                       ShadowMap* pShadowMap, ITexture* pLightMap);
    void DrawDirLight(RenderContext *pContext, const DirLightProperties& prop, ShadowMap* pShadowMap);

    void DrawFog(RenderContext *pContext);
};

} // namespace Renderer
} // namespace NFE
