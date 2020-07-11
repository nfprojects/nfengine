/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's device.
 */

#pragma once

#include "../RendererCommon/Device.hpp"
#include "Common.hpp"
#include "PipelineState.hpp"
#include "HeapAllocator.hpp"
#include "RingBuffer.hpp"
#include "ShaderCompiler.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/WeakPtr.hpp"

#include <atomic>


namespace NFE {
namespace Renderer {

class Device;
class CommandRecorder;
class CommandListManager;

using CommandRecorderWeakPtr = Common::WeakPtr<CommandRecorder>;

struct FenceData
{
    static constexpr uint64 InitialFenceValue = 0xfefefefefefefefeULL;

    FenceData();

    bool Init(Device* device);
    void Release();
    bool Flush(ID3D12CommandQueue* queue);

    D3DPtr<ID3D12Fence> fenceObject;
    Common::RWLock lock;
    std::atomic<uint64> value;
    HANDLE waitEvent;
};

class Device : public IDevice
{
    friend class Backbuffer;
    friend class CommandRecorder;
    friend class InternalCommandList;
    friend class RenderTarget;
    friend class Buffer;
    friend class Texture;

    D3D_FEATURE_LEVEL mFeatureLevel;

    Common::DynArray<D3DPtr<IDXGIAdapter>> mAdapters;
    int mAdapterInUse;

    D3DPtr<IDXGIFactory4> mDXGIFactory;
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<ID3D12CommandQueue> mGraphicsQueue;
    D3DPtr<ID3D12CommandQueue> mResourceUploadQueue;
    D3DPtr<ID3D12DebugDevice> mDebugDevice;
    D3DPtr<ID3D12InfoQueue> mInfoQueue;

    // global D3D12 command lists manager
    Common::UniquePtr<CommandListManager> mCommandListManager;

    // List of command recorders which ref count reached 1, which means they are not in use by the user
    // However, they cannot be destroyed immediately, because they may keep references to resources
    // that are used by the GPU.
    Common::DynArray<CommandRecorderPtr> mCommandRecordersToRemove;

    // synchronization objects
    FenceData mGraphicsQueueFence;
    FenceData mResourceUploadQueueFence;

    RingBuffer mRingBuffer;

    HeapAllocator mCbvSrvUavHeapAllocator;
    HeapAllocator mRtvHeapAllocator;
    HeapAllocator mDsvHeapAllocator;

    ShaderCompiler mShaderCompiler;

    bool mDebugLayerEnabled;

    bool InitDebugLayer(int32 level);
    bool InitializeDevice(const DeviceInitParams* params);
    bool DetectFeatureLevel();
    bool PrepareD3DDebugLayer();
    bool PrepareDXGIDebugLayer();
    bool DetectVideoCards(int preferredId);
    bool CreateResources();

public:
    Device();
    ~Device();
    bool Init(const DeviceInitParams* params);

    ID3D12Device* GetDevice() const { return mDevice.Get(); }
    ID3D12CommandQueue* GetGraphicsQueue() const { return mGraphicsQueue.Get(); }
    ID3D12CommandQueue* GetResourceUploadQueue() const { return mResourceUploadQueue.Get(); }
    void* GetHandle() const override;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(Format format) override;

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
    bool Execute(const Common::ArrayView<ICommandList*> commandLists) override;
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

    ShaderCompiler& GetShaderCompiler()
    {
        return mShaderCompiler;
    }

    bool IsDebugLayerEnabled() const
    {
        return mDebugLayerEnabled;
    }

    RingBuffer& GetRingBuffer()
    {
        return mRingBuffer;
    }
};

} // namespace Renderer
} // namespace NFE
