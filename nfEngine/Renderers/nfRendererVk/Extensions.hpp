/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan extensions
 */

#pragma once

#ifdef WIN32
#include "Win/Win32Extensions.hpp"
#elif defined(__linux__) | defined(__LINUX__)
#include "Linux/XcbExtensions.hpp"
#else
#error "Target platform not supported."
#endif


namespace NFE {
namespace Renderer {

// Instance extractor
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

// Instance extensions
extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkDestroyDevice vkDestroyDevice;
extern PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
extern PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;

/**
 * Initializes Vulkan Instance extensions used by nfRendererVk.
 *
 * @param  Valid Instance object.
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfvkInstanceExtensionsInit(VkInstance instance);


// Device extensions

// Command Pool
extern PFN_vkCreateCommandPool vkCreateCommandPool;
extern PFN_vkDestroyCommandPool vkDestroyCommandPool;

// Command Buffers
extern PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
extern PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
extern PFN_vkEndCommandBuffer vkEndCommandBuffer;
extern PFN_vkFreeCommandBuffers vkFreeCommandBuffers;

// Render Passes and Framebuffers
extern PFN_vkCreateFramebuffer vkCreateFramebuffer;
extern PFN_vkCreateRenderPass vkCreateRenderPass;
extern PFN_vk
extern PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
extern PFN_vkDestroyRenderPass vkDestroyRenderPass;

// Commands
extern PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
extern PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
extern PFN_vkCmdSetScissor vkCmdSetScissor;
extern PFN_vkCmdSetViewport vkCmdSetViewport;

/**
 * Initializes Vulkan Device extensions used by nfRendererVk.
 *
 * @param  Valid Device object.
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfvkDeviceExtensionsInit(VkDevice device);

} // namespace Renderer
} // namespace NFE
