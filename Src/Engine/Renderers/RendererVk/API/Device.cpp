/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"

// modules
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "VertexLayout.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"
#include "Fence.hpp"
#include "MemoryBlock.hpp"

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

} // namespace

namespace NFE {
namespace Renderer {

Common::UniquePtr<Device> gDevice;

Device::Device()
    : mInstance()
    , mPhysicalDevice(VK_NULL_HANDLE)
    , mMemoryProperties()
    , mDevice(VK_NULL_HANDLE)
    , mPipelineCache(VK_NULL_HANDLE)
    , mSupportedFormats()
    , mQueueFamilyManager()
    , mDescriptorSetCache()
    , mFenceSignaller()
    , mRenderPassManager(nullptr)
    , mRingBuffer(nullptr)
    , mLayoutTracker()
    , mDebugEnable(false)
{
}

Device::~Device()
{
    if (mDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(mDevice);

    mRingBuffer.Reset();
    mRenderPassManager.Reset();
    mFenceSignaller.Release();
    for (auto& cbm: mCommandBufferManagers)
        cbm.Release();
    mQueueFamilyManager.Release();
    mDescriptorSetCache.Release();

    if (mDefaultSampler != VK_NULL_HANDLE)
        vkDestroySampler(mDevice, mDefaultSampler, nullptr);
    if (mPipelineCache != VK_NULL_HANDLE)
        vkDestroyPipelineCache(mDevice, mPipelineCache, nullptr);

    Debugger::Instance().ReleaseDebugObjectAnnotation();

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
        NFE_LOG_ERROR("No physical devices available.");
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

    // create a temporary platform-specific surface to get platform's capabilities
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
    if (!mQueueFamilyManager.PreInit(mPhysicalDevice))
    {
        NFE_LOG_ERROR("Queue Family Manager failed to pre-initialize");
        return false;
    }


    Common::DynArray<const char*> enabledExtensions;
    enabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    enabledExtensions.PushBack(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    enabledExtensions.PushBack(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);


    VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeatures;
    VK_ZERO_MEMORY(robustnessFeatures);
    robustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    robustnessFeatures.nullDescriptor = VK_TRUE;

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT divisorFeatures;
    VK_ZERO_MEMORY(divisorFeatures);
    divisorFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    divisorFeatures.pNext = &robustnessFeatures;
    divisorFeatures.vertexAttributeInstanceRateDivisor = VK_TRUE;
    divisorFeatures.vertexAttributeInstanceRateZeroDivisor = VK_TRUE;

    VkPhysicalDeviceFeatures2 features;
    VK_ZERO_MEMORY(features);
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &divisorFeatures;
    features.features.samplerAnisotropy = VK_TRUE;
    features.features.tessellationShader = VK_TRUE;


    VkDeviceCreateInfo devInfo;
    VK_ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = &features;
    devInfo.queueCreateInfoCount = mQueueFamilyManager.GetCreateInfos().Size();
    devInfo.pQueueCreateInfos = mQueueFamilyManager.GetCreateInfos().Data();
    devInfo.enabledExtensionCount = enabledExtensions.Size();
    devInfo.ppEnabledExtensionNames = enabledExtensions.Data();

    result = vkCreateDevice(mPhysicalDevice, &devInfo, nullptr, &mDevice);
    CHECK_VKRESULT(result, "Failed to create Vulkan device");

    if (!nfvkDeviceExtensionsInit(mDevice))
    {
        NFE_LOG_ERROR("Failed to initialize Vulkan device extensions.");
        return false;
    }

    if (params->debugLevel > 0)
    {
        if (!Debugger::Instance().InitDebugObjectAnnotation(mDevice))
        {
            NFE_LOG_WARNING("Debug Object Annotation unavailable");
        }
    }

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mDevice), VK_OBJECT_TYPE_DEVICE, "Device");

    if (!mQueueFamilyManager.Init(mDevice))
    {
        NFE_LOG_ERROR("Failed to finish Queue Family Manager initialization");
        return false;
    }

    for (uint32 i = 0; i < static_cast<uint32>(CommandQueueType::Max); ++i)
    {
        if (!mCommandBufferManagers[i].Init(mDevice, mQueueFamilyManager, static_cast<CommandQueueType>(i)))
        {
            NFE_LOG_ERROR("Failed to initialize Command Buffer Manager for Queue Family #%u", i);
            return false;
        }
    }

    mRenderPassManager = Common::MakeUniquePtr<RenderPassManager>(mDevice);

    // TODO Ring Buffer must be per command pool most probably
    mRingBuffer = Common::MakeUniquePtr<RingBuffer>(mDevice);
    mRingBuffer->Init(1024 * 1024);


    VkPipelineCacheCreateInfo pipeCacheInfo;
    VK_ZERO_MEMORY(pipeCacheInfo);
    pipeCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    result = vkCreatePipelineCache(mDevice, &pipeCacheInfo, nullptr, &mPipelineCache);
    CHECK_VKRESULT(result, "Failed to create Pipeline Cache");


    VkSamplerCreateInfo samplerInfo;
    VK_ZERO_MEMORY(samplerInfo);
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = false;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.compareEnable = false;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = FLT_MIN;
    samplerInfo.maxLod = FLT_MAX;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    result = vkCreateSampler(mDevice, &samplerInfo, nullptr, &mDefaultSampler);
    CHECK_VKRESULT(result, "Failed to create default Sampler");


    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mPipelineCache), VK_OBJECT_TYPE_PIPELINE_CACHE, "Device-PipelineCache");


    if (!mDescriptorSetCache.Init())
    {
        NFE_LOG_ERROR("Failed to initialize Descriptor Set Cache");
        return false;
    }

    if (!mFenceSignaller.Init())
    {
        NFE_LOG_ERROR("Failed to start Fence Signaller");
        return false;
    }


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

FencePtr Device::CreateFence(const FenceFlags flags)
{
    auto f = Common::MakeSharedPtr<Fence>(flags);
    if (!f)
    {
        return nullptr;
    }

    if (!f->Init())
    {
        return nullptr;
    }

    return f;
}

void* Device::GetHandle() const
{
    return reinterpret_cast<void*>(mDevice);
}

VertexLayoutPtr Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    return GenericCreateResource<VertexLayout, VertexLayoutDesc>(desc);
}

MemoryBlockPtr Device::CreateMemoryBlock(const MemoryBlockDesc& desc)
{
    return GenericCreateResource<MemoryBlock, MemoryBlockDesc>(desc);
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

CommandQueuePtr Device::CreateCommandQueue(CommandQueueType type, const char* debugName)
{
    auto cq = Common::MakeSharedPtr<CommandQueue>();
    if (!cq)
    {
        return nullptr;
    }

    if (!cq->Init(type, debugName))
    {
        return nullptr;
    }

    return cq;
}

bool Device::IsBackbufferFormatSupported(Format format)
{
    VkFormat bbFormat = TranslateFormatToVkFormat(format);
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

bool Device::CalculateTexturePlacementInfo(Format format, uint32 width, uint32 height, uint32 depth,
                                           TexturePlacementInfo& outInfo) const
{
    NFE_UNUSED(format);
    NFE_UNUSED(width);
    NFE_UNUSED(height);
    NFE_UNUSED(depth);
    NFE_UNUSED(outInfo);

    return false;
}

bool Device::FinishFrame()
{
    mRingBuffer->FinishFrame();
    for (auto& cbm: mCommandBufferManagers)
        cbm.FinishFrame();

    return true;
}

CommandListPtr Device::CreateCommandList(CommandQueueType queueType, VkCommandBuffer cmdBuffer, const UsedDescriptorSetsArray& sets)
{
    auto cl = Common::MakeUniquePtr<CommandList>(queueType, cmdBuffer, sets);
    if (!cl)
    {
        return nullptr;
    }

    return cl;
}

IDevice* Init(const DeviceInitParams* params)
{
    if (gDevice == nullptr)
    {
        gDevice = Common::MakeUniquePtr<Device>();
        if (!gDevice->Init(params))
        {
            gDevice.Reset();
            return nullptr;
        }
    }

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
