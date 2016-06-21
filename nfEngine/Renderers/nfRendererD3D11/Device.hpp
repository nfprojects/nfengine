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

class CommandBuffer;

class Device : public IDevice
{
    friend class Backbuffer;
    friend class RenderTarget;

    D3DPtr<ID3D11Device> mDevice;
    std::unique_ptr<CommandBuffer> mDefaultCommandBuffer;
    D3DPtr<IDXGIDevice> mDXGIDevice;
    D3DPtr<IDXGIFactory> mDXGIFactory;
    D3DPtr<IDXGIAdapter> mDXGIAdapter;
    D3D_FEATURE_LEVEL mFeatureLevel;

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

    ICommandBuffer* GetDefaultCommandBuffer() override;
    ICommandBuffer* CreateCommandBuffer() override;
};

} // namespace Renderer
} // namespace NFE
