/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Lights Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "LightsRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<LightsRenderer> LightsRenderer::mPtr;


void LightsRenderer::SetUp(RenderContext *pContext, IRenderTarget* pRT, const CameraRenderDesc* pCamera,
                           const Vector& ambientLightColor, const Vector& backgroundColor)
{
}

void LightsRenderer::DrawAmbientLight(RenderContext *pContext, const Vector& ambientLightColor,
                                      const Vector& backgroundColor)
{
}

void LightsRenderer::TileBasedPass(RenderContext *pContext, uint32 lightsCount,
                                   const TileOmniLightDesc* pLights)
{
}

void LightsRenderer::DrawOmniLight(RenderContext *pContext, const Vector& pos, float radius,
                                   const Vector& color, ShadowMap* pShadowMap)
{
}

void LightsRenderer::DrawSpotLight(RenderContext *pContext, const SpotLightProperties& prop,
                                   const Frustum& frustum, ShadowMap* pShadowMap,
                                   ITexture* pLightMap)
{
}

void LightsRenderer::DrawDirLight(RenderContext *pContext, const DirLightProperties& prop,
                                  ShadowMap* pShadowMap)
{
}

void LightsRenderer::DrawFog(RenderContext *pContext)
{
}

} // namespace Renderer
} // namespace NFE
