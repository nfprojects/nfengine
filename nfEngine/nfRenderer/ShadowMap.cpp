/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of high-level renderer resources.
 */

#include "PCH.hpp"
#include "ShadowMap.hpp"
#include "HighLevelRenderer.hpp"
#include "LightsRenderer.hpp"

#include "../Renderers/RendererInterface/Device.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

ShadowMap::ShadowMap()
    : mSize(0)
    , mType(Type::None)
    , mSplits(0)
{
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::Release()
{
    for (uint32 i = 0; i < MAX_CASCADE_SPLITS; ++i)
        mRenderTargets[i].Reset();
    mBindingInstance.Reset();
    mTexture.Reset();
    mDepthBuffer.Reset();

    mSize = 0;
    mType = Type::None;
    mSplits = 0;
}

bool ShadowMap::Resize(HighLevelRenderer& renderer, uint16 size, Type type, uint8 splits)
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

    IDevice* device = renderer.GetDevice();

    TextureDesc depthBufferDesc;
    depthBufferDesc.type = TextureType::Texture2D;
    depthBufferDesc.mode = BufferMode::GPUOnly;
    depthBufferDesc.width = depthBufferDesc.height = size;
    depthBufferDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH;
    depthBufferDesc.mipmaps = 1;
    depthBufferDesc.depthBufferFormat = DepthBufferFormat::Depth32;
    depthBufferDesc.debugName = "ShadowMap::mDepthBuffer";
    mDepthBuffer = device->CreateTexture(depthBufferDesc);
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

    mTexture = device->CreateTexture(texDesc);
    if (mTexture == nullptr)
    {
        LOG_ERROR("Failed to create shadow map texture");
        Release();
        return false;
    }

    // create binding instance
    mBindingInstance = device->CreateResourceBindingInstance(
        LightsRenderer::Get()->GetShadowMapBindingSet());
    if (!mBindingInstance)
    {
        LOG_ERROR("Failed to create shadow map's resource binding instance");
        return false;
    }
    if (!mBindingInstance->WriteTextureView(0, mTexture))
    {
        LOG_ERROR("Failed to write shadow map's binding instance");
        Release();
        return false;
    }

    if (type == Type::Flat)
    {
        RenderTargetElement rtElement;
        rtElement.texture = mTexture;

        RenderTargetDesc rtDesc;
        rtDesc.numTargets = 1;
        rtDesc.targets = &rtElement;
        rtDesc.depthBuffer = mDepthBuffer;
        rtDesc.debugName = "ShadowMap::mRenderTarget";

        mRenderTargets[0]= device->CreateRenderTarget(rtDesc);
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
        rtElement.texture = mTexture;

        RenderTargetDesc rtDesc;
        rtDesc.numTargets = 1;
        rtDesc.targets = &rtElement;
        rtDesc.depthBuffer = mDepthBuffer;
        rtDesc.debugName = "ShadowMap::mRenderTarget";

        // create rendertargets for each split / cube face
        for (uint32 i = 0; i < mSplits; ++i)
        {
            rtElement.layer = i;

            mRenderTargets[i]= device->CreateRenderTarget(rtDesc);
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


} // namespace Renderer
} // namespace NFE
