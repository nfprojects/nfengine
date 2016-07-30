/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Debugger singleton
 */

#pragma once

#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class Debugger
{
    // some instance functions to debug
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;

    // and some device debug functions
    PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT;
    PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT;
    PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT;
    PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;
    PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT;

    VkDebugReportCallbackEXT mDebugCallback;

    bool mUseable;
    VkInstance mVkInstance;
    VkDevice mVkDevice;

    Debugger();
    Debugger(const Debugger&) = delete;
    Debugger(Debugger&&) = delete;
    Debugger& operator=(const Debugger&) = delete;
    Debugger& operator=(Debugger&&) = delete;
    ~Debugger();

public:
    static Debugger& Instance();

    /**
     * Initializes debugger.
     *
     * Should be called right after the Device is successfully created.
     */
    bool Init(VkInstance instance, VkDevice device, VkDebugReportFlagsEXT flags);

    /**
     * Frees debugger resources.
     */
    void Release();
};

} // namespace Renderer
} // namespace NFE
