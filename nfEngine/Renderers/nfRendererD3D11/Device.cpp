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
#include "Translations.hpp"

#include "ResourceBinding.hpp"
#include "nfCommon/Win/Common.hpp"
#include "nfCommon/Logger.hpp"



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
    : mAdapterInUse(-1)
    , mDebugLayerEnabled(false)
{
}

bool Device::Init(const DeviceInitParams* params)
{
    DeviceInitParams defaultParams;
    if (!params)
        params = &defaultParams;

    HRESULT hr;
    UINT flags = 0;

    hr = D3D_CALL_CHECK(CreateDXGIFactory(IID_PPV_ARGS(&mDXGIFactory)));
    if (FAILED(hr))
        return false;

    int preferredCardId = params != nullptr ? params->preferredCardId : -1;
    if (!DetectVideoCards(preferredCardId))
    {
        LOG_ERROR("Failed to detect video cards");
        return false;
    }

    if (params->debugLevel > 0)
        flags |= D3D11_CREATE_DEVICE_DEBUG;

    hr = D3D_CALL_CHECK(D3D11CreateDevice(mAdapters[mAdapterInUse].get(), D3D_DRIVER_TYPE_UNKNOWN,
                                          NULL, flags, NULL, 0, D3D11_SDK_VERSION,
                                          &mDevice, &mFeatureLevel, &mImmediateContext));
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11CreateDevice() failed");
        return false;
    }

    const char* featureLevelStr = "unknown";
    switch (mFeatureLevel)
    {
    case D3D_FEATURE_LEVEL_9_1:
        featureLevelStr = "9_1";
        break;
    case D3D_FEATURE_LEVEL_9_2:
        featureLevelStr = "9_2";
        break;
    case D3D_FEATURE_LEVEL_9_3:
        featureLevelStr = "9_3";
        break;
    case D3D_FEATURE_LEVEL_10_0:
        featureLevelStr = "10_0";
        break;
    case D3D_FEATURE_LEVEL_10_1:
        featureLevelStr = "10_1";
        break;
    case D3D_FEATURE_LEVEL_11_0:
        featureLevelStr = "11_0";
        break;
    case D3D_FEATURE_LEVEL_11_1:
        featureLevelStr = "11_1";
        break;
    }
    LOG_INFO("Direct3D 11 device created with %s feature level", featureLevelStr);

    if (params->debugLevel > 0)
    {
        mDebugLayerEnabled = true;
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
    }

    // print device info
    // TODO: move to separate file (common for all renderers)
    DeviceInfo deviceInfo;
    if (GetDeviceInfo(deviceInfo))
    {
        LOG_INFO("GPU name: %s", deviceInfo.description.c_str());
        LOG_INFO("GPU info: %s", deviceInfo.misc.c_str());

        std::string features;
        for (size_t i = 0; i < deviceInfo.features.size(); ++i)
        {
            if (i > 0)
                features += ", ";
            features += deviceInfo.features[i];
        }
        LOG_INFO("GPU features: %s", features.c_str());
    }

    return true;
}

Device::~Device()
{
    if (IsDebugLayerEnabled())
    {
        D3DPtr<ID3D11Debug> mDebug;
        D3D_CALL_CHECK(mDevice->QueryInterface(IID_PPV_ARGS(&mDebug)));

        if (mDebug.get() != nullptr)
        {
            // flush the pipeline
            mImmediateContext->ClearState();
            mImmediateContext->Flush();

            mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, FALSE);
            mInfoQueue.reset();

            mDXGIFactory.reset();
            mAdapters.clear();
            mImmediateContext.reset();
            mDevice.reset();

            mDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
    }
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

bool Device::WaitForGPU()
{
    // not required
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
                      static_cast<size_t>(GetElementFormatSize(texture->mFormat));
    memcpy(data, mapped.pData, dataSize);

    mImmediateContext->Unmap(res, subresource);
    return true;
}


bool Device::DetectVideoCards(int preferredId)
{
    for (uint32 i = 0; ; ++i)
    {
        IDXGIAdapter* adapter;
        HRESULT hr = mDXGIFactory->EnumAdapters(i, &adapter);

        if (hr == DXGI_ERROR_NOT_FOUND)
            break;

        if (FAILED(hr))
        {
            LOG_ERROR("EnumAdapters1 failed for id=%u", i);
            continue;
        }

        DXGI_ADAPTER_DESC adapterDesc;
        adapter->GetDesc(&adapterDesc);

        // get GPU description
        std::wstring wideDesc = adapterDesc.Description;
        std::string descString;
        Common::UTF16ToUTF8(wideDesc, descString);

        if (static_cast<uint32>(preferredId) == i)
            mAdapterInUse = i;

        LOG_INFO("Adapter found at slot %u: %s", i, descString.c_str());
        mAdapters.push_back(std::move(adapter));
    }

    if (mAdapters.size() > 0)
    {
        if (mAdapterInUse < 0)
            mAdapterInUse = 0;
        return true;
    }

    return false;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    if (!mDevice.get())
        return false;

    HRESULT hr;
    DXGI_ADAPTER_DESC adapterDesc;
    mAdapters[mAdapterInUse]->GetDesc(&adapterDesc);

    // get GPU description
    std::wstring wideDesc = adapterDesc.Description;
    Common::UTF16ToUTF8(wideDesc, info.description);

    // get various GPU information
    info.misc =
        "Vendor ID: " + std::to_string(adapterDesc.VendorId) +
        ", Device ID: " + std::to_string(adapterDesc.DeviceId) +
        ", Sub System ID: " + std::to_string(adapterDesc.SubSysId) +
        ", Revision: " + std::to_string(adapterDesc.Revision) +
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
