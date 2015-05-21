/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Lights Renderer
 */

#pragma once

#include "Renderer.hpp"
#include "Multishader.hpp"
#include "RendererResources.hpp"

namespace NFE {
namespace Renderer {

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

class LightsRenderer
{
    void DrawAmbientLight(NFE_CONTEXT_ARG, const Vector& ambientLightColor,
                          const Vector& backgroundColor);

public:
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetUp(NFE_CONTEXT_ARG, IRenderTarget* pRT, const CameraRenderDesc* pCamera,
               const Vector& ambientLightColor, const Vector& backgroundColor);

    // draws max 1024 omnilights
    void TileBasedPass(NFE_CONTEXT_ARG, uint32 lightsCount, const TileOmniLightDesc* pLights);

    void DrawOmniLight(NFE_CONTEXT_ARG, const Vector& pos, float radius, const Vector& color,
                       ShadowMap* pShadowMap);
    void DrawSpotLight(NFE_CONTEXT_ARG, const SpotLightProperties& prop, const Frustum& frustum,
                       ShadowMap* pShadowMap, ITexture* pLightMap);
    void DrawDirLight(NFE_CONTEXT_ARG, const DirLightProperties& prop, ShadowMap* pShadowMap);

    void DrawFog(NFE_CONTEXT_ARG);
};

} // namespace Renderer
} // namespace NFE
