/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "PCH.hpp"
#include "Core.hpp"
#include "HighLevelRenderer.hpp"
#include "View.hpp"
#include "Engine.hpp"
#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/Logger.hpp"

#include "GuiRenderer.hpp"
#include "DebugRenderer.hpp"
#include "GeometryRenderer.hpp"
#include "LightsRenderer.hpp"
#include "PostProcessRenderer.hpp"

namespace NFE {
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

    DeviceInfo deviceInfo;
    if (!mRenderingDevice->GetDeviceInfo(deviceInfo))
        LOG_ERROR("Failed to get rendering device information");
    else
    {
        LOG_INFO("GPU name: %s", deviceInfo.description.c_str());
        LOG_INFO("GPU info: %s", deviceInfo.misc.c_str());

        std::string features;
        for (size_t i = 0; i < deviceInfo.features.size(); ++i)
        {
            if (i > 0)
                features += ", ";
            features += deviceInfo.features[i];
        }
        LOG_INFO("GPU features: %s", features.c_str());
    }

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    /// create default and deferred rendering contexts
    mDefaultContext.reset(new RenderContext);
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
    mDefaultSampler.reset();
    mDefaultDiffuseTexture.reset();
    mDefaultNormalTexture.reset();
    mDefaultSpecularTexture.reset();

    mDeferredContexts.reset();
    mDefaultContext.reset();

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
    mDefaultSampler.reset(mRenderingDevice->CreateSampler(samplerDesc));

    TextureDataDesc texDataDesc;
    texDataDesc.lineSize = texDataDesc.sliceSize = 4 * sizeof(uint8);
    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.access = BufferAccess::GPU_ReadOnly;
    texDesc.width = texDesc.height = 1;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.mipmaps = 1;
    texDesc.dataDesc = &texDataDesc;
    texDesc.format = ElementFormat::Uint_8_norm;
    texDesc.texelSize = 4;

    /// default textures (used when no texture is bound to a material)

    uint8 diffuseColor[] = { 255, 255, 255, 255 };
    texDataDesc.data = diffuseColor;
    texDesc.debugName = "DefaultDiffuseTexture";
    mDefaultDiffuseTexture.reset(mRenderingDevice->CreateTexture(texDesc));

    uint8 normalColor[] = { 127, 127, 255, 255 };
    texDataDesc.data = normalColor;
    texDesc.debugName = "DefaultNormalTexture";
    mDefaultNormalTexture.reset(mRenderingDevice->CreateTexture(texDesc));

    // TODO: this should be configurable
    uint8 specularColor[] = { 25, 25, 25, 255 };
    texDataDesc.data = specularColor;
    texDesc.debugName = "DefaultSpecularTexture";
    mDefaultSpecularTexture.reset(mRenderingDevice->CreateTexture(texDesc));
}

RenderContext* HighLevelRenderer::GetDefaultContext() const
{
    return mDefaultContext.get();
}

RenderContext* HighLevelRenderer::GetDeferredContext(size_t id) const
{
    return mDeferredContexts.get() + id;
}

std::string HighLevelRenderer::GetShadersPath() const
{
    // TODO: It's temporary. Shader location must be dependent on low-level renderer.
    return "nfEngine/Shaders/D3D11/";
}

} // namespace Renderer
} // namespace NFE
