/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
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
     * @param debugLevel Engine's requested debug level.
     * @return True on success
     */
    bool Init(int debugLevel);

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
