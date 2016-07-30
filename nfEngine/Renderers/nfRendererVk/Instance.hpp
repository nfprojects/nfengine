/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Instance
 */

#pragma once

#include "Defines.hpp"

#include "nfCommon/Library.hpp"

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
     * @param enableDebug Set to true if debug validation layer should be enabled.
     * @return True on success
     */
    bool Init(bool enableDebug);

    /**
     * Acquire Vulkan instance.
     */
    inline const VkInstance& Get() const
    {
        return mInstance;
    }

    /**
     * Release Vulkan instance.
     */
    void Release();
};

} // namespace Renderer
} // namespace NFE
