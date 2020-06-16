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
    // Instance-related debugging functions - Debug Messengers
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

    // Debug primitives
    VkDebugUtilsMessengerCreateInfoEXT mEarlyDebugMessengerInfo;
    VkDebugUtilsMessengerEXT mDebugMessenger;

    bool mMarkersUseable;
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
     * Initializes debug messenger.
     *
     * Should be called right after the Instance is successfully created.
     */
    bool InitMessenger(VkInstance instance,
                       VkDebugUtilsMessageSeverityFlagsEXT severity,
                       VkDebugUtilsMessageTypeFlagsEXT type);

    /**
     * Returns "early" debug messenger create info.
     *
     * This structure can be passed directly to vkCreateInstance to provide logging utilities when
     * Vulkan Instance is created.
     */
    NFE_INLINE VkDebugUtilsMessengerCreateInfoEXT* GetEarlyDebugMessengerInfo()
    {
        return &mEarlyDebugMessengerInfo;
    }

    /**
     * Frees debug messenger.
     */
    void ReleaseMessenger();
};

} // namespace Renderer
} // namespace NFE
