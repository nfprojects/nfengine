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
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = VK_NULL_HANDLE;

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
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);

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


// Command Pool
PFN_vkCreateCommandPool vkCreateCommandPool = VK_NULL_HANDLE;
PFN_vkDestroyCommandPool vkDestroyCommandPool = VK_NULL_HANDLE;

// Command Buffer
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = VK_NULL_HANDLE;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = VK_NULL_HANDLE;
PFN_vkEndCommandBuffer vkEndCommandBuffer = VK_NULL_HANDLE;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = VK_NULL_HANDLE;

// Render Passes and Framebuffers
PFN_vkCreateFramebuffer vkCreateFramebuffer = VK_NULL_HANDLE;
PFN_vkCreateRenderPass vkCreateRenderPass = VK_NULL_HANDLE;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer = VK_NULL_HANDLE;
PFN_vkDestroyRenderPass vkDestroyRenderPass = VK_NULL_HANDLE;

// Commands
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = VK_NULL_HANDLE;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass = VK_NULL_HANDLE;
PFN_vkCmdSetScissor vkCmdSetScissor = VK_NULL_HANDLE;
PFN_vkCmdSetViewport vkCmdSetViewport = VK_NULL_HANDLE;

// Swapchain
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = VK_NULL_HANDLE;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = VK_NULL_HANDLE;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = VK_NULL_HANDLE;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = VK_NULL_HANDLE;

// Queue
PFN_vkGetDeviceQueue vkGetDeviceQueue = VK_NULL_HANDLE;
PFN_vkQueuePresentKHR vkQueuePresentKHR = VK_NULL_HANDLE;

// Buffers and Images
PFN_vkCreateBuffer vkCreateBuffer = VK_NULL_HANDLE;
PFN_vkCreateImage vkCreateImage = VK_NULL_HANDLE;
PFN_vkCreateImageView vkCreateImageView = VK_NULL_HANDLE;
PFN_vkDestroyBuffer vkDestroyBuffer = VK_NULL_HANDLE;
PFN_vkDestroyImage vkDestroyImage = VK_NULL_HANDLE;
PFN_vkDestroyImageView vkDestroyImageView = VK_NULL_HANDLE;

// Synchronization
PFN_vkCreateFence vkCreateFence = VK_NULL_HANDLE;
PFN_vkCreateSemaphore vkCreateSemaphore = VK_NULL_HANDLE;
PFN_vkDestroyFence vkDestroyFence = VK_NULL_HANDLE;
PFN_vkDestroySemaphore vkDestroySemaphore = VK_NULL_HANDLE;

bool nfvkDeviceExtensionsInit(VkDevice device)
{
    bool allExtensionsAvailable = true;

    // Command Pool
    VK_GET_DEVICEPROC(device, vkCreateCommandPool);
    VK_GET_DEVICEPROC(device, vkDestroyCommandPool);

    // Command Buffer
    VK_GET_DEVICEPROC(device, vkAllocateCommandBuffers);
    VK_GET_DEVICEPROC(device, vkBeginCommandBuffer);
    VK_GET_DEVICEPROC(device, vkEndCommandBuffer);
    VK_GET_DEVICEPROC(device, vkFreeCommandBuffers);

    // Render Passes and Framebuffers
    VK_GET_DEVICEPROC(device, vkCreateFramebuffer);
    VK_GET_DEVICEPROC(device, vkCreateRenderPass);
    VK_GET_DEVICEPROC(device, vkDestroyFramebuffer);
    VK_GET_DEVICEPROC(device, vkDestroyRenderPass);

    // Commands
    VK_GET_DEVICEPROC(device, vkCmdBeginRenderPass);
    VK_GET_DEVICEPROC(device, vkCmdEndRenderPass);
    VK_GET_DEVICEPROC(device, vkCmdSetScissor);
    VK_GET_DEVICEPROC(device, vkCmdSetViewport);

    // Swapchain
    VK_GET_DEVICEPROC(device, vkAcquireNextImageKHR);
    VK_GET_DEVICEPROC(device, vkCreateSwapchainKHR);
    VK_GET_DEVICEPROC(device, vkDestroySwapchainKHR);
    VK_GET_DEVICEPROC(device, vkGetSwapchainImagesKHR);

    // Queue
    VK_GET_DEVICEPROC(device, vkGetDeviceQueue);
    VK_GET_DEVICEPROC(device, vkQueuePresentKHR);

    // Buffers and Images
    VK_GET_DEVICEPROC(device, vkCreateBuffer);
    VK_GET_DEVICEPROC(device, vkCreateImage);
    VK_GET_DEVICEPROC(device, vkCreateImageView);
    VK_GET_DEVICEPROC(device, vkDestroyBuffer);
    VK_GET_DEVICEPROC(device, vkDestroyImage);
    VK_GET_DEVICEPROC(device, vkDestroyImageView);

    // Synchronization
    VK_GET_DEVICEPROC(device, vkCreateFence);
    VK_GET_DEVICEPROC(device, vkCreateSemaphore);
    VK_GET_DEVICEPROC(device, vkDestroyFence);
    VK_GET_DEVICEPROC(device, vkDestroySemaphore);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE
