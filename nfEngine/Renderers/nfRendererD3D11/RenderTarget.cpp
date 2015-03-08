/**
 * @file    RenderTarget.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's render target
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

int RenderTarget::Resize(int newWidth, int newHeight)
{
    mWidth = newWidth;
    mHeight = newHeight;

    if (mWidth == 0 || mHeight == 0)
        return 1;

    HRESULT hr;
    if (!mSwapChain.get())
        return 1;

    // Release all outstanding references to the swap chain's buffers.
    mRTV.reset();
    mSRV.reset();
    D3D_SAFE_RELEASE(mTexture2D);

    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = mSwapChain->ResizeBuffers(0, mWidth, mHeight, DXGI_FORMAT_UNKNOWN, 0);

    if (FAILED(hr))
    {
        // TODO: logging
        return 1;
    }

    // Get buffer and create a render-target-view.
    hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mTexture2D);
    if (FAILED(hr))
    {
        // TODO: logging
        return 1;
    }

    hr = gDevice->mDevice->CreateRenderTargetView(mTexture2D, NULL, &mRTV);
    if (FAILED(hr))
    {
        // TODO: logging
        return 1;
    }

    return 0;
}

int RenderTarget::InitSwapChain(int width, int height, HWND window)
{
    HRESULT hr;

    type = TextureType::Texture2D;
    mWindow = window;
    mWidth = width;
    mHeight = height;

    DXGI_USAGE usageFlags = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    if (gDevice->mFeatureLevel == D3D_FEATURE_LEVEL_11_0)
        usageFlags |= DXGI_USAGE_UNORDERED_ACCESS;

    // create swap chain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = usageFlags;
    scd.OutputWindow = window;
    scd.SampleDesc.Count = 1;
    scd.Windowed = 1;

    hr = gDevice->mDXGIFactory->CreateSwapChain(gDevice->mDevice.get(), &scd, &mSwapChain);
    if (FAILED(hr))
    {
        // TODO: logging
        return 1;
    }

    // get the address of the back buffer
    hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mTexture2D);
    if (FAILED(hr))
    {
        // TODO: logging
        return 1;
    }

    hr = gDevice->mDevice->CreateRenderTargetView(mTexture2D, NULL, &mRTV);
    if (FAILED(hr))
        return 1;

    hr = gDevice->mDevice->CreateRenderTargetView(mTexture2D, NULL, &mRTV);
    if (FAILED(hr))
        return 1;

    return 0;
}

int RenderTarget::Present()
{
    IDXGISwapChain* swapChain = mSwapChain.get();

    if (!swapChain)
        return 1;

    HRESULT hr = swapChain->Present(0, 0);
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
