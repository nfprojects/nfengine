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
    : mSurface(VK_NULL_HANDLE)
    , mColorSpace(VK_COLORSPACE_SRGB_NONLINEAR_KHR)
    , mPresentQueueIndex(UINT32_MAX)
    , mPresentQueue(VK_NULL_HANDLE)
    , mPresentCommandPool(VK_NULL_HANDLE)
    , mPresentCommandBuffer(VK_NULL_HANDLE)
{
}

Backbuffer::~Backbuffer()
{
    if (mPresentCommandBuffer != VK_NULL_HANDLE)
        vkFreeCommandBuffers(gDevice->GetDevice(), mPresentCommandPool, 1, &mPresentCommandBuffer);
    if (mPresentCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(gDevice->GetDevice(), mPresentCommandPool, nullptr);
    if (mSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(gDevice->GetInstance(), mSurface, nullptr);

    CleanupPlatformSpecifics();
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    if (!CreateSurface(desc))
    {
        LOG_ERROR("Failed to create Vulkan Surface.");
        return false;
    }

    // check which queue supports presenting
    uint32 queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, nullptr);

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

    if (mPresentQueueIndex == UINT32_MAX)
    {
        LOG_ERROR("Current physical device has no queue which supports present operations.");
        return false;
    }

    vkGetDeviceQueue(gDevice->GetDevice(), mPresentQueueIndex, 0, &mPresentQueue);

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

    // One entry with VK_FORMAT_UNDEFINED means no preferred formats - select BGRA_UNORM as default
    if ((formatCount == 1) && (formats[0].format == VK_FORMAT_UNDEFINED))
        mFormat = VK_FORMAT_B8G8R8A8_UNORM;
    else
        mFormat = formats[0].format;

    mColorSpace = formats[0].colorSpace;


    // Present command pool and command buffer creation
    VkCommandPoolCreateInfo poolInfo;
    VK_ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mPresentQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(gDevice->GetDevice(), &poolInfo, nullptr, &mPresentCommandPool);
    CHECK_VKRESULT(result, "Unable to create present command pool");

    VkCommandBufferAllocateInfo buffInfo = {};
    VK_ZERO_MEMORY(buffInfo);
    buffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffInfo.commandPool = mPresentCommandPool;
    buffInfo.commandBufferCount = 1;
    buffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(gDevice->GetDevice(), &buffInfo, &mPresentCommandBuffer);
    CHECK_VKRESULT(result, "Unable to allocate present command buffer");


    // Gather possible present mdoes
    uint32 presentModeCount = UINT32_MAX;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                       &presentModeCount, nullptr);
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                       &presentModeCount, presentModes.data());
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

    // By default, assume FIFO present mode (aka vSync enabled)
    VkPresentModeKHR swapPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!desc.vSync)
    {
        // If vsync is to be disabled, go through surface's present modes and find a better one
        for (uint32 i = 0; i < presentModeCount; ++i)
        {
            // Mailbox would be perfect, as it would negate tearing without much latency
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            // ...however, immediate can be used as well if possible (will produce tearing)
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                swapPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    // Gather surface's capabilities
    VkSurfaceCapabilitiesKHR surfCaps;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                       &surfCaps);
    CHECK_VKRESULT(result, "Failed to acquire surface's capabilities");

    // Ideally we should be able to double-buffer, but for safety check this against caps
    mBuffersNum = 2;
    if (mBuffersNum > surfCaps.maxImageCount)
        mBuffersNum = surfCaps.maxImageCount;

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
    swapInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.presentMode = swapPresentMode;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;
    swapInfo.clipped = VK_TRUE;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    result = vkCreateSwapchainKHR(gDevice->GetDevice(), &swapInfo, nullptr, &mSwapchain);
    CHECK_VKRESULT(result, "Failed to create a swapchain");

    uint32 swapImageCount = 0;
    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain, &swapImageCount, nullptr);
    CHECK_VKRESULT(result, "Failed to get swapchain image count");
    if (swapImageCount < mBuffersNum)
    {
        LOG_ERROR("Not enough swap images created (%d, requested %d)", swapImageCount, mBuffersNum);
        return false;
    }
    mBuffersNum = swapImageCount;

    mBuffers.resize(mBuffersNum);
    mBufferViews.resize(mBuffersNum);

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

    mCurrentBuffer = 0;
    mWidth = desc.width;
    mHeight = desc.height;
    mType = TextureType::Texture2D;
    mFromSwapchain = true;

    // Build post-present command buffer
    // It will contain only a PipelineBarrier which will convert our image back to color attachment
    VkCommandBufferBeginInfo cmdBeginInfo;
    VK_ZERO_MEMORY(cmdBeginInfo);
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    result = vkBeginCommandBuffer(mPresentCommandBuffer, &cmdBeginInfo);
    CHECK_VKRESULT(result, "Failed to start recording present command buffer");

    VkImageMemoryBarrier postPresentBarrier;
    VK_ZERO_MEMORY(postPresentBarrier);
    postPresentBarrier.srcAccessMask = 0;
    postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(mPresentCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_FLAGS_NONE,
                         0, nullptr, 0, nullptr,
                         1, &)

    result = vkEndCommandBuffer(mPresentCommandBuffer);
    CHECK_VKRESULT(result, "Error during present command buffer recording");

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
    VkResult result;

    // present current image
    VkPresentInfoKHR presentInfo;
    VK_ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBuffer;
    presentInfo.waitSemaphoreCount = 1;
    const VkSemaphore& sem = gDevice->GetRenderSemaphore();
    presentInfo.pWaitSemaphores = &sem;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    CHECK_VKRESULT(result, "Failed to present image on current swap chain");

    // acquire next image for future use
    // mPresentSemaphore will be waited for in CommandBuffer::Finish to inform
    // about finished present operation
    // TODO maybe move to a separate Backbuffer call?
    vkAcquireNextImageKHR(gDevice->GetDevice(), mSwapchain, UINT64_MAX,
                          gDevice->GetPresentSemaphore(), VK_NULL_HANDLE, &mCurrentBuffer);
    return true;
}

} // namespace Renderer
} // namespace NFE
