/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's backbuffer
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "CommandQueue.hpp"
#include "Engine/Common/Utils/StringUtils.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : Resource(D3D12_RESOURCE_STATE_PRESENT)
    , mWindow(NULL)
    , mVSyncInterval(0u)
    , mWidth(0)
    , mHeight(0)
    , mFormat(DXGI_FORMAT_UNKNOWN)
    , mCurrentBuffer(0)
{ }

Backbuffer::~Backbuffer()
{
    for (const FencePtr& fence : mPendingFramesFences)
    {
        fence->Wait();
    }

    mFrameFence.Release();
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

    if (!desc.commandQueue)
    {
        NFE_LOG_ERROR("Invalid command queue");
        return false;
    }

    mWidth = desc.width;
    mHeight = desc.height;
    mWindow = static_cast<HWND>(desc.windowHandle);
    mVSyncInterval = desc.vSyncInterval;
    mFormat = TranslateFormat(desc.format);
    mMode = ResourceAccessMode::GPUOnly;
    mCommandQueue = desc.commandQueue;

    mFrameFence.Init();

    mBuffers.Resize(2); // make it configurable

    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.BufferCount = mBuffers.Size();
    scd.Width = desc.width;
    scd.Height = desc.height;
    scd.Format = mFormat;
    scd.Scaling = DXGI_SCALING_NONE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.SampleDesc.Count = 1;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    if (gDevice->GetCaps().tearingSupport)
    {
        scd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    CommandQueue* commandQueue = static_cast<CommandQueue*>(desc.commandQueue.Get());
    NFE_ASSERT(commandQueue, "Invalid command queue");

    hr = D3D_CALL_CHECK(gDevice->mDXGIFactory->CreateSwapChainForHwnd(
        commandQueue->GetQueue(), static_cast<HWND>(desc.windowHandle), &scd, nullptr, nullptr,
        reinterpret_cast<IDXGISwapChain1**>(&mSwapChain)));

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create swap chain");
        return false;
    }

    // disable Alt-Enter
    gDevice->mDXGIFactory->MakeWindowAssociation(mWindow, DXGI_MWA_NO_ALT_ENTER);

    if (!ExtractBuffers())
    {
        return false;
    }

    NFE_LOG_DEBUG("Swapchain created successfully (width=%i, height=%i, format=%s)", desc.width, desc.height, GetFormatName(desc.format));
    return true;
}

bool Backbuffer::ExtractBuffers()
{
    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    NFE_ASSERT(mCurrentBuffer < mBuffers.Size(), "Invalid buffer index");

    for (uint32 i = 0; i < mBuffers.Size(); i++)
    {
        if (FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mBuffers[i].GetPtr()))))
        {
            NFE_LOG_ERROR("Failed to get swap chain buffer for n = %u", i);
            return false;
        }

        const Common::String debugName = "Backbuffer" + Common::ToString(i);
        if (!SetDebugName(mBuffers[i].Get(), debugName))
        {
            NFE_LOG_WARNING("Failed to set debug name");
        }
    }

    return true;
}

bool Backbuffer::Resize(uint32 newWidth, uint32 newHeight)
{
    if (!mSwapChain)
    {
        // TODO try to create?
        return false;
    }

    if (mWidth == newWidth && mHeight == newHeight)
    {
        return true;
    }

    for (const FencePtr& fence : mPendingFramesFences)
    {
        fence->Wait();
    }
    mPendingFramesFences.Clear();

    for (uint32 i = 0; i < mBuffers.Size(); i++)
    {
        mBuffers[i].Reset();
    }

    uint32 flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    if (gDevice->GetCaps().tearingSupport)
    {
        flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    if (FAILED(mSwapChain->ResizeBuffers(mBuffers.Size(), newWidth, newHeight, mFormat, flags)))
    {
        NFE_LOG_ERROR("Failed to resize backbuffe: width=%u, height=%u", newWidth, newHeight);
        return false;
    }

    if (!ExtractBuffers())
    {
        return false;
    }

    mWidth = newWidth;
    mHeight = newHeight;

    NFE_LOG_ERROR("Backbuffer resized: width=%u, height=%u", newWidth, newHeight);
    return true;
}

bool Backbuffer::Present()
{
    // wait for old frames
    {
        FencePtr oldFence;
        if (mPendingFramesFences.Size() >= MaxPendingFrames)
        {
            oldFence = std::move(mPendingFramesFences.Front());
            mPendingFramesFences.Erase(mPendingFramesFences.Begin(), mPendingFramesFences.Begin() + 1);
        }

        if (oldFence)
        {
            oldFence->Wait();
        }
    }

    uint32 vsyncInterval = Math::Min<uint32>(mVSyncInterval, 4u);
    uint32 presentFlags = 0;

    if (gDevice->GetCaps().tearingSupport && vsyncInterval == 0u)
    {
        presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
    }

    HRESULT hr = D3D_CALL_CHECK(mSwapChain->Present(vsyncInterval, presentFlags));
    if (FAILED(hr))
    {
        return false;
    }

    FencePtr newFence = mCommandQueue->Signal();
    if (newFence)
    {
        mPendingFramesFences.PushBack(std::move(newFence));
    }

    mCurrentBuffer = mSwapChain->GetCurrentBackBufferIndex();
    NFE_ASSERT(mCurrentBuffer < mBuffers.Size(), "Invalid buffer counter");

    return true;
}

} // namespace Renderer
} // namespace NFE
