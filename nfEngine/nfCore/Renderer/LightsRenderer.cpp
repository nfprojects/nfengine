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

void LightsRenderer::Enter(NFE_CONTEXT_ARG)
{
}

void LightsRenderer::Leave(NFE_CONTEXT_ARG)
{
}

void LightsRenderer::SetUp(NFE_CONTEXT_ARG, IRenderTarget* pRT, const CameraRenderDesc* pCamera,
                           const Vector& ambientLightColor, const Vector& backgroundColor)
{
}

void LightsRenderer::DrawAmbientLight(NFE_CONTEXT_ARG, const Vector& ambientLightColor,
                                      const Vector& backgroundColor)
{
}

void LightsRenderer::TileBasedPass(NFE_CONTEXT_ARG, uint32 lightsCount,
                                   const TileOmniLightDesc* pLights)
{
}

void LightsRenderer::DrawOmniLight(NFE_CONTEXT_ARG, const Vector& pos, float radius,
                                   const Vector& color, ShadowMap* pShadowMap)
{
}

void LightsRenderer::DrawSpotLight(NFE_CONTEXT_ARG, const SpotLightProperties& prop,
                                   const Frustum& frustum, ShadowMap* pShadowMap,
                                   ITexture* pLightMap)
{
}

void LightsRenderer::DrawDirLight(NFE_CONTEXT_ARG, const DirLightProperties& prop,
                                  ShadowMap* pShadowMap)
{
}

void LightsRenderer::DrawFog(NFE_CONTEXT_ARG)
{
}

} // namespace Renderer
} // namespace NFE
