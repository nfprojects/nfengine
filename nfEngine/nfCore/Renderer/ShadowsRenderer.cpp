/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Shadows Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "ShadowsRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<ShadowRenderer> ShadowRenderer::mPtr;


void ShadowRenderer::SetDestination(RenderContext* context, const CameraRenderDesc* camera,
                                    ShadowMap* shadowMap, uint32 faceID)
{
}

void ShadowRenderer::SetMaterial(RenderContext* context, const RendererMaterial* material)
{
}

void ShadowRenderer::Draw(RenderContext* context, const RenderCommandBuffer& buffer)
{
}

} // namespace Renderer
} // namespace NFE
