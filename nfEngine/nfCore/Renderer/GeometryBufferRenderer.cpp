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

// renderer modules instance definition
std::unique_ptr<GBufferRenderer> GBufferRenderer::mPtr;


void GBufferRenderer::SetTarget(RenderContext *pContext, IRenderTarget* pTarget)
{
}

void GBufferRenderer::SetCamera(RenderContext *pContext, const CameraRenderDesc* pCamera)
{
}

void GBufferRenderer::SetMaterial(RenderContext *pContext, const RendererMaterial* pMaterial)
{
}

void GBufferRenderer::Draw(RenderContext *pContext, const RenderCommandBuffer& buffer)
{
}

} // namespace Renderer
} // namespace NFE
