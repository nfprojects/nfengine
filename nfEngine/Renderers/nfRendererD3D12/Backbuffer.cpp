/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's backbuffer
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"



namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mWindow(NULL)
    , mVSync(false)
{ }

Backbuffer::~Backbuffer()
{
    gDevice->WaitForGPU();
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

    if (desc.width < 1 || desc.height < 1 ||
        desc.width >= std::numeric_limits<uint16>::max() ||
        desc.height >= std::numeric_limits<uint16>::max())
    {
        LOG_ERROR("Invalid backbuffer size");
        return false;
    }

    mType = TextureType::Texture2D;
    mWidth = static_cast<uint16>(desc.width);
    mHeight = static_cast<uint16>(desc.height);
    mSrvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    mLayers = 1;
    mWindow = static_cast<HWND>(desc.windowHandle);
    mVSync = desc.vSync;

    // set Texture properties
    mClass = Class::Backbuffer;
    mBuffersNum = NUM_BUFFERS;
    mResourceState = D3D12_RESOURCE_STATE_PRESENT;
    mTargetResourceState = D3D12_RESOURCE_STATE_PRESENT;

    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.BufferCount = mBuffersNum;
    scd.Width = desc.width;
    scd.Height = desc.height;
    scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.SampleDesc.Count = 1;

    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChainForHwnd(
        gDevice->mCommandQueue.get(), static_cast<HWND>(desc.windowHandle), &scd, nullptr, nullptr,
        reinterpret_cast<IDXGISwapChain1**>(&mSwapChain)));
    if (FAILED(hr))
        return false;

    // disable Alt-Enter
    gDevice->mDXGIFactory->MakeWindowAssociation(mWindow, DXGI_MWA_NO_ALT_ENTER);

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    LOG_DEBUG("current buffer = %u", mCurrentBuffer);

    for (UINT n = 0; n < mBuffersNum; n++)
    {
        hr = mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mBuffers[n]));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to get swap chain buffer for n = %u", n);
            return false;
        }
    }

    LOG_DEBUG("Swapchain created successfully.");
    return true;
}

bool Backbuffer::Present()
{
    HRESULT hr = D3D_CALL_CHECK(mSwapChain->Present(mVSync ? 1 : 0, 0));
    if (FAILED(hr))
        return false;

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    return true;
}

} // namespace Renderer
} // namespace NFE
