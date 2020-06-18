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

    // Device-related debugging functions
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTagEXT;

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
     * Initializes Debug Object Annotation extension
     *
     * Should be called right after successfully creating Device
     */
    bool InitDebugObjectAnnotation(VkDevice device);

    /**
     * Names a Vulkan object via Debug Object Annotation extension
     */
    bool NameObject(uint64_t handle, VkObjectType type, const char* name);

    /**
     * Tags a Vulkan object with extra information via Debug Object Annotation extension.
     */
    bool TagObject(uint64_t handle, VkObjectType type, uint64_t tagName, void* tag, size_t tagSize);

    NFE_INLINE bool IsDebugAnnotationActive() const
    {
        return (vkSetDebugUtilsObjectNameEXT != nullptr) && (vkSetDebugUtilsObjectTagEXT != nullptr);
    }

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

    /**
     * Frees debug object annotation.
     */
    void ReleaseDebugObjectAnnotation();
};

} // namespace Renderer
} // namespace NFE
