/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Instance
 */

#include "PCH.hpp"
#include "Instance.hpp"
#include "GetProc.hpp"
#include "Debugger.hpp"

#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {


Instance::Instance()
    : mInstance(VK_NULL_HANDLE)
{
}

Instance::~Instance()
{
    Release();
}

bool Instance::Init(int debugLevel)
{
    if (mInstance)
        return true;

    bool enableDebug = (debugLevel > 0);

    VkApplicationInfo appInfo = {};
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "nfRendererVk";
    appInfo.pEngineName = "nfEngine";
    appInfo.apiVersion = VK_API_VERSION_1_1;
    appInfo.applicationVersion = 1;

    // TODO extensions need to be in common with VkDevice
    // TODO make sure extensions we need are available via vkEnumerateInstanceExtensionProperties
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
        enabledExtensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
        // NOTE Early message severity does not have info and verbose output due to over-spam from
        //      Instance creator. Consider adding it somehow based on Engine's debugLevel.
        instInfo.pNext = Debugger::Instance().GetEarlyDebugMessengerInfo();
    }

    VkResult result = vkCreateInstance(&instInfo, nullptr, &mInstance);
    CHECK_VKRESULT(result, "Failed to create Vulkan Instance");

    if (enableDebug)
    {
        VkDebugUtilsMessageSeverityFlagsEXT severityFlags =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

        VkDebugUtilsMessageTypeFlagsEXT typeFlags =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        if (!Debugger::Instance().InitMessenger(mInstance, severityFlags, typeFlags))
        {
            NFE_LOG_ERROR("Vulkan debug messenger failed to initialize.");
            return false; // TODO should we close here? App could easily run without Debugger's help
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
    Debugger::Instance().ReleaseMessenger();

    if (mInstance)
        vkDestroyInstance(mInstance, nullptr);
}

} // namespace Renderer
} // namespace NFE
