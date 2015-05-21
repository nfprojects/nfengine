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

void DebugRenderer::SetTarget(RenderContext *pContext, IRenderTarget* pTarget)
{
}

void DebugRenderer::SetCamera(RenderContext *pContext, const Matrix& viewMatrix, const Matrix& projMatrix)
{
}

void DebugRenderer::DrawLine(RenderContext *pContext, const Vector& A, const Vector& B, const uint32 color)
{
}

void DebugRenderer::DrawLine(RenderContext *pContext, const Float3& A, const Float3& B, const uint32 color)
{
}

void DebugRenderer::DrawBox(RenderContext *pContext, const Box& box, const uint32 color)
{
}

void DebugRenderer::DrawFilledBox(RenderContext *pContext, const Box& box, const uint32 color)
{
}

void DebugRenderer::DrawFrustum(RenderContext *pContext, const Frustum& frustum, const uint32 color)
{
}

void DebugRenderer::DrawQuad(RenderContext *pContext, const Vector& pos, ITexture* pTexture, const uint32 color)
{
}

} // namespace Renderer
} // namespace NFE
