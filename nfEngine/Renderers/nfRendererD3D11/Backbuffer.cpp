/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's backbuffer
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "RendererD3D11.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    mWidth = newWidth;
    mHeight = newHeight;

    if (mWidth == 0 || mHeight == 0)
        return false;

    HRESULT hr;
    if (!mSwapChain.get())
        return false;

    // Release all outstanding references to the swap chain's buffers.
    mSRV.reset();
    D3D_SAFE_RELEASE(mTexture2D);

    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = D3D_CALL_CHECK(mSwapChain->ResizeBuffers(0, mWidth, mHeight, DXGI_FORMAT_UNKNOWN, 0));
    if (FAILED(hr))
        return false;

    // Get buffer and create a render-target-view.
    hr = D3D_CALL_CHECK(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mTexture2D));
    if (FAILED(hr))
        return false;

    // TODO: what about rendertargets that are using the backbuffer?

    return true;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    HRESULT hr;

    type = TextureType::Texture2D;
    mWindow = static_cast<HWND>(desc.windowHandle);
    mWidth = desc.width;
    mHeight = desc.height;
    mVSync = desc.vSync;

    DXGI_USAGE usageFlags = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    if (gDevice->mFeatureLevel == D3D_FEATURE_LEVEL_11_0)
        usageFlags |= DXGI_USAGE_UNORDERED_ACCESS;

    // create swap chain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;
    scd.BufferDesc.Width = mWidth;
    scd.BufferDesc.Height = mHeight;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = usageFlags;
    scd.OutputWindow = mWindow;
    scd.SampleDesc.Count = 1;
    scd.Windowed = 1;

    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChain(gDevice->mDevice.get(), &scd,
                        &mSwapChain));
    if (FAILED(hr))
        return false;

    // get the address of the back buffer
    hr = D3D_CALL_CHECK(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&mTexture2D));
    if (FAILED(hr))
        return false;

    return true;
}

bool Backbuffer::Present()
{
    IDXGISwapChain* swapChain = mSwapChain.get();

    if (!swapChain)
        return false;

    HRESULT hr = swapChain->Present(mVSync ? 1 : 0, 0);
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
