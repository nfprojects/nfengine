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


namespace {

template<typename Type, typename Desc>
Type* CreateGenericResource(const Desc& desc)
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
    mDefaultCommandBuffer.reset(new CommandBuffer());
}

Device::~Device()
{
}


IVertexLayout* Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBuffer* Device::CreateBuffer(const BufferDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

ITexture* Device::CreateTexture(const TextureDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBackbuffer* Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    return CreateGenericResource<Backbuffer, BackbufferDesc>(desc);
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    return CreateGenericResource<RenderTarget, RenderTargetDesc>(desc);
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
    UNUSED(desc);
    return nullptr;
}

IShaderProgram* Device::CreateShaderProgram(const ShaderProgramDesc& desc)
{
    UNUSED(desc);
    return nullptr;
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
