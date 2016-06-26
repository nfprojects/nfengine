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

    hr = D3D_CALL_CHECK(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0,
                                          D3D11_SDK_VERSION, &mDevice, &mFeatureLevel,
                                          &mImmediateContext));

#if defined(_DEBUG)
    if (SUCCEEDED(mDevice->QueryInterface(IID_PPV_ARGS(&mInfoQueue))))
    {
        D3D11_MESSAGE_ID messagesToHide[] =
        {
            D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
        };

        D3D11_INFO_QUEUE_FILTER filter;
        memset(&filter, 0, sizeof(filter));
        filter.DenyList.NumIDs = _countof(messagesToHide);
        filter.DenyList.pIDList = messagesToHide;
        mInfoQueue->AddStorageFilterEntries(&filter);

        mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    if (FAILED(hr))
        throw std::exception("D3D11CreateDevice() failed");

    /// get DXGI factory for created Direct3D device
    hr = D3D_CALL_CHECK(mDevice->QueryInterface(IID_PPV_ARGS(&mDXGIDevice)));
    if (SUCCEEDED(hr) && mDXGIDevice.get() != nullptr)
    {
        hr = D3D_CALL_CHECK(mDXGIDevice->GetParent(IID_PPV_ARGS(&mDXGIAdapter)));
        if (SUCCEEDED(hr) && mDXGIAdapter.get() != nullptr)
            D3D_CALL_CHECK(mDXGIAdapter->GetParent(IID_PPV_ARGS(&mDXGIFactory)));
    }
}

Device::~Device()
{
#ifdef D3D_DEBUGGING
    D3DPtr<ID3D11Debug> mDebug;
    /// get D3D debug layer interface
    D3D_CALL_CHECK(mDevice->QueryInterface(IID_PPV_ARGS(&mDebug)));

    if (mDebug.get() != nullptr)
    {
        // flush the pipeline
        mImmediateContext->ClearState();
        mImmediateContext->Flush();

        mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, FALSE);
        mInfoQueue.reset();

        mDXGIFactory.reset();
        mDXGIAdapter.reset();
        mDXGIDevice.reset();
        mImmediateContext.reset();
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


bool Device::Execute(ICommandList* commandList)
{
    CommandList* list = dynamic_cast<CommandList*>(commandList);
    if (!list || !list->mD3DList)
        return false;

    mImmediateContext->ExecuteCommandList(list->mD3DList.get(), FALSE);
    return true;
}

bool Device::DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf)
        return false;

    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.get());
    HRESULT hr = D3D_CALL_CHECK(mImmediateContext->Map(res, 0, D3D11_MAP_READ, 0, &mapped));
    if (FAILED(hr))
        return false;

    memcpy(data, reinterpret_cast<char*>(mapped.pData) + offset, size);
    mImmediateContext->Unmap(res, 0);
    return true;
}

bool Device::DownloadTexture(ITexture* tex, void* data, int mipmap, int layer)
{
    Texture* texture = dynamic_cast<Texture*>(tex);
    if (!texture)
    {
        LOG_ERROR("Invalid 'tex' pointer");
        return false;
    }

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    UINT subresource = D3D11CalcSubresource(mipmap, layer, texture->mMipmaps);
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(texture->mTextureGeneric);
    hr = D3D_CALL_CHECK(mImmediateContext->Map(res, subresource, D3D11_MAP_READ, 0, &mapped));
    if (FAILED(hr))
        return false;

    size_t dataSize = static_cast<size_t>(texture->mWidth) *
                      static_cast<size_t>(texture->mHeight) *
                      static_cast<size_t>(texture->mTexelSize);
    memcpy(data, mapped.pData, dataSize);

    mImmediateContext->Unmap(res, subresource);
    return true;
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
