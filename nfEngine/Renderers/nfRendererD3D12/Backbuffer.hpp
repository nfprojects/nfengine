/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer
 */

#pragma once

#include "../RendererInterface/Backbuffer.hpp"
#include "Common.hpp"
#include "Resource.hpp"
#include "nfCommon/Containers/DynArray.hpp"

namespace NFE {
namespace Renderer {

class Backbuffer : public Resource, public IBackbuffer
{
    friend class CommandRecorder;

public:
    Backbuffer();
    ~Backbuffer();
    bool Resize(int newWidth, int newHeight);
    bool Present();
    bool Init(const BackbufferDesc& desc);

    NFE_INLINE DXGI_FORMAT GetFormat() const
    {
        return mFormat;
    }

    NFE_INLINE ID3D12Resource* GetCurrentBuffer() const
    {
        return mBuffers[mCurrentBuffer].Get();
    }

private:
    uint32 mWidth;
    uint32 mHeight;

    DXGI_FORMAT mFormat;

    HWND mWindow;
    bool mVSync;

    D3DPtr<IDXGISwapChain3> mSwapChain;

    // textures for each backbuffer slice
    Common::DynArray<D3DPtr<ID3D12Resource>> mBuffers;

    uint32 mCurrentBuffer;
    HANDLE mWaitableObject;
};

} // namespace Renderer
} // namespace NFE
