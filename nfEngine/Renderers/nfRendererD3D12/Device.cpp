/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "RendererD3D12.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Sampler.hpp"

#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

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

Device::Device()
{
    HRESULT hr;

#if defined(_DEBUG)
    // Enable the D3D12 debug layer
    {
        D3DPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            LOG_INFO("Enabling D3D12 debug layer");
            debugController->EnableDebugLayer();
        }
    }
#endif

    hr = D3D_CALL_CHECK(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
                                          IID_PPV_ARGS(&mDevice)));
    if (FAILED(hr))
        return;

    hr = D3D_CALL_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&mDXGIFactory)));
    if (FAILED(hr))
        return;

    /// create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = D3D_CALL_CHECK(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
    if (FAILED(hr))
        return;


    // TODO: temporary
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 256;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = D3D_CALL_CHECK(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));
    if (FAILED(hr))
        return;

    mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    LOG_INFO("RTV descriptor heap handle increment: %u", mRtvDescSize);


    mDefaultCommandBuffer.reset(new CommandBuffer(mDevice.get()));
}

Device::~Device()
{
}

ID3D12Device* Device::GetDevice() const
{
    return mDevice.get();
}

void* Device::GetHandle() const
{
    return mDevice.get();
}

IVertexLayout* Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    return CreateGenericResource<VertexLayout, VertexLayoutDesc>(desc);
}

IBuffer* Device::CreateBuffer(const BufferDesc& desc)
{
    return CreateGenericResource<Buffer, BufferDesc>(desc);
}

ITexture* Device::CreateTexture(const TextureDesc& desc)
{
    return CreateGenericResource<Texture, TextureDesc>(desc);
}

IBackbuffer* Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    return CreateGenericResource<Backbuffer, BackbufferDesc>(desc);
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
return CreateGenericResource<RenderTarget, RenderTargetDesc>(desc);
}

IPipelineState* Device::CreatePipelineState(const PipelineStateDesc& desc)
{
    return CreateGenericResource<PipelineState, PipelineStateDesc>(desc);
}

ISampler* Device::CreateSampler(const SamplerDesc& desc)
{
    return CreateGenericResource<Sampler, SamplerDesc>(desc);
}

IShader* Device::CreateShader(const ShaderDesc& desc)
{
    return CreateGenericResource<Shader, ShaderDesc>(desc);
}

IShaderProgram* Device::CreateShaderProgram(const ShaderProgramDesc& desc)
{
    return new (std::nothrow) ShaderProgram(desc);
}

ICommandBuffer* Device::GetDefaultCommandBuffer()
{
    return mDefaultCommandBuffer.get();
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    UNUSED(info);
    return false;
}

ICommandBuffer* Device::CreateCommandBuffer()
{
    return nullptr;
}

FullPipelineState* Device::GetFullPipelineState(const FullPipelineStateParts& parts)
{
    std::unique_ptr<FullPipelineState>& fullState = mPipelineStateMap[parts];
    if (!fullState)
    {
        fullState.reset(new FullPipelineState);
        fullState->Init(parts);
    }

    return fullState.get();
}

} // namespace Renderer
} // namespace NFE
