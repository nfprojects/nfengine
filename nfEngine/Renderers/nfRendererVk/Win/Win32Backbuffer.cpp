/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer for Win32 Vulkan surface
 */

#include "../PCH.hpp"
#include "../Backbuffer.hpp"
#include "../Device.hpp"

namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
{
}

Backbuffer::~Backbuffer()
{
    vkDestroySurfaceKHR(gDevice->GetInstance(), mVkSurface, nullptr);
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
    mHWND = (HWND)desc.windowHandle;

    VkWin32SurfaceCreateInfoKHR surfInfo;
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hwnd = mHWND;
    surfInfo.hinstance = (HINSTANCE)GetWindowLongPtr(mHWND, GWLP_HINSTANCE);
    VkResult result = vkCreateWin32SurfaceKHR(gDevice->GetInstance(), &surfInfo, nullptr, &mVkSurface);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create Vulkan Win32 surface.");
        return false;
    }

    // check which queue supports presenting
    uint32 queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, queueProps.data());

    // TODO it might be a good idea to find separate graphics and present queue
    //      For now we search for queue that supports both at once.
    mGraphicsAndPresentQueueIndex = UINT32_MAX;
    for (uint32 i = 0; i < queueCount; ++i)
    {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 supportsPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(gDevice->GetPhysicalDevice(), i, mVkSurface,
                                                 &supportsPresent);
            if (supportsPresent)
            {
                mGraphicsAndPresentQueueIndex = i;
            }
        }
    }

    if (mGraphicsAndPresentQueueIndex == UINT32_MAX)
    {
        LOG_ERROR("Current physical device has no queue which supports both graphics and present.");
        return false;
    }

    /*
        // Get list of supported surface formats
    uint32_t formatCount;
    err = fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    assert(!err);
    assert(formatCount > 0);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    err = fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
    assert(!err);

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        // Always select the first available color format
        // If you need a specific format (e.g. SRGB) you'd need to
        // iterate over the list of available surface format and
        // check for it's presence
        colorFormat = surfaceFormats[0].format;
    }
    colorSpace = surfaceFormats[0].colorSpace;*/

    return true;
}

bool Backbuffer::Present()
{
    return true;
}

} // namespace Renderer
} // namespace NFE
