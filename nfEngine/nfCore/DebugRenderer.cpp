/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Debug Renderer
 */

#pragma once

#include "PCH.hpp"
#include "DebugRenderer.hpp"

namespace NFE {
namespace Renderer {

DebugRenderer::DebugRenderer()
{

}

DebugRenderer::~DebugRenderer()
{

}

void DebugRenderer::Release()
{

}

void DebugRenderer::Enter(NFE_CONTEXT_ARG)
{

}

void DebugRenderer::Leave(NFE_CONTEXT_ARG)
{

}

void DebugRenderer::SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget)
{

}

void DebugRenderer::SetCamera(NFE_CONTEXT_ARG, const Matrix& viewMatrix, const Matrix& projMatrix)
{

}

void DebugRenderer::DrawLine(NFE_CONTEXT_ARG, const Vector& A, const Vector& B, const UINT color)
{

}

void DebugRenderer::DrawLine(NFE_CONTEXT_ARG, const Float3& A, const Float3& B, const UINT color)
{

}

void DebugRenderer::DrawBox(NFE_CONTEXT_ARG, const Box& box, const UINT color)
{

}

void DebugRenderer::DrawFilledBox(NFE_CONTEXT_ARG, const Box& box, const UINT color)
{

}

void DebugRenderer::DrawFrustum(NFE_CONTEXT_ARG, const Frustum& frustum, const UINT color)
{

}

void DebugRenderer::DrawQuad(NFE_CONTEXT_ARG, const Vector& pos, ITexture* pTexture, const UINT color)
{

}

} // namespace Renderer
} // namespace NFE
