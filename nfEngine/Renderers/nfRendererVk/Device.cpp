/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "Debugger.hpp"

// modules
#include "Translations.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "ResourceBinding.hpp"
#include "Buffer.hpp"
#include "VertexLayout.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"


namespace {

template<typename Type, typename Desc>
std::shared_ptr<Type> GenericCreateResource(const Desc& desc)
{
    std::shared_ptr<Type> resource = std::make_shared<Type>();
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

std::unique_ptr<Device> gDevice;

Device::Device()
    : mInstance()
    , mPhysicalDevice(VK_NULL_HANDLE)
    , mDevice(VK_NULL_HANDLE)
    , mCommandPool(VK_NULL_HANDLE)
    , mCurrentCommandBuffer(0)
    , mGraphicsQueueIndex(UINT32_MAX)
    , mGraphicsQueue(VK_NULL_HANDLE)
    , mPipelineCache(VK_NULL_HANDLE)
    , mRenderPassManager(nullptr)
    , mSemaphorePool(nullptr)
    , mRingBuffer(nullptr)
    , mDebugEnable(false)
{
}

Device::~Device()
{
    WaitForGPU();

    mRingBuffer.reset();
    mSemaphorePool.reset();
    mRenderPassManager.reset();

    if (mCommandBufferPool.size())
        vkFreeCommandBuffers(mDevice, mCommandPool, COMMAND_BUFFER_COUNT, mCommandBufferPool.data());

    if (mPipelineCache != VK_NULL_HANDLE)
        vkDestroyPipelineCache(mDevice, mPipelineCache, nullptr);
    if (mCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    if (mDevice != VK_NULL_HANDLE)
        vkDestroyDevice(mDevice, nullptr);

    Debugger::Instance().Release();
}

VkPhysicalDevice Device::SelectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices, int preferredId)
{
    if (preferredId < 0)
        preferredId = 0;

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
        LOG_DEBUG("  VP Bounds:  %f-%f", devProps.limits.viewportBoundsRange[0], devProps.limits.viewportBoundsRange[1]);
        LOG_DEBUG("  MaxBufSize: %u", devProps.limits.maxUniformBufferRange);
    }

    if (static_cast<size_t>(preferredId) >= devices.size())
    {
        LOG_ERROR("Preferred device ID #%i is not available", preferredId);
        return VK_NULL_HANDLE;
    }

    LOG_INFO("Selected device ID: %i", preferredId);
    return devices[preferredId];
}

bool Device::Init(const DeviceInitParams* params)
{
    DeviceInitParams defaultParams;
    if (!params)
        params = &defaultParams;

    if (!mInstance.Init(params->debugLevel > 0))
    {
        LOG_ERROR("Vulkan instance failed to initialize");
        return false;
    }

    const VkInstance& instance = mInstance.Get();

    // acquire GPU count first
    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    CHECK_VKRESULT(result, "Unable to enumerate physical devices");
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

    mPhysicalDevice = SelectPhysicalDevice(devices, params->preferredCardId);
    if (mPhysicalDevice == VK_NULL_HANDLE)
    {
        LOG_ERROR("Unable to select a physical device");
        return false;
    }

    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    // create a temporary platform-specific surface to get format capabilities
    VkSurfaceKHR tempSurface = VK_NULL_HANDLE;
    if (!CreateTemporarySurface(tempSurface))
    {
        LOG_ERROR("Unable to create a temporary surface to gather available formats");
        return false;
    }

    uint32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, tempSurface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        LOG_ERROR("Temporary surface does not have any formats on this physical device");
        return false;
    }

    mSupportedFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, tempSurface, &formatCount, mSupportedFormats.data());
    CleanupTemporarySurface(tempSurface);

    // Grab queue properties from our selected device
    uint32 queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOG_ERROR("Physical device does not have any queue family properties.");
        return false;
    }

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueCount, queueProps.data());

    for (uint32 i = 0; i < queueCount; ++i)
    {
        LOG_DEBUG("Queue #%u:", i);
        LOG_DEBUG("  Flags: %x", queueProps[i].queueFlags);
    }

    for (mGraphicsQueueIndex = 0; mGraphicsQueueIndex < queueCount; mGraphicsQueueIndex++)
        if (queueProps[mGraphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            break;

    if (mGraphicsQueueIndex == queueCount)
    {
        LOG_ERROR("Selected physical device does not support graphics queue.");
        return false;
    }

    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueInfo;
    VK_ZERO_MEMORY(queueInfo);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = mGraphicsQueueIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    std::vector<const char*> enabledExtensions;
    enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);


    if (params->debugLevel > 0)
    {
        // TODO right now Debug Markers are unsupported by drivers
        //      Uncomment when driver support appears
        //enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };

    VkDeviceCreateInfo devInfo;
    VK_ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = nullptr;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.pEnabledFeatures = nullptr;
    devInfo.enabledExtensionCount = static_cast<uint32>(enabledExtensions.size());
    devInfo.ppEnabledExtensionNames = enabledExtensions.data();
    if (params->debugLevel > 0)
    {
        devInfo.enabledLayerCount = 1;
        devInfo.ppEnabledLayerNames = enabledLayers;
    }

    result = vkCreateDevice(mPhysicalDevice, &devInfo, nullptr, &mDevice);
    CHECK_VKRESULT(result, "Failed to create Vulkan device");

    if (params->debugLevel > 0)
    {
        if (!Debugger::Instance().InitMarkers(mDevice))
        {
            LOG_ERROR("Vulkan debugging layer was requested, but is unavailable. Closing.");
            return false;
        }
    }

    if (!nfvkDeviceExtensionsInit(mDevice))
    {
        LOG_ERROR("Failed to initialize Vulkan device extensions.");
        return false;
    }

    vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);

    VkCommandPoolCreateInfo poolInfo;
    VK_ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mGraphicsQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create a graphics command pool");
        return false;
    }

    mCommandBufferPool.resize(COMMAND_BUFFER_COUNT);

    VkCommandBufferAllocateInfo cbInfo;
    VK_ZERO_MEMORY(cbInfo);
    cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbInfo.commandPool = mCommandPool;
    cbInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbInfo.commandBufferCount = COMMAND_BUFFER_COUNT;
    result = vkAllocateCommandBuffers(mDevice, &cbInfo, mCommandBufferPool.data());
    CHECK_VKRESULT(result, "Failed to initialize Command Buffer Pool");

    mRenderPassManager.reset(new RenderPassManager(mDevice));

    mSemaphorePool.reset(new SemaphorePool(mDevice));
    mSemaphorePool->Init(VK_SEMAPHORE_POOL_SIZE);

    mRingBuffer.reset(new RingBuffer(mDevice));
    mRingBuffer->Init(1024 * 1024);

    VkPipelineCacheCreateInfo pipeCacheInfo;
    VK_ZERO_MEMORY(pipeCacheInfo);
    pipeCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    result = vkCreatePipelineCache(mDevice, &pipeCacheInfo, nullptr, &mPipelineCache);

    LOG_INFO("Vulkan device initialized successfully");
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
    std::shared_ptr<ResourceBindingInstance> rbi = std::make_shared<ResourceBindingInstance>();
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
    std::shared_ptr<CommandRecorder> cr = std::make_shared<CommandRecorder>();
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
    VkPhysicalDeviceProperties devProps = {};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &devProps);

    info.description = devProps.deviceName;
    info.misc = "Vulkan API version: "
              + std::to_string(VK_VERSION_MAJOR(devProps.apiVersion)) + "."
              + std::to_string(VK_VERSION_MINOR(devProps.apiVersion)) + "."
              + std::to_string(VK_VERSION_PATCH(devProps.apiVersion));

    VkPhysicalDeviceFeatures devFeatures = {};
    vkGetPhysicalDeviceFeatures(mPhysicalDevice, &devFeatures);

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

bool Device::Execute(CommandListID commandList)
{
    if (commandList == INVALID_COMMAND_LIST_ID)
    {
        LOG_ERROR("Invalid Command List ID provided for execution");
        return false;
    }

    gDevice->WaitForGPU(); // TODO TEMPORARY

    // each time we submit a new command list, grab new semaphore to signal
    mSemaphorePool->Advance();

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSemaphore waitSemaphore = mSemaphorePool->GetPreviousSemaphore();
    VkSemaphore signalSemaphore = mSemaphorePool->GetCurrentSemaphore();

    VkSubmitInfo submitInfo;
    VK_ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBufferPool[commandList - 1];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
    submitInfo.pWaitDstStageMask = &pipelineStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;

    VkResult result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    CHECK_VKRESULT(result, "Failed to submit graphics operations");

    return true;
}

bool Device::WaitForGPU()
{
    VkResult result = vkQueueWaitIdle(mGraphicsQueue);
    CHECK_VKRESULT(result, "Failed to wait for graphics queue");
    return true;
}

bool Device::FinishFrame()
{
    return false;
}

bool Device::DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

bool Device::DownloadTexture(const TexturePtr& tex, void* data, int mipmap, int layer)
{
    UNUSED(tex);
    UNUSED(data);
    UNUSED(mipmap);
    UNUSED(layer);
    return false;
}

IDevice* Init(const DeviceInitParams* params)
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new (std::nothrow) Device);
        if (!gDevice->Init(params))
        {
            gDevice.reset();
            return nullptr;
        }
    }

    // initialize glslang library for shader processing
    // TODO right now glslang leaks lots of memory (one leak per TShader object,
    //      and one per glslang's Instance).
    //      Bump glslang version, fix it by yourself, or use other library for GLSL/HLSL->SPV.
    glslang::InitializeProcess();

    return gDevice.get();
}

void Release()
{
    glslang::FinalizeProcess();
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
