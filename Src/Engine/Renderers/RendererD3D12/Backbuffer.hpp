/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer
 */

#pragma once

#include "../RendererCommon/Backbuffer.hpp"
#include "Common.hpp"
#include "Resource.hpp"
#include "Fence.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"

namespace NFE {
namespace Renderer {

class Backbuffer : public Resource, public IBackbuffer
{
    friend class CommandRecorder;

public:

    Backbuffer();
    ~Backbuffer();
    virtual bool Resize(uint32 newWidth, uint32 newHeight) override;
    virtual bool Present() override;
    bool Init(const BackbufferDesc& desc);

    NFE_INLINE DXGI_FORMAT GetFormat() const
    {
        return mFormat;
    }

    NFE_INLINE ID3D12Resource* GetCurrentBuffer() const
    {
        return mBuffers[mCurrentBuffer].Get();
    }

    NFE_FORCE_INLINE virtual ID3D12Resource* GetD3DResource() const override
    {
        return GetCurrentBuffer();
    }

private:

    bool ExtractBuffers();

    uint32 mWidth;
    uint32 mHeight;

    DXGI_FORMAT mFormat;

    HWND mWindow;
    uint8 mVSyncInterval;

    CommandQueuePtr mCommandQueue;

    D3DPtr<IDXGISwapChain3> mSwapChain;

    // textures for each backbuffer slice
    Common::StaticArray<D3DPtr<ID3D12Resource>, DXGI_MAX_SWAP_CHAIN_BUFFERS> mBuffers;
    uint32 mCurrentBuffer;

    // for frame synchronization
    static constexpr uint32 MaxPendingFrames = 3;
    Common::StaticArray<FencePtr, MaxPendingFrames> mPendingFramesFences;
    FenceData mFrameFence;
};

} // namespace Renderer
} // namespace NFE
