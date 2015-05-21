/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Renderer {

void* HighLevelRenderer::GetDevice() const
{
    return nullptr;
}

int HighLevelRenderer::Init()
{
    mImmediateContext.reset(new RenderContext());
    mGuiRenderer.reset(new GuiRenderer());
    mPostProcessRenderer.reset(new PostProcessRenderer());
    mDebugRenderer.reset(new DebugRenderer());
    mShadowRenderer.reset(new ShadowRenderer());
    mGBufferRenderer.reset(new GBufferRenderer());
    mLightsRenderer.reset(new LightsRenderer());

    return 0;
}

int HighLevelRenderer::InitModules()
{
    return 0;
}

void HighLevelRenderer::ReleaseModules()
{
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

GuiRenderer* HighLevelRenderer::GetGuiRenderer() const
{
    return mGuiRenderer.get();
}

PostProcessRenderer* HighLevelRenderer::GetPostProcessRenderer() const
{
    return mPostProcessRenderer.get();
}

DebugRenderer* HighLevelRenderer::GetDebugRenderer() const
{
    return mDebugRenderer.get();
}

ShadowRenderer* HighLevelRenderer::GetShadowRenderer() const
{
    return mShadowRenderer.get();
}

GBufferRenderer* HighLevelRenderer::GetGBufferRenderer() const
{
    return mGBufferRenderer.get();
}

LightsRenderer* HighLevelRenderer::GetLightsRenderer() const
{
    return mLightsRenderer.get();
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
