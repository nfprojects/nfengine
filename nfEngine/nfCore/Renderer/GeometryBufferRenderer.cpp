/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Geometry Buffer Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "GeometryBufferRenderer.hpp"

namespace NFE {
namespace Renderer {

void GBufferRenderer::Enter(NFE_CONTEXT_ARG)
{
}

void GBufferRenderer::Leave(NFE_CONTEXT_ARG)
{
}

void GBufferRenderer::SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget)
{
}

void GBufferRenderer::SetCamera(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera)
{
}

void GBufferRenderer::SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial)
{
}

void GBufferRenderer::Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer)
{
}

} // namespace Renderer
} // namespace NFE
