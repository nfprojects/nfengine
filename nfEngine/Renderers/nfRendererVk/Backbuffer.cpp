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
    , mImageAcquired(false)
{
}

Backbuffer::~Backbuffer()
{
    vkQueueWaitIdle(mPresentQueue);

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
        NFE_LOG_ERROR("No queues to choose from for Present operations");
        return false;
    }

    Common::DynArray<VkQueueFamilyProperties> queueProps;
    if (queueProps.Resize(queueCount))
    {
        vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, queueProps.Data());
    }

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

    NFE_LOG_DEBUG("Present queue selected #%u", mPresentQueueIndex);

    if (mPresentQueueIndex == UINT32_MAX)
    {
        NFE_LOG_ERROR("Current physical device has no queue which supports present operations.");
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
    VK_RETURN_FALSE_IF_FAILED(result, "Unable to retrieve surface format count.");

    if (formatCount == 0)
    {
        NFE_LOG_ERROR("No surface formats to choose from.");
        return false;
    }

    Common::DynArray<VkSurfaceFormatKHR> formats;
    if (formats.Resize(formatCount))
    {
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                      &formatCount, formats.Data());
    }

    VK_RETURN_FALSE_IF_FAILED(result, "Unable to retrieve surface formats");
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
        NFE_LOG_ERROR("Requested format for Backbuffer is unsupported.");
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
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to acquire surface's present modes");

    Common::DynArray<VkPresentModeKHR> presentModes;
    if (presentModes.Resize(presentModeCount))
    {
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                           &presentModeCount, presentModes.Data());
    }
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to acquire surface's present modes");

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
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to acquire surface's capabilities");

    // Ideally we should be able to double-buffer, but for safety check this against caps
    mBuffersNum = 2;
    if (mSurfaceCapabilities.maxImageCount > 0)
    {
        if (mBuffersNum > mSurfaceCapabilities.maxImageCount)
        {
            NFE_LOG_WARNING("Requested %d swapchain image count exceeds max limit %d - reducing",
                        mBuffersNum, mSurfaceCapabilities.maxImageCount);
            mBuffersNum = mSurfaceCapabilities.maxImageCount;
        }
    }

    if (mSurfaceCapabilities.minImageCount > 0)
    {
        if (mBuffersNum < mSurfaceCapabilities.minImageCount)
        {
            NFE_LOG_WARNING("Requested %d swapchain image count exceeds minimum limit %d - increasing",
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
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create a swapchain");

    return true;
}

bool Backbuffer::CreateSwapchainImageViews()
{
    uint32 swapImageCount = 0;
    VkResult result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &swapImageCount, nullptr);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to get swapchain image count");
    if (swapImageCount < mBuffersNum)
    {
        NFE_LOG_ERROR("Not enough swap images created (%d, requested %d)", swapImageCount, mBuffersNum);
        return false;
    }
    else if (swapImageCount > mBuffersNum)
        NFE_LOG_WARNING("Created more swapchain images than requested (%d, requested %d)", swapImageCount, mBuffersNum);

    // replace buffers count with actual number we acquired from Swapchain
    mBuffersNum = swapImageCount;

    Common::DynArray<VkImage> images;
    images.Resize(mBuffersNum);
    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &mBuffersNum, images.Data());
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to get swapchain images");

    mImages.Resize(mBuffersNum);
    NFE_LOG_DEBUG("%d swapchain buffers acquired", mBuffersNum);

    for (mCurrentBuffer = 0; mCurrentBuffer < mBuffersNum; mCurrentBuffer++)
    {
        mImages[mCurrentBuffer].image = images[mCurrentBuffer];

        VkImageViewCreateInfo ivInfo;
        VK_ZERO_MEMORY(ivInfo);
        ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivInfo.image = mImages[mCurrentBuffer].image;
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
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to generate Image View from Swapchain image");
    }

    mCurrentBuffer = 0;
    return true;
}

bool Backbuffer::AcquireNextImage()
{
    // prevent reacquisition of Image within same frame
    // flag is reset on Present() operation
    if (mImageAcquired)
        return;

    // TODO handle VK_ERROR_OUT_OF_DATE (happens ex. during resize)
    VkResult result = vkAcquireNextImageKHR(gDevice->GetDevice(), mSwapchain, UINT64_MAX,
                                            VK_NULL_HANDLE, VK_NULL_HANDLE, &mCurrentBuffer);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to acquire next image");

    mImageAcquired = true;

    return true;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    if (!CreateSurface(desc))
    {
        NFE_LOG_ERROR("Failed to create Vulkan Surface.");
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

    NFE_LOG_INFO("Backbuffer initialized successfully.");
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
    // Present whatever was drawn
    VkResult result;
    VkPresentInfoKHR presentInfo;
    VK_ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBuffer;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    //VK_RETURN_FALSE_IF_FAILED(result, "Failed to present image on current swap chain");
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Failed to present image on current swap chain: %d (%s)", result,
                      TranslateVkResultToString(result));
    }

    mImageAcquired = false;
    return true;
}

} // namespace Renderer
} // namespace NFE
