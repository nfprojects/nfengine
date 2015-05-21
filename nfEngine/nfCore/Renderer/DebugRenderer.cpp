/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Debug Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "DebugRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<DebugRenderer> DebugRenderer::mPtr;

void DebugRenderer::SetTarget(RenderContext *context, IRenderTarget* target)
{
}

void DebugRenderer::SetCamera(RenderContext *context, const Matrix& viewMatrix, const Matrix& projMatrix)
{
}

void DebugRenderer::DrawLine(RenderContext *context, const Vector& A, const Vector& B, const uint32 color)
{
}

void DebugRenderer::DrawLine(RenderContext *context, const Float3& A, const Float3& B, const uint32 color)
{
}

void DebugRenderer::DrawBox(RenderContext *context, const Box& box, const uint32 color)
{
}

void DebugRenderer::DrawFilledBox(RenderContext *context, const Box& box, const uint32 color)
{
}

void DebugRenderer::DrawFrustum(RenderContext *context, const Frustum& frustum, const uint32 color)
{
}

void DebugRenderer::DrawQuad(RenderContext *context, const Vector& pos, ITexture* texture, const uint32 color)
{
}

} // namespace Renderer
} // namespace NFE
