/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer for Win32 Vulkan surface
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "Device.hpp"
#include "Internal/Debugger.hpp"

#include <Engine/Common/Utils/StringUtils.hpp>
#include <string.h>


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mSurface(VK_NULL_HANDLE)
    , mWidth(0)
    , mHeight(0)
    , mFormat(VK_FORMAT_UNDEFINED)
    , mImageNum(0)
    , mCurrentImage(0)
    , mImages()
    , mResourceIDs()
    , mColorSpace(VK_COLORSPACE_SRGB_NONLINEAR_KHR)
    , mPresentQueueIndex(UINT32_MAX)
    , mPresentQueue(VK_NULL_HANDLE)
    , mSwapchain(VK_NULL_HANDLE)
    , mSwapPresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR)
{
}

Backbuffer::~Backbuffer()
{
    vkQueueWaitIdle(mPresentQueue);

    if (mAcquireNextImageFence != VK_NULL_HANDLE)
        vkDestroyFence(gDevice->GetDevice(), mAcquireNextImageFence, nullptr);
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
    CHECK_VKRESULT(result, "Unable to retrieve surface format count.");

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

    CHECK_VKRESULT(result, "Unable to retrieve surface formats");
    uint32 formatIndex = 0;
    mFormat = TranslateFormatToVkFormat(desc.format);
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
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

    Common::DynArray<VkPresentModeKHR> presentModes;
    if (presentModes.Resize(presentModeCount))
    {
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                           &presentModeCount, presentModes.Data());
    }
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

    // TODO vSyncInterval support
    mSwapPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (desc.vSyncInterval == 0)
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
    mImageNum = 2;
    if (mSurfaceCapabilities.maxImageCount > 0)
    {
        if (mImageNum > mSurfaceCapabilities.maxImageCount)
        {
            NFE_LOG_WARNING("Requested %d swapchain image count exceeds max limit %d - reducing",
                        mImageNum, mSurfaceCapabilities.maxImageCount);
            mImageNum = mSurfaceCapabilities.maxImageCount;
        }
    }

    if (mSurfaceCapabilities.minImageCount > 0)
    {
        if (mImageNum < mSurfaceCapabilities.minImageCount)
        {
            NFE_LOG_WARNING("Requested %d swapchain image count exceeds minimum limit %d - increasing",
                        mImageNum, mSurfaceCapabilities.minImageCount);
            mImageNum = mSurfaceCapabilities.minImageCount;
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
    swapInfo.minImageCount = mImageNum;
    swapInfo.imageFormat = mFormat;
    swapInfo.imageColorSpace = mColorSpace;
    swapInfo.imageExtent.width = desc.width;
    swapInfo.imageExtent.height = desc.height;
    swapInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapInfo.preTransform = mSurfaceCapabilities.currentTransform;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.presentMode = mSwapPresentMode;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;
    swapInfo.clipped = VK_TRUE;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkResult result = vkCreateSwapchainKHR(gDevice->GetDevice(), &swapInfo, nullptr, &mSwapchain);
    CHECK_VKRESULT(result, "Failed to create a swapchain");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mSwapchain), VK_OBJECT_TYPE_SWAPCHAIN_KHR, desc.debugName);

    uint32 swapImageCount = 0;
    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &swapImageCount, nullptr);
    CHECK_VKRESULT(result, "Failed to get swapchain image count");
    if (swapImageCount < mImageNum)
    {
        NFE_LOG_ERROR("Not enough swap images created (%d, requested %d)", swapImageCount, mImageNum);
        return false;
    }
    else if (swapImageCount > mImageNum)
        NFE_LOG_WARNING("Created more swapchain images than requested (%d, requested %d)", swapImageCount, mImageNum);

    mImageNum = swapImageCount;

    mImages.Resize(mImageNum);
    mResourceIDs.Resize(mImageNum);

    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &mImageNum, mImages.Data());
    CHECK_VKRESULT(result, "Failed to get swapchain images");

    mImageSubresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mImageSubresRange.baseArrayLayer = 0;
    mImageSubresRange.baseMipLevel = 0;
    mImageSubresRange.layerCount = 1;
    mImageSubresRange.levelCount = 1;

    NFE_LOG_DEBUG("%d swapchain buffers acquired", mImageNum);

    if (desc.debugName)
    {
        Common::String imageNamePrefix(desc.debugName);
        imageNamePrefix += "-Image";
        for (uint32 i = 0; i < mImageNum; ++i)
        {
            Common::String name = imageNamePrefix + Common::ToString(i);
            Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mImages[i]), VK_OBJECT_TYPE_IMAGE, name.Str());
        }
    }

    return true;
}

bool Backbuffer::CreateNextImageFence()
{
    VkFenceCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &mAcquireNextImageFence);
    CHECK_VKRESULT(result, "Failed to create next image fence");

    // FIXME this name gets free'd (somehow) on vkAcquireNextImageKHR at nVidia's Linux driver,
    //       which causes a plenty memory issues and crashes. Uncomment when fixed.
    //Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mAcquireNextImageFence), VK_OBJECT_TYPE_FENCE, "AcquireNextImageFence");

    return true;
}

bool Backbuffer::AcquireNextImage()
{
    // TODO handle VK_ERROR_OUT_OF_DATE (happens ex. during resize)
    VkResult result = vkAcquireNextImageKHR(gDevice->GetDevice(), mSwapchain, UINT64_MAX,
                                            VK_NULL_HANDLE, mAcquireNextImageFence, &mCurrentImage);
    CHECK_VKRESULT(result, "Failed to acquire next image");

    result = vkWaitForFences(gDevice->GetDevice(), 1, &mAcquireNextImageFence, VK_TRUE, UINT64_MAX);
    CHECK_VKRESULT(result, "Failed to wait for next image");

    result = vkResetFences(gDevice->GetDevice(), 1, &mAcquireNextImageFence);
    CHECK_VKRESULT(result, "Failed to reset next image fence");

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
    if (!CreateNextImageFence()) return false;

    mWidth = desc.width;
    mHeight = desc.height;

    for (uint32 i = 0; i < mImages.Size(); ++i)
    {
        mResourceIDs[i] = gDevice->GetLayoutTracker().Register(mImages[i], mImageSubresRange);
    }

    if (!AcquireNextImage()) return false;

    NFE_LOG_INFO("Backbuffer initialized successfully.");
    return true;
}

bool Backbuffer::Resize(uint32 newWidth, uint32 newHeight)
{
    mWidth = newWidth;
    mHeight = newHeight;

    // TODO
    return true;
}

bool Backbuffer::Present()
{
    VkResult result = VK_SUCCESS;

    VkPresentInfoKHR presentInfo;
    VK_ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentImage;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    CHECK_VKRESULT(result, "Failed to present image on current swap chain");

    return AcquireNextImage();
}
/*
void Backbuffer::Transition(VkCommandBuffer cb, VkImageLayout dstLayout)
{
    VkImage& img = mImages[mCurrentImage];
    ImageExtraData& data = mImageExtraDatas[mCurrentImage];

    // no need to transition if destination is the same
    if (dstLayout == data.layout)
        return;

    // TODO take access masks and pipeline stages into account
    VkImageMemoryBarrier imageBarrier;
    VK_ZERO_MEMORY(imageBarrier);
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = 0;
    imageBarrier.oldLayout = data.layout;
    imageBarrier.newLayout = dstLayout;
    imageBarrier.image = img;
    imageBarrier.subresourceRange = mImageSubresRange;

    // assume all barriers are full blocking, like in D3D12 renderer
    vkCmdPipelineBarrier(cb,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &imageBarrier);

    data.layout = dstLayout;
}*/

} // namespace Renderer
} // namespace NFE
