/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's device.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"


namespace NFE {
namespace Renderer {

class Device : public IDevice
{
    friend class Backbuffer;
    friend class RenderTarget;

    D3DPtr<ID3D11Device> mDevice;
    D3DPtr<ID3D11DeviceContext> mImmediateContext;
    D3DPtr<IDXGIDevice> mDXGIDevice;
    D3DPtr<IDXGIFactory> mDXGIFactory;
    D3DPtr<IDXGIAdapter> mDXGIAdapter;
    D3D_FEATURE_LEVEL mFeatureLevel;

#ifdef _DEBUG
    D3DPtr<ID3D11InfoQueue> mInfoQueue;
#endif // _DEBUG

public:
    Device();
    ~Device();

    void* GetHandle() const override;
    ID3D11Device* Get() const;
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

    ICommandBuffer* CreateCommandBuffer() override;
    bool Execute(ICommandList* commandList) override;
    bool DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(ITexture* tex, void* data, int mipmap, int layer) override;
};

} // namespace Renderer
} // namespace NFE
