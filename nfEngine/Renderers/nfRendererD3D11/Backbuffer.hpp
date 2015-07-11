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

public:
    bool Resize(int newWidth, int newHeight);
    bool Present();
    bool Init(const BackbufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE
