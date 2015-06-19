/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "Renderer.hpp"
#include "View.hpp"

namespace NFE {
namespace Renderer {

HighLevelRenderer::HighLevelRenderer()
    : mRenderingDevice(nullptr)
    , mCommandBuffer(nullptr)
{
}

HighLevelRenderer::~HighLevelRenderer()
{
    Release();
}

IDevice* HighLevelRenderer::GetDevice()
{
    return mRenderingDevice;
}

bool HighLevelRenderer::Init(const std::string& preferredRendererName)
{
    if (!mLowLevelRendererLib.Open(preferredRendererName))
        return false;

    auto proc = static_cast<RendererInitFunc>(mLowLevelRendererLib.GetSymbol(RENDERER_INIT_FUNC));
    if (proc == NULL)
        return false;

    mRenderingDevice = proc();
    if (mRenderingDevice == nullptr)
        return false;

    mCommandBuffer = mRenderingDevice->GetDefaultCommandBuffer();

    mImmediateContext.reset(new RenderContext());
    mGuiRenderer.reset(new GuiRenderer());
    mPostProcessRenderer.reset(new PostProcessRenderer());
    mDebugRenderer.reset(new DebugRenderer());
    mShadowRenderer.reset(new ShadowRenderer());
    mGBufferRenderer.reset(new GBufferRenderer());
    mLightsRenderer.reset(new LightsRenderer());

    return true;
}

void HighLevelRenderer::Release()
{
    if (mRenderingDevice != nullptr)
    {
        mRenderingDevice = nullptr;
        auto proc = static_cast<RendererReleaseFunc>(mLowLevelRendererLib.GetSymbol("Release"));
        if (proc == NULL)
            return;
        proc();
    }

    mLowLevelRendererLib.Close();
}

int HighLevelRenderer::InitModules()
{
    return 0;
}

void HighLevelRenderer::ReleaseModules()
{
}

void HighLevelRenderer::ProcessView(View* view)
{
    // TODO: set viewport
    mCommandBuffer->SetRenderTarget(view->GetRenderTarget());
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
