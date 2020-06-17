/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Device for Xcb Vulkan surface
 */

#include "PCH.hpp"
#include "../Device.hpp"

namespace {
    xcb_connection_t* xcbConnection = nullptr;
    xcb_window_t xcbTempWindow = 0;
} // namespace

namespace NFE {
namespace Renderer {


bool Device::CreateTemporarySurface(VkSurfaceKHR& surface)
{
    // TODO use connection and window provided by Common::Window

    // Init XCB connection
    int connScreen = 0;
    xcbConnection = xcb_connect(nullptr, &connScreen);
    if (xcb_connection_has_error(xcbConnection))
    {
        NFE_LOG_ERROR("Failed to connect to X11");
        return false;
    }

    // find root screen
    const xcb_setup_t* xcbSetup;
    xcb_screen_iterator_t xcbIt;

    xcbSetup = xcb_get_setup(xcbConnection);
    xcbIt = xcb_setup_roots_iterator(xcbSetup);
    while (connScreen-- > 0)
        xcb_screen_next(&xcbIt);
    xcb_screen_t* screen = xcbIt.data;

    // create a temporary unmapped window
    xcbTempWindow = xcb_generate_id(xcbConnection);
    xcb_void_cookie_t cookie = xcb_create_window(xcbConnection,
                XCB_COPY_FROM_PARENT,
                xcbTempWindow,
                screen->root,
                0, 0,
                1, 1, 0,
                XCB_WINDOW_CLASS_COPY_FROM_PARENT,
                screen->root_visual,
                0, nullptr);

    xcb_generic_error_t* err = xcb_request_check(xcbConnection, cookie);
    if (err)
    {
        NFE_LOG_ERROR("Failed to create a temporary window: XCB protocol error %d", err->error_code);
        free(err);
        return false;
    }

    xcb_flush(xcbConnection);

    // create a temporary surface
    VkXcbSurfaceCreateInfoKHR surfInfo;
    VK_ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfInfo.connection = xcbConnection;
    surfInfo.window = xcbTempWindow;
    VkResult result = vkCreateXcbSurfaceKHR(gDevice->GetInstance(), &surfInfo, nullptr, &surface);

    return (result == VK_SUCCESS);
}

void Device::CleanupTemporarySurface(VkSurfaceKHR& surface)
{
    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(mInstance.Get(), surface, nullptr);

    xcb_void_cookie_t cookie = xcb_destroy_window(xcbConnection, xcbTempWindow);
    xcb_generic_error_t* err = xcb_request_check(xcbConnection, cookie);
    if (err)
    {
        NFE_LOG_ERROR("Failed to destroy temporary window: XCB protocol error %d", err->error_code);
        free(err);
        return;
    }

    xcb_flush(xcbConnection);

    xcb_disconnect(xcbConnection);
    xcbTempWindow = 0;
    xcbConnection = nullptr;
}


} // namespace Renderer
} // namespace NFE
