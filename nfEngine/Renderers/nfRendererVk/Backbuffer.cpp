/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer for Win32 Vulkan surface
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "Device.hpp"

#include <string.h>


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : Texture()
    , mSurface(VK_NULL_HANDLE)
    , mColorSpace(VK_COLORSPACE_SRGB_NONLINEAR_KHR)
    , mPresentQueueIndex(UINT32_MAX)
    , mPresentQueue(VK_NULL_HANDLE)
    , mSwapchain(VK_NULL_HANDLE)
    , mSwapPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
{
}

Backbuffer::~Backbuffer()
{
    vkQueueWaitIdle(mPresentQueue);

    if (mAcquireFence != VK_NULL_HANDLE)
        vkDestroyFence(gDevice->GetDevice(), mAcquireFence, nullptr);
    if (mSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(gDevice->GetDevice(), mSwapchain, nullptr);
    if (mSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(gDevice->GetInstance(), mSurface, nullptr);

    CleanupPlatformSpecifics();
}

bool Backbuffer::SelectPresentQueue()
{
    // check which queue supports presenting
    uint32 queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOG_ERROR("No queues to choose from for Present operations");
        return false;
    }

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, queueProps.data());

    mPresentQueueIndex = UINT32_MAX;
    for (uint32 i = 0; i < queueCount; ++i)
    {
        VkBool32 supportsPresent;
        vkGetPhysicalDeviceSurfaceSupportKHR(gDevice->GetPhysicalDevice(), i, mSurface,
                                                &supportsPresent);
        if (supportsPresent)
        {
            mPresentQueueIndex = i;
            break;
        }
    }

    LOG_DEBUG("Present queue selected #%u", mPresentQueueIndex);

    if (mPresentQueueIndex == UINT32_MAX)
    {
        LOG_ERROR("Current physical device has no queue which supports present operations.");
        return false;
    }

    vkGetDeviceQueue(gDevice->GetDevice(), mPresentQueueIndex, 0, &mPresentQueue);

    return true;
}

bool Backbuffer::SelectSurfaceFormat(const BackbufferDesc& desc)
{
    // get surface's format
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                  &formatCount, nullptr);
    CHECK_VKRESULT(result, "Unable to retrieve surface format count.");

    if (formatCount == 0)
    {
        LOG_ERROR("No surface formats to choose from.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                  &formatCount, formats.data());
    CHECK_VKRESULT(result, "Unable to retrieve surface formats");
    uint32 formatIndex = 0;
    mFormat = TranslateElementFormatToVkFormat(desc.format);
    // check if format is supported
    // the only exception is having 1 format - VK_FORMAT_UNDEFINED
    // then we can be sure, the physical device doesn't care what format we choose
    if ((formatCount > 1) || (formats[0].format != VK_FORMAT_UNDEFINED))
        for (formatIndex = 0; formatIndex < formatCount; ++formatIndex)
            if (formats[formatIndex].format == mFormat)
                break;

    if (formatIndex == formatCount)
    {
        LOG_ERROR("Requested format for Backbuffer is unsupported.");
        return false;
    }

    mColorSpace = formats[formatIndex].colorSpace;

    return true;
}

bool Backbuffer::SelectPresentMode(const BackbufferDesc& desc)
{
    // Gather possible present mdoes
    uint32 presentModeCount = UINT32_MAX;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                                &presentModeCount, nullptr);
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                       &presentModeCount, presentModes.data());
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

    // By default, assume FIFO present mode (aka vSync enabled)
    mSwapPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!desc.vSync)
    {
        // If vsync is to be disabled, go through surface's present modes and find a better one
        for (uint32 i = 0; i < presentModeCount; ++i)
        {
            // Mailbox would be perfect, as it would negate tearing without much latency
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                mSwapPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            // ...however, immediate can be used as well if possible (will produce tearing)
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                mSwapPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    return true;
}

bool Backbuffer::SelectBufferCount()
{
    // Gather surface's capabilities
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                                &mSurfaceCapabilities);
    CHECK_VKRESULT(result, "Failed to acquire surface's capabilities");

    // Ideally we should be able to double-buffer, but for safety check this against caps
    mBuffersNum = 2;
    if (mSurfaceCapabilities.maxImageCount > 0)
    {
        if (mBuffersNum > mSurfaceCapabilities.maxImageCount)
        {
            LOG_WARNING("Requested %d swapchain image count exceeds max limit %d - reducing",
                        mBuffersNum, mSurfaceCapabilities.maxImageCount);
            mBuffersNum = mSurfaceCapabilities.maxImageCount;
        }
    }

    if (mSurfaceCapabilities.minImageCount > 0)
    {
        if (mBuffersNum < mSurfaceCapabilities.minImageCount)
        {
            LOG_WARNING("Requested %d swapchain image count exceeds minimum limit %d - increasing",
                        mBuffersNum, mSurfaceCapabilities.minImageCount);
            mBuffersNum = mSurfaceCapabilities.minImageCount;
        }
    }

    return true;
}

bool Backbuffer::CreateSwapchain(const BackbufferDesc& desc)
{
    // Create swapchain
    VkSwapchainCreateInfoKHR swapInfo;
    VK_ZERO_MEMORY(swapInfo);
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = mSurface;
    swapInfo.minImageCount = mBuffersNum;
    swapInfo.imageFormat = mFormat;
    swapInfo.imageColorSpace = mColorSpace;
    swapInfo.imageExtent.width = desc.width;
    swapInfo.imageExtent.height = desc.height;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapInfo.preTransform = mSurfaceCapabilities.currentTransform;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.presentMode = mSwapPresentMode;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;
    swapInfo.clipped = VK_TRUE;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkResult result = vkCreateSwapchainKHR(gDevice->GetDevice(), &swapInfo, nullptr, &mSwapchain);
    CHECK_VKRESULT(result, "Failed to create a swapchain");

    return true;
}

bool Backbuffer::CreateSwapchainImageViews()
{
    uint32 swapImageCount = 0;
    VkResult result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &swapImageCount, nullptr);
    CHECK_VKRESULT(result, "Failed to get swapchain image count");
    if (swapImageCount < mBuffersNum)
    {
        LOG_ERROR("Not enough swap images created (%d, requested %d)", swapImageCount, mBuffersNum);
        return false;
    }
    else if (swapImageCount > mBuffersNum)
        LOG_WARNING("Created more swapchain images than requested (%d, requested %d)", swapImageCount, mBuffersNum);

    mBuffersNum = swapImageCount;

    // get images from swapchain and store them (we will need them in Present Command Buffers)
    mImages.resize(mBuffersNum);
    std::vector<VkImage> images(mBuffersNum);
    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &mBuffersNum, images.data());
    CHECK_VKRESULT(result, "Failed to get swapchain images");

    LOG_DEBUG("%d swapchain buffers acquired", mBuffersNum);
    for (mCurrentBuffer = 0; mCurrentBuffer < mBuffersNum; mCurrentBuffer++)
    {
        VkImageViewCreateInfo ivInfo;
        VK_ZERO_MEMORY(ivInfo);
        ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivInfo.image = images[mCurrentBuffer];
        ivInfo.format = mFormat;
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivInfo.components = {
            // order of variables in VkComponentMapping is r, g, b, a
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
        };
        ivInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivInfo.subresourceRange.baseMipLevel = 0;
        ivInfo.subresourceRange.levelCount = 1;
        ivInfo.subresourceRange.baseArrayLayer = 0;
        ivInfo.subresourceRange.layerCount = 1;
        result = vkCreateImageView(gDevice->GetDevice(), &ivInfo, nullptr, &mImages[mCurrentBuffer].view);
        CHECK_VKRESULT(result, "Failed to generate Image View from Swapchain image");

        mImages[mCurrentBuffer].image = images[mCurrentBuffer];
    }

    return true;
}

bool Backbuffer::BuildAcquireFence()
{
    VkFenceCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &mAcquireFence);
    CHECK_VKRESULT(result, "Failed to create image acquisition fence");

    return true;
}

bool Backbuffer::AcquireNextImage()
{
    // TODO handle VK_ERROR_OUT_OF_DATE (happens ex. during resize)
    VkResult result = vkAcquireNextImageKHR(gDevice->GetDevice(), mSwapchain, UINT64_MAX,
                                            VK_NULL_HANDLE, mAcquireFence, &mCurrentBuffer);
    CHECK_VKRESULT(result, "Failed to acquire next image");

    // The best position for this wait would be before first submission. Unfortunately
    // we have no access to this fence from Device level, so we'll wait for the image here.
    vkWaitForFences(gDevice->GetDevice(), 1, &mAcquireFence, VK_TRUE, UINT64_MAX);
    vkResetFences(gDevice->GetDevice(), 1, &mAcquireFence);

    gDevice->SignalPresent();

    return true;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    if (!CreateSurface(desc))
    {
        LOG_ERROR("Failed to create Vulkan Surface.");
        return false;
    }

    if (!SelectPresentQueue()) return false;
    if (!SelectSurfaceFormat(desc)) return false;
    if (!SelectPresentMode(desc)) return false;
    if (!SelectBufferCount()) return false;
    if (!CreateSwapchain(desc)) return false;
    if (!CreateSwapchainImageViews()) return false;

    mWidth = desc.width;
    mHeight = desc.height;
    mType = TextureType::Texture2D;
    mFromSwapchain = true;
    // TODO temporary to prevent crashes, extract from Swapchain when multisampling is added to desc
    mSamplesNum = VK_SAMPLE_COUNT_1_BIT;
    mSubresRange.baseMipLevel = 0;
    mSubresRange.levelCount = 1;
    mSubresRange.baseArrayLayer = 0;
    mSubresRange.layerCount = 1;
    mSubresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    for (auto& image: mImages)
    {
        image.defaultImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        image.currentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (!BuildAcquireFence()) return false;
    if (!AcquireNextImage()) return false;

    LOG_INFO("Backbuffer initialized successfully.");
    return true;
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    mWidth = newWidth;
    mHeight = newHeight;

    // TODO
    return true;
}

bool Backbuffer::Present()
{
    // our semaphore to wait for will be the last used from SemaphorePool
    VkSemaphore lastRenderSem = gDevice->GetSemaphorePool()->GetCurrentSemaphore();
    VkResult result = VK_SUCCESS;

    // Present whatever was drawn (only if render semaphore is signalled)
    VkPresentInfoKHR presentInfo;
    VK_ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBuffer;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &lastRenderSem;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    //CHECK_VKRESULT(result, "Failed to present image on current swap chain");
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to present image on current swap chain: %d (%s)", result,
                  TranslateVkResultToString(result));
    }

    return AcquireNextImage();
}

} // namespace Renderer
} // namespace NFE
