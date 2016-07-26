/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 renderer's backbuffer
 */

#pragma once

#include "../RendererInterface/Backbuffer.hpp"
#include "Common.hpp"
#include "Texture.hpp"

namespace NFE {
namespace Renderer {

class Backbuffer : public IBackbuffer, public Texture
{
    friend class CommandBuffer;

    HWND mWindow;
    D3DPtr<IDXGISwapChain> mSwapChain;
    bool mVSync;

    std::string mDebugName;

    // get ID3D11Texture2D from the IDXGISwapChain
    bool GetBackbufferTexture();
    void Release();

public:
    ~Backbuffer();
    bool Resize(int newWidth, int newHeight) override;
    bool Present() override;
    bool Init(const BackbufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE