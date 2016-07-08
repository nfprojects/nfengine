/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer for Xcb Vulkan surface
 */

#include "../PCH.hpp"
#include "../Backbuffer.hpp"
#include "../Device.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mConnection(nullptr)
    , mFormat(VK_FORMAT_UNDEFINED)
    , mColorSpace(VK_COLOR_SPACE_MAX_ENUM_KHR)
    , mPresentQueueIndex(0)
{
}

Backbuffer::~Backbuffer()
{
    vkDestroySurfaceKHR(gDevice->GetInstance(), mVkSurface, nullptr);
    xcb_disconnect(mConnection);
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
    // Init XCB connection
    // TODO use connection provided by Window
    mConnection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(mConnection))
    {
        LOG_ERROR("Failed to connect to X11");
        return false;
    }

    VkXcbSurfaceCreateInfoKHR surfInfo;
    surfInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfInfo.connection = mConnection;
    surfInfo.window = static_cast<xcb_window_t>(reinterpret_cast<uintptr_t>(desc.windowHandle));
    VkResult result = vkCreateXcbSurfaceKHR(gDevice->GetInstance(), &surfInfo, nullptr, &mVkSurface);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create Vulkan Xcb surface.");
        return false;
    }

    // check which queue supports presenting
    uint32 queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->GetPhysicalDevice(), &queueCount,
                                             queueProps.data());

    // TODO it might be a good idea to find separate graphics and present queue
    //      For now we search for queue that supports both at once.
    mPresentQueueIndex = UINT32_MAX;
    for (uint32 i = 0; i < queueCount; ++i)
    {
        VkBool32 supportsPresent;
        vkGetPhysicalDeviceSurfaceSupportKHR(gDevice->GetPhysicalDevice(), i, mVkSurface,
                                             &supportsPresent);
        if (supportsPresent)
            mPresentQueueIndex = i;
    }

    if (mPresentQueueIndex == UINT32_MAX)
    {
        LOG_ERROR("Current physical device has no queue which supports present operations.");
        return false;
    }

    // get surface's format
    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->GetPhysicalDevice(), mVkSurface,
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

    LOG_INFO("XCB Backbuffer initialized successfully.");
    return true;
}

bool Backbuffer::Present()
{
    return true;
}

} // namespace Renderer
} // namespace NFE
