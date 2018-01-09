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
#include "Texture.hpp"
#include "Sampler.hpp"
#include "PipelineState.hpp"
#include "MasterContext.hpp"


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
    return GenericCreateResource<Texture, TextureDesc>(desc);
}

IBackbuffer* Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    return GenericCreateResource<Backbuffer, BackbufferDesc>(desc);
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    return GenericCreateResource<RenderTarget, RenderTargetDesc>(desc);
}

IPipelineState* Device::CreatePipelineState(const PipelineStateDesc& desc)
{
    return GenericCreateResource<PipelineState, PipelineStateDesc>(desc);
}

ISampler* Device::CreateSampler(const SamplerDesc& desc)
{
    return GenericCreateResource<Sampler, SamplerDesc>(desc);
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

ICommandBuffer* Device::CreateCommandBuffer()
{
    return nullptr;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    NFE_UNUSED(info);
    return false;
}

IDevice* Init()
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new Device);
    }

    if (!MasterContext::Instance().Init())
    {
        gDevice.reset();
        return nullptr;
    }

    return gDevice.get();
}

void Release()
{
    MasterContext::Instance().Release();
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
