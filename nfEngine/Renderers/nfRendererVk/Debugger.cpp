/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Debugger singleton
 */

#include "PCH.hpp"

#include "Debugger.hpp"
#include "GetProc.hpp"
#include "nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace Renderer {

namespace {

VkBool32 DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                     uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                     const char* pMessage, void* pUserData)
{
    NFE_UNUSED(objectType);
    NFE_UNUSED(object);
    NFE_UNUSED(location);
    NFE_UNUSED(pUserData); // for now, but it might be a good idea to use it in the future

    const char* report = "VK_REPORT";
    const char* perf = "VK_PERF";
    const char* format = "%s (code %i) [%s]: %s";

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        LOG_ERROR(format, report, messageCode, pLayerPrefix, pMessage);
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        LOG_WARNING(format, report, messageCode, pLayerPrefix, pMessage);
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        LOG_INFO(format, report, messageCode, pLayerPrefix, pMessage);
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        LOG_DEBUG(format, report, messageCode, pLayerPrefix, pMessage);
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        LOG_WARNING(format, perf, messageCode, pLayerPrefix, pMessage);

    // returning VK_TRUE here would cause Vulkan APIs to return VK_ERROR_VALIDATION_FAILED_EXT
    // right now we don't want that, but for debugging purposes it can be changed.
    //if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        return VK_TRUE;
   // else
       // return VK_FALSE;
}

} // namespace

Debugger::Debugger()
    : mDebugCallback(VK_NULL_HANDLE)
    , mMarkersUseable(false)
    , mVkInstance(VK_NULL_HANDLE)
{
}

Debugger::~Debugger()
{
    Release();
}

Debugger& Debugger::Instance()
{
    static Debugger instance;
    return instance;
}

bool Debugger::InitReport(VkInstance instance, VkDebugReportFlagsEXT flags)
{
    // Gather debug-specific extensions
    // We should be here only when debug extension is requested from instance & device
    bool allExtensionsAvailable = true;

    VK_GET_INSTANCEPROC(instance, vkCreateDebugReportCallbackEXT);
    VK_GET_INSTANCEPROC(instance, vkDestroyDebugReportCallbackEXT);
    VK_GET_INSTANCEPROC(instance, vkDebugReportMessageEXT);

    if (!allExtensionsAvailable)
        return false;

    mVkInstance = instance;

    VkDebugReportCallbackCreateInfoEXT debugInfo;
    VK_ZERO_MEMORY(debugInfo);
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugInfo.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(DebugReport);
    debugInfo.flags = flags;
    VkResult result = vkCreateDebugReportCallbackEXT(mVkInstance, &debugInfo, nullptr, &mDebugCallback);
    CHECK_VKRESULT(result, "Failed to allocate debug report callback");

    LOG_DEBUG("Vulkan debug reports initialized successfully.");
    return true;
}

bool Debugger::InitMarkers(VkDevice device)
{
    // TODO right now debug markers extension is unsupported through drivers
    // Uncomment below and implement markers when drivers catch up

    bool allExtensionsAvailable = true;

    /*
    VK_GET_DEVICEPROC(device, vkCmdDebugMarkerBeginEXT);
    VK_GET_DEVICEPROC(device, vkCmdDebugMarkerEndEXT);
    VK_GET_DEVICEPROC(device, vkCmdDebugMarkerInsertEXT);
    VK_GET_DEVICEPROC(device, vkDebugMarkerSetObjectNameEXT);
    VK_GET_DEVICEPROC(device, vkDebugMarkerSetObjectTagEXT);
    */

    if (!allExtensionsAvailable)
        return false;

    mVkDevice = device;
    mMarkersUseable = false;

    LOG_DEBUG("Vulkan debug markers initialized successfully.");
    return true;
}

void Debugger::Release()
{
    if (vkDestroyDebugReportCallbackEXT && (mDebugCallback != VK_NULL_HANDLE))
        vkDestroyDebugReportCallbackEXT(mVkInstance, mDebugCallback, nullptr);

    mDebugCallback = VK_NULL_HANDLE;
    mMarkersUseable = false;
}

} // namespace Renderer
} // namespace NFE
