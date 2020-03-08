/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Instance
 */

#pragma once

#include "Defines.hpp"

#include "Engine/Common/System/Library.hpp"


namespace NFE {
namespace Renderer {

class Instance
{
private:
    VkInstance mInstance;

public:
    Instance();
    ~Instance();

    /**
     * Initialize Vulkan instance for further use.
     *
     * @param enableDebug Set to true if debug validation layer should be enabled.
     * @return True on success
     */
    bool Init(bool enableDebug, VkDebugReportFlagBitsEXT flags = VK_DEBUG_REPORT_ERROR_BIT_EXT);

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
