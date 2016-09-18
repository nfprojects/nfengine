/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Device
 */

#pragma once

#include "Defines.hpp"

#include "../RendererInterface/Device.hpp"

#include "CommandBuffer.hpp"
#include "Instance.hpp"


namespace NFE {
namespace Renderer {

class Device : public IDevice
{
private:
    Instance mInstance;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VkDevice mDevice;
    VkCommandPool mCommandPool;
    uint32 mGraphicsQueueIndex;
    VkQueue mGraphicsQueue;
    VkSemaphore mRenderSemaphore;
    VkSemaphore mPresentSemaphore;
    VkSemaphore mPostPresentSemaphore;
    VkPipelineCache mPipelineCache;
    std::vector<VkSurfaceFormatKHR> mSupportedFormats;
    bool mDebugEnable;

    VkPhysicalDevice SelectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices, int preferredId);

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

    NFE_INLINE const VkCommandPool& GetCommandPool() const
    {
        return mCommandPool;
    }

    NFE_INLINE const VkSemaphore& GetRenderSemaphore() const
    {
        return mRenderSemaphore;
    }

    NFE_INLINE const VkSemaphore& GetPresentSemaphore() const
    {
        return mPresentSemaphore;
    }

    NFE_INLINE const VkSemaphore& GetPostPresentSemaphore() const
    {
        return mPostPresentSemaphore;
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

    uint32 GetMemoryTypeIndex(uint32 typeBits, VkFlags properties);

    // overrides
    void* GetHandle() const override;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(ElementFormat format) override;

    IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc) override;
    IBuffer* CreateBuffer(const BufferDesc& desc) override;
    ITexture* CreateTexture(const TextureDesc& desc) override;
    IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc) override;
    IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc) override;
    IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) override;
    IComputePipelineState* CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
    ISampler* CreateSampler(const SamplerDesc& desc) override;
    IShader* CreateShader(const ShaderDesc& desc) override;
    IResourceBindingSet* CreateResourceBindingSet(const ResourceBindingSetDesc& desc) override;
    IResourceBindingLayout* CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) override;
    IResourceBindingInstance* CreateResourceBindingInstance(IResourceBindingSet* set) override;

    ICommandBuffer* CreateCommandBuffer() override;
    bool Execute(ICommandList* commandList) override;
    bool WaitForGPU() override;

    bool DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(ITexture* tex, void* data, int mipmap = 0, int layer = 0) override;
};

extern std::unique_ptr<Device> gDevice;

extern "C" RENDERER_API IDevice* Init(const DeviceInitParams* params);
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namepsace NFE
