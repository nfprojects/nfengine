/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mWidth(0)
    , mHeight(0)
{
}

RenderTarget::~RenderTarget()
{
    HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();

    for (size_t i = 0; i < 2; ++i)
    {
        for (uint32 offset : mRTVs[i])
            allocator.Free(offset, 1);
    }
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    HeapAllocator& rtvAllocator = gDevice->GetRtvHeapAllocator();

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
            uint32 offset = rtvAllocator.Allocate(1);
            if (offset == -1)
                return false;

            D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvAllocator.GetCpuHandle();
            handle.ptr += rtvAllocator.GetDescriptorSize() * offset;
            gDevice->mDevice->CreateRenderTargetView(tex->mBuffers[n].get(), nullptr, handle);
            mRTVs[n].push_back(offset);
        }

        mTextures.push_back(tex);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
