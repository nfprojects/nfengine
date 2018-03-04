/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Device for Win32 Vulkan surface
 */

#include "../PCH.hpp"
#include "../Device.hpp"


namespace NFE {
namespace Renderer {

bool Device::CreateTemporarySurface(VkSurfaceKHR& surface)
{
    if (!mInvisibleWindow.Init())
        return false;

    mInvisibleWindow.SetInvisible(true);

    if (!mInvisibleWindow.Open())
        return false;

    VkWin32SurfaceCreateInfoKHR surfInfo;
    VK_ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hwnd = reinterpret_cast<HWND>(mInvisibleWindow.GetHandle());
    surfInfo.hinstance = GetModuleHandle(0);
    VkResult result = vkCreateWin32SurfaceKHR(mInstance.Get(), &surfInfo, nullptr, &surface);

    return (result == VK_SUCCESS);
}

void Device::CleanupTemporarySurface(VkSurfaceKHR& surface)
{
    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(mInstance.Get(), surface, nullptr);

    mInvisibleWindow.Close();
}


} // namespace Renderer
} // namespace NFE
