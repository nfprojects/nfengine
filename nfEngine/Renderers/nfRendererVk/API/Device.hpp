/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Device
 */

#pragma once

#include "Defines.hpp"

#include "../RendererInterface/Device.hpp"

#include "CommandRecorder.hpp"
#include "Internal/Instance.hpp"
#include "Internal/RenderPassManager.hpp"
#include "Internal/SemaphorePool.hpp"
#include "Internal/RingBuffer.hpp"

#include "nfCommon/Containers/UniquePtr.hpp"
#include "nfCommon/System/Window.hpp"


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
    VkCommandPool mCommandPool;
    Common::DynArray<VkCommandBuffer> mCommandBufferPool;
    uint32 mCurrentCommandBuffer;
    uint32 mGraphicsQueueIndex;
    VkQueue mGraphicsQueue;
    VkPipelineCache mPipelineCache;
    VkSemaphore mPrePresentSemaphore;
    VkSemaphore mPresentSemaphore;
    VkSemaphore mPostPresentSemaphore;
    bool mPresented;
    Common::DynArray<VkSurfaceFormatKHR> mSupportedFormats;
    Common::UniquePtr<RenderPassManager> mRenderPassManager;
    Common::UniquePtr<SemaphorePool> mSemaphorePool;
    Common::UniquePtr<RingBuffer> mRingBuffer;
    bool mDebugEnable;

    VkPhysicalDevice SelectPhysicalDevice(const Common::DynArray<VkPhysicalDevice>& devices, int preferredId);

    bool CreateTemporarySurface(VkSurfaceKHR& surface);
    void CleanupTemporarySurface(VkSurfaceKHR& surface);

    NFE_INLINE void SignalPresent()
    {
        mPresented = true;
    }

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

    NFE_INLINE const VkCommandPool& GetCommandPool() const
    {
        return mCommandPool;
    }

    NFE_INLINE const VkPipelineCache& GetPipelineCache() const
    {
        return mPipelineCache;
    }

    NFE_INLINE const VkQueue& GetQueue() const
    {
        return mGraphicsQueue;
    }

    NFE_INLINE const uint32& GetQueueIndex() const
    {
        return mGraphicsQueueIndex;
    }

    NFE_INLINE const VkSemaphore& GetPresentSemaphore() const
    {
        return mPresentSemaphore;
    }

    NFE_INLINE const VkSemaphore& GetPostPresentSemaphore() const
    {
        return mPostPresentSemaphore;
    }

    NFE_INLINE RenderPassManager* GetRenderPassManager() const
    {
        return mRenderPassManager.Get();
    }

    NFE_INLINE SemaphorePool* GetSemaphorePool() const
    {
        return mSemaphorePool.Get();
    }

    NFE_INLINE RingBuffer* GetRingBuffer() const
    {
        return mRingBuffer.Get();
    }

    NFE_INLINE uint32 GetCurrentCommandBuffer() const
    {
        return mCurrentCommandBuffer;
    }

    uint32 GetMemoryTypeIndex(uint32 typeBits, VkFlags properties);

    VkCommandBuffer GetAvailableCommandBuffer();

    // overrides
    void* GetHandle() const override;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(ElementFormat format) override;

    VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) override;
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
    bool Execute(CommandListID commandList) override;
    bool WaitForGPU() override;
    bool FinishFrame() override;

    bool DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer) override;
};

extern Common::UniquePtr<Device> gDevice;

extern "C" RENDERER_API IDevice* Init(const DeviceInitParams* params);
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namepsace NFE
