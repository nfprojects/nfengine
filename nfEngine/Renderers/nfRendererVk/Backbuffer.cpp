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
    , mPresentCommandPool(VK_NULL_HANDLE)
    , mSwapPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
{
}

Backbuffer::~Backbuffer()
{
    vkQueueWaitIdle(mPresentQueue);

    if (mPresentCommandBuffers.size())
        vkFreeCommandBuffers(gDevice->GetDevice(), mPresentCommandPool,
                             static_cast<uint32>(mPresentCommandBuffers.size()), mPresentCommandBuffers.data());

    if (mPresentCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(gDevice->GetDevice(), mPresentCommandPool, nullptr);
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

    mBuffers.resize(mBuffersNum);
    mBufferViews.resize(mBuffersNum);
    mPresentCommandBuffers.resize(mBuffersNum);

    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &mBuffersNum, mBuffers.data());
    CHECK_VKRESULT(result, "Failed to get swapchain images");

    LOG_DEBUG("%d swapchain buffers acquired", mBuffersNum);
    for (mCurrentBuffer = 0; mCurrentBuffer < mBuffersNum; mCurrentBuffer++)
    {
        VkImageViewCreateInfo ivInfo;
        VK_ZERO_MEMORY(ivInfo);
        ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivInfo.image = mBuffers[mCurrentBuffer];
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
        result = vkCreateImageView(gDevice->GetDevice(), &ivInfo, nullptr, &mBufferViews[mCurrentBuffer]);
        CHECK_VKRESULT(result, "Failed to generate Image View from Swapchain image");
    }

    return true;
}

bool Backbuffer::BuildPresentCommandBuffers()
{
    // Present command pool and command buffer creation
    VkCommandPoolCreateInfo poolInfo;
    VK_ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mPresentQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult result = vkCreateCommandPool(gDevice->GetDevice(), &poolInfo, nullptr, &mPresentCommandPool);
    CHECK_VKRESULT(result, "Unable to create present command pool");

    VkCommandBufferAllocateInfo buffInfo = {};
    VK_ZERO_MEMORY(buffInfo);
    buffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffInfo.commandPool = mPresentCommandPool;
    buffInfo.commandBufferCount = mBuffersNum;
    buffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(gDevice->GetDevice(), &buffInfo, mPresentCommandBuffers.data());
    CHECK_VKRESULT(result, "Unable to allocate present command buffer");

    // Build post-present command buffers
    // These will contain only a PipelineBarrier which will convert our image back to color attachment
    VkImageMemoryBarrier postPresentBarrier;
    VK_ZERO_MEMORY(postPresentBarrier);
    postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postPresentBarrier.srcAccessMask = 0;
    postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    postPresentBarrier.subresourceRange.baseMipLevel = 0;
    postPresentBarrier.subresourceRange.levelCount = 1;
    postPresentBarrier.subresourceRange.baseArrayLayer = 0;
    postPresentBarrier.subresourceRange.layerCount = 1;

    VkCommandBufferBeginInfo cmdBeginInfo;
    VK_ZERO_MEMORY(cmdBeginInfo);
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    for (uint32 i = 0; i < mBuffersNum; ++i)
    {
        result = vkBeginCommandBuffer(mPresentCommandBuffers[i], &cmdBeginInfo);
        CHECK_VKRESULT(result, "Failed to start recording present command buffer");

        postPresentBarrier.image = mBuffers[i];

        vkCmdPipelineBarrier(mPresentCommandBuffers[i], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                             0, nullptr, 0, nullptr,
                             1, &postPresentBarrier);

        result = vkEndCommandBuffer(mPresentCommandBuffers[i]);
        CHECK_VKRESULT(result, "Error during present command buffer recording");
    }

    return true;
}

bool Backbuffer::AcquireNextImage()
{
    // TODO handle VK_ERROR_OUT_OF_DATE (happens ex. during resize)
    VkResult result = vkAcquireNextImageKHR(gDevice->GetDevice(), mSwapchain, UINT64_MAX, gDevice->mPresentSemaphore,
                                            VK_NULL_HANDLE, &mCurrentBuffer);
    CHECK_VKRESULT(result, "Failed to acquire next image");

    // Submit a pipeline barrier call to ensure our buffer is a color attachment
    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo;
    VK_ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mPresentCommandBuffers[mCurrentBuffer];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &gDevice->mPresentSemaphore;
    submitInfo.pWaitDstStageMask = &pipelineStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &gDevice->mPostPresentSemaphore;
    result = vkQueueSubmit(mPresentQueue, 1, &submitInfo, VK_NULL_HANDLE);
    CHECK_VKRESULT(result, "Failed to submit present operations");

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

    if (!BuildPresentCommandBuffers()) return false;
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
    VkSemaphore renderSem = gDevice->GetSemaphorePool()->GetCurrentSemaphore();

    // Present whatever was drawn (only if render semaphore is signalled)
    VkResult result;
    VkPresentInfoKHR presentInfo;
    VK_ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBuffer;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderSem;
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
