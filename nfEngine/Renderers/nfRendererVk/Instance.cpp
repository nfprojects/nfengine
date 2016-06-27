/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Instance
 */

#include "PCH.hpp"
#include "Instance.hpp"

#include <memory.h>

// TODO TEMPORARY
#define VK_GET_FUNC(x) do { \
    if (!mVulkanLib.GetSymbol(#x, x)) \
        LOG_FATAL("Failed to acquire " #x); \
} while(0)

#define VK_GET_INSTANCEPROC(inst, x) do { \
    (x = (PFN_##x)vkGetInstanceProcAddr(inst, #x)); \
    if (!x) \
        LOG_FATAL("Failed to get " #x " from instance"); \
} while(0)

namespace NFE {
namespace Renderer {

PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = VK_NULL_HANDLE;
PFN_vkCreateDevice vkCreateDevice = VK_NULL_HANDLE;
PFN_vkDestroyDevice vkDestroyDevice = VK_NULL_HANDLE;

Instance::Instance()
    : mVulkanLib()
    , mInstance(VK_NULL_HANDLE)
{
}

Instance::~Instance()
{
    Release();
}

bool Instance::Init(bool validation)
{
    if (mInstance)
        return true;

    // TODO This probably should be managed somewhere else
#ifdef WIN32
    mVulkanLib.Open("vulkan-1");
#elif defined(__linux__) || defined(__LINUX__)
    mVulkanLib.Open("vulkan");
#else
#error "Target platform not supported."
#endif
    if (!mVulkanLib.IsOpened())
        LOG_FATAL("Unable to open Vulkan library.");

    VK_GET_FUNC(vkCreateInstance);
    VK_GET_FUNC(vkDestroyInstance);
    VK_GET_FUNC(vkGetInstanceProcAddr);

    VkApplicationInfo appInfo = {};
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "nfRendererVk";
    appInfo.pEngineName = "nfRendererVk";
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = 1;

    // TODO XLIB_SURFACE is still unavailable
    //      Use XCB surface instead for now
    // TODO extensions need to be in common with VkDevice
    const char* enabledExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,

    #ifdef WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif defined(__linux__) || defined(__LINUX__)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    #else
    #error "Target platform not supported."
    #endif

    };

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };

    VkInstanceCreateInfo instInfo = {};
    memset(&instInfo, 0, sizeof(instInfo));
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = 2;
    instInfo.ppEnabledExtensionNames = enabledExtensions;
    if (validation)
    {
        instInfo.enabledLayerCount = 1;
        instInfo.ppEnabledLayerNames = enabledLayers;
    }

    VkResult result = vkCreateInstance(&instInfo, nullptr, &mInstance);
    if (result != VK_SUCCESS)
        LOG_FATAL("Failed to create Vulkan instance: %i", result);

    // TODO move to Extensions per-instance module
    VK_GET_INSTANCEPROC(mInstance, vkEnumeratePhysicalDevices);
    VK_GET_INSTANCEPROC(mInstance, vkGetPhysicalDeviceProperties);
    VK_GET_INSTANCEPROC(mInstance, vkGetPhysicalDeviceFeatures);
    VK_GET_INSTANCEPROC(mInstance, vkGetPhysicalDeviceQueueFamilyProperties);
    VK_GET_INSTANCEPROC(mInstance, vkCreateDevice);
    VK_GET_INSTANCEPROC(mInstance, vkDestroyDevice);

    /* debugging layer extensions
     * VK_GET_FUNC(vkCreateDebugReportCallbackEXT);
     * VK_GET_FUNC(vkDestroyDebugReportCallbackEXT);
     * VK_GET_FUNC(vkDebugReportMessageEXT);
     */

    return true;
}

const VkInstance& Instance::Get() const
{
    return mInstance;
}

void Instance::Release()
{
    if (mInstance)
        vkDestroyInstance(mInstance, nullptr);

    mVulkanLib.Close();
}

} // namespace Renderer
} // namespace NFE
