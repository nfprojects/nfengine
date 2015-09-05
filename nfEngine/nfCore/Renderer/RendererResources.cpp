/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of high-level renderer resources.
 */

#include "../PCH.hpp"
#include "RendererResources.hpp"
#include "HighLevelRenderer.hpp"
#include "../Engine.hpp"
#include "../Renderers/RendererInterface/Device.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Renderer {

void ShadowMap::Release()
{

}

int ShadowMap::Resize(uint32 size, Type type, uint32 splits)
{
    return 0;
}

uint32 ShadowMap::GetSize() const
{
    return 0;
}


void GeometryBuffer::Release()
{
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
    depthBufferDesc.access = BufferAccess::GPU_ReadWrite;
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
    texDesc.access = BufferAccess::GPU_ReadWrite;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.mipmaps = 1;

    /// TODO: this could be extended in the future
    ElementFormat elementFormats[] =
    {
        ElementFormat::Float_16,
        ElementFormat::Float_16,
        ElementFormat::Float_16,
        ElementFormat::Float_16,
    };
    int elementSizes[] = { 4, 4, 4, 2 };

    /// create G-Buffer textures
    for (int i = 0; i < gLayers; ++i)
    {
        texDesc.format = elementFormats[i];
        texDesc.texelSize = elementSizes[i];
        mTextures[i].reset(renderer->GetDevice()->CreateTexture(texDesc));
        if (!mTextures[i])
        {
            LOG_ERROR("Failed to create G-Buffer's texture (i = %i)", i);
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
