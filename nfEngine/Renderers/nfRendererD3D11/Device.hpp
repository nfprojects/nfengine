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

    void* GetHandle() const;
    ID3D11Device* Get() const;
    bool GetDeviceInfo(DeviceInfo& info);

    /// Resources creation functions

    IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc);
    IBuffer* CreateBuffer(const BufferDesc& desc);
    ITexture* CreateTexture(const TextureDesc& desc);
    IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc);
    IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc);
    IPipelineState* CreatePipelineState(const PipelineStateDesc& desc);
    ISampler* CreateSampler(const SamplerDesc& desc);
    IShader* CreateShader(const ShaderDesc& desc);
    IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc);

    ICommandBuffer* GetDefaultCommandBuffer();
    ICommandBuffer* CreateCommandBuffer();
};

} // namespace Renderer
} // namespace NFE
