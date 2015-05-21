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


void ShadowRenderer::SetDestination(RenderContext *pContext, const CameraRenderDesc* pCamera,
                                    ShadowMap* pShadowMap, uint32 faceID)
{
}

void ShadowRenderer::SetMaterial(RenderContext *pContext, const RendererMaterial* pMaterial)
{
}

void ShadowRenderer::Draw(RenderContext *pContext, const RenderCommandBuffer& buffer)
{
}

} // namespace Renderer
} // namespace NFE
