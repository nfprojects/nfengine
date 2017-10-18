/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "CommandRecorder.hpp"
#include "RendererD3D12.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"
#include "Sampler.hpp"
#include "ResourceBinding.hpp"
#include "Translations.hpp"

#include "nfCommon/System/Win/Common.hpp"
#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

namespace {

template<typename Type, typename Desc>
Common::SharedPtr<Type> CreateGenericResource(const Desc& desc)
{
    auto resource = Common::MakeSharedPtr<Type>();
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
    : mCbvSrvUavHeapAllocator(HeapAllocator::Type::CbvSrvUav, 1024)
    , mRtvHeapAllocator(HeapAllocator::Type::Rtv, 512)
    , mDsvHeapAllocator(HeapAllocator::Type::Dsv, 512)
    , mAdapterInUse(-1)
    , mFenceValue(1)
    , mDebugLayerEnabled(false)
{
}

bool Device::Init(const DeviceInitParams* params)
{
    DeviceInitParams defaultParams;
    if (!params)
        params = &defaultParams;

    HRESULT hr;

    // Enable the D3D12 debug layer
    if (params->debugLevel > 0)
    {
        D3DPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetPtr()))))
        {
            LOG_INFO("Enabling D3D12 debug layer");
            debugController->EnableDebugLayer();
        }
    }

    hr = D3D_CALL_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(mDXGIFactory.GetPtr())));
    if (FAILED(hr))
        return false;

    int preferredCardId = params != nullptr ? params->preferredCardId : -1;
    if (!DetectVideoCards(preferredCardId))
    {
        LOG_ERROR("Failed to detect video cards");
        return false;
    }

    hr = D3D_CALL_CHECK(D3D12CreateDevice(mAdapters[mAdapterInUse].Get(),
                                          D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(mDevice.GetPtr())));
    if (FAILED(hr))
        return false;

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_9_1,  D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_12_1,
    };
    D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelsInfo;
    featureLevelsInfo.NumFeatureLevels = 9;
    featureLevelsInfo.pFeatureLevelsRequested = featureLevels;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelsInfo,
                                      sizeof(featureLevelsInfo));
    if (SUCCEEDED(hr))
    {
        const char* featureLevelStr = "unknown";
        mFeatureLevel = featureLevelsInfo.MaxSupportedFeatureLevel;
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
        case D3D_FEATURE_LEVEL_12_0:
            featureLevelStr = "12_0";
            break;
        case D3D_FEATURE_LEVEL_12_1:
            featureLevelStr = "12_1";
            break;
        }
        LOG_INFO("Direct3D 12 device created with %s feature level", featureLevelStr);
    }
    else
    {
        LOG_ERROR("Failed to obtain Direct3D feature level");
        mFeatureLevel = D3D_FEATURE_LEVEL_9_1;
    }

    if (params->debugLevel > 0)
    {
        hr = D3D_CALL_CHECK(mDevice->QueryInterface(IID_PPV_ARGS(mDebugDevice.GetPtr())));
        if (FAILED(hr))
        {
            LOG_ERROR("D3D12 device debugging won't be supported");
        }
    }

    // print device info
    // TODO: move to separate file (common for all renderers)
    {
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
    }

    if (params->debugLevel > 0)
    {
        mDebugLayerEnabled = true;
        if (SUCCEEDED(mDevice->QueryInterface(IID_PPV_ARGS(mInfoQueue.GetPtr()))))
        {
            D3D12_MESSAGE_ID messagesToHide[] =
            {
                // this warning makes debugging with VS Graphics Debugger impossible
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,

                // performance warning - let's ignore it for now
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            };

            D3D12_INFO_QUEUE_FILTER filter;
            memset(&filter, 0, sizeof(filter));
            filter.DenyList.NumIDs = _countof(messagesToHide);
            filter.DenyList.pIDList = messagesToHide;
            mInfoQueue->AddStorageFilterEntries(&filter);

            mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        }
    }

    /// create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = D3D_CALL_CHECK(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mCommandQueue.GetPtr())));
    if (FAILED(hr))
        return false;

    // create fence for frame synchronization
    hr = D3D_CALL_CHECK(gDevice->mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetPtr())));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create D3D12 fence object");
        return false;
    }

    // Create an event handle to use for frame synchronization.
    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (mFenceEvent == nullptr)
    {
        LOG_ERROR("Failed to create fence event object");
        return false;
    }


    if (!mCbvSrvUavHeapAllocator.Init())
    {
        LOG_ERROR("Failed to initialize heap allocator for CBV, SRV and UAV");
        return false;
    }

    if (!mRtvHeapAllocator.Init())
    {
        LOG_ERROR("Failed to initialize heap allocator for RTV");
        return false;
    }

    if (!mDsvHeapAllocator.Init())
    {
        LOG_ERROR("Failed to initialize heap allocator for DSV");
        return false;
    }

    return true;
}

Device::~Device()
{
    WaitForGPU();

    mCbvSrvUavHeapAllocator.Release();
    mRtvHeapAllocator.Release();
    mDsvHeapAllocator.Release();

    mDXGIFactory.Reset();
    mAdapters.clear();
    mCommandQueue.Reset();
    mFence.Reset();
    mDevice.Reset();

    ::CloseHandle(mFenceEvent);

    if (mDebugDevice)
    {
        mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
        mInfoQueue.Reset();

        mDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    }
}

ID3D12Device* Device::GetDevice() const
{
    return mDevice.Get();
}

ID3D12CommandQueue* Device::GetCommandQueue() const
{
    return mCommandQueue.Get();
}

void* Device::GetHandle() const
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

bool Device::DetectVideoCards(int preferredId)
{
    for (uint32 i = 0; ; ++i)
    {
        IDXGIAdapter1* adapter;
        HRESULT hr = mDXGIFactory->EnumAdapters1(i, &adapter);

        if (hr == DXGI_ERROR_NOT_FOUND)
            break;

        if (FAILED(hr))
        {
            LOG_ERROR("EnumAdapters1 failed for id=%u", i);
            continue;
        }

        DXGI_ADAPTER_DESC1 adapterDesc;
        adapter->GetDesc1(&adapterDesc);

        // get GPU description
        std::wstring wideDesc = adapterDesc.Description;
        std::string descString;
        Common::UTF16ToUTF8(wideDesc, descString);

        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            descString += " [software adapter]";
        else if (mAdapterInUse < 0)
            mAdapterInUse = i;

        if (static_cast<uint32>(preferredId) == i)
            mAdapterInUse = i;

        LOG_INFO("Adapter found at slot %u: %s", i, descString.c_str());
        mAdapters.push_back(D3DPtr<IDXGIAdapter>(adapter));
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
    if (!mDevice)
        return false;

    HRESULT hr;
    DXGI_ADAPTER_DESC adapterDesc;
    hr = D3D_CALL_CHECK(mAdapters[mAdapterInUse]->GetDesc(&adapterDesc));
    if (FAILED(hr))
        return false;

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

    D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12options;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12options, sizeof(d3d12options));
    if (FAILED(hr))
        LOG_ERROR("Failed to obtain D3D12 options info");
    else
    {
        info.features.push_back("TiledResourcesTier=" +
                                std::to_string(static_cast<int>(d3d12options.TiledResourcesTier)));
        info.features.push_back("ResourceBindingTier=" +
                                std::to_string(static_cast<int>(d3d12options.ResourceBindingTier)));
        info.features.push_back("ResourceHeapTier=" +
                                std::to_string(static_cast<int>(d3d12options.ResourceHeapTier)));

        info.features.push_back("DoublePrecisionFloatShaderOps=" +
                                std::to_string(d3d12options.DoublePrecisionFloatShaderOps));
        info.features.push_back("OutputMergerLogicOp=" +
                                std::to_string(d3d12options.OutputMergerLogicOp));
        info.features.push_back("PSSpecifiedStencilRefSupported=" +
                                std::to_string(d3d12options.PSSpecifiedStencilRefSupported));
        info.features.push_back("TypedUAVLoadAdditionalFormats=" +
                                std::to_string(d3d12options.TypedUAVLoadAdditionalFormats));
        info.features.push_back("ROVsSupported=" +
                                std::to_string(d3d12options.ROVsSupported));
        info.features.push_back("ConservativeRasterizationTier=" +
                                std::to_string(d3d12options.ConservativeRasterizationTier));
        info.features.push_back("MaxGPUVirtualAddressBitsPerResource=" +
                                std::to_string(d3d12options.MaxGPUVirtualAddressBitsPerResource));
        info.features.push_back("StandardSwizzle64KBSupported=" +
                                std::to_string(d3d12options.StandardSwizzle64KBSupported));
        info.features.push_back("CrossAdapterRowMajorTextureSupported=" +
                                std::to_string(d3d12options.CrossAdapterRowMajorTextureSupported));

        const char* minPrecissionSupportStr = "none";
        switch (d3d12options.MinPrecisionSupport)
        {
        case D3D12_SHADER_MIN_PRECISION_SUPPORT_10_BIT:
            minPrecissionSupportStr = "10bit";
            break;
        case D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT:
            minPrecissionSupportStr = "10bit";
            break;
        }
        info.features.push_back(std::string("MinPrecisionSupport=") + minPrecissionSupportStr);

        const char* crossNodeSharingStr = "notSupported";
        switch (d3d12options.CrossNodeSharingTier)
        {
        case D3D12_CROSS_NODE_SHARING_TIER_1_EMULATED:
            crossNodeSharingStr = "1_emulated";
            break;
        case D3D12_CROSS_NODE_SHARING_TIER_1:
            crossNodeSharingStr = "1";
            break;
        case D3D12_CROSS_NODE_SHARING_TIER_2:
            crossNodeSharingStr = "2";
            break;
        }
        info.features.push_back(std::string("CrossNodeSharingTier=") + crossNodeSharingStr);
    }

    return true;
}

bool Device::IsBackbufferFormatSupported(ElementFormat format)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatData;
    formatData.Format = TranslateElementFormat(format);

    HRESULT hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatData, sizeof(formatData));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to check format support info");
        return false;
    }

    return (formatData.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY) == D3D12_FORMAT_SUPPORT1_DISPLAY;
}

CommandRecorderPtr Device::CreateCommandRecorder()
{
    auto commandRecorder = Common::MakeSharedPtr<CommandRecorder>();
    if (!commandRecorder->Init(mDevice.Get()))
    {
        return nullptr;
    }

    return commandRecorder;
}

bool Device::Execute(CommandListID commandList)
{
    if (commandList == INVALID_COMMAND_LIST_ID)
    {
        LOG_ERROR("Invalid command list");
        return false;
    }

    /*
    CommandList* list = dynamic_cast<CommandList*>(commandList);
    if (!list || !list->commandRecorder)
        return false;

    ID3D12CommandList* commandLists[] = { list->commandRecorder->mCommandList.Get() };
    gDevice->mCommandQueue->ExecuteCommandLists(1, commandLists);

    return list->commandRecorder->MoveToNextFrame(gDevice->mCommandQueue.Get());
    */

    // TODO
    return false;
}

bool Device::FinishFrame()
{
    // TODO insert fence (extract code from CommandRecorder)
    return false;
}

bool Device::DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

bool Device::DownloadTexture(const TexturePtr& tex, void* data, int mipmap, int layer)
{
    UNUSED(mipmap);
    UNUSED(layer);

    const Texture* texture = dynamic_cast<Texture*>(tex.Get());
    if (!texture)
    {
        LOG_ERROR("Invalid texture pointer");
        return false;
    }

    WaitForGPU();

    char* mappedData;
    HRESULT hr = texture->GetResource()->Map(0, NULL, reinterpret_cast<void**>(&mappedData));
    if (FAILED(hr))
    {
        return 0;
    }

    size_t rowSize = static_cast<size_t>(GetElementFormatSize(texture->GetFormat())) * static_cast<size_t>(texture->GetWidth());
    size_t rowPitch = texture->GetRowPitch();
    for (uint16 i = 0; i < texture->GetHeight(); ++i)
    {
        char* targetRow = reinterpret_cast<char*>(data) + rowSize * i;
        const char* sourceRow = reinterpret_cast<const char*>(mappedData) + rowPitch * i;
        memcpy(targetRow, sourceRow, rowSize);
    }

    texture->GetResource()->Unmap(0, NULL);

    return true;
}

bool Device::WaitForGPU()
{
    HRESULT hr;

    // Signal and increment the fence value
    const uint64 prevFenceValue = mFenceValue++;
    hr = D3D_CALL_CHECK(mCommandQueue->Signal(mFence.Get(), prevFenceValue));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to enqueue fence value update");
        return false;
    }

    if (mFence->GetCompletedValue() < prevFenceValue)
    {
        LOG_DEBUG("Waiting for GPU...");

        // Wait for the fence value to be updated
        hr = D3D_CALL_CHECK(mFence->SetEventOnCompletion(prevFenceValue, mFenceEvent));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to set completion event for fence");
            return false;
        }

        if (WaitForSingleObject(mFenceEvent, INFINITE) != WAIT_OBJECT_0)
        {
            LOG_ERROR("WaitForSingleObject failed");
            return false;
        }
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
