/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of main class of high-level Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "../Core.hpp"
#include "HighLevelRenderer.hpp"
#include "View.hpp"
#include "../Globals.hpp"
#include "../Engine.hpp"
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

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    /// create immediate and deferred rendering contexts
    mImmediateContext.reset(new RenderContext(mCommandBuffer));
    mDeferredContexts.reset(new RenderContext[threadPool->GetThreadsNumber()]);

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

    mDefaultBlendState.reset();
    mDefaultSampler.reset();
    mDefaultDepthState.reset();
    mDefaultDiffuseTexture.reset();
    mDefaultNormalTexture.reset();
    mDefaultSpecularTexture.reset();

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
    BlendStateDesc bsDesc;
    bsDesc.debugName = "HighLevelRenderer::mDefaultDepthState";
    mDefaultBlendState.reset(mRenderingDevice->CreateBlendState(bsDesc));

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


    TextureDataDesc texDataDesc;
    texDataDesc.lineSize = texDataDesc.sliceSize = 4 * sizeof(uchar);
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

    uchar diffuseColor[] = { 255, 255, 255, 255 };
    texDataDesc.data = diffuseColor;
    texDesc.debugName = "DefaultDiffuseTexture";
    mDefaultDiffuseTexture.reset(mRenderingDevice->CreateTexture(texDesc));

    uchar normalColor[] = { 127, 127, 255, 255 };
    texDataDesc.data = normalColor;
    texDesc.debugName = "DefaultNormalTexture";
    mDefaultNormalTexture.reset(mRenderingDevice->CreateTexture(texDesc));

    // TODO: this should be configurable
    uchar specularColor[] = { 25, 25, 25, 255 };
    texDataDesc.data = specularColor;
    texDesc.debugName = "DefaultSpecularTexture";
    mDefaultSpecularTexture.reset(mRenderingDevice->CreateTexture(texDesc));
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
