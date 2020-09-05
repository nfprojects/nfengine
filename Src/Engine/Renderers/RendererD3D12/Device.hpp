/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's device.
 */

#pragma once

#include "../RendererCommon/Device.hpp"
#include "Fence.hpp"
#include "PipelineState.hpp"
#include "HeapAllocator.hpp"
#include "RingBuffer.hpp"
#include "ShaderCompiler.hpp"

#include "D3D12MemAlloc.h"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"
#include "Engine/Common/Containers/WeakPtr.hpp"

#include <atomic>


namespace NFE {
namespace Renderer {

class Device;
class CommandRecorder;
class CommandListManager;

using CommandRecorderWeakPtr = Common::WeakPtr<CommandRecorder>;

struct MonitorInfo
{
    Common::String name;
    int32 x;
    int32 y;
    uint32 width;
    uint32 height;
    bool valid;
};

struct DeviceCaps
{
    bool tearingSupport = false;
};

class Device : public IDevice
{
    friend class Backbuffer;
    friend class CommandRecorder;
    friend class InternalCommandList;
    friend class RenderTarget;
    friend class Buffer;
    friend class Texture;

    DeviceCaps mCaps;

    D3D_FEATURE_LEVEL mFeatureLevel;

    D3DPtr<IDXGIAdapter> mAdapter;
    D3DPtr<IDXGIFactory4> mDXGIFactory;
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<ID3D12CommandQueue> mGraphicsQueue;
    D3DPtr<ID3D12CommandQueue> mResourceUploadQueue;
    D3DPtr<ID3D12DebugDevice> mDebugDevice;
    D3DPtr<ID3D12InfoQueue> mInfoQueueD3D;
    D3DPtr<IDXGIInfoQueue> mInfoQueueDXGI;

    D3D12MA::Allocator* mAllocator = nullptr;

    // global D3D12 command lists manager
    Common::UniquePtr<CommandListManager> mCommandListManager;

    // List of command recorders which ref count reached 1, which means they are not in use by the user
    // However, they cannot be destroyed immediately, because they may keep references to resources
    // that are used by the GPU.
    Common::DynArray<CommandRecorderPtr> mCommandRecordersToRemove;

    // synchronization objects
    FenceData mFrameFence;
    FenceData mGraphicsQueueFence;
    FenceData mResourceUploadQueueFence;

    FenceManager mFenceManager;

    static constexpr uint32 MaxPendingFrames = 2;
    Common::StaticArray<FencePtr, MaxPendingFrames> mPendingFramesFences;

    RingBuffer mRingBuffer;

    HeapAllocator mCbvSrvUavHeapStagingAllocator;   // CPU visible
    HeapAllocator mCbvSrvUavHeapAllocator;          // GPU visible
    HeapAllocator mRtvHeapAllocator;
    HeapAllocator mDsvHeapAllocator;

    ShaderCompiler mShaderCompiler;

    bool mDebugLayerEnabled;

    bool InitializeDevice(const DeviceInitParams* params);
    bool DetectFeatureLevel();
    bool PrepareD3DDebugLayer();
    bool DetectVideoCards(int preferredId);
    bool DetectMonitors();
    bool CreateResources();

public:
    Device();
    ~Device();
    bool Init(const DeviceInitParams* params);

    const DeviceCaps& GetCaps() const { return mCaps; }
    ID3D12Device* GetDevice() const { return mDevice.Get(); }
    ID3D12CommandQueue* GetGraphicsQueue() const { return mGraphicsQueue.Get(); }
    ID3D12CommandQueue* GetResourceUploadQueue() const { return mResourceUploadQueue.Get(); }
    void* GetHandle() const override;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(Format format) override;
    Common::DynArray<MonitorInfo> GetMonitorsInfo() const;

    D3D12MA::Allocator* GetAllocator() const { return mAllocator; }

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

    FencePtr WaitForGPU() override;

    CommandListManager* GetCommandListManager() const { return mCommandListManager.Get(); }

    HeapAllocator& GetCbvSrvUavHeapStagingAllocator() { return mCbvSrvUavHeapStagingAllocator; }
    HeapAllocator& GetCbvSrvUavHeapAllocator() { return mCbvSrvUavHeapAllocator; }
    HeapAllocator& GetRtvHeapAllocator() { return mRtvHeapAllocator; }
    HeapAllocator& GetDsvHeapAllocator() { return mDsvHeapAllocator; }

    ShaderCompiler& GetShaderCompiler() { return mShaderCompiler; }

    bool IsDebugLayerEnabled() const { return mDebugLayerEnabled; }

    RingBuffer& GetRingBuffer() { return mRingBuffer; }
};

} // namespace Renderer
} // namespace NFE
