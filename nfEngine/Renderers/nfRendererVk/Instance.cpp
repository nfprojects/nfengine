/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Instance
 */

#include "PCH.hpp"
#include "Instance.hpp"
#include "GetProc.hpp"
#include "Debugger.hpp"

#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

namespace {

#ifdef WIN32
const Common::StringView vulkanLibraryName("vulkan-1");
#elif defined(__linux__) || defined(__LINUX__)
const Common::StringView vulkanLibraryName("vulkan");
#else
#error "Target platform not supported."
#endif

}


Instance::Instance()
    : mVulkanLib()
    , mInstance(VK_NULL_HANDLE)
{
}

Instance::~Instance()
{
    Release();
}

bool Instance::Init(bool enableDebug, VkDebugReportFlagBitsEXT flags)
{
    if (mInstance)
        return true;

    // TODO This probably should be managed somewhere else
    if (!mVulkanLib.Open(vulkanLibraryName))
    {
        NFE_LOG_ERROR("Unable to open Vulkan library.");
        return false;
    }

    VK_GET_LIBPROC(mVulkanLib, vkCreateInstance);
    VK_GET_LIBPROC(mVulkanLib, vkDestroyInstance);
    VK_GET_LIBPROC(mVulkanLib, vkGetInstanceProcAddr);

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
    Common::DynArray<const char*> enabledExtensions;
    enabledExtensions.PushBack(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef WIN32
    enabledExtensions.PushBack(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__) || defined(__LINUX__)
    enabledExtensions.PushBack(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#else
#error "Target platform not supported."
#endif

    if (enableDebug)
        enabledExtensions.PushBack(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };

    VkInstanceCreateInfo instInfo = {};
    memset(&instInfo, 0, sizeof(instInfo));
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = static_cast<uint32>(enabledExtensions.Size());
    instInfo.ppEnabledExtensionNames = enabledExtensions.Data();
    if (enableDebug)
    {
        instInfo.enabledLayerCount = 1;
        instInfo.ppEnabledLayerNames = enabledLayers;
    }

    VkResult result = vkCreateInstance(&instInfo, nullptr, &mInstance);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create Vulkan Instance");

    if (enableDebug)
    {
        if (!Debugger::Instance().InitReport(mInstance, flags))
        {
            NFE_LOG_ERROR("Vulkan debug reports failed to initialize.");
            return false;
        }
    }

    if (!nfvkInstanceExtensionsInit(mInstance))
    {
        NFE_LOG_ERROR("Failed to initialize Vulkan Instance extensions.");
        return false;
    }

    return true;
}

void Instance::Release()
{
    if (mInstance)
        vkDestroyInstance(mInstance, nullptr);

    mVulkanLib.Close();
}

} // namespace Renderer
} // namespace NFE
