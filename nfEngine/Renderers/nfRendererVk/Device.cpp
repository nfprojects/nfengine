/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"

#include <memory.h>

// modules
#include "Translations.hpp"

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

Device::Device()
    : mVkInstance()
    , mVkDevice(VK_NULL_HANDLE)
    , mVkPhysicalDevice(VK_NULL_HANDLE)
{
}

Device::~Device()
{
    if (mVkDevice)
        vkDestroyDevice(mVkDevice, nullptr);
}

VkPhysicalDevice Device::SelectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices)
{
    VkPhysicalDeviceProperties devProps;

    // Debugging-related device description printing
    LOG_DEBUG("%u physical devices detected:", devices.size());
    for (unsigned int i = 0; i < devices.size(); ++i)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devProps);
        LOG_DEBUG("Device #%u - %s:", i, devProps.deviceName);
        LOG_DEBUG("  ID:         %u",       devProps.deviceID);
        LOG_DEBUG("  Type:       %d (%s)",  devProps.deviceType,
                                            TranslateDeviceTypeToString(devProps.deviceType));
        LOG_DEBUG("  Vendor ID:  %u",       devProps.vendorID);
        LOG_DEBUG("  API ver:    %u.%u.%u", VK_VERSION_MAJOR(devProps.apiVersion),
                                            VK_VERSION_MINOR(devProps.apiVersion),
                                            VK_VERSION_PATCH(devProps.apiVersion));
        LOG_DEBUG("  Driver ver: %u.%u.%u", VK_VERSION_MAJOR(devProps.driverVersion),
                                            VK_VERSION_MINOR(devProps.driverVersion),
                                            VK_VERSION_PATCH(devProps.driverVersion));
    }

    // TODO so far we select the first device available. Here might be a good place to:
    //   * Select more devices
    //   * Create some rules to determine the best device
    return devices[0];
}

bool Device::Init()
{
    if (!mVkInstance.Init(false))
    {
        LOG_ERROR("Vulkan instance failed to initialize");
        return false;
    }

    const VkInstance& instance = mVkInstance.Get();

    // acquire GPU count first
    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Unable to enumerate physical devices");
        return false;
    }
    if (gpuCount == 0)
    {
        LOG_ERROR("0 physical devices available.");
        return false;
    }

    // and now get details about our GPUs
    std::vector<VkPhysicalDevice> devices(gpuCount);
    result = vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Unable to get more information about physical devices");
        return false;
    }

    mVkPhysicalDevice = SelectPhysicalDevice(devices);

    // Grab queue properties from our selected device
    uint32 graphicsQueue = 0;
    uint32 queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOG_ERROR("Physical device does not have any queue family properties.");
        return false;
    }

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queueCount, queueProps.data());

    for (graphicsQueue = 0; graphicsQueue < queueCount; graphicsQueue++)
        if (queueProps[graphicsQueue].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            break;

    if (graphicsQueue == queueCount)
    {
        LOG_ERROR("Selected physical device does not support graphics queue.");
        return false;
    }

    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsQueue;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    const char* enabledExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo devInfo;
    memset(&devInfo, 0, sizeof(devInfo));
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = nullptr;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.pEnabledFeatures = nullptr;
    devInfo.enabledExtensionCount = 1;
    devInfo.ppEnabledExtensionNames = enabledExtensions;

    result = vkCreateDevice(mVkPhysicalDevice, &devInfo, nullptr, &mVkDevice);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create Vulkan Device.");
        return false;
    }

    if (!nfvkDeviceExtensionsInit(mVkDevice))
    {
        LOG_ERROR("Failed to initialize Vulkan device extensions.");
        return false;
    }

    LOG_INFO("Vulkan device initialized successfully");
    return true;
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
    VkPhysicalDeviceProperties devProps = {};
    vkGetPhysicalDeviceProperties(mVkPhysicalDevice, &devProps);

    info.description = devProps.deviceName;
    info.misc = "Vulkan API version: "
              + std::to_string(VK_VERSION_MAJOR(devProps.apiVersion)) + "."
              + std::to_string(VK_VERSION_MINOR(devProps.apiVersion)) + "."
              + std::to_string(VK_VERSION_PATCH(devProps.apiVersion));

    VkPhysicalDeviceFeatures devFeatures = {};
    vkGetPhysicalDeviceFeatures(mVkPhysicalDevice, &devFeatures);

    // Description of below features is available on Vulkan registry:
    //   https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkGetPhysicalDeviceFeatures.html
    info.features.push_back("RobustBufferAccess=" + std::to_string(devFeatures.robustBufferAccess));
    info.features.push_back("FullDrawIndexUint32=" + std::to_string(devFeatures.fullDrawIndexUint32));
    info.features.push_back("ImageCubeArray=" + std::to_string(devFeatures.imageCubeArray));
    info.features.push_back("IndependentBlend=" + std::to_string(devFeatures.independentBlend));
    info.features.push_back("GeometryShader=" + std::to_string(devFeatures.geometryShader));
    info.features.push_back("TesselationShader=" + std::to_string(devFeatures.tessellationShader));
    info.features.push_back("SampleRateShading=" + std::to_string(devFeatures.sampleRateShading));
    info.features.push_back("DualSourceBlend=" + std::to_string(devFeatures.dualSrcBlend));
    info.features.push_back("LogicOp=" + std::to_string(devFeatures.logicOp));
    info.features.push_back("DepthClamp=" + std::to_string(devFeatures.depthClamp));
    info.features.push_back("DepthBiasClamp=" + std::to_string(devFeatures.depthBiasClamp));
    info.features.push_back("DepthBounds=" + std::to_string(devFeatures.depthBounds));
    info.features.push_back("FillModeNonSolid=" + std::to_string(devFeatures.fillModeNonSolid));
    info.features.push_back("WideLines=" + std::to_string(devFeatures.wideLines));
    info.features.push_back("LargePoints=" + std::to_string(devFeatures.largePoints));
    info.features.push_back("TextureCompressionETC2=" + std::to_string(devFeatures.textureCompressionETC2));
    info.features.push_back("TextureCompressionASTC_LDR=" + std::to_string(devFeatures.textureCompressionASTC_LDR));
    info.features.push_back("TextureCompressionBC=" + std::to_string(devFeatures.textureCompressionBC));
    info.features.push_back("OcclusionQueryPrecise=" + std::to_string(devFeatures.occlusionQueryPrecise));
    info.features.push_back("PipelineStatisticsQuery=" + std::to_string(devFeatures.pipelineStatisticsQuery));
    info.features.push_back("VertexPipelineStoresAndAtomics=" + std::to_string(devFeatures.vertexPipelineStoresAndAtomics));
    info.features.push_back("FragmentStoresAndAtomics=" + std::to_string(devFeatures.fragmentStoresAndAtomics));
    info.features.push_back("ShaderTessellationAndGeometryPointSize=" + std::to_string(devFeatures.shaderTessellationAndGeometryPointSize));
    info.features.push_back("ShaderImageGatherExtended=" + std::to_string(devFeatures.shaderImageGatherExtended));
    info.features.push_back("ShaderStorageImageExtendedFormats=" + std::to_string(devFeatures.shaderStorageImageExtendedFormats));
    info.features.push_back("ShaderStorageImageMultisample=" + std::to_string(devFeatures.shaderStorageImageMultisample));
    info.features.push_back("ShaderUniformBufferArrayDynamicIndexing=" + std::to_string(devFeatures.shaderUniformBufferArrayDynamicIndexing));
    info.features.push_back("ShaderSampledImageArrayDynamicIndexing=" + std::to_string(devFeatures.shaderSampledImageArrayDynamicIndexing));
    info.features.push_back("ShaderStorageBufferArrayDynamicIndexing=" + std::to_string(devFeatures.shaderStorageBufferArrayDynamicIndexing));
    info.features.push_back("ShaderStorageImageArrayDynamicIndexing=" + std::to_string(devFeatures.shaderStorageImageArrayDynamicIndexing));
    info.features.push_back("ShaderClipDistance=" + std::to_string(devFeatures.shaderClipDistance));
    info.features.push_back("ShaderCullDistance=" + std::to_string(devFeatures.shaderCullDistance));
    info.features.push_back("ShaderFloat64=" + std::to_string(devFeatures.shaderFloat64));
    info.features.push_back("ShaderInt64=" + std::to_string(devFeatures.shaderInt64));
    info.features.push_back("ShaderInt16=" + std::to_string(devFeatures.shaderInt16));
    info.features.push_back("ShaderResourceResidency=" + std::to_string(devFeatures.shaderResourceResidency));
    info.features.push_back("ShaderResourceMinLod=" + std::to_string(devFeatures.shaderResourceMinLod));
    info.features.push_back("AlphaToOne=" + std::to_string(devFeatures.alphaToOne));
    info.features.push_back("SparseBinding=" + std::to_string(devFeatures.sparseBinding));
    info.features.push_back("SparseResidencyBuffer=" + std::to_string(devFeatures.sparseResidencyBuffer));
    info.features.push_back("SparseResidencyImage2D=" + std::to_string(devFeatures.sparseResidencyImage2D));
    info.features.push_back("SparseResidencyImage3D=" + std::to_string(devFeatures.sparseResidencyImage3D));
    info.features.push_back("SparseResidency2Samples=" + std::to_string(devFeatures.sparseResidency2Samples));
    info.features.push_back("SparseResidency4Samples=" + std::to_string(devFeatures.sparseResidency4Samples));
    info.features.push_back("SparseResidency8Samples=" + std::to_string(devFeatures.sparseResidency8Samples));
    info.features.push_back("SparseResidency16Samples=" + std::to_string(devFeatures.sparseResidency16Samples));
    info.features.push_back("SparseResidencyAliased=" + std::to_string(devFeatures.sparseResidencyAliased));

    return true;
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
        if (!gDevice->Init())
        {
            gDevice.reset();
            return nullptr;
        }
    }

    return gDevice.get();
}

void Release()
{
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
