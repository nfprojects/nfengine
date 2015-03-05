/**
 * @file    Device.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's device
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

Device::Device()
{
    HRESULT hr;
    UINT flags = 0;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11DeviceContext* immediateContext;
    hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0,
                           D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &immediateContext);
    mDefaultCommandBuffer.reset(new CommandBuffer(immediateContext));

    if (FAILED(hr))
    {
        throw std::exception("D3D11CreateDevice() failed");
    }

    /// get DXGI factory for created Direct3D device
    D3DPtr<IDXGIDevice> pDXGIDevice;
    hr = mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    D3DPtr<IDXGIAdapter> pDXGIAdapter;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&mDXGIFactory);
}

Device::~Device()
{
}

ID3D11Device* Device::Get() const
{
    return mDevice.get();
}

IVertexLayout* Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    VertexLayout* vl = new (std::nothrow) VertexLayout;
    if (vl == nullptr)
        return nullptr;

    if (!vl->Init(desc))
    {
        delete vl;
        return nullptr;
    }

    return vl;
}

IBuffer* Device::CreateBuffer(const BufferDesc& desc)
{
    Buffer* buffer = new (std::nothrow) Buffer;
    if (buffer == nullptr)
        return nullptr;

    if (!buffer->Init(desc))
    {
        delete buffer;
        return nullptr;
    }

    return buffer;
}

ITexture* Device::CreateTexture(const TextureDesc& desc)
{
    return nullptr;
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    RenderTarget* rt = new (std::nothrow) RenderTarget;
    if (rt == nullptr)
        return nullptr;

    if (desc.windowHandle)
    {
        rt->InitSwapChain(desc.width, desc.height, static_cast<HWND>(desc.windowHandle));
        // TODO: error checking
    }


    mRenderTargets.emplace(rt);
    return rt;
}

IBlendState* Device::CreateBlendState(const BlendStateDesc& desc)
{
    BlendState* bs = new (std::nothrow) BlendState;
    if (bs == nullptr)
        return nullptr;

    if (!bs->Init(desc))
    {
        delete bs;
        return nullptr;
    }

    return bs;
}

IDepthState* Device::CreateDepthState(const DepthStateDesc& desc)
{
    return nullptr;
}

IRasterizerState* Device::CreateRasterizerState(const RasterizerStateDesc& desc)
{
    RasterizerState* rs = new (std::nothrow) RasterizerState;
    if (rs == nullptr)
        return nullptr;

    if (!rs->Init(desc))
    {
        delete rs;
        return nullptr;
    }

    return rs;
}

IShader* Device::CreateShader(const ShaderDesc& desc)
{
    Shader* shader = new (std::nothrow) Shader;
    if (shader == nullptr)
        return nullptr;

    if (!shader->Init(desc))
    {
        delete shader;
        return nullptr;
    }

    mShaders.emplace(shader);
    return shader;
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
}


} // namespace Renderer
} // namespace NFE
