/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

#include "GetProc.hpp"

#include "nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = VK_NULL_HANDLE;

PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = VK_NULL_HANDLE;
PFN_vkCreateDevice vkCreateDevice = VK_NULL_HANDLE;
PFN_vkDestroyDevice vkDestroyDevice = VK_NULL_HANDLE;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = VK_NULL_HANDLE;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = VK_NULL_HANDLE;

bool nfvkInstanceExtensionsInit(VkInstance instance)
{
    bool allExtensionsAvailable = true;

    VK_GET_INSTANCEPROC(instance, vkEnumeratePhysicalDevices);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceProperties);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceFeatures);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceQueueFamilyProperties);
    VK_GET_INSTANCEPROC(instance, vkCreateDevice);
    VK_GET_INSTANCEPROC(instance, vkDestroyDevice);
    VK_GET_INSTANCEPROC(instance, vkGetDeviceProcAddr);
    VK_GET_INSTANCEPROC(instance, vkDestroySurfaceKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);

#ifdef WIN32
    if (!nfvkWin32InstanceExtensionsInit(instance))
        LOG_WARNING("Win32 extensions failed to init, some Vulkan Win32 APIs might be unavailable.");
#elif defined(__linux__) | defined(__LINUX__)
    if (!nfvkXcbInstanceExtensionsInit(instance))
        LOG_WARNING("Xcb extensions failed to init, some Vulkan Xcb APIs might be unavailable.");
#else
#error "Target platform not supported."
#endif

    return allExtensionsAvailable;
}


PFN_vkCreateCommandPool vkCreateCommandPool = VK_NULL_HANDLE;
PFN_vkDestroyCommandPool vkDestroyCommandPool = VK_NULL_HANDLE;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = VK_NULL_HANDLE;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = VK_NULL_HANDLE;

bool nfvkDeviceExtensionsInit(VkDevice device)
{
    bool allExtensionsAvailable = true;

    VK_GET_DEVICEPROC(device, vkCreateCommandPool);
    VK_GET_DEVICEPROC(device, vkDestroyCommandPool);
    VK_GET_DEVICEPROC(device, vkAllocateCommandBuffers);
    VK_GET_DEVICEPROC(device, vkFreeCommandBuffers);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE
