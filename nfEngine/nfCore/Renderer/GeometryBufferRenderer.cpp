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


void GBufferRenderer::SetTarget(RenderContext *context, IRenderTarget* target)
{
}

void GBufferRenderer::SetCamera(RenderContext *context, const CameraRenderDesc* camera)
{
}

void GBufferRenderer::SetMaterial(RenderContext *context, const RendererMaterial* material)
{
}

void GBufferRenderer::Draw(RenderContext *context, const RenderCommandBuffer& buffer)
{
}

} // namespace Renderer
} // namespace NFE
