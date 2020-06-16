/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"

// modules
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "ResourceBinding.hpp"
#include "Buffer.hpp"
#include "VertexLayout.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "Internal/Translations.hpp"
#include "Internal/Debugger.hpp"

#include "Engine/Common/Utils/StringUtils.hpp"


namespace {

template<typename Type, typename Desc>
NFE::Common::SharedPtr<Type> GenericCreateResource(const Desc& desc)
{
    auto resource = NFE::Common::MakeSharedPtr<Type>();
    if (!resource)
    {
        return nullptr;
    }

    if (!resource->Init(desc))
    {
        return nullptr;
    }

    return resource;
}

const NFE::uint32 COMMAND_BUFFER_COUNT = 5;

} // namespace

namespace NFE {
namespace Renderer {

Common::UniquePtr<Device> gDevice;

Device::Device()
    : mInstance()
    , mPhysicalDevice(VK_NULL_HANDLE)
    , mMemoryProperties()
    , mDevice(VK_NULL_HANDLE)
    , mDescriptorPool(VK_NULL_HANDLE)
    , mCommandPool(VK_NULL_HANDLE)
    , mCommandBufferPool()
    , mCurrentCommandBuffer(0)
    , mGraphicsQueueIndex(UINT32_MAX)
    , mGraphicsQueue(VK_NULL_HANDLE)
    , mPipelineCache(VK_NULL_HANDLE)
    , mSupportedFormats()
    , mRenderPassManager(nullptr)
    , mRingBuffer(nullptr)
    , mDebugEnable(false)
{
}

Device::~Device()
{
    if (mDevice != VK_NULL_HANDLE)
        WaitForGPU();

    mRingBuffer.Reset();
    mRenderPassManager.Reset();

    if (mCommandBufferPool.Size())
        vkFreeCommandBuffers(mDevice, mCommandPool, COMMAND_BUFFER_COUNT, mCommandBufferPool.Data());

    if (mPipelineCache != VK_NULL_HANDLE)
        vkDestroyPipelineCache(mDevice, mPipelineCache, nullptr);
    if (mCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
    if (mDevice != VK_NULL_HANDLE)
        vkDestroyDevice(mDevice, nullptr);
}

VkPhysicalDevice Device::SelectPhysicalDevice(const Common::DynArray<VkPhysicalDevice>& devices, int preferredId)
{
    if (preferredId < 0)
        preferredId = 0;

    VkPhysicalDeviceProperties devProps;

    // Debugging-related device description printing
    NFE_LOG_DEBUG("%u physical devices detected:", devices.Size());
    for (uint32 i = 0; i < devices.Size(); ++i)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devProps);
        NFE_LOG_DEBUG("Device #%u - %s:", i, devProps.deviceName);
        NFE_LOG_DEBUG("  ID:         %u",       devProps.deviceID);
        NFE_LOG_DEBUG("  Type:       %d (%s)",  devProps.deviceType,
                                            TranslateDeviceTypeToString(devProps.deviceType));
        NFE_LOG_DEBUG("  Vendor ID:  %u",       devProps.vendorID);
        NFE_LOG_DEBUG("  API ver:    %u.%u.%u", VK_VERSION_MAJOR(devProps.apiVersion),
                                            VK_VERSION_MINOR(devProps.apiVersion),
                                            VK_VERSION_PATCH(devProps.apiVersion));
        NFE_LOG_DEBUG("  Driver ver: %u.%u.%u", VK_VERSION_MAJOR(devProps.driverVersion),
                                            VK_VERSION_MINOR(devProps.driverVersion),
                                            VK_VERSION_PATCH(devProps.driverVersion));
        NFE_LOG_DEBUG("  VP Bounds:  %f-%f", devProps.limits.viewportBoundsRange[0], devProps.limits.viewportBoundsRange[1]);
        NFE_LOG_DEBUG("  MaxBufSize: %u", devProps.limits.maxUniformBufferRange);
    }

    if (static_cast<uint32>(preferredId) >= devices.Size())
    {
        NFE_LOG_ERROR("Preferred device ID #%i is not available", preferredId);
        return VK_NULL_HANDLE;
    }

    NFE_LOG_INFO("Selected device ID: %i", preferredId);
    return devices[preferredId];
}

bool Device::Init(const DeviceInitParams* params)
{
    DeviceInitParams defaultParams;
    if (!params)
        params = &defaultParams;

    if (!mInstance.Init(params->debugLevel > 0))
    {
        NFE_LOG_ERROR("Vulkan instance failed to initialize");
        return false;
    }

    const VkInstance& instance = mInstance.Get();

    // acquire GPU count first
    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    CHECK_VKRESULT(result, "Unable to enumerate physical devices");
    if (gpuCount == 0)
    {
        NFE_LOG_ERROR("0 physical devices available.");
        return false;
    }

    // and now get details about our GPUs
    Common::DynArray<VkPhysicalDevice> devices;
    devices.Resize(gpuCount);
    result = vkEnumeratePhysicalDevices(instance, &gpuCount, devices.Data());
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Unable to get more information about physical devices");
        return false;
    }

    mPhysicalDevice = SelectPhysicalDevice(devices, params->preferredCardId);
    if (mPhysicalDevice == VK_NULL_HANDLE)
    {
        NFE_LOG_ERROR("Unable to select a physical device");
        return false;
    }

    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    // create a temporary platform-specific surface to get format capabilities
    VkSurfaceKHR tempSurface = VK_NULL_HANDLE;
    if (!CreateTemporarySurface(tempSurface))
    {
        NFE_LOG_ERROR("Unable to create a temporary surface to gather available formats");
        return false;
    }

    uint32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, tempSurface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        NFE_LOG_ERROR("Temporary surface does not have any formats on this physical device");
        return false;
    }

    mSupportedFormats.Resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, tempSurface, &formatCount, mSupportedFormats.Data());
    CleanupTemporarySurface(tempSurface);

    // Grab queue properties from our selected device
    uint32 queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        NFE_LOG_ERROR("Physical device does not have any queue family properties.");
        return false;
    }

    Common::DynArray<VkQueueFamilyProperties> queueProps;
    queueProps.Resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueCount, queueProps.Data());

    for (uint32 i = 0; i < queueCount; ++i)
    {
        NFE_LOG_DEBUG("Queue #%u:", i);
        NFE_LOG_DEBUG("  Flags: %x", queueProps[i].queueFlags);
    }

    for (mGraphicsQueueIndex = 0; mGraphicsQueueIndex < queueCount; mGraphicsQueueIndex++)
        if (queueProps[mGraphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            break;

    if (mGraphicsQueueIndex == queueCount)
    {
        NFE_LOG_ERROR("Selected physical device does not support graphics queue.");
        return false;
    }

    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueInfo;
    VK_ZERO_MEMORY(queueInfo);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = mGraphicsQueueIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    Common::DynArray<const char*> enabledExtensions;
    enabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };

    VkPhysicalDeviceFeatures features;
    VK_ZERO_MEMORY(features);
    features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo devInfo;
    VK_ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = nullptr;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.pEnabledFeatures = &features;
    devInfo.enabledExtensionCount = enabledExtensions.Size();
    devInfo.ppEnabledExtensionNames = enabledExtensions.Data();
    if (params->debugLevel > 0)
    {
        devInfo.enabledLayerCount = 1;
        devInfo.ppEnabledLayerNames = enabledLayers;
    }

    result = vkCreateDevice(mPhysicalDevice, &devInfo, nullptr, &mDevice);
    CHECK_VKRESULT(result, "Failed to create Vulkan device");

    if (!nfvkDeviceExtensionsInit(mDevice))
    {
        NFE_LOG_ERROR("Failed to initialize Vulkan device extensions.");
        return false;
    }

    vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);


    // TODO resize these if we run out of space
    // ALSO TODO move to separate manager which will resize the pool when needed
    Common::StaticArray<VkDescriptorPoolSize, 4> descPoolSizes;
    descPoolSizes.Resize(4);
    descPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    descPoolSizes[0].descriptorCount = 32;
    descPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descPoolSizes[1].descriptorCount = 256;
    descPoolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descPoolSizes[2].descriptorCount = 256;
    descPoolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descPoolSizes[3].descriptorCount = 8;

    VkDescriptorPoolCreateInfo descPoolInfo;
    VK_ZERO_MEMORY(descPoolInfo);
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.poolSizeCount = descPoolSizes.Size();
    descPoolInfo.pPoolSizes = descPoolSizes.Data();
    descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    for (const auto& s: descPoolSizes)
        descPoolInfo.maxSets += s.descriptorCount;
    result = vkCreateDescriptorPool(mDevice, &descPoolInfo, nullptr, &mDescriptorPool);
    CHECK_VKRESULT(result, "Failed to create Descriptor Pool");


    VkCommandPoolCreateInfo poolInfo;
    VK_ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mGraphicsQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool);
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Failed to create a graphics command pool");
        return false;
    }

    mCommandBufferPool.Resize(COMMAND_BUFFER_COUNT);

    VkCommandBufferAllocateInfo cbInfo;
    VK_ZERO_MEMORY(cbInfo);
    cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbInfo.commandPool = mCommandPool;
    cbInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbInfo.commandBufferCount = COMMAND_BUFFER_COUNT;
    result = vkAllocateCommandBuffers(mDevice, &cbInfo, mCommandBufferPool.Data());
    CHECK_VKRESULT(result, "Failed to initialize Command Buffer Pool");

    mRenderPassManager.Reset(new RenderPassManager(mDevice));

    mRingBuffer.Reset(new RingBuffer(mDevice));
    mRingBuffer->Init(1024 * 1024);

    VkPipelineCacheCreateInfo pipeCacheInfo;
    VK_ZERO_MEMORY(pipeCacheInfo);
    pipeCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    result = vkCreatePipelineCache(mDevice, &pipeCacheInfo, nullptr, &mPipelineCache);

    NFE_LOG_INFO("Vulkan device initialized successfully");
    return true;
}

uint32 Device::GetMemoryTypeIndex(uint32 typeBits, VkFlags properties)
{
    for (uint32 i = 0; i < mMemoryProperties.memoryTypeCount; ++i)
    {
        if (typeBits & (1 << i))
            if (mMemoryProperties.memoryTypes[i].propertyFlags & properties)
                return i;
    }

    return UINT32_MAX;
}

VkCommandBuffer Device::GetAvailableCommandBuffer()
{
    mCurrentCommandBuffer++;
    if (mCurrentCommandBuffer >= COMMAND_BUFFER_COUNT)
    {
        mCurrentCommandBuffer = 0;
    }

    return mCommandBufferPool[mCurrentCommandBuffer];
}

void* Device::GetHandle() const
{
    return nullptr;
}

VertexLayoutPtr Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    return GenericCreateResource<VertexLayout, VertexLayoutDesc>(desc);
}

BufferPtr Device::CreateBuffer(const BufferDesc& desc)
{
    return GenericCreateResource<Buffer, BufferDesc>(desc);
}

TexturePtr Device::CreateTexture(const TextureDesc& desc)
{
    return GenericCreateResource<Texture, TextureDesc>(desc);
}

BackbufferPtr Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    return GenericCreateResource<Backbuffer, BackbufferDesc>(desc);
}

RenderTargetPtr Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    return GenericCreateResource<RenderTarget, RenderTargetDesc>(desc);
}

PipelineStatePtr Device::CreatePipelineState(const PipelineStateDesc& desc)
{
    return GenericCreateResource<PipelineState, PipelineStateDesc>(desc);
}

ComputePipelineStatePtr Device::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
{
    return GenericCreateResource<ComputePipelineState, ComputePipelineStateDesc>(desc);
}

SamplerPtr Device::CreateSampler(const SamplerDesc& desc)
{
    return GenericCreateResource<Sampler, SamplerDesc>(desc);
}

ShaderPtr Device::CreateShader(const ShaderDesc& desc)
{
    return GenericCreateResource<Shader, ShaderDesc>(desc);
}

ResourceBindingSetPtr Device::CreateResourceBindingSet(const ResourceBindingSetDesc& desc)
{
    return GenericCreateResource<ResourceBindingSet, ResourceBindingSetDesc>(desc);
}

ResourceBindingLayoutPtr Device::CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc)
{
    return GenericCreateResource<ResourceBindingLayout, ResourceBindingLayoutDesc>(desc);
}

ResourceBindingInstancePtr Device::CreateResourceBindingInstance(const ResourceBindingSetPtr& set)
{
    auto rbi = Common::MakeSharedPtr<ResourceBindingInstance>();
    if (!rbi)
    {
        return nullptr;
    }

    if (!rbi->Init(set))
    {
        return nullptr;
    }

    return rbi;
}

CommandRecorderPtr Device::CreateCommandRecorder()
{
    auto cr = Common::MakeSharedPtr<CommandRecorder>();
    if (!cr)
    {
        return nullptr;
    }

    if (!cr->Init())
    {
        return nullptr;
    }

    return cr;
}

bool Device::IsBackbufferFormatSupported(ElementFormat format)
{
    VkFormat bbFormat = TranslateElementFormatToVkFormat(format);
    for (auto& f: mSupportedFormats)
        if (f.format == bbFormat)
            return true;

    return false;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    using namespace Common;

    VkPhysicalDeviceProperties devProps = {};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &devProps);

    info.description = devProps.deviceName;
    info.misc = "Vulkan API version: "
              + ToString(VK_VERSION_MAJOR(devProps.apiVersion)) + "."
              + ToString(VK_VERSION_MINOR(devProps.apiVersion)) + "."
              + ToString(VK_VERSION_PATCH(devProps.apiVersion));

    VkPhysicalDeviceFeatures devFeatures = {};
    vkGetPhysicalDeviceFeatures(mPhysicalDevice, &devFeatures);

    // Description of below features is available on Vulkan registry:
    //   https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkGetPhysicalDeviceFeatures.html
    info.features.PushBack("RobustBufferAccess=" + ToString(devFeatures.robustBufferAccess));
    info.features.PushBack("FullDrawIndexUint32=" + ToString(devFeatures.fullDrawIndexUint32));
    info.features.PushBack("ImageCubeArray=" + ToString(devFeatures.imageCubeArray));
    info.features.PushBack("IndependentBlend=" + ToString(devFeatures.independentBlend));
    info.features.PushBack("GeometryShader=" + ToString(devFeatures.geometryShader));
    info.features.PushBack("TesselationShader=" + ToString(devFeatures.tessellationShader));
    info.features.PushBack("SampleRateShading=" + ToString(devFeatures.sampleRateShading));
    info.features.PushBack("DualSourceBlend=" + ToString(devFeatures.dualSrcBlend));
    info.features.PushBack("LogicOp=" + ToString(devFeatures.logicOp));
    info.features.PushBack("DepthClamp=" + ToString(devFeatures.depthClamp));
    info.features.PushBack("DepthBiasClamp=" + ToString(devFeatures.depthBiasClamp));
    info.features.PushBack("DepthBounds=" + ToString(devFeatures.depthBounds));
    info.features.PushBack("FillModeNonSolid=" + ToString(devFeatures.fillModeNonSolid));
    info.features.PushBack("WideLines=" + ToString(devFeatures.wideLines));
    info.features.PushBack("LargePoints=" + ToString(devFeatures.largePoints));
    info.features.PushBack("TextureCompressionETC2=" + ToString(devFeatures.textureCompressionETC2));
    info.features.PushBack("TextureCompressionASTC_LDR=" + ToString(devFeatures.textureCompressionASTC_LDR));
    info.features.PushBack("TextureCompressionBC=" + ToString(devFeatures.textureCompressionBC));
    info.features.PushBack("OcclusionQueryPrecise=" + ToString(devFeatures.occlusionQueryPrecise));
    info.features.PushBack("PipelineStatisticsQuery=" + ToString(devFeatures.pipelineStatisticsQuery));
    info.features.PushBack("VertexPipelineStoresAndAtomics=" + ToString(devFeatures.vertexPipelineStoresAndAtomics));
    info.features.PushBack("FragmentStoresAndAtomics=" + ToString(devFeatures.fragmentStoresAndAtomics));
    info.features.PushBack("ShaderTessellationAndGeometryPointSize=" + ToString(devFeatures.shaderTessellationAndGeometryPointSize));
    info.features.PushBack("ShaderImageGatherExtended=" + ToString(devFeatures.shaderImageGatherExtended));
    info.features.PushBack("ShaderStorageImageExtendedFormats=" + ToString(devFeatures.shaderStorageImageExtendedFormats));
    info.features.PushBack("ShaderStorageImageMultisample=" + ToString(devFeatures.shaderStorageImageMultisample));
    info.features.PushBack("ShaderUniformBufferArrayDynamicIndexing=" + ToString(devFeatures.shaderUniformBufferArrayDynamicIndexing));
    info.features.PushBack("ShaderSampledImageArrayDynamicIndexing=" + ToString(devFeatures.shaderSampledImageArrayDynamicIndexing));
    info.features.PushBack("ShaderStorageBufferArrayDynamicIndexing=" + ToString(devFeatures.shaderStorageBufferArrayDynamicIndexing));
    info.features.PushBack("ShaderStorageImageArrayDynamicIndexing=" + ToString(devFeatures.shaderStorageImageArrayDynamicIndexing));
    info.features.PushBack("ShaderClipDistance=" + ToString(devFeatures.shaderClipDistance));
    info.features.PushBack("ShaderCullDistance=" + ToString(devFeatures.shaderCullDistance));
    info.features.PushBack("ShaderFloat64=" + ToString(devFeatures.shaderFloat64));
    info.features.PushBack("ShaderInt64=" + ToString(devFeatures.shaderInt64));
    info.features.PushBack("ShaderInt16=" + ToString(devFeatures.shaderInt16));
    info.features.PushBack("ShaderResourceResidency=" + ToString(devFeatures.shaderResourceResidency));
    info.features.PushBack("ShaderResourceMinLod=" + ToString(devFeatures.shaderResourceMinLod));
    info.features.PushBack("AlphaToOne=" + ToString(devFeatures.alphaToOne));
    info.features.PushBack("SparseBinding=" + ToString(devFeatures.sparseBinding));
    info.features.PushBack("SparseResidencyBuffer=" + ToString(devFeatures.sparseResidencyBuffer));
    info.features.PushBack("SparseResidencyImage2D=" + ToString(devFeatures.sparseResidencyImage2D));
    info.features.PushBack("SparseResidencyImage3D=" + ToString(devFeatures.sparseResidencyImage3D));
    info.features.PushBack("SparseResidency2Samples=" + ToString(devFeatures.sparseResidency2Samples));
    info.features.PushBack("SparseResidency4Samples=" + ToString(devFeatures.sparseResidency4Samples));
    info.features.PushBack("SparseResidency8Samples=" + ToString(devFeatures.sparseResidency8Samples));
    info.features.PushBack("SparseResidency16Samples=" + ToString(devFeatures.sparseResidency16Samples));
    info.features.PushBack("SparseResidencyAliased=" + ToString(devFeatures.sparseResidencyAliased));

    return true;
}

bool Device::Execute(CommandListID commandList)
{
    if (commandList == INVALID_COMMAND_LIST_ID)
    {
        NFE_LOG_ERROR("Invalid Command List ID provided for execution");
        return false;
    }

    gDevice->WaitForGPU(); // TODO TEMPORARY

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo submitInfo;
    VK_ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBufferPool[commandList - 1];
    submitInfo.pWaitDstStageMask = &pipelineStages;

    VkResult result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    CHECK_VKRESULT(result, "Failed to submit graphics operations");

    return true;
}

bool Device::WaitForGPU()
{
    VkResult result = vkDeviceWaitIdle(mDevice);
    CHECK_VKRESULT(result, "Failed to wait for graphics device");
    return true;
}

bool Device::FinishFrame()
{
    return false;
}

bool Device::DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data)
{
    NFE_UNUSED(buffer);
    NFE_UNUSED(offset);
    NFE_UNUSED(size);
    NFE_UNUSED(data);
    return false;
}

bool Device::DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer)
{
    NFE_UNUSED(tex);
    NFE_UNUSED(data);
    NFE_UNUSED(mipmap);
    NFE_UNUSED(layer);
    return false;
}

IDevice* Init(const DeviceInitParams* params)
{
    if (gDevice == nullptr)
    {
        gDevice.Reset(new Device);
        if (!gDevice->Init(params))
        {
            gDevice.Reset();
            return nullptr;
        }
    }

    // initialize glslang library for shader processing
    // TODO right now glslang leaks lots of memory (one leak per TShader object,
    //      and one per glslang's Instance).
    //      Bump glslang version, fix it by yourself, or use other library for GLSL/HLSL->SPV.
    glslang::InitializeProcess();

    return gDevice.Get();
}

void Release()
{
    glslang::FinalizeProcess();
    gDevice.Reset();
}

} // namespace Renderer
} // namespace NFE
