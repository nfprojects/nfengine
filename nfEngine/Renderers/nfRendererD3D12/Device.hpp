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

#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/WeakPtr.hpp"

#include <atomic>


namespace NFE {
namespace Renderer {

class CommandRecorder;
class CommandListManager;

using CommandRecorderWeakPtr = Common::WeakPtr<CommandRecorder>;

class Device : public IDevice
{
    friend class Backbuffer;
    friend class CommandRecorder;
    friend class RenderTarget;

    D3D_FEATURE_LEVEL mFeatureLevel;

    Common::DynArray<D3DPtr<IDXGIAdapter>> mAdapters;
    int mAdapterInUse;

    D3DPtr<IDXGIFactory4> mDXGIFactory;
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<ID3D12CommandQueue> mCommandQueue;
    D3DPtr<ID3D12DebugDevice> mDebugDevice;
    D3DPtr<ID3D12InfoQueue> mInfoQueue;

    uint64 mFrameCounter;       // total frame counter
    uint32 mBufferingDepth;     // number of queued frames
    uint32 mFrameBufferIndex;   // current frame (command allocator index)
    uint32 mEnqueuedFrames;     // number of frames submitted to the GPU

    // global D3D12 command lists manager
    Common::UniquePtr<CommandListManager> mCommandListManager;

    // created command recorders, in-use by user
    Common::DynArray<CommandRecorderPtr> mCommandRecorders;

    // List of command recorders which ref count reached 1, which means they are not in use by the user
    // However, they cannot be destroyed immediately, because they may keep references to resources
    // that are used by the GPU.
    Common::DynArray<CommandRecorderPtr> mCommandRecordersToRemove;

    // synchronization objects
    D3DPtr<ID3D12Fence> mFence;
    Common::DynArray<uint64> mFenceValues;
    HANDLE mFenceEvent;

    HeapAllocator mCbvSrvUavHeapAllocator;
    HeapAllocator mRtvHeapAllocator;
    HeapAllocator mDsvHeapAllocator;

    bool mDebugLayerEnabled;

    void NotifyCommandRecordersFrameCompleted(uint64 completedFrameIndex);
    
    bool InitDebugLayer(int32 level);
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
    bool DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer) override;

    bool WaitForGPU() override;

    CommandListManager* GetCommandListManager() const
    {
        return mCommandListManager.Get();
    }

    HeapAllocator& GetCbvSrvUavHeapAllocator()
    {
        return mCbvSrvUavHeapAllocator;
    }

    HeapAllocator& GetRtvHeapAllocator()
    {
        return mRtvHeapAllocator;
    }

    HeapAllocator& GetDsvHeapAllocator()
    {
        return mDsvHeapAllocator;
    }

    bool IsDebugLayerEnabled() const
    {
        return mDebugLayerEnabled;
    }
};

} // namespace Renderer
} // namespace NFE
