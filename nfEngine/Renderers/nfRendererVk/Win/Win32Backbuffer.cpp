#include "../PCH.hpp"
#include "../Backbuffer.hpp"

#include "../Device.hpp"

namespace NFE {
namespace Renderer {

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    mHWND = (HWND)desc.windowHandle;

    VkWin32SurfaceCreateInfoKHR surfInfo;
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hwnd = mHWND;
    surfInfo.hinstance = (HINSTANCE)GetWindowLongPtr(mHWND, GWLP_HINSTANCE);
    VkResult result = vkCreateWin32SurfaceKHR(gDevice->GetInstance(), &surfInfo, nullptr, &mVkSurface);

    return (result == VK_SUCCESS);
}

void Backbuffer::CleanupPlatformSpecifics()
{
    // nothing to clean here
}


} // namespace Renderer
} // namespace NFE
