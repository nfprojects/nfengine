/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer
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

    // TODO: support for tripple buffering, etc.
    static const UINT NUM_BUFFERS = 2;

    HWND mWindow;
    bool mVSync;

    D3DPtr<IDXGISwapChain3> mSwapChain;

public:
    Backbuffer();
    ~Backbuffer();
    bool Resize(int newWidth, int newHeight);
    bool Present();
    bool Init(const BackbufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE
