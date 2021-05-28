/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Device
 */

#pragma once

#include "Defines.hpp"

#include "../RendererCommon/Device.hpp"

#include "CommandRecorder.hpp"
#include "Internal/Instance.hpp"
#include "Internal/QueueFamilyManager.hpp"
#include "Internal/CommandBufferManager.hpp"
#include "Internal/FenceSignaller.hpp"
#include "Internal/RenderPassManager.hpp"
#include "Internal/SemaphorePool.hpp"
#include "Internal/RingBuffer.hpp"
#include "Internal/Utilities.hpp"

#include "Engine/Common/Containers/UniquePtr.hpp"
#include "Engine/Common/System/Window.hpp"


namespace NFE {
namespace Renderer {

class Device : public IDevice
{
private:
    friend class Backbuffer;

    // required to collect available formats for Backbuffer
    Common::Window mInvisibleWindow;

    Instance mInstance;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;
    VkSampler mDefaultSampler;
    VkPipelineCache mPipelineCache;
    Common::DynArray<VkSurfaceFormatKHR> mSupportedFormats;
    QueueFamilyManager mQueueFamilyManager;
    Common::FixedArray<CommandBufferManager, static_cast<uint32>(CommandQueueType::Max)> mCommandBufferManagers;
    FenceSignaller mFenceSignaller;
    Common::UniquePtr<RenderPassManager> mRenderPassManager;
    Common::UniquePtr<RingBuffer> mRingBuffer;
    bool mDebugEnable;

    VkPhysicalDevice SelectPhysicalDevice(const Common::DynArray<VkPhysicalDevice>& devices, int preferredId);

    bool CreateTemporarySurface(VkSurfaceKHR& surface);
    void CleanupTemporarySurface(VkSurfaceKHR& surface);

public:
    Device();
    ~Device();

    bool Init(const DeviceInitParams* params);

    NFE_INLINE const VkInstance& GetInstance() const
    {
        return mInstance.Get();
    }

    NFE_INLINE const VkDevice& GetDevice() const
    {
        return mDevice;
    }

    NFE_INLINE const VkPhysicalDevice& GetPhysicalDevice() const
    {
        return mPhysicalDevice;
    }

    NFE_INLINE const VkDescriptorPool& GetDescriptorPool() const
    {
        return mDescriptorPool;
    }

    NFE_INLINE const VkPipelineCache& GetPipelineCache() const
    {
        return mPipelineCache;
    }

    NFE_INLINE QueueFamilyManager& GetQueueFamilyManager()
    {
        return mQueueFamilyManager;
    }

    NFE_INLINE CommandBufferManager& GetCommandBufferManager(CommandQueueType type)
    {
        return mCommandBufferManagers[Util::CommandQueueTypeToIndex(type)];
    }

    NFE_INLINE FenceSignaller& GetFenceSignaller()
    {
        return mFenceSignaller;
    }

    NFE_INLINE RenderPassManager* GetRenderPassManager() const
    {
        return mRenderPassManager.Get();
    }

    NFE_INLINE RingBuffer* GetRingBuffer() const
    {
        return mRingBuffer.Get();
    }


    NFE_INLINE const VkSampler& GetDefaultSampler() const
    {
        return mDefaultSampler;
    }

    uint32 GetMemoryTypeIndex(uint32 typeBits, VkFlags properties);
    FencePtr CreateFence(const FenceFlags flags);

    // overrides
    void* GetHandle() const override;

    VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) override;
    MemoryBlockPtr CreateMemoryBlock(const MemoryBlockDesc& desc) override;
    BufferPtr CreateBuffer(const BufferDesc& desc) override;
    TexturePtr CreateTexture(const TextureDesc& desc) override;
    BackbufferPtr CreateBackbuffer(const BackbufferDesc& desc) override;
    RenderTargetPtr CreateRenderTarget(const RenderTargetDesc& desc) override;
    PipelineStatePtr CreatePipelineState(const PipelineStateDesc& desc) override;
    ComputePipelineStatePtr CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
    SamplerPtr CreateSampler(const SamplerDesc& desc) override;
    ShaderPtr CreateShader(const ShaderDesc& desc) override;
    ResourceBindingSetPtr CreateResourceBindingSet(const ResourceBindingSetDesc& desc) override;
    ResourceBindingLayoutPtr CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) override;
    ResourceBindingInstancePtr CreateResourceBindingInstance(const ResourceBindingSetPtr& set) override;
    CommandRecorderPtr CreateCommandRecorder() override;
    CommandQueuePtr CreateCommandQueue(CommandQueueType type, const char* debugName) override;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(Format format) override;
    bool CalculateTexturePlacementInfo(Format format, uint32 width, uint32 height, uint32 depth,
                                       TexturePlacementInfo& outInfo) const override;

    bool FinishFrame() override;

    CommandListPtr CreateCommandList(CommandQueueType queueType, VkCommandBuffer cmdBuffer);
};

extern Common::UniquePtr<Device> gDevice;

extern "C" RENDERER_API IDevice* Init(const DeviceInitParams* params);
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namepsace NFE
