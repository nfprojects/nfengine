/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "HighLevelRenderer.hpp"
#include "View.hpp"
#include "../Globals.hpp"
#include "../../nfCommon/ThreadPool.hpp"

#include "GuiRenderer.hpp"
#include "DebugRenderer.hpp"
#include "GeometryBufferRenderer.hpp"
#include "LightsRenderer.hpp"
#include "ShadowsRenderer.hpp"
#include "PostProcessRenderer.hpp"

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

    RendererInitFunc initFunc;
    if (!mLowLevelRendererLib.GetSymbol(RENDERER_INIT_FUNC, initFunc))
        return false;

    mRenderingDevice = initFunc();
    if (mRenderingDevice == nullptr)
        return false;

    mCommandBuffer = mRenderingDevice->GetDefaultCommandBuffer();

    /// create immediate and deferred rendering contexts
    mImmediateContext.reset(new RenderContext(mCommandBuffer));
    mDeferredContexts.reset(new RenderContext[g_pMainThreadPool->GetThreadsNumber()]);

    // TODO: multithreaded modules initialization

    GuiRenderer::Init();
    PostProcessRenderer::Init();
    DebugRenderer::Init();
    ShadowRenderer::Init();
    GBufferRenderer::Init();
    LightsRenderer::Init();

    CreateCommonResources();

    return true;
}

void HighLevelRenderer::Release()
{
    GuiRenderer::Release();
    PostProcessRenderer::Release();
    DebugRenderer::Release();
    ShadowRenderer::Release();
    GBufferRenderer::Release();
    LightsRenderer::Release();

    mDefaultSampler.reset();
    mDefaultDepthState.reset();

    if (mRenderingDevice != nullptr)
    {
        mRenderingDevice = nullptr;

        RendererReleaseFunc releaseFunc;
        if (mLowLevelRendererLib.GetSymbol("Release", releaseFunc))
            releaseFunc();
    }

    mLowLevelRendererLib.Close();
}

void HighLevelRenderer::CreateCommonResources()
{
    SamplerDesc samplerDesc;
    samplerDesc.magFilter = TextureMagFilter::Linear;
    samplerDesc.minFilter = TextureMinFilter::LinearMipmapLinear;
    samplerDesc.debugName = "HighLevelRenderer::mDefaultSampler";
    mDefaultSampler.reset(mRenderingDevice->CreateSampler(samplerDesc));

    DepthStateDesc dsDesc;
    dsDesc.depthCompareFunc = CompareFunc::LessEqual;
    dsDesc.depthTestEnable = true;
    dsDesc.depthWriteEnable = true;
    dsDesc.debugName = "HighLevelRenderer::mDefaultDepthState";
    mDefaultDepthState.reset(mRenderingDevice->CreateDepthState(dsDesc));
}

void HighLevelRenderer::ProcessView(View* view)
{

}

RenderContext* HighLevelRenderer::GetImmediateContext() const
{
    return mImmediateContext.get();
}

RenderContext* HighLevelRenderer::GetDeferredContext(size_t id) const
{
    return mDeferredContexts.get() + id;
}

void HighLevelRenderer::ExecuteDeferredContext(RenderContext* pContext)
{
    // TODO
}

std::string HighLevelRenderer::GetShadersPath() const
{
    // TODO: It's temporary. Shader location must be dependent on low-level renderer.
    return "nfEngine/Shaders/D3D11/";
}

} // namespace Renderer
} // namespace NFE
