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

    VkSurfaceKHR mSurface;

#ifdef WIN32
    HWND mHWND;
#elif defined(__linux__) || defined(__LINUX__)
    xcb_connection_t* mConnection;
#else
#error Target platform not supported.
#endif

    VkFormat mFormat;
    VkColorSpaceKHR mColorSpace;
    uint32 mPresentQueueIndex;

    VkCommandPool mPresentCommandPool;
    VkCommandBuffer mPresentCommandBuffer;

    // platform-specific surface creator
    bool CreateSurface(const BackbufferDesc& desc);

    // right now needed purely by XCB implementation
    void CleanupPlatformSpecifics();

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const BackbufferDesc& desc);

    bool Resize(int newWidth, int newHeight) override;
    bool Present() override;
};

} // namespace Renderer
} // namespace NFE
