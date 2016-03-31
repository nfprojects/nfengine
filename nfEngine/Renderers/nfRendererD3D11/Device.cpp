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
#include "ResourceBinding.hpp"
#include "../../nfCommon/Win/Common.hpp"
#include "../../nfCommon/Logger.hpp"

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
    hr = D3D_CALL_CHECK(mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&mDXGIDevice));
    if (SUCCEEDED(hr) && mDXGIDevice.get() != nullptr)
    {
        hr = D3D_CALL_CHECK(mDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&mDXGIAdapter));
        if (SUCCEEDED(hr) && mDXGIAdapter.get() != nullptr)
            D3D_CALL_CHECK(mDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&mDXGIFactory));
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
        mDXGIAdapter.reset();
        mDXGIDevice.reset();
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

IResourceBindingSet* Device::CreateResourceBindingSet(const ResourceBindingSetDesc& desc)
{
    return CreateGenericResource<ResourceBindingSet, ResourceBindingSetDesc>(desc);
}

IResourceBindingLayout* Device::CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc)
{
    return CreateGenericResource<ResourceBindingLayout, ResourceBindingLayoutDesc>(desc);
}

IResourceBindingInstance* Device::CreateResourceBindingInstance(IResourceBindingSet* set)
{
    return CreateGenericResource<ResourceBindingInstance, IResourceBindingSet*>(set);
}

ICommandBuffer* Device::GetDefaultCommandBuffer()
{
    return mDefaultCommandBuffer.get();
}

ICommandBuffer* Device::CreateCommandBuffer()
{
    HRESULT hr;
    ID3D11DeviceContext* context;
    hr = D3D_CALL_CHECK(mDevice->CreateDeferredContext(0, &context));
    if (FAILED(hr))
        return nullptr;

    CommandBuffer* commandBuffer = new (std::nothrow) CommandBuffer(context);
    if (!commandBuffer)
    {
        D3D_SAFE_RELEASE(context);
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    return commandBuffer;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    if (!mDXGIAdapter.get() || !mDevice.get())
        return false;

    HRESULT hr;
    DXGI_ADAPTER_DESC adapterDesc;
    mDXGIAdapter->GetDesc(&adapterDesc);

    // get GPU description
    std::wstring wideDesc = adapterDesc.Description;
    Common::UTF16ToUTF8(wideDesc, info.description);

    // get various GPU information
    info.misc =
        "Vendor ID: " + std::to_string(adapterDesc.VendorId) +
        ", Device ID: " + std::to_string(adapterDesc.VendorId) +
        ", Sub System ID: " + std::to_string(adapterDesc.VendorId) +
        ", Revision: " + std::to_string(adapterDesc.VendorId) +
        ", Dedicated Video Memory: " + std::to_string(adapterDesc.DedicatedVideoMemory >> 10) + " KB"
        ", Dedicated System Memory: " + std::to_string(adapterDesc.DedicatedSystemMemory >> 10) + " KB"
        ", Shared System Memory: " + std::to_string(adapterDesc.SharedSystemMemory >> 10) + " KB";


    info.features.clear();

    D3D11_FEATURE_DATA_THREADING threadingData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingData,
                                      sizeof(threadingData));
    if (SUCCEEDED(hr))
    {
        if (threadingData.DriverCommandLists)
            info.features.push_back("DriverCommandLists");
        if (threadingData.DriverConcurrentCreates)
            info.features.push_back("DriverConcurrentCreates");
    }

    D3D11_FEATURE_DATA_DOUBLES doublesData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &doublesData, sizeof(doublesData));
    if (SUCCEEDED(hr))
    {
        if (doublesData.DoublePrecisionFloatShaderOps)
            info.features.push_back("DoublePrecisionFloatShaderOps");
    }

    D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10HardwareData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10HardwareData,
                                      sizeof(d3d10HardwareData));
    if (SUCCEEDED(hr))
    {
        if (d3d10HardwareData.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
            info.features.push_back("ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x");
    }

    D3D11_FEATURE_DATA_D3D9_OPTIONS d3d9Data;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D9_OPTIONS, &d3d9Data, sizeof(d3d9Data));
    if (SUCCEEDED(hr))
    {
        if (d3d9Data.FullNonPow2TextureSupport)
            info.features.push_back("FullNonPow2TextureSupport");
    }

    D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT minPrecisionData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT,
                                      &minPrecisionData, sizeof(minPrecisionData));
    if (SUCCEEDED(hr))
    {
        info.features.push_back("PixelShaderMinPrecision=" +
                                std::to_string(minPrecisionData.PixelShaderMinPrecision));
        info.features.push_back("AllOtherShaderStagesMinPrecision=" +
                                std::to_string(minPrecisionData.AllOtherShaderStagesMinPrecision));
    }

    D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT shadowSupportData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D9_SHADOW_SUPPORT, &shadowSupportData,
                                      sizeof(shadowSupportData));
    if (SUCCEEDED(hr))
    {
        if (shadowSupportData.SupportsDepthAsTextureWithLessEqualComparisonFilter)
            info.features.push_back("SupportsDepthAsTextureWithLessEqualComparisonFilter");
    }

    // TODO: Direct3D 11.x support

    return true;
}


} // namespace Renderer
} // namespace NFE
