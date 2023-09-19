/**
 * @file
 * @author  Witek902
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
    D3D_FEATURE_LEVEL featureLevel;
    D3D12_FEATURE_DATA_D3D12_OPTIONS d3dOptions;
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

    D3DPtr<IDXGIAdapter> mAdapter;
    D3DPtr<IDXGIFactory4> mDXGIFactory;
    D3DPtr<ID3D12Device1> mDevice;
    D3DPtr<ID3D12DebugDevice> mDebugDevice;
    D3DPtr<ID3D12InfoQueue> mInfoQueueD3D;
    D3DPtr<IDXGIInfoQueue> mInfoQueueDXGI;
    D3DPtr<ID3D12RootSignature> mGraphicsRootSignature;
    D3DPtr<ID3D12RootSignature> mComputeRootSignature;
    D3DPtr<ID3D12CommandSignature> mIndirectDispatchCommandSignature;

    D3D12MA::Allocator* mAllocator = nullptr;

    // global D3D12 command lists manager
    Common::UniquePtr<CommandListManager> mCommandListManager;

    Common::DynArray<Common::WeakPtr<CommandQueue>> mCommandQueues;
    Common::RWLock mCommandQueuesLock;

    FenceManager mFenceManager;

    Common::UniquePtr<RingBuffer> mRingBuffer;

    HeapAllocator mCbvSrvUavHeapStagingAllocator;   // CPU visible
    HeapAllocator mCbvSrvUavHeapAllocator;          // GPU visible
    HeapAllocator mSamplerHeapStagingAllocator;     // CPU visible
    HeapAllocator mSamplerHeapAllocator;            // GPU visible
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
    bool CreateGraphicsRootSignature();
    bool CreateComputeRootSignature();
    bool CreateIndirectDispatchCommandSignature();

    uint32 ReleaseUnusedCommandQueues();

public:
    Device();
    ~Device();
    bool Init(const DeviceInitParams* params);

    const DeviceCaps& GetCaps() const { return mCaps; }
    ID3D12Device1* GetDevice() const { return mDevice.Get(); }
    ID3D12RootSignature* GetGraphicsRootSignature() const { return mGraphicsRootSignature.Get(); }
    ID3D12RootSignature* GetComputeRootSignature() const { return mComputeRootSignature.Get(); }
    ID3D12CommandSignature* GetIndirectDispatchCommandSignature() const { return mIndirectDispatchCommandSignature.Get(); }

    virtual void* GetHandle() const override;
    virtual bool GetDeviceInfo(DeviceInfo& info) override;
    virtual bool IsBackbufferFormatSupported(Format format) override;
    virtual bool CalculateTexturePlacementInfo(Format format, uint32 width, uint32 height, uint32 depth, TexturePlacementInfo& outInfo) const override;
    Common::DynArray<MonitorInfo> GetMonitorsInfo() const;

    D3D12MA::Allocator* GetAllocator() const { return mAllocator; }

    /// Resources creation functions

    virtual VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) override;
    virtual MemoryBlockPtr CreateMemoryBlock(const MemoryBlockDesc& desc) override;
    virtual BufferPtr CreateBuffer(const BufferDesc& desc) override;
    virtual TexturePtr CreateTexture(const TextureDesc& desc) override;
    virtual BackbufferPtr CreateBackbuffer(const BackbufferDesc& desc) override;
    virtual RenderTargetPtr CreateRenderTarget(const RenderTargetDesc& desc) override;
    virtual PipelineStatePtr CreatePipelineState(const PipelineStateDesc& desc) override;
    virtual ComputePipelineStatePtr CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
    virtual SamplerPtr CreateSampler(const SamplerDesc& desc) override;
    virtual ShaderPtr CreateShader(const ShaderDesc& desc) override;

    virtual CommandRecorderPtr CreateCommandRecorder() override;
    virtual CommandQueuePtr CreateCommandQueue(CommandQueueType type, const char* debugName) override;
    virtual bool FinishFrame() override;

    CommandListManager* GetCommandListManager() const { return mCommandListManager.Get(); }
    FenceManager& GetFenceManager() { return mFenceManager; }
    RingBuffer* GetRingBuffer() { return mRingBuffer.Get(); }

    HeapAllocator& GetCbvSrvUavHeapStagingAllocator() { return mCbvSrvUavHeapStagingAllocator; }
    HeapAllocator& GetCbvSrvUavHeapAllocator() { return mCbvSrvUavHeapAllocator; }
    HeapAllocator& GetSamplerHeapStagingAllocator() { return mSamplerHeapStagingAllocator; }
    HeapAllocator& GetSamplerHeapAllocator() { return mSamplerHeapAllocator; }
    HeapAllocator& GetRtvHeapAllocator() { return mRtvHeapAllocator; }
    HeapAllocator& GetDsvHeapAllocator() { return mDsvHeapAllocator; }

    ShaderCompiler& GetShaderCompiler() { return mShaderCompiler; }

    bool IsDebugLayerEnabled() const { return mDebugLayerEnabled; }
};

} // namespace Renderer
} // namespace NFE
