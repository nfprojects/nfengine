/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's backbuffer
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "RendererD3D12.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mWindow(NULL)
    , mBuffersNum(0)
    , mCurrentBuffer(0)
{
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    UNUSED(newWidth);
    UNUSED(newHeight);
    return false;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    HRESULT hr;

    mWindow = static_cast<HWND>(desc.windowHandle);
    mBuffersNum = NUM_BUFFERS;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = mBuffersNum;
    scd.BufferDesc.Width = desc.width;
    scd.BufferDesc.Height = desc.height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.OutputWindow = mWindow;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChain(gDevice->mCommandQueue.get(),
             &scd, reinterpret_cast<IDXGISwapChain**>(&mSwapChain)));
    if (FAILED(hr))
        return false;

    // disable Alt-Enter
    gDevice->mDXGIFactory->MakeWindowAssociation(mWindow, DXGI_MWA_NO_ALT_ENTER);

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    LOG_DEBUG("current buffer = %u", mCurrentBuffer);


    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = gDevice->mRtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create a RTV for each frame.
    for (UINT n = 0; n < mBuffersNum; n++)
    {
        hr = mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mBuffers[n]));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to get swap chain buffer for n = %u", n);
            return false;
        }

        gDevice->mDevice->CreateRenderTargetView(mBuffers[n].get(), nullptr, rtvHandle);
        rtvHandle.ptr += gDevice->mRtvDescSize;
    }

    LOG_DEBUG("Swapchain created successfully.");
    return true;
}

bool Backbuffer::Present()
{
    return false;
}

} // namespace Renderer
} // namespace NFE
