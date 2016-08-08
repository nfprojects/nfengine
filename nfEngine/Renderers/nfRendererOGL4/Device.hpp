/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL 4 Device
 */

#pragma once

#include "Defines.hpp"

#include "../RendererInterface/Device.hpp"

#include "CommandBuffer.hpp"

namespace NFE {
namespace Renderer {

class Device : public IDevice
{
private:
    std::unique_ptr<CommandBuffer> mDefaultCommandBuffer;

public:
    Device();
    ~Device();

    void* GetHandle() const;
    bool GetDeviceInfo(DeviceInfo& info);

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

extern std::unique_ptr<Device> gDevice;

extern "C" RENDERER_API IDevice* Init();
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namepsace NFE