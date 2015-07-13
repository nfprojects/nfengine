/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "RendererD3D11.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Sampler.hpp"

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
    UINT flags = 0;

#ifdef D3D_DEBUGGING
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // D3D_DEBUGGING

    ID3D11DeviceContext* immediateContext;
    hr = D3D_CALL_CHECK(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0,
                                          D3D11_SDK_VERSION, &mDevice, &mFeatureLevel,
                                          &immediateContext));

    if (FAILED(hr))
        throw std::exception("D3D11CreateDevice() failed");

    mDefaultCommandBuffer.reset(new CommandBuffer(immediateContext));

    /// get DXGI factory for created Direct3D device
    D3DPtr<IDXGIDevice> pDXGIDevice;
    hr = D3D_CALL_CHECK(mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice));

    if (SUCCEEDED(hr) && pDXGIDevice.get() != nullptr)
    {
        D3DPtr<IDXGIAdapter> pDXGIAdapter;
        hr = D3D_CALL_CHECK(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter));

        if (SUCCEEDED(hr) && pDXGIAdapter.get() != nullptr)
            D3D_CALL_CHECK(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&mDXGIFactory));
    }
}

Device::~Device()
{
#ifdef D3D_DEBUGGING
    D3DPtr<ID3D11Debug> mDebug;
    /// get D3D debug layer interface
    D3D_CALL_CHECK(mDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&mDebug));

    if (mDebug.get() != nullptr)
    {
        // flush the pipeline
        mDefaultCommandBuffer->mContext->ClearState();
        mDefaultCommandBuffer->mContext->Flush();

        mDXGIFactory.reset();
        mDefaultCommandBuffer.reset();
        mDevice.reset();

        mDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
#endif // D3D_DEBUGGING
}

void* Device::GetHandle() const
{
    return mDevice.get();
}

ID3D11Device* Device::Get() const
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

IBlendState* Device::CreateBlendState(const BlendStateDesc& desc)
{
    return CreateGenericResource<BlendState, BlendStateDesc>(desc);
}

IDepthState* Device::CreateDepthState(const DepthStateDesc& desc)
{
    return CreateGenericResource<DepthState, DepthStateDesc>(desc);
}

IRasterizerState* Device::CreateRasterizerState(const RasterizerStateDesc& desc)
{
    return CreateGenericResource<RasterizerState, RasterizerStateDesc>(desc);
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

void Device::Execute(ICommandBuffer* commandBuffer, bool saveState)
{
    UNUSED(commandBuffer);
    UNUSED(saveState);
}


} // namespace Renderer
} // namespace NFE
