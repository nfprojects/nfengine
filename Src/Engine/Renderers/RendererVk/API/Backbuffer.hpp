/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's backbuffer
 */

#pragma once

#include "../Interface/Backbuffer.hpp"
#include "Texture.hpp"

namespace NFE {
namespace Renderer {


class Backbuffer : public IBackbuffer
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

    struct ImageExtraData
    {
        // to keep some data related to transitioning layouts
        VkImageLayout layout;
    };

    uint32 mWidth;
    uint32 mHeight;
    VkFormat mFormat;
    uint32 mImageNum;
    uint32 mCurrentImage;
    Common::DynArray<VkImage> mImages;
    Common::DynArray<ImageExtraData> mImageExtraDatas;
    VkImageSubresourceRange mImageSubresRange;

    VkColorSpaceKHR mColorSpace;
    uint32 mPresentQueueIndex;
    VkQueue mPresentQueue;
    VkSurfaceCapabilitiesKHR mSurfaceCapabilities;
    VkSwapchainKHR mSwapchain;
    VkPresentModeKHR mSwapPresentMode;
    VkFence mAcquireNextImageFence;

    // platform-specific surface creator
    bool CreateSurface(const BackbufferDesc& desc);

    // right now needed purely by XCB implementation
    void CleanupPlatformSpecifics();

    bool SelectPresentQueue();
    bool SelectSurfaceFormat(const BackbufferDesc& desc);
    bool SelectPresentMode(const BackbufferDesc& desc);
    bool SelectBufferCount();
    bool CreateSwapchain(const BackbufferDesc& desc);
    bool CreateNextImageFence();
    bool AcquireNextImage();

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const BackbufferDesc& desc);

    bool Resize(int newWidth, int newHeight) override;
    bool Present() override;

    void Transition(VkCommandBuffer cb, VkImageLayout dstLayout);
};

} // namespace Renderer
} // namespace NFE
