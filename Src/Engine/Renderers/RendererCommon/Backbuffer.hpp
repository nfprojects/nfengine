/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Texture.hpp"
#include "Types.hpp"
#include "CommandQueue.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of a backbuffer (a texture that represents window surface).
 */
struct BackbufferDesc
{
    CommandQueuePtr commandQueue;   //< command queue to be used for presenting
    void* windowHandle;             //< platform dependent window handle
    uint32 width;
    uint32 height;
    Format format;
    bool vSync;
    const char* debugName;          //< optional debug name
    // TODO: options such as: UAV usage, mulitsampling, texture format, etc.
    // TODO: buffers number (double buffering, triple buffering, etc.)

    BackbufferDesc()
        : windowHandle(nullptr)
        , width(0)
        , height(0)
        , format(Format::R8G8B8A8_U_Norm)
        , vSync(false)
        , debugName(nullptr)
    {
    }
};

class IBackbuffer
{
public:
    virtual ~IBackbuffer() {}

    // should be called when window has been resized
    virtual bool Resize(int newWidth, int newHeight) = 0;

    // swap buffers
    virtual bool Present() = 0;
};

} // namespace Renderer
} // namespace NFE
