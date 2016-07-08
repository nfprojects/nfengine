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
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;

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
extern PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
extern PFN_vkDestroyRenderPass vkDestroyRenderPass;

// Commands
extern PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
extern PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
extern PFN_vkCmdSetScissor vkCmdSetScissor;
extern PFN_vkCmdSetViewport vkCmdSetViewport;

// Swapchain
extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;

// Queue
extern PFN_vkGetDeviceQueue vkGetDeviceQueue;
extern PFN_vkQueuePresentKHR vkQueuePresentKHR;

// Buffers and Images
extern PFN_vkCreateBuffer vkCreateBuffer;
extern PFN_vkCreateImage vkCreateImage;
extern PFN_vkCreateImageView vkCreateImageView;
extern PFN_vkDestroyBuffer vkDestroyBuffer;
extern PFN_vkDestroyImage vkDestroyImage;
extern PFN_vkDestroyImageView vkDestroyImageView;

// Synchronization
extern PFN_vkCreateFence vkCreateFence;
extern PFN_vkCreateSemaphore vkCreateSemaphore;
extern PFN_vkDestroyFence vkDestroyFence;
extern PFN_vkDestroySemaphore vkDestroySemaphore;

/**
 * Initializes Vulkan Device extensions used by nfRendererVk.
 *
 * @param  Valid Device object.
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfvkDeviceExtensionsInit(VkDevice device);

} // namespace Renderer
} // namespace NFE
