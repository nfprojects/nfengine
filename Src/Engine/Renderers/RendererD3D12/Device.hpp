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
class CommandQueue;
class RingBuffer;

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
    D3DPtr<ID3D12Device1> mDevice;
    D3DPtr<ID3D12DebugDevice> mDebugDevice;
    D3DPtr<ID3D12InfoQueue> mInfoQueueD3D;
    D3DPtr<IDXGIInfoQueue> mInfoQueueDXGI;

    D3D12MA::Allocator* mAllocator = nullptr;

    // global D3D12 command lists manager
    Common::UniquePtr<CommandListManager> mCommandListManager;

    Common::DynArray<Common::WeakPtr<CommandQueue>> mCommandQueues;
    Common::RWLock mCommandQueuesLock;

    FenceManager mFenceManager;

    Common::UniquePtr<RingBuffer> mRingBuffer;

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

    uint32 ReleaseUnusedCommandQueues();

public:
    Device();
    ~Device();
    bool Init(const DeviceInitParams* params);

    const DeviceCaps& GetCaps() const { return mCaps; }
    ID3D12Device1* GetDevice() const { return mDevice.Get(); }
    virtual void* GetHandle() const override;
    virtual bool GetDeviceInfo(DeviceInfo& info) override;
    virtual bool IsBackbufferFormatSupported(Format format) override;
    Common::DynArray<MonitorInfo> GetMonitorsInfo() const;

    D3D12MA::Allocator* GetAllocator() const { return mAllocator; }

    /// Resources creation functions

    virtual VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) override;
    virtual BufferPtr CreateBuffer(const BufferDesc& desc) override;
    virtual TexturePtr CreateTexture(const TextureDesc& desc) override;
    virtual BackbufferPtr CreateBackbuffer(const BackbufferDesc& desc) override;
    virtual RenderTargetPtr CreateRenderTarget(const RenderTargetDesc& desc) override;
    virtual PipelineStatePtr CreatePipelineState(const PipelineStateDesc& desc) override;
    virtual ComputePipelineStatePtr CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
    virtual SamplerPtr CreateSampler(const SamplerDesc& desc) override;
    virtual ShaderPtr CreateShader(const ShaderDesc& desc) override;
    virtual ResourceBindingSetPtr CreateResourceBindingSet(const ResourceBindingSetDesc& desc) override;
    virtual ResourceBindingLayoutPtr CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) override;
    virtual ResourceBindingInstancePtr CreateResourceBindingInstance(const ResourceBindingSetPtr& set) override;

    virtual CommandRecorderPtr CreateCommandRecorder() override;
    virtual CommandQueuePtr CreateCommandQueue(CommandQueueType type) override;
    virtual bool FinishFrame() override;

    virtual bool DownloadBuffer(const BufferPtr& buf, const ResourceDownloadCallback& callback, Common::TaskBuilder& builder, uint32 offset, uint32 size) override;
    virtual bool DownloadTexture(const TexturePtr& tex, const ResourceDownloadCallback& callback, Common::TaskBuilder& builder, uint32 mipmap, uint32 layer) override;

    CommandListManager* GetCommandListManager() const { return mCommandListManager.Get(); }
    FenceManager& GetFenceManager() { return mFenceManager; }
    RingBuffer* GetRingBuffer() { return mRingBuffer.Get(); }

    HeapAllocator& GetCbvSrvUavHeapStagingAllocator() { return mCbvSrvUavHeapStagingAllocator; }
    HeapAllocator& GetCbvSrvUavHeapAllocator() { return mCbvSrvUavHeapAllocator; }
    HeapAllocator& GetRtvHeapAllocator() { return mRtvHeapAllocator; }
    HeapAllocator& GetDsvHeapAllocator() { return mDsvHeapAllocator; }

    ShaderCompiler& GetShaderCompiler() { return mShaderCompiler; }

    bool IsDebugLayerEnabled() const { return mDebugLayerEnabled; }
};

} // namespace Renderer
} // namespace NFE
