/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "PCH.hpp"
#include "nfRenderer.hpp"
#include "HighLevelRenderer.hpp"
#include "View.hpp"
#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Logger/Logger.hpp"

#include "GuiRenderer.hpp"
#include "DebugRenderer.hpp"
#include "GeometryRenderer.hpp"
#include "LightsRenderer.hpp"
#include "PostProcessRenderer.hpp"

namespace NFE {

ConfigVariable<const char*> gRendererBackendName("renderer/backend/name", "nfRendererD3D11");
ConfigVariable<int> gRendererPreferredCard("renderer/backend/card", -1);
ConfigVariable<int> gRendererDebugLevel("renderer/backend/debugLevel", 0);

namespace Renderer {

RendererConfig::RendererConfig()
    : gamma(2.0f)
{}

HighLevelRenderer::HighLevelRenderer()
    : mRenderingDevice(nullptr)
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

bool HighLevelRenderer::Init()
{
    if (!mLowLevelRendererLib.Open(gRendererBackendName.Get()))
        return false;

    RendererInitFunc initFunc;
    if (!mLowLevelRendererLib.GetSymbol(RENDERER_INIT_FUNC, initFunc))
        return false;

    // TODO read this parameters from config
    DeviceInitParams params;
    params.preferredCardId = gRendererPreferredCard.Get();
#ifdef  _DEBUG
    params.debugLevel = 1;
#else
    params.debugLevel = gRendererDebugLevel.Get();
#endif //  _DEBUG

    mRenderingDevice = initFunc(&params);
    if (mRenderingDevice == nullptr)
        return false;

    // find suitable back buffer format
    ElementFormat preferredFormats[] =
    {
        ElementFormat::R16G16B16A16_U_Norm,
        ElementFormat::R10G10B10A2_U_Norm,
        ElementFormat::R8G8B8A8_U_Norm,
        ElementFormat::B8G8R8A8_U_Norm,
        ElementFormat::R8G8B8A8_U_Norm_sRGB,
        ElementFormat::B8G8R8A8_U_Norm_sRGB,
    };

    mBackbufferFormat = ElementFormat::Unknown;
    for (size_t i = 0; i < ArraySize(preferredFormats); ++i)
    {
        if (mRenderingDevice->IsBackbufferFormatSupported(preferredFormats[i]))
        {
            mBackbufferFormat = preferredFormats[i];
            break;
        }
    }

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    const size_t numThreads = threadPool->GetThreadsNumber();
    mGeometryCLs.resize(numThreads);
    mShadowsCLs.resize(numThreads);
    mLightsCLs.resize(numThreads);
    mDebugCLs.resize(numThreads);

    /// create default and deferred rendering contexts
    mDeferredContexts.reset(new RenderContext[threadPool->GetThreadsNumber()]);

    // TODO: multithreaded modules initialization

    CreateCommonResources();

    GuiRenderer::Init();
    PostProcessRenderer::Init();
    DebugRenderer::Init();
    GeometryRenderer::Init();
    LightsRenderer::Init();

    return true;
}

void HighLevelRenderer::ReleaseModules()
{
    GuiRenderer::Release();
    PostProcessRenderer::Release();
    DebugRenderer::Release();
    GeometryRenderer::Release();
    LightsRenderer::Release();
}

void HighLevelRenderer::Release()
{
    mDefaultSampler.Reset();
    mDefaultDiffuseTexture.Reset();
    mDefaultNormalTexture.Reset();
    mDefaultSpecularTexture.Reset();

    mDeferredContexts.reset();

    if (mRenderingDevice != nullptr)
    {
        mRenderingDevice = nullptr;

        RendererReleaseFunc releaseFunc;
        if (mLowLevelRendererLib.GetSymbol(RENDERER_RELEASE_FUNC, releaseFunc))
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
    mDefaultSampler = mRenderingDevice->CreateSampler(samplerDesc);

    TextureDataDesc texDataDesc;
    texDataDesc.lineSize = texDataDesc.sliceSize = 4 * sizeof(uint8);
    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.mode = BufferMode::Static;
    texDesc.width = texDesc.height = 1;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.mipmaps = 1;
    texDesc.dataDesc = &texDataDesc;
    texDesc.format = ElementFormat::R8G8B8A8_U_Norm;

    /// default textures (used when no texture is bound to a material)

    uint8 diffuseColor[] = { 255, 255, 255, 255 };
    texDataDesc.data = diffuseColor;
    texDesc.debugName = "DefaultDiffuseTexture";
    mDefaultDiffuseTexture = mRenderingDevice->CreateTexture(texDesc);

    uint8 normalColor[] = { 127, 127, 255, 255 };
    texDataDesc.data = normalColor;
    texDesc.debugName = "DefaultNormalTexture";
    mDefaultNormalTexture = mRenderingDevice->CreateTexture(texDesc);

    // TODO: this should be configurable
    uint8 specularColor[] = { 25, 25, 25, 255 };
    texDataDesc.data = specularColor;
    texDesc.debugName = "DefaultSpecularTexture";
    mDefaultSpecularTexture = mRenderingDevice->CreateTexture(texDesc);
}

RenderContext* HighLevelRenderer::GetDeferredContext(size_t id) const
{
    return mDeferredContexts.get() + id;
}

std::string HighLevelRenderer::GetShadersPath() const
{
    // TODO: It's temporary. Shader location must be dependent on low-level renderer.
    return "nfEngine/Shaders/HLSL5/";
}

void HighLevelRenderer::ResetCommandBuffers()
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    // TODO: this is temporary solution to make D3D12 work
    // reset command buffers
    for (size_t i = 0; i < threadPool->GetThreadsNumber(); ++i)
    {
        RenderContext* ctx = GetDeferredContext(i);

        ctx->commandRecorderShadows->Begin();
        ctx->commandRecorderGeometry->Begin();
        ctx->commandRecorderLights->Begin();
        ctx->commandRecorderDebug->Begin();
    }
}

void HighLevelRenderer::FinishAndExecuteCommandBuffers()
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    // TODO: this is temporary solution to make D3D12 work

    for (size_t i = 0; i < threadPool->GetThreadsNumber(); ++i)
    {
        RenderContext* ctx = GetDeferredContext(i);
        mRenderingDevice->Execute(ctx->commandRecorderShadows->Finish());
    }

    for (size_t i = 0; i < threadPool->GetThreadsNumber(); ++i)
    {
        RenderContext* ctx = GetDeferredContext(i);
        mRenderingDevice->Execute(ctx->commandRecorderGeometry->Finish());
    }

    for (size_t i = 0; i < threadPool->GetThreadsNumber(); ++i)
    {
        RenderContext* ctx = GetDeferredContext(i);
        mRenderingDevice->Execute(ctx->commandRecorderLights->Finish());
    }

    for (size_t i = 0; i < threadPool->GetThreadsNumber(); ++i)
    {
        RenderContext* ctx = GetDeferredContext(i);
        mRenderingDevice->Execute(ctx->commandRecorderDebug->Finish());
    }
}

} // namespace Renderer
} // namespace NFE
