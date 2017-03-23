/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's backbuffer
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::~Backbuffer()
{
    Release();
}

bool Backbuffer::GetBackbufferTexture()
{
    HRESULT hr;

    // get the address of the back buffer
    hr = D3D_CALL_CHECK(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&mTexture2D)));
    if (FAILED(hr))
        return false;

    if (gDevice->IsDebugLayerEnabled())
    {
        String textureName = "NFE::Renderer::Backbuffer \"" + mDebugName + '"';
        D3D_CALL_CHECK(mSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName,
                                                  static_cast<UINT>(textureName.length()), textureName.Str()));
    }

    return true;
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    mWidth = static_cast<uint16>(newWidth);
    mHeight = static_cast<uint16>(newHeight);

    if (mWidth == 0 || mHeight == 0)
        return false;

    LOG_INFO("Resizing backbuffer '%s'", mDebugName.Str());

    HRESULT hr;
    if (!mSwapChain.get())
        return false;

    // Release all outstanding references to the swap chain's buffers.
    D3D_SAFE_RELEASE(mTexture2D);

    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = D3D_CALL_CHECK(mSwapChain->ResizeBuffers(0, mWidth, mHeight, DXGI_FORMAT_UNKNOWN, 0));
    if (FAILED(hr))
        return false;

    // TODO: what about rendertargets that are using the backbuffer?

    return GetBackbufferTexture();
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    HRESULT hr;

    mType = TextureType::Texture2D;
    mWindow = static_cast<HWND>(desc.windowHandle);
    mWidth = static_cast<uint16>(desc.width);
    mHeight = static_cast<uint16>(desc.height);
    mVSync = desc.vSync;
    mLayers = 1;
    mSamples = 1;

    if (desc.debugName)
        mDebugName = desc.debugName;

    DXGI_USAGE usageFlags = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    if (gDevice->mFeatureLevel == D3D_FEATURE_LEVEL_11_0)
        usageFlags |= DXGI_USAGE_UNORDERED_ACCESS;

    // create swap chain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;
    scd.BufferDesc.Width = mWidth;
    scd.BufferDesc.Height = mHeight;
    scd.BufferDesc.Format = TranslateElementFormat(desc.format);
    scd.BufferUsage = usageFlags;
    scd.OutputWindow = mWindow;
    scd.SampleDesc.Count = 1;
    scd.Windowed = 1;

    Release();
    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChain(gDevice->mDevice.get(), &scd,
                        &mSwapChain));
    if (FAILED(hr))
        return false;

    if (gDevice->IsDebugLayerEnabled())
    {
        String swapChainName = "NFE::Renderer::Backbuffer \"" + mDebugName + '"';
        D3D_CALL_CHECK(mSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName,
                                                  static_cast<UINT>(swapChainName.length()), swapChainName.Str()));
    }

    LOG_DEBUG("Swapchain created successfully (width=%i, height=%i, format=%s)",
              desc.width, desc.height, GetElementFormatName(desc.format));
    return GetBackbufferTexture();
}

bool Backbuffer::Present()
{
    IDXGISwapChain* swapChain = mSwapChain.get();

    if (!swapChain)
        return false;

    HRESULT hr = swapChain->Present(mVSync ? 1 : 0, 0);
    return SUCCEEDED(hr);
}

void Backbuffer::Release()
{
    IDXGISwapChain* swapChain = mSwapChain.get();
    if (swapChain)
    {
        // swap chain must enter windowed state before releasing
        swapChain->SetFullscreenState(FALSE, nullptr);
        mSwapChain.reset();
    }
}

} // namespace Renderer
} // namespace NFE
