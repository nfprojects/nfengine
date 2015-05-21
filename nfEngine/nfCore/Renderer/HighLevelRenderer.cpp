/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "HighLevelRenderer.hpp"

#include "GuiRenderer.hpp"
#include "DebugRenderer.hpp"
#include "GeometryBufferRenderer.hpp"
#include "LightsRenderer.hpp"
#include "ShadowsRenderer.hpp"
#include "PostProcessRenderer.hpp"

namespace NFE {
namespace Renderer {

void* HighLevelRenderer::GetDevice() const
{
    return nullptr;
}

int HighLevelRenderer::Init()
{
    mImmediateContext.reset(new RenderContext());

    // TODO: multithreaded modules initialization

    GuiRenderer::Init();
    PostProcessRenderer::Init();
    DebugRenderer::Init();
    ShadowRenderer::Init();
    GBufferRenderer::Init();
    LightsRenderer::Init();

    return 0;
}

void HighLevelRenderer::Begin()
{
}

void HighLevelRenderer::SwapBuffers(IRenderTarget* pRenderTarget, ViewSettings* pViewSettings, float dt)
{
}

void HighLevelRenderer::ExecuteDeferredContext(RenderContext* pContext)
{
}

RenderContext* HighLevelRenderer::GetImmediateContext() const
{
    return mImmediateContext.get();
}

RenderContext* HighLevelRenderer::CreateDeferredContext()
{
    return new RenderContext();
}

} // namespace Renderer
} // namespace NFE
