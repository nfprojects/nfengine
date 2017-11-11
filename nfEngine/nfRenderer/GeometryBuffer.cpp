/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of high-level renderer resources.
 */

#include "PCH.hpp"
#include "GeometryBuffer.hpp"

#include "../Renderers/RendererInterface/Device.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {


void GeometryBuffer::Release()
{
    mBindingInstance.Reset();
    mRenderTarget.Reset();

    for (int i = 0; i < NumBuffers; ++i)
    {
        mTextures[i].Reset();
    }
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
    mDepthBuffer = renderer->GetDevice()->CreateTexture(depthBufferDesc);
    if (mDepthBuffer == nullptr)
    {
        LOG_ERROR("Failed to create depth buffer");
        return false;
    }

    RenderTargetElement rtTargets[NumBuffers];
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


    mBindingInstance = renderer->GetDevice()->CreateResourceBindingInstance(LightsRenderer::Get()->GetGBufferBindingSet());
    if (!mBindingInstance)
    {
        LOG_ERROR("Failed to create G-Buffer's resource binding instance");
        return false;
    }
    if (!mBindingInstance->WriteTextureView(0, mDepthBuffer))
    {
        LOG_ERROR("Failed to write depth buffer texture to binding instance");
        Release();
        return false;
    }

    // create G-Buffer textures
    for (uint32 i = 0; i < NumBuffers; ++i)
    {
        texDesc.format = elementFormats[i];
        mTextures[i]= renderer->GetDevice()->CreateTexture(texDesc);
        if (!mTextures[i])
        {
            LOG_ERROR("Failed to create G-Buffer's texture (i = %i)", i);
            Release();
            return false;
        }

        if (!mBindingInstance->WriteTextureView(i + 1, mTextures[i]))
        {
            LOG_ERROR("Failed to write G-Buffer's texture (i = %i) to binding instance", i);
            Release();
            return false;
        }

        rtTargets[i].texture = mTextures[i];
    }

    // create G-Buffer render target
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = NumBuffers;
    rtDesc.targets = rtTargets;
    rtDesc.depthBuffer = mDepthBuffer;
    rtDesc.debugName = "GeometryBuffer::mRenderTarget";
    mRenderTarget = renderer->GetDevice()->CreateRenderTarget(rtDesc);
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
