/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's backbuffer
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : Resource(D3D12_RESOURCE_STATE_PRESENT)
    , mWindow(NULL)
    , mVSync(false)
    , mWidth(0)
    , mHeight(0)
    , mCurrentBuffer(0)
{ }

Backbuffer::~Backbuffer()
{
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    NFE_UNUSED(newWidth);
    NFE_UNUSED(newHeight);
    return false;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    HRESULT hr;

    if (desc.width < 1 || desc.height < 1 ||
        desc.width >= std::numeric_limits<uint16>::max() ||
        desc.height >= std::numeric_limits<uint16>::max())
    {
        NFE_LOG_ERROR("Invalid backbuffer size");
        return false;
    }

    mWidth = static_cast<uint32>(desc.width);
    mHeight = static_cast<uint32>(desc.height);
    mWindow = static_cast<HWND>(desc.windowHandle);
    mVSync = desc.vSync;

    mBuffers.Resize(3); // make it configurable

    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.BufferCount = mBuffers.Size();
    scd.Width = desc.width;
    scd.Height = desc.height;
    scd.Format = TranslateElementFormat(desc.format);
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    scd.SampleDesc.Count = 1;

    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChainForHwnd(
        gDevice->mCommandQueue.Get(), static_cast<HWND>(desc.windowHandle), &scd, nullptr, nullptr,
        reinterpret_cast<IDXGISwapChain1**>(&mSwapChain)));

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create swap chain");
        return false;
    }

    mSwapChain->SetMaximumFrameLatency(1);
    mWaitableObject = mSwapChain->GetFrameLatencyWaitableObject();

    // disable Alt-Enter
    gDevice->mDXGIFactory->MakeWindowAssociation(mWindow, DXGI_MWA_NO_ALT_ENTER);

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();

    for (UINT n = 0; n < mBuffers.Size(); n++)
    {
        hr = mSwapChain->GetBuffer(n, IID_PPV_ARGS(mBuffers[n].GetPtr()));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("Failed to get swap chain buffer for n = %u", n);
            return false;
        }
    }

    NFE_LOG_DEBUG("Swapchain created successfully (width=%i, height=%i, format=%s)", desc.width, desc.height, GetElementFormatName(desc.format));
    return true;
}

bool Backbuffer::Present()
{
    const DWORD result = WaitForSingleObjectEx(mWaitableObject, 0u, TRUE);
    if (result == WAIT_TIMEOUT)
    {
        NFE_LOG_ERROR("Waiting for swapchain waitable object failed");
        return false;
    }

    HRESULT hr = D3D_CALL_CHECK(mSwapChain->Present(mVSync ? 1 : 0, 0));
    if (FAILED(hr))
        return false;

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    NFE_ASSERT(mCurrentBuffer < mBuffers.Size(), "Invalid buffer counter");

    return true;
}

} // namespace Renderer
} // namespace NFE
