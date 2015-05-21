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


void LightsRenderer::SetUp(RenderContext* context, IRenderTarget* target,
                           const CameraRenderDesc* pCamera, const Vector& ambientLightColor,
                           const Vector& backgroundColor)
{
}

void LightsRenderer::DrawAmbientLight(RenderContext* context, const Vector& ambientLightColor,
                                      const Vector& backgroundColor)
{
}

void LightsRenderer::TileBasedPass(RenderContext* context, uint32 lightsCount,
                                   const TileOmniLightDesc* lights)
{
}

void LightsRenderer::DrawOmniLight(RenderContext* context, const Vector& pos, float radius,
                                   const Vector& color, ShadowMap* shadowMap)
{
}

void LightsRenderer::DrawSpotLight(RenderContext* context, const SpotLightProperties& prop,
                                   const Frustum& frustum, ShadowMap* shadowMap,
                                   ITexture* lightMap)
{
}

void LightsRenderer::DrawDirLight(RenderContext* context, const DirLightProperties& prop,
                                  ShadowMap* shadowMap)
{
}

void LightsRenderer::DrawFog(RenderContext* context)
{
}

} // namespace Renderer
} // namespace NFE
