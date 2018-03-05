/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer for Xcb Vulkan surface
 */

#include "PCH.hpp"
#include "../Backbuffer.hpp"
#include "../Device.hpp"


namespace NFE {
namespace Renderer {

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    // Init XCB connection
    // TODO use connection provided by Window
    mConnection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(mConnection))
    {
        NFE_LOG_ERROR("Failed to connect to X11");
        return false;
    }

    VkXcbSurfaceCreateInfoKHR surfInfo;
    VK_ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfInfo.connection = mConnection;
    surfInfo.window = static_cast<xcb_window_t>(reinterpret_cast<uintptr_t>(desc.windowHandle));
    VkResult result = vkCreateXcbSurfaceKHR(mDevicePtr->GetInstance(), &surfInfo, nullptr, &mSurface);

    return (result == VK_SUCCESS);
}

void Backbuffer::CleanupPlatformSpecifics()
{
    xcb_disconnect(mConnection);
}

} // namespace Renderer
} // namespace NFE
