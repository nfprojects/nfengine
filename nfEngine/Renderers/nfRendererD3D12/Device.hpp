/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's device.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"
#include "PipelineState.hpp"
#include "HeapAllocator.hpp"
#include "RingBuffer.hpp"

#include <map>
#include <atomic>


namespace NFE {
namespace Renderer {

class CommandRecorder;
class CommandListManager;

class Device : public IDevice
{
    friend class Backbuffer;
    friend class CommandRecorder;
    friend class RenderTarget;

    D3D_FEATURE_LEVEL mFeatureLevel;

    std::vector<D3DPtr<IDXGIAdapter>> mAdapters;
    int mAdapterInUse;

    D3DPtr<IDXGIFactory4> mDXGIFactory;
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<ID3D12CommandQueue> mCommandQueue;
    D3DPtr<ID3D12DebugDevice> mDebugDevice;
    D3DPtr<ID3D12InfoQueue> mInfoQueue;

    uint64 mFrameCounter;       // total frame counter
    uint32 mFrameCount;         // number of queued frames
    uint32 mFrameBufferIndex;   // current frame (command allocator index)
    std::vector<D3DPtr<ID3D12CommandAllocator>> mCommandAllocators;
    D3DPtr<ID3D12GraphicsCommandList> mCommandList;

    // synchronization objects
    D3DPtr<ID3D12Fence> mFence;
    std::vector<uint64> mFenceValues;
    HANDLE mFenceEvent;

    std::unique_ptr<CommandListManager> mCommandListManager;

    HeapAllocator mCbvSrvUavHeapAllocator;
    HeapAllocator mRtvHeapAllocator;
    HeapAllocator mDsvHeapAllocator;

    bool mDebugLayerEnabled;

    bool InitializeDevice(const DeviceInitParams* params);
    bool DetectFeatureLevel();
    bool PrepareDebugLayer();
    bool DetectVideoCards(int preferredId);
    bool CreateResources();

public:
    Device();
    ~Device();
    bool Init(const DeviceInitParams* params);

    ID3D12Device* GetDevice() const;
    ID3D12CommandQueue* GetCommandQueue() const;
    void* GetHandle() const override;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(ElementFormat format) override;

    /// Resources creation functions

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
    bool FinishFrame() override;

    bool DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(const TexturePtr& tex, void* data, int mipmap, int layer) override;

    bool WaitForGPU() override;

    NFE_INLINE CommandListManager* GetCommandListManager() const
    {
        return mCommandListManager.get();
    }

    NFE_INLINE HeapAllocator& GetCbvSrvUavHeapAllocator()
    {
        return mCbvSrvUavHeapAllocator;
    }

    NFE_INLINE HeapAllocator& GetRtvHeapAllocator()
    {
        return mRtvHeapAllocator;
    }

    NFE_INLINE HeapAllocator& GetDsvHeapAllocator()
    {
        return mDsvHeapAllocator;
    }

    NFE_INLINE bool IsDebugLayerEnabled()
    {
        return mDebugLayerEnabled;
    }
};

} // namespace Renderer
} // namespace NFE
