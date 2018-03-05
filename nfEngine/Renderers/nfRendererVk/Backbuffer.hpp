/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's backbuffer
 */

#pragma once

#include "../RendererInterface/Backbuffer.hpp"
#include "Texture.hpp"


namespace NFE {
namespace Renderer {


class Backbuffer : public IBackbuffer, public Texture
{
    friend class CommandRecorder;

    VkSurfaceKHR mSurface;

#ifdef WIN32
    HWND mHWND;
#elif defined(__linux__) || defined(__LINUX__)
    xcb_connection_t* mConnection;
#else
#error Target platform not supported.
#endif

    VkColorSpaceKHR mColorSpace;
    uint32 mPresentQueueIndex;
    VkQueue mPresentQueue;
    VkSurfaceCapabilitiesKHR mSurfaceCapabilities;
    VkSwapchainKHR mSwapchain;
    VkPresentModeKHR mSwapPresentMode;
    uint32 mBuffersNum;
    bool mImageAcquired;

    // platform-specific surface creator
    bool CreateSurface(const BackbufferDesc& desc);

    // right now needed purely by XCB implementation
    void CleanupPlatformSpecifics();

    bool SelectPresentQueue();
    bool SelectSurfaceFormat(const BackbufferDesc& desc);
    bool SelectPresentMode(const BackbufferDesc& desc);
    bool SelectBufferCount();
    bool CreateSwapchain(const BackbufferDesc& desc);
    bool CreateSwapchainImageViews();

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const BackbufferDesc& desc);
    bool AcquireNextImage();

    bool Resize(int newWidth, int newHeight) override;
    bool Present() override;
};

} // namespace Renderer
} // namespace NFE
