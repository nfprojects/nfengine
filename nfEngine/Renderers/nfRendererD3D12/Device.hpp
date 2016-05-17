/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's device.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"
#include "PipelineState.hpp"

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
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<ID3D12CommandQueue> mCommandQueue;
    D3DPtr<ID3D12DescriptorHeap> mRtvHeap;
    D3DPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap;
    D3DPtr<ID3D12DescriptorHeap> mSamplerHeap;

    std::vector<bool> mCbvSrvUavHeapMap;

    std::map<FullPipelineStateParts, std::unique_ptr<FullPipelineState>> mPipelineStateMap;

    UINT mCbvSrvUavDescSize;
    UINT mSamplerDescSize;
    UINT mRtvDescSize;
    UINT mDsvDescSize;

public:
    Device();
    ~Device();

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

    FullPipelineState* GetFullPipelineState(const FullPipelineStateParts& parts);

    void GetCbvSrvUavHeapInfo(UINT& descriptorSize, D3D12_CPU_DESCRIPTOR_HANDLE& ptr);
    size_t AllocateCbvSrvUavHeap(size_t numDescriptors);
    void FreeCbvSrvUavHeap(size_t offset, size_t numDescriptors);

    void GetSamplerHeapInfo(UINT& descriptorSize, D3D12_CPU_DESCRIPTOR_HANDLE& ptr);

    /**
     * Waits until all operations sent to the command queue has been completed.
     */
    bool WaitForGPU();
};

} // namespace Renderer
} // namespace NFE
