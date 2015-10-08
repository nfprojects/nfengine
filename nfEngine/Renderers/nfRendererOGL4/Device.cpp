/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"

// modules
#include "RenderTarget.hpp"
#include "Backbuffer.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "VertexLayout.hpp"


namespace {

template<typename Type, typename Desc>
Type* GenericCreateResource(const Desc& desc)
{
    Type* resource = new (std::nothrow) Type;
    if (resource == nullptr)
        return nullptr;

    if (!resource->Init(desc))
    {
        delete resource;
        return nullptr;
    }

    return resource;
}

} // namespace

namespace NFE {
namespace Renderer {

std::unique_ptr<Device> gDevice;

Device::Device()
{
    // FIXME Placing this here BREAKS OPENGL RENDERER ON WINDOWS
    //       OGL requires a window to retrieve a device context and rendering context. Only
    //       afterwards it is possible to get OGL Extensions.
    //       A workaround would be to create a temporary invisible window here. Afterwards,
    //       if Backbuffer will be initialized, the "window" created here must be trashed and
    //       extensions reinitialized on new context.
    if (!nfglExtensionsInit())
        return;

    mDefaultCommandBuffer.reset(new CommandBuffer());
}

Device::~Device()
{
}

void* Device::GetHandle() const
{
    return nullptr;
}


IVertexLayout* Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    return GenericCreateResource<VertexLayout, VertexLayoutDesc>(desc);
}

IBuffer* Device::CreateBuffer(const BufferDesc& desc)
{
    return GenericCreateResource<Buffer, BufferDesc>(desc);
}

ITexture* Device::CreateTexture(const TextureDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBackbuffer* Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    return GenericCreateResource<Backbuffer, BackbufferDesc>(desc);
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    return GenericCreateResource<RenderTarget, RenderTargetDesc>(desc);
}

IBlendState* Device::CreateBlendState(const BlendStateDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IDepthState* Device::CreateDepthState(const DepthStateDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IRasterizerState* Device::CreateRasterizerState(const RasterizerStateDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

ISampler* Device::CreateSampler(const SamplerDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IShader* Device::CreateShader(const ShaderDesc& desc)
{
    return GenericCreateResource<Shader, ShaderDesc>(desc);
}

IShaderProgram* Device::CreateShaderProgram(const ShaderProgramDesc& desc)
{
    return GenericCreateResource<ShaderProgram, ShaderProgramDesc>(desc);
}

ICommandBuffer* Device::GetDefaultCommandBuffer()
{
    return mDefaultCommandBuffer.get();
}

void Device::Execute(ICommandBuffer* commandBuffer, bool saveState)
{
    UNUSED(commandBuffer);
    UNUSED(saveState);
}

IDevice* Init()
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new Device);
    }

    return gDevice.get();
}

void Release()
{
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
