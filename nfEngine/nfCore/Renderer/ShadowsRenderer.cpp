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

void ShadowRenderer::Enter(NFE_CONTEXT_ARG)
{
}

void ShadowRenderer::Leave(NFE_CONTEXT_ARG)
{
}

void ShadowRenderer::SetDestination(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera,
                                    ShadowMap* pShadowMap, uint32 faceID)
{
}

void ShadowRenderer::SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial)
{
}

void ShadowRenderer::Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer)
{
}

} // namespace Renderer
} // namespace NFE
