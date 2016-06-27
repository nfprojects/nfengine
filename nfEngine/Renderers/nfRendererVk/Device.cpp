/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"

// modules


namespace {

template<typename Type, typename Desc>
Type* GenericCreateResource(const Desc& desc)
{
    Type* resource = new (std::nothrow) Type;
    if (resource == nullptr)
        return nullptr;

    if (!resource->Init(desc))
    {
        delete resource;
        return nullptr;
    }

    return resource;
}

} // namespace

namespace NFE {
namespace Renderer {

std::unique_ptr<Device> gDevice;

// TODO TEMPORARY
#define VK_GET_FUNC(x) do { \
    if (!mVulkanLib.GetSymbol(#x, x)) \
        LOG_FATAL("Failed to acquire " #x); \
} while(0)

Device::Device()
{
    // TODO THIS IS VERY TEMPORARY
    mVulkanLib.Open("vulkan.so");
    if (!mVulkanLib.IsOpened())
        LOG_FATAL("Unable to open Vulkan library.");

    VK_GET_FUNC(vkCreateInstance);
    VK_GET_FUNC(vkDestroyInstance);


    VkApplicationInfo appInfo = {};
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "nfRendererVk";
    appInfo.pEngineName = "nfRendererVk";
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = 1;

    const char* enabledExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
    #ifdef WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif defined(__linux__) || defined(__LINUX__)
        //VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    #else
    #error "Target platform not supported."
    #endif
    };
/*
    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };
*/
    VkInstanceCreateInfo instInfo = {};
    memset(&instInfo, 0, sizeof(instInfo));
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = 1;
    instInfo.ppEnabledExtensionNames = enabledExtensions;
/*    instInfo.enabledLayerCount = 1;
    instInfo.ppEnabledLayerNames = enabledLayers;*/

    VkResult result = vkCreateInstance(&instInfo, nullptr, &mInstance);
    if (result != VK_SUCCESS)
        LOG_FATAL("Failed to create Vulkan instance: %i", result);

    // TODO init extension functions here
    /* debugging layer extensions
     * VK_GET_FUNC(vkCreateDebugReportCallbackEXT);
     * VK_GET_FUNC(vkDestroyDebugReportCallbackEXT);
     * VK_GET_FUNC(vkDebugReportMessageEXT);
     */

    LOG_INFO("Initialized Vulkan instance successfully");
}

Device::~Device()
{
    vkDestroyInstance(mInstance, nullptr);
}

void* Device::GetHandle() const
{
    return nullptr;
}


IVertexLayout* Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBuffer* Device::CreateBuffer(const BufferDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

ITexture* Device::CreateTexture(const TextureDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBackbuffer* Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IPipelineState* Device::CreatePipelineState(const PipelineStateDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

ISampler* Device::CreateSampler(const SamplerDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IShader* Device::CreateShader(const ShaderDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IShaderProgram* Device::CreateShaderProgram(const ShaderProgramDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IResourceBindingSet* Device::CreateResourceBindingSet(const ResourceBindingSetDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IResourceBindingLayout* Device::CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IResourceBindingInstance* Device::CreateResourceBindingInstance(IResourceBindingSet* set)
{
    UNUSED(set);
    return nullptr;
}

ICommandBuffer* Device::CreateCommandBuffer()
{
    return nullptr;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    UNUSED(info);
    return false;
}

bool Device::Execute(ICommandList* commandList)
{
    UNUSED(commandList);
    return false;
}

bool Device::DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

bool Device::DownloadTexture(ITexture* tex, void* data, int mipmap, int layer)
{
    UNUSED(tex);
    UNUSED(data);
    UNUSED(mipmap);
    UNUSED(layer);
    return false;
}

IDevice* Init()
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new (std::nothrow) Device);
    }

    return gDevice.get();
}

void Release()
{
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
