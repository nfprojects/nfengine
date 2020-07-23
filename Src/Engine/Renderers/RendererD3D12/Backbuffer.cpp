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
    , mFormat(DXGI_FORMAT_UNKNOWN)
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
    mFormat = TranslateElementFormat(desc.format);

    mBuffers.Resize(2); // make it configurable

    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.BufferCount = mBuffers.Size();
    scd.Width = desc.width;
    scd.Height = desc.height;
    scd.Format = mFormat;
    scd.Scaling = DXGI_SCALING_NONE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.SampleDesc.Count = 1;

    if (gDevice->GetCaps().tearingSupport)
    {
        scd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChainForHwnd(
        gDevice->GetGraphicsQueue(), static_cast<HWND>(desc.windowHandle), &scd, nullptr, nullptr,
        reinterpret_cast<IDXGISwapChain1**>(&mSwapChain)));

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create swap chain");
        return false;
    }

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

        if (!SetDebugName(mBuffers[n].Get(), "Backbuffer"))
        {
            NFE_LOG_WARNING("Failed to set debug name");
        }
    }

    NFE_LOG_DEBUG("Swapchain created successfully (width=%i, height=%i, format=%s)", desc.width, desc.height, GetElementFormatName(desc.format));
    return true;
}

bool Backbuffer::Present()
{
    uint32 vsyncInterval = 0;
    uint32 presentFlags = 0;

    if (gDevice->GetCaps().tearingSupport /* && windowedMode */)
    {
        presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
    }

    if (mVSync)
    {
        vsyncInterval = 1;
    }

    HRESULT hr = D3D_CALL_CHECK(mSwapChain->Present(vsyncInterval, presentFlags));
    if (FAILED(hr))
        return false;

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    NFE_ASSERT(mCurrentBuffer < mBuffers.Size(), "Invalid buffer counter");

    return true;
}

} // namespace Renderer
} // namespace NFE
