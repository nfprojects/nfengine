/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mWidth(0)
    , mHeight(0)
{
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = gDevice->mRtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (unsigned int i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture);
        if (tex == nullptr)
        {
            LOG_ERROR("Invalid target texture at index %u", i);
            return false;
        }

        // Create a RTV for each frame
        for (UINT n = 0; n < tex->mBuffersNum; n++)
        {
            gDevice->mDevice->CreateRenderTargetView(tex->mBuffers[n].get(), nullptr, rtvHandle);
            mRTVs[n].push_back(rtvHandle);

            rtvHandle.ptr += gDevice->mRtvDescSize;
        }

        mTextures.push_back(tex);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
