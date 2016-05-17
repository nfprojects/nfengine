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

#include <map>


namespace NFE {
namespace Renderer {

class CommandBuffer;

class Device : public IDevice
{
    friend class Backbuffer;
    friend class CommandBuffer;
    friend class RenderTarget;

    D3D_FEATURE_LEVEL mFeatureLevel;

    D3DPtr<IDXGIFactory4> mDXGIFactory;
    D3DPtr<IDXGIAdapter> mPrimaryAdapter;
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<ID3D12CommandQueue> mCommandQueue;

    HeapAllocator mCbvSrvUavHeapAllocator;
    HeapAllocator mRtvHeapAllocator;
    HeapAllocator mDsvHeapAllocator;

    std::map<FullPipelineStateParts, D3DPtr<ID3D12PipelineState>> mPipelineStateMap;

public:
    Device();
    ~Device();
    bool Init();

    ID3D12Device* GetDevice() const;
    ID3D12CommandQueue* GetCommandQueue() const;
    void* GetHandle() const override;
    bool GetDeviceInfo(DeviceInfo& info) override;

    /// Resources creation functions

    IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc) override;
    IBuffer* CreateBuffer(const BufferDesc& desc) override;
    ITexture* CreateTexture(const TextureDesc& desc) override;
    IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc) override;
    IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc) override;
    IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) override;
    ISampler* CreateSampler(const SamplerDesc& desc) override;
    IShader* CreateShader(const ShaderDesc& desc) override;
    IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc) override;
    IResourceBindingSet* CreateResourceBindingSet(const ResourceBindingSetDesc& desc) override;
    IResourceBindingLayout* CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) override;
    IResourceBindingInstance* CreateResourceBindingInstance(IResourceBindingSet* set) override;

    ICommandBuffer* CreateCommandBuffer() override;
    bool Execute(ICommandList* commandList) override;
    bool DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(ITexture* tex, void* data, int mipmap, int layer) override;

    ID3D12PipelineState* GetFullPipelineState(const FullPipelineStateParts& parts);
    void OnShaderProgramDestroyed(IShaderProgram* program);
    void OnPipelineStateDestroyed(IPipelineState* pipelineState);

    /**
     * Waits until all operations sent to the command queue has been completed.
     */
    bool WaitForGPU();

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
};

} // namespace Renderer
} // namespace NFE
