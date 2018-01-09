/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Device for Xcb Vulkan surface
 */

#include "../PCH.hpp"
#include "../Device.hpp"

namespace {
    xcb_connection_t* xcbConnection = nullptr;
} // namespace

namespace NFE {
namespace Renderer {


bool Device::CreateTemporarySurface(VkSurfaceKHR& surface)
{
    // Init XCB connection
    // TODO use connection provided by Window
    xcbConnection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(xcbConnection))
    {
        NFE_LOG_ERROR("Failed to connect to X11");
        return false;
    }

    VkXcbSurfaceCreateInfoKHR surfInfo;
    VK_ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfInfo.connection = xcbConnection;
    VkResult result = vkCreateXcbSurfaceKHR(gDevice->GetInstance(), &surfInfo, nullptr, &surface);

    return (result == VK_SUCCESS);
}

void Device::CleanupTemporarySurface(VkSurfaceKHR& surface)
{
    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(mInstance.Get(), surface, nullptr);

    xcb_disconnect(xcbConnection);
    xcbConnection = nullptr;
}


} // namespace Renderer
} // namespace NFE
