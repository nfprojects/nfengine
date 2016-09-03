/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of high-level renderer resources.
 */

#include "PCH.hpp"
#include "RendererResources.hpp"
#include "HighLevelRenderer.hpp"
#include "LightsRenderer.hpp"
#include "Engine.hpp"
#include "../Renderers/RendererInterface/Device.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

ShadowMap::ShadowMap()
    : mSize(0)
    , mType(Type::None)
    , mSplits(0)
{
}

void ShadowMap::Release()
{
    for (uint32 i = 0; i < MAX_CASCADE_SPLITS; ++i)
        mRenderTargets[i].reset();
    mBindingInstance.reset();
    mTexture.reset();
    mDepthBuffer.reset();

    mSize = 0;
    mType = Type::None;
    mSplits = 0;
}

bool ShadowMap::Resize(uint32 size, Type type, uint32 splits)
{
    if (size == mSize || type == mType || splits == mSplits)
        return true;

    switch (type)
    {
    case Type::Flat:
        mSplits = 1;
        break;
    case Type::Cube:
        mSplits = 6;
        break;
    case Type::Cascaded:
        if (splits < 1 || splits > MAX_CASCADE_SPLITS)
        {
            LOG_ERROR("Invalid splits number for cascaded shadow map (%u)", splits);
            Release();
            return false;
        }
        mSplits = static_cast<uint8>(splits);
        break;
    default:
        LOG_ERROR("Invalid shadow type");
        Release();
        return false;
    }

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    TextureDesc depthBufferDesc;
    depthBufferDesc.type = TextureType::Texture2D;
    depthBufferDesc.mode = BufferMode::GPUOnly;
    depthBufferDesc.width = depthBufferDesc.height = size;
    depthBufferDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH;
    depthBufferDesc.mipmaps = 1;
    depthBufferDesc.depthBufferFormat = DepthBufferFormat::Depth32;
    depthBufferDesc.debugName = "ShadowMap::mDepthBuffer";
    mDepthBuffer.reset(renderer->GetDevice()->CreateTexture(depthBufferDesc));
    if (mDepthBuffer == nullptr)
    {
        LOG_ERROR("Failed to create depth buffer");
        Release();
        return false;
    }

    TextureDesc texDesc;
    texDesc.type = (type == Type::Cube) ? TextureType::TextureCube : TextureType::Texture2D;
    texDesc.mode = BufferMode::GPUOnly;
    texDesc.width = texDesc.height = size;
    texDesc.layers = mSplits;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.mipmaps = 1;
    texDesc.format = ElementFormat::R32_Float;
    texDesc.debugName = "ShadowMap::mTexture";

    mTexture.reset(renderer->GetDevice()->CreateTexture(texDesc));
    if (mTexture == nullptr)
    {
        LOG_ERROR("Failed to create shadow map texture");
        Release();
        return false;
    }

    // create binding instance
    mBindingInstance.reset(renderer->GetDevice()->CreateResourceBindingInstance(
        LightsRenderer::Get()->GetShadowMapBindingSet().get()));
    if (!mBindingInstance)
    {
        LOG_ERROR("Failed to create shadow map's resource binding instance");
        return false;
    }
    if (!mBindingInstance->WriteTextureView(0, mTexture.get()))
    {
        LOG_ERROR("Failed to write shadow map's binding instance");
        Release();
        return false;
    }

    if (type == Type::Flat)
    {
        RenderTargetElement rtElement;
        rtElement.texture = mTexture.get();

        RenderTargetDesc rtDesc;
        rtDesc.numTargets = 1;
        rtDesc.targets = &rtElement;
        rtDesc.depthBuffer = mDepthBuffer.get();
        rtDesc.debugName = "ShadowMap::mRenderTarget";

        mRenderTargets[0].reset(renderer->GetDevice()->CreateRenderTarget(rtDesc));
        if (!mRenderTargets[0])
        {
            LOG_ERROR("Failed to create shadow map's render target");
            Release();
            return false;
        }
    }
    else
    {
        RenderTargetElement rtElement;
        rtElement.texture = mTexture.get();

        RenderTargetDesc rtDesc;
        rtDesc.numTargets = 1;
        rtDesc.targets = &rtElement;
        rtDesc.depthBuffer = mDepthBuffer.get();
        rtDesc.debugName = "ShadowMap::mRenderTarget";

        // create rendertargets for each split / cube face
        for (uint32 i = 0; i < mSplits; ++i)
        {
            rtElement.layer = i;

            mRenderTargets[i].reset(renderer->GetDevice()->CreateRenderTarget(rtDesc));
            if (!mRenderTargets[i])
            {
                LOG_ERROR("Failed to create shadow map's render target for i = %u", i);
                Release();
                return false;
            }
        }
    }

    mType = type;
    mSize = static_cast<uint16>(size);
    return true;
}


void GeometryBuffer::Release()
{
    mBindingInstance.reset();
    mRenderTarget.reset();
    for (int i = 0; i < gLayers; ++i)
        mTextures[i].reset();
}

bool GeometryBuffer::Resize(int width, int height)
{
    mWidth = width;
    mHeight = height;

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    TextureDesc depthBufferDesc;
    depthBufferDesc.type = TextureType::Texture2D;
    depthBufferDesc.mode = BufferMode::GPUOnly;
    depthBufferDesc.width = width;
    depthBufferDesc.height = height;
    depthBufferDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH | NFE_RENDERER_TEXTURE_BIND_SHADER;
    depthBufferDesc.mipmaps = 1;
    depthBufferDesc.depthBufferFormat = DepthBufferFormat::Depth32;
    depthBufferDesc.debugName = "GeometryBuffer::mDepthBuffer";
    mDepthBuffer.reset(renderer->GetDevice()->CreateTexture(depthBufferDesc));
    if (mDepthBuffer == nullptr)
    {
        LOG_ERROR("Failed to create depth buffer");
        return false;
    }

    RenderTargetElement rtTargets[gLayers];
    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.mode = BufferMode::GPUOnly;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.mipmaps = 1;

    /// TODO: this could be extended in the future
    ElementFormat elementFormats[] =
    {
        ElementFormat::R16G16B16A16_Float,
        ElementFormat::R16G16B16A16_Float,
        ElementFormat::R16G16B16A16_Float,
        ElementFormat::R16G16_Float,
    };


    mBindingInstance.reset(renderer->GetDevice()->CreateResourceBindingInstance(
        LightsRenderer::Get()->GetGBufferBindingSet().get()));
    if (!mBindingInstance)
    {
        LOG_ERROR("Failed to create G-Buffer's resource binding instance");
        return false;
    }
    if (!mBindingInstance->WriteTextureView(0, mDepthBuffer.get()))
    {
        LOG_ERROR("Failed to write depth buffer texture to binding instance");
        Release();
        return false;
    }

    /// create G-Buffer textures
    for (int i = 0; i < gLayers; ++i)
    {
        texDesc.format = elementFormats[i];
        mTextures[i].reset(renderer->GetDevice()->CreateTexture(texDesc));
        if (!mTextures[i])
        {
            LOG_ERROR("Failed to create G-Buffer's texture (i = %i)", i);
            Release();
            return false;
        }

        if (!mBindingInstance->WriteTextureView(i + 1, mTextures[i].get()))
        {
            LOG_ERROR("Failed to write G-Buffer's texture (i = %i) to binding instance", i);
            Release();
            return false;
        }

        rtTargets[i].texture = mTextures[i].get();
    }

    /// create G-Buffer render target
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = gLayers;
    rtDesc.targets = rtTargets;
    rtDesc.depthBuffer = mDepthBuffer.get();
    rtDesc.debugName = "GeometryBuffer::mRenderTarget";
    mRenderTarget.reset(renderer->GetDevice()->CreateRenderTarget(rtDesc));
    if (!mRenderTarget)
    {
        LOG_ERROR("Failed to create G-Buffer's render target");
        Release();
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
