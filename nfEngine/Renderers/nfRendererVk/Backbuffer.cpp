/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer for Win32 Vulkan surface
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"
#include "Device.hpp"

namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
{
}

Backbuffer::~Backbuffer()
{
    vkDestroySurfaceKHR(gDevice->GetInstance(), mVkSurface, nullptr);
    CleanupPlatformSpecifics();
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    mWidth = newWidth;
    mHeight = newHeight;

    // TODO
    return true;
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
        vkGetPhysicalDeviceSurfaceSupportKHR(gDevice->GetPhysicalDevice(), i, mVkSurface,
                                                &supportsPresent);
        if (supportsPresent)
        {
            mPresentQueueIndex = i;
            break;
        }
    }

    if (mPresentQueueIndex == UINT32_MAX)
    {
        LOG_ERROR("Current physical device has no queue which supports both graphics and present.");
        return false;
    }

    // get surface's format
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mVkSurface,
                                                  &formatCount, nullptr);
    if (result != VK_SUCCESS || formatCount == 0)
    {
        LOG_ERROR("Unable to retrieve surface format count.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mVkSurface,
                                                  &formatCount, formats.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Unable to retrieve surface formats.");
        return false;
    }

    // One entry with VK_FORMAT_UNDEFINED means no preferred formats - select BGRA_UNORM as default
    if ((formatCount == 1) && (formats[0].format == VK_FORMAT_UNDEFINED))
        mFormat = VK_FORMAT_B8G8R8A8_UNORM;
    else
        mFormat = formats[0].format;

    mColorSpace = formats[0].colorSpace;

    // TODO Present VkCommandPool and VkCommandBuffer

    LOG_INFO("Backbuffer initialized successfully.");
    return true;

}

bool Backbuffer::Present()
{
    return true;
}

} // namespace Renderer
} // namespace NFE
