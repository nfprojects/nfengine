/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Debugger singleton
 */

#include "PCH.hpp"

#include "Debugger.hpp"
#include "GetProc.hpp"
#include "Engine/Common/Logger/Logger.hpp"

namespace NFE {
namespace Renderer {

namespace {

VkBool32 DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                        void* pUserData)
{
    NFE_UNUSED(pUserData);

    const char* format = "%s [%s]: %s";
    const char* type = nullptr;

    switch (messageTypes)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        type = "GENERAL";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        type = "VALIDATION";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        type = "PERFORMANCE";
        break;
    default:
        // shouldn't happen, but will shut up compiler warnings
        return VK_TRUE;
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        NFE_LOG_ERROR(format, type, pCallbackData->pMessageIdName, pCallbackData->pMessage);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        NFE_LOG_WARNING(format, type, pCallbackData->pMessageIdName, pCallbackData->pMessage);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        NFE_LOG_INFO(format, type, pCallbackData->pMessageIdName, pCallbackData->pMessage);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        NFE_LOG_DEBUG(format, type, pCallbackData->pMessageIdName, pCallbackData->pMessage);

    // returning VK_FALSE here would cause Vulkan APIs to return VK_ERROR_VALIDATION_FAILED_EXT
    // right now we don't want that, but for debugging purposes it can be changed.
    //if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        return VK_TRUE;
   // else
       // return VK_FALSE;
}

} // namespace

Debugger::Debugger()
    : mEarlyDebugMessengerInfo()
    , mDebugMessenger(VK_NULL_HANDLE)
    , mMarkersUseable(false)
    , mVkInstance(VK_NULL_HANDLE)
{
    VK_ZERO_MEMORY(mEarlyDebugMessengerInfo);
    mEarlyDebugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    mEarlyDebugMessengerInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    mEarlyDebugMessengerInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    mEarlyDebugMessengerInfo.pfnUserCallback = DebugMessenger;
}

Debugger::~Debugger()
{
    ReleaseMessenger();
}

Debugger& Debugger::Instance()
{
    static Debugger instance;
    return instance;
}

bool Debugger::InitMessenger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type)
{
    bool allExtensionsAvailable = true;

    VK_GET_INSTANCEPROC(instance, vkCreateDebugUtilsMessengerEXT);
    VK_GET_INSTANCEPROC(instance, vkDestroyDebugUtilsMessengerEXT);

    if (!allExtensionsAvailable)
    {
        NFE_LOG_ERROR("Debug utils extension not available - cannot initialize Debug Messenger");
        return false;
    }

    mVkInstance = instance;

    VkDebugUtilsMessengerCreateInfoEXT info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = severity;
    info.messageType = type;
    info.pfnUserCallback = DebugMessenger;
    VkResult result = vkCreateDebugUtilsMessengerEXT(mVkInstance, &info, nullptr, &mDebugMessenger);
    CHECK_VKRESULT(result, "Failed to create Debug Messenger");

    NFE_LOG_DEBUG("Vulkan debug messenger initialized successfully.");
    return true;
}

bool Debugger::InitDebugObjectAnnotation(VkDevice device)
{
    bool allExtensionsAvailable = true;

    VK_GET_DEVICEPROC(device, vkSetDebugUtilsObjectNameEXT);
    VK_GET_DEVICEPROC(device, vkSetDebugUtilsObjectTagEXT);

    if (!allExtensionsAvailable)
    {
        NFE_LOG_ERROR("Debug utils extension not available - cannot initialize Debug Messenger");
        return false;
    }

    mVkDevice = device;

    return true;
}

bool Debugger::NameObject(uint64_t handle, VkObjectType type, const char* name)
{
    if (vkSetDebugUtilsObjectNameEXT == nullptr)
        return true; // quietly pretend everything is okay - extension is unavailable or debugging is turned off

    VkDebugUtilsObjectNameInfoEXT info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectHandle = handle;
    info.objectType = type;
    info.pObjectName = name;
    VkResult result = vkSetDebugUtilsObjectNameEXT(mVkDevice, &info);
    CHECK_VKRESULT(result, "Failed to set object name");

    return true;
}

bool Debugger::TagObject(uint64_t handle, VkObjectType type, uint64_t tagName, void* tag, size_t tagSize)
{
    if (vkSetDebugUtilsObjectTagEXT == nullptr)
        return true; // quietly pretend everything is okay - extension is unavailable or debugging is turned off

    VkDebugUtilsObjectTagInfoEXT info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectHandle = handle;
    info.objectType = type;
    info.tagName = tagName;
    info.pTag = tag;
    info.tagSize = tagSize;
    VkResult result = vkSetDebugUtilsObjectTagEXT(mVkDevice, &info);
    CHECK_VKRESULT(result, "Failed to set object tag");

    return true;
}

void Debugger::ReleaseMessenger()
{
    if (vkDestroyDebugUtilsMessengerEXT && (mDebugMessenger != VK_NULL_HANDLE))
        vkDestroyDebugUtilsMessengerEXT(mVkInstance, mDebugMessenger, nullptr);

    mDebugMessenger = VK_NULL_HANDLE;
    vkCreateDebugUtilsMessengerEXT = nullptr;
    vkDestroyDebugUtilsMessengerEXT = nullptr;
}

void Debugger::ReleaseDebugObjectAnnotation()
{
    vkSetDebugUtilsObjectNameEXT = nullptr;
    vkSetDebugUtilsObjectTagEXT = nullptr;
}

} // namespace Renderer
} // namespace NFE
