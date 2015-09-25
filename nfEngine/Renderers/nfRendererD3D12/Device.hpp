/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's device.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class CommandBuffer;

class Device : public IDevice
{
    friend class Backbuffer;

    //D3DPtr<IDXGISwapChain3> mSwapChain;
    D3DPtr<ID3D12Device> mDevice;
    D3DPtr<IDXGIFactory4> mDXGIFactory;
    //D3DPtr<ID3D12Resource> mRenderTargets[FrameCount];
    D3DPtr<ID3D12CommandAllocator> mCommandAllocator;
    D3DPtr<ID3D12CommandQueue> mCommandQueue;
    D3DPtr<ID3D12DescriptorHeap> mRtvHeap;
    //D3DPtr<ID3D12PipelineState> mPipelineState;
    //D3DPtr<ID3D12GraphicsCommandList> mCommandList;

    UINT mRtvDescSize;

public:
    Device();
    ~Device();

    void* GetHandle() const;

    /// Resources creation functions

    IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc);
    IBuffer* CreateBuffer(const BufferDesc& desc);
    ITexture* CreateTexture(const TextureDesc& desc);
    IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc);
    IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc);
    IBlendState* CreateBlendState(const BlendStateDesc& desc);
    IDepthState* CreateDepthState(const DepthStateDesc& desc);
    IRasterizerState* CreateRasterizerState(const RasterizerStateDesc& desc);
    ISampler* CreateSampler(const SamplerDesc& desc);
    IShader* CreateShader(const ShaderDesc& desc);
    IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc);

    ICommandBuffer* GetDefaultCommandBuffer();
    void Execute(ICommandBuffer* commandBuffer, bool saveState);
};

} // namespace Renderer
} // namespace NFE
