/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Instance
 */

#pragma once

#include "Defines.hpp"

#include "Library.hpp"

namespace NFE {
namespace Renderer {

extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkDestroyDevice vkDestroyDevice;

class Instance
{
private:
    NFE::Common::Library mVulkanLib;
    VkInstance mInstance;

    // TODO temporary location for Vulkan functions/extensions
    // library-extracted entry points
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

public:
    Instance();
    ~Instance();

    /**
     * Initialize Vulkan instance for further use.
     *
     * @param validation Set to true if debug validation layer should be enabled.
     * @return True on success
     */
    bool Init(bool validation);

    /**
     * Acquire Vulkan instance.
     */
    const VkInstance& Get() const;

    /**
     * Release Vulkan instance.
     */
    void Release();
};

} // namespace Renderer
} // namespace NFE
