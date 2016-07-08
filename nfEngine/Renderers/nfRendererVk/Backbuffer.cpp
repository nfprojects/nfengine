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
    , mFormat(VK_FORMAT_UNDEFINED)
    , mColorSpace(VK_COLORSPACE_SRGB_NONLINEAR_KHR)
    , mPresentQueueIndex(UINT32_MAX)
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

    // get surface's format
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                  &formatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_VKRESULT("Unable to retrieve surface format count.", result);
        return false;
    }
    if (formatCount == 0)
    {
        LOG_ERROR("No surface formats to choose from.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                  &formatCount, formats.data());
    if (result != VK_SUCCESS)
    {
        LOG_VKRESULT("Unable to retrieve surface formats.", result);
        return false;
    }

    // One entry with VK_FORMAT_UNDEFINED means no preferred formats - select BGRA_UNORM as default
    if ((formatCount == 1) && (formats[0].format == VK_FORMAT_UNDEFINED))
        mFormat = VK_FORMAT_B8G8R8A8_UNORM;
    else
        mFormat = formats[0].format;

    mColorSpace = formats[0].colorSpace;


    // present command pool and command buffer creation
    VkCommandPoolCreateInfo poolInfo;
    memset(&poolInfo, 0, sizeof(poolInfo));
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mPresentQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(gDevice->GetDevice(), &poolInfo, nullptr, &mPresentCommandPool);
    if (result != VK_SUCCESS)
    {
        LOG_VKRESULT("Unable to create present command pool", result);
        return false;
    }

    VkCommandBufferAllocateInfo buffInfo = {};
    memset(&buffInfo, 0, sizeof(buffInfo));
    buffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffInfo.commandPool = mPresentCommandPool;
    buffInfo.commandBufferCount = 1;
    buffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(gDevice->GetDevice(), &buffInfo, &mPresentCommandBuffer);
    if (result != VK_SUCCESS)
    {
        LOG_VKRESULT("Unable to allocate present command buffer", result);
        return false;
    }


    // swapchain
    uint32 presentModeCount = UINT32_MAX;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                       &presentModeCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_VKRESULT("Failed to acquire surface's present modes", result);
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->GetPhysicalDevice(), mSurface,
                                                       &presentModeCount, presentModes.data());
    if (result != VK_SUCCESS)
    {
        LOG_VKRESULT("Failed to acquire surface's present modes", result);
        return false;
    }

    // by default, assume FIFO present mode (aka vSync enabled)
    VkPresentModeKHR swapPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!desc.vSync)
    {
        // if vsync is to be disabled, go through surface's present modes and find a better one
        for (uint32 i = 0; i < presentModeCount; ++i)
        {
            // mailbox would be perfect, as it would negate tearing
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            // ...however, immediate can be used as well if possible (will tear)
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                swapPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    VkSwapchainCreateInfoKHR swapInfo;
    memset(&swapInfo, 0, sizeof(swapInfo));
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = mSurface;
    swapInfo.minImageCount = /**/;
    swapInfo.imageFormat = mFormat;
    swapInfo.imageColorSpace = mColorSpace;
    swapInfo.imageExtent = { desc.width, desc.height };
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.presentMode = swapPresentMode;

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
    return true;
}

} // namespace Renderer
} // namespace NFE
