/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Instance
 */

#pragma once

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Instance
{
private:
    NFE::Common::Library mVulkanLib;
    VkInstance mInstance;

    // TODO temporary location for Vulkan functions/extensions
    // library-extracted entry points
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkDestroyInstance vkDestroyInstance;

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
