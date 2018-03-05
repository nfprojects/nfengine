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
#include "Internal/Debugger.hpp"
#include "Internal/RenderPassManager.hpp"
#include "Internal/RingBuffer.hpp"
#include "Internal/QueueManager.hpp"

#include "nfCommon/Containers/UniquePtr.hpp"
#include "nfCommon/Containers/SharedPtr.hpp"
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
    Debugger mDebugger;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VkDevice mDevice;

    Common::DynArray<VkCommandBuffer> mCommandBufferPool;
    uint32 mCurrentCommandBuffer;

    Common::DynArray<VkSurfaceFormatKHR> mSupportedFormats;
    RenderPassManager mRenderPassManager;
    QueueManager mQueueManager;
    RingBuffer mRingBuffer;

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

    NFE_INLINE VkDevice GetDevice() const
    {
        return mDevice;
    }

    NFE_INLINE VkPhysicalDevice GetPhysicalDevice() const
    {
        return mPhysicalDevice;
    }

    NFE_INLINE VkCommandPool GetCommandPool(QueueType queue) const
    {
        return mQueueManager.GetCommandPool(queue);
    }

    NFE_INLINE RenderPassManager* GetRenderPassManager()
    {
        return &mRenderPassManager;
    }

    NFE_INLINE RingBuffer* GetRingBuffer()
    {
        return &mRingBuffer;
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
    VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) override;
    ResourceBindingInstancePtr CreateResourceBindingInstance(const ResourceBindingSetPtr& set) override;
    CommandRecorderPtr CreateCommandRecorder() override;

    bool Execute(CommandListID commandList) override;
    bool WaitForGPU() override;
    bool FinishFrame() override;

    bool DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer) override;
};

extern "C" RENDERER_API IDevice* Init(const DeviceInitParams* params);
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namepsace NFE
