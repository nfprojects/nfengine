/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's backbuffer
 */

#include "../RendererInterface/Backbuffer.hpp"
#include "Texture.hpp"

namespace NFE {
namespace Renderer {


class Backbuffer : public IBackbuffer, public Texture
{
    friend class CommandBuffer;

#ifdef WIN32
    VkSurfaceKHR mVkSurface;
    HWND mHWND;
#elif defined(__linux__) || defined(__LINUX__)
#else
#error Target platform not supported.
#endif

    uint32 mGraphicsAndPresentQueueIndex;

public:
    Backbuffer();
    ~Backbuffer();

    bool Resize(int newWidth, int newHeight);
    bool Init(const BackbufferDesc& desc);
    bool Present();
};

} // namespace Renderer
} // namespace NFE
