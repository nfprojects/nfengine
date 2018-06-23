/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "CommandRecorder.hpp"
#include "RendererD3D11.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

#include "ResourceBinding.hpp"
#include "nfCommon/System/Win/Common.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"
#include "nfCommon/Utils/StringUtils.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

namespace {

template<typename Type, typename Desc>
SharedPtr<Type> CreateGenericResource(const Desc& desc)
{
    auto resource = MakeSharedPtr<Type>();
    if (!resource)
    {
        return nullptr;
    }

    if (!resource->Init(desc))
    {
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

    hr = D3D_CALL_CHECK(CreateDXGIFactory(IID_PPV_ARGS(mDXGIFactory.GetPtr())));
    if (FAILED(hr))
        return false;

    int preferredCardId = params != nullptr ? params->preferredCardId : -1;
    if (!DetectVideoCards(preferredCardId))
    {
        NFE_LOG_ERROR("Failed to detect video cards");
        return false;
    }

    if (params->debugLevel > 0)
        flags |= D3D11_CREATE_DEVICE_DEBUG;

    hr = D3D_CALL_CHECK(D3D11CreateDevice(mAdapters[mAdapterInUse].Get(), D3D_DRIVER_TYPE_UNKNOWN,
                                          NULL, flags, NULL, 0, D3D11_SDK_VERSION,
                                          mDevice.GetPtr(), &mFeatureLevel, mImmediateContext.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("D3D11CreateDevice() failed");
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
    NFE_LOG_INFO("Direct3D 11 device created with %s feature level", featureLevelStr);

    if (params->debugLevel > 0)
    {
        mDebugLayerEnabled = true;
        if (SUCCEEDED(mDevice->QueryInterface(IID_PPV_ARGS(mInfoQueue.GetPtr()))))
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
        NFE_LOG_INFO("GPU name: %s", deviceInfo.description.Str());
        NFE_LOG_INFO("GPU info: %s", deviceInfo.misc.Str());

        String features;
        for (uint32 i = 0; i < deviceInfo.features.Size(); ++i)
        {
            if (i > 0)
                features += ", ";
            features += deviceInfo.features[i];
        }
        NFE_LOG_INFO("GPU features: %s", features.Str());
    }

    return true;
}

Device::~Device()
{
    if (IsDebugLayerEnabled())
    {
        D3DPtr<ID3D11Debug> debugInterface;
        D3D_CALL_CHECK(mDevice->QueryInterface(IID_PPV_ARGS(debugInterface.GetPtr())));

        if (debugInterface)
        {
            // flush the pipeline
            mImmediateContext->ClearState();
            mImmediateContext->Flush();

            mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, FALSE);
            mInfoQueue.Reset();

            mDXGIFactory.Reset();
            mAdapters.Clear();
            mImmediateContext.Reset();
            mDevice.Reset();

            debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
    }
}

void* Device::GetHandle() const
{
    return mDevice.Get();
}

ID3D11Device* Device::Get() const
{
    return mDevice.Get();
}

VertexLayoutPtr Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    return CreateGenericResource<VertexLayout, VertexLayoutDesc>(desc);
}

BufferPtr Device::CreateBuffer(const BufferDesc& desc)
{
    return CreateGenericResource<Buffer, BufferDesc>(desc);
}

TexturePtr Device::CreateTexture(const TextureDesc& desc)
{
    return CreateGenericResource<Texture, TextureDesc>(desc);
}

BackbufferPtr Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    return CreateGenericResource<Backbuffer, BackbufferDesc>(desc);
}

RenderTargetPtr Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    return CreateGenericResource<RenderTarget, RenderTargetDesc>(desc);
}

PipelineStatePtr Device::CreatePipelineState(const PipelineStateDesc& desc)
{
    return CreateGenericResource<PipelineState, PipelineStateDesc>(desc);
}

ComputePipelineStatePtr Device::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
{
    return CreateGenericResource<ComputePipelineState, ComputePipelineStateDesc>(desc);
}

SamplerPtr Device::CreateSampler(const SamplerDesc& desc)
{
    return CreateGenericResource<Sampler, SamplerDesc>(desc);
}

ShaderPtr Device::CreateShader(const ShaderDesc& desc)
{
    return CreateGenericResource<Shader, ShaderDesc>(desc);
}

ResourceBindingSetPtr Device::CreateResourceBindingSet(const ResourceBindingSetDesc& desc)
{
    return CreateGenericResource<ResourceBindingSet, ResourceBindingSetDesc>(desc);
}

ResourceBindingLayoutPtr Device::CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc)
{
    return CreateGenericResource<ResourceBindingLayout, ResourceBindingLayoutDesc>(desc);
}

ResourceBindingInstancePtr Device::CreateResourceBindingInstance(const ResourceBindingSetPtr& set)
{
    return CreateGenericResource<ResourceBindingInstance, ResourceBindingSetPtr>(set);
}

CommandRecorderPtr Device::CreateCommandRecorder()
{
    HRESULT hr;
    ID3D11DeviceContext* context;
    hr = D3D_CALL_CHECK(mDevice->CreateDeferredContext(0, &context));
    if (FAILED(hr))
        return nullptr;

    auto commandRecorder = MakeSharedPtr<CommandRecorder>(context);
    if (!commandRecorder)
    {
        D3D_SAFE_RELEASE(context);
        NFE_LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    return commandRecorder;
}

CommandListID Device::RegisterCommandList(ID3D11CommandList* commandList)
{
    CommandListID id = INVALID_COMMAND_LIST_ID;
    {
        ScopedExclusiveLock<Mutex> lock(mCommandListsMutex);
        mCommandLists.PushBack(commandList);
        id = static_cast<CommandListID>(mCommandLists.Size());
    }

    return id;
}

bool Device::Execute(CommandListID id)
{
    if (id == INVALID_COMMAND_LIST_ID)
    {
        NFE_LOG_ERROR("Invalid command list");
        return false;
    }

    ID3D11CommandList* list = nullptr;
    {
        ScopedExclusiveLock<Mutex> lock(mCommandListsMutex);
        if (id > mCommandLists.Size())
        {
            NFE_LOG_ERROR("Invalid command list ID");
            return false;
        }

        list = mCommandLists[id - 1];
        if (!list)
        {
            NFE_LOG_ERROR("Invalid command list ID - already executed?");
            return false;
        }

        // mark as used
        mCommandLists[id - 1] = nullptr;
    }

    mImmediateContext->ExecuteCommandList(list, FALSE);
    D3D_SAFE_RELEASE(list);
    return true;
}

bool Device::FinishFrame()
{
    uint32 danglingLists = 0;

    {
        ScopedExclusiveLock<Mutex> lock(mCommandListsMutex);

        // cleanup command list array
        for (ID3D11CommandList* commandList : mCommandLists)
        {
            if (commandList)
            {
                danglingLists++;
                D3D_SAFE_RELEASE(commandList);
            }
        }

        mCommandLists.Clear();
    }

    if (danglingLists > 0)
    {
        NFE_LOG_WARNING("There are %u recorded but unused command lists in this frame", danglingLists);
    }

    return true;
}

bool Device::WaitForGPU()
{
    // not required in D3D11
    return true;
}

bool Device::DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data)
{
    const Buffer* buf = dynamic_cast<Buffer*>(buffer.Get());
    if (!buf)
        return false;

    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.Get());
    HRESULT hr = D3D_CALL_CHECK(mImmediateContext->Map(res, 0, D3D11_MAP_READ, 0, &mapped));
    if (FAILED(hr))
        return false;

    memcpy(data, reinterpret_cast<char*>(mapped.pData) + offset, size);
    mImmediateContext->Unmap(res, 0);
    return true;
}

bool Device::DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer)
{
    const Texture* texture = dynamic_cast<Texture*>(tex.Get());
    if (!texture)
    {
        NFE_LOG_ERROR("Invalid 'tex' pointer");
        return false;
    }

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    UINT subresource = D3D11CalcSubresource(mipmap, layer, texture->mMipmaps);
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(texture->mTextureGeneric);
    hr = D3D_CALL_CHECK(mImmediateContext->Map(res, subresource, D3D11_MAP_READ, 0, &mapped));
    if (FAILED(hr))
        return false;

    size_t rowSize = static_cast<size_t>(texture->mWidth) *
                     static_cast<size_t>(GetElementFormatSize(texture->mFormat));

    for (int i = 0; i < texture->mHeight; ++i)
    {
        char* targetRow = reinterpret_cast<char*>(data) + rowSize * i;
        const char* sourceRow = reinterpret_cast<const char*>(mapped.pData) + mapped.RowPitch * i;
        memcpy(targetRow, sourceRow, rowSize);
    }

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
            NFE_LOG_ERROR("EnumAdapters1 failed for id=%u", i);
            continue;
        }

        DXGI_ADAPTER_DESC adapterDesc;
        adapter->GetDesc(&adapterDesc);

        // get GPU description
        std::wstring wideDesc = adapterDesc.Description;
        String descString;
        UTF16ToUTF8(wideDesc, descString);

        if (static_cast<uint32>(preferredId) == i)
            mAdapterInUse = i;

        NFE_LOG_INFO("Adapter found at slot %u: %s", i, descString.Str());
        mAdapters.PushBack(D3DPtr<IDXGIAdapter>(adapter));
    }

    if (mAdapters.Size() > 0)
    {
        if (mAdapterInUse < 0)
            mAdapterInUse = 0;
        return true;
    }

    return false;
}

bool Device::IsBackbufferFormatSupported(ElementFormat format)
{
    UINT formatSupport;
    HRESULT hr = mDevice->CheckFormatSupport(TranslateElementFormat(format), &formatSupport);
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to check format support info");
        return false;
    }

    return (formatSupport & D3D11_FORMAT_SUPPORT_DISPLAY) == D3D11_FORMAT_SUPPORT_DISPLAY;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    if (!mDevice.Get())
        return false;

    HRESULT hr;
    DXGI_ADAPTER_DESC adapterDesc;
    mAdapters[mAdapterInUse]->GetDesc(&adapterDesc);

    // get GPU description
    std::wstring wideDesc = adapterDesc.Description;
    UTF16ToUTF8(wideDesc, info.description);

    // get various GPU information
    info.misc =
        "Vendor ID: " + ToString(adapterDesc.VendorId) +
        ", Device ID: " + ToString(adapterDesc.DeviceId) +
        ", Sub System ID: " + ToString(adapterDesc.SubSysId) +
        ", Revision: " + ToString(adapterDesc.Revision) +
        ", Dedicated Video Memory: " + ToString(uint64(adapterDesc.DedicatedVideoMemory >> 10u)) + " KB"
        ", Dedicated System Memory: " + ToString(uint64(adapterDesc.DedicatedSystemMemory >> 10u)) + " KB"
        ", Shared System Memory: " + ToString(uint64(adapterDesc.SharedSystemMemory >> 10u)) + " KB";


    info.features.Clear();

    D3D11_FEATURE_DATA_THREADING threadingData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingData,
                                      sizeof(threadingData));
    if (SUCCEEDED(hr))
    {
        if (threadingData.DriverCommandLists)
            info.features.PushBack("DriverCommandLists");
        if (threadingData.DriverConcurrentCreates)
            info.features.PushBack("DriverConcurrentCreates");
    }

    D3D11_FEATURE_DATA_DOUBLES doublesData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &doublesData, sizeof(doublesData));
    if (SUCCEEDED(hr))
    {
        if (doublesData.DoublePrecisionFloatShaderOps)
            info.features.PushBack("DoublePrecisionFloatShaderOps");
    }

    D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3d10HardwareData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3d10HardwareData,
                                      sizeof(d3d10HardwareData));
    if (SUCCEEDED(hr))
    {
        if (d3d10HardwareData.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
            info.features.PushBack("ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x");
    }

    D3D11_FEATURE_DATA_D3D9_OPTIONS d3d9Data;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D9_OPTIONS, &d3d9Data, sizeof(d3d9Data));
    if (SUCCEEDED(hr))
    {
        if (d3d9Data.FullNonPow2TextureSupport)
            info.features.PushBack("FullNonPow2TextureSupport");
    }

    D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT minPrecisionData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT,
                                      &minPrecisionData, sizeof(minPrecisionData));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("PixelShaderMinPrecision=" +
                               ToString(minPrecisionData.PixelShaderMinPrecision));
        info.features.PushBack("AllOtherShaderStagesMinPrecision=" +
                               ToString(minPrecisionData.AllOtherShaderStagesMinPrecision));
    }

    D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT shadowSupportData;
    hr = mDevice->CheckFeatureSupport(D3D11_FEATURE_D3D9_SHADOW_SUPPORT, &shadowSupportData,
                                      sizeof(shadowSupportData));
    if (SUCCEEDED(hr))
    {
        if (shadowSupportData.SupportsDepthAsTextureWithLessEqualComparisonFilter)
            info.features.PushBack("SupportsDepthAsTextureWithLessEqualComparisonFilter");
    }

    // TODO: Direct3D 11.x support

    return true;
}


} // namespace Renderer
} // namespace NFE
