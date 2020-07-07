/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "CommandRecorder.hpp"
#include "CommandListManager.hpp"
#include "RendererD3D12.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "ComputePipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

#include "Engine/Common/Utils/StringUtils.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"
#include "Engine/Common/Utils/ScopedLock.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/System/Thread.hpp"


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

FenceData::FenceData()
    : value(0)
    , waitEvent(INVALID_HANDLE_VALUE)
{}

bool FenceData::Init(Device* device)
{
    HRESULT hr = D3D_CALL_CHECK(device->GetDevice()->CreateFence(UINT64_MAX, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fenceObject.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 fence object");
        return false;
    }

    // Create an event handle to use for frame synchronization.
    waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (waitEvent == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to create fence event object");
        return false;
    }

    return true;
}

bool FenceData::Flush(ID3D12CommandQueue* queue)
{
    uint64 fenceValue;
    {
        // force ordering of fence values passed to Signal()
        NFE_SCOPED_LOCK(lock);

        fenceValue = value++;

        if (FAILED(D3D_CALL_CHECK(queue->Signal(fenceObject.Get(), fenceValue))))
        {
            NFE_LOG_ERROR("Failed to enqueue fence value update");
            return false;
        }
    }

    // Wait for the fence value to be updated
    if (FAILED(D3D_CALL_CHECK(fenceObject->SetEventOnCompletion(fenceValue, waitEvent))))
    {
        NFE_LOG_ERROR("Failed to set completion event for fence");
        return false;
    }

    if (WaitForSingleObject(waitEvent, INFINITE) != WAIT_OBJECT_0)
    {
        NFE_LOG_ERROR("WaitForSingleObject failed");
        return false;
    }

    HRESULT hr = D3D_CALL_CHECK(gDevice->GetDevice()->GetDeviceRemovedReason());
    if (FAILED(hr))
    {
        return false;
    }

    uint64 completedFenceValue = fenceObject->GetCompletedValue();
    NFE_ASSERT(completedFenceValue == fenceValue, "Invalid completed fence value");

    return true;
}

void FenceData::Release()
{
    fenceObject.Reset();
    CloseHandle(waitEvent);
}

Device::Device()
    : mCbvSrvUavHeapAllocator(HeapAllocator::Type::CbvSrvUav, 1024)
    , mRtvHeapAllocator(HeapAllocator::Type::Rtv, 512)
    , mDsvHeapAllocator(HeapAllocator::Type::Dsv, 512)
    , mAdapterInUse(-1)
    , mDebugLayerEnabled(false)
{
}

bool Device::Init(const DeviceInitParams* params)
{
    DeviceInitParams defaultParams;
    if (!params)
        params = &defaultParams;

    HRESULT hr;

    if (params->debugLevel > 0)
    {
        if (PrepareDXGIDebugLayer())
        {
            NFE_LOG_INFO("Using DXGI debug layer");
        }
        else
        {
            NFE_LOG_ERROR("Failed to setup DXGI debug layer");
        }
    }

    if (!InitializeDevice(params))
    {
        return false;
    }

    if (!DetectFeatureLevel())
    {
        return false;
    }

    if (params->debugLevel > 0)
    {
        if (PrepareD3DDebugLayer())
        {
            NFE_LOG_INFO("Using Direct3D 12 debug layer");
        }
        else
        {
            NFE_LOG_ERROR("Failed to setup Direct3D 12 debug layer");
        }
    }

    // print device info
    // TODO: move to separate file (common for all renderers)
    {
        DeviceInfo deviceInfo;
        if (GetDeviceInfo(deviceInfo))
        {
            NFE_LOG_INFO("GPU name: %s", deviceInfo.description.Str());
            NFE_LOG_INFO("GPU info: %s", deviceInfo.misc.Str());

            Common::String features;
            for (uint32 i = 0; i < deviceInfo.features.Size(); ++i)
            {
                if (i > 0)
                    features += ", ";
                features += deviceInfo.features[i];
            }
            NFE_LOG_INFO("GPU features: %s", features.Str());
        }
    }

    /// create command queues
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        hr = D3D_CALL_CHECK(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mGraphicsQueue.GetPtr())));
        if (FAILED(hr))
        {
            return false;
        }

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        hr = D3D_CALL_CHECK(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mResourceUploadQueue.GetPtr())));
        if (FAILED(hr))
        {
            return false;
        }
    }

    mGraphicsQueueFence.Init(this);
    mResourceUploadQueueFence.Init(this);

    if (!mRingBuffer.Init(8 * 1024 * 1024))
    {
        NFE_LOG_ERROR("Failed to initialize ring buffer");
        return false;
    }

    // create command lists manager
    mCommandListManager = Common::MakeUniquePtr<CommandListManager>();
    if (!mCommandListManager)
    {
        NFE_LOG_ERROR("Failed to allocate command list manager");
        return false;
    }

    if (!CreateResources())
    {
        NFE_LOG_ERROR("Failed to create low-level renderer resources");
        return false;
    }

    return true;
}

Device::~Device()
{
    WaitForGPU();

    mCommandListManager.Reset();

    mCbvSrvUavHeapAllocator.Release();
    mRtvHeapAllocator.Release();
    mDsvHeapAllocator.Release();

    mDXGIFactory.Reset();
    mAdapters.Clear();
    mGraphicsQueue.Reset();
    mResourceUploadQueue.Reset();
    mGraphicsQueueFence.Release();
    mResourceUploadQueueFence.Release();
    mDevice.Reset();

    if (mDebugDevice)
    {
        mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
        mInfoQueue.Reset();

        mDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    }
}

bool Device::InitDebugLayer(int32 level)
{
    if (level > 0)
    {
        D3DPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetPtr()))))
        {
            NFE_LOG_INFO("D3D12 device: Enabling debug layer");
            debugController->EnableDebugLayer();
        }
        else
        {
            return false;
        }

        if (level > 1)
        {
            D3DPtr<ID3D12Debug1> debugController1;
            if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(debugController1.GetPtr()))))
            {
                NFE_LOG_INFO("D3D12 device: Enabling GPU based validation");
                debugController1->SetEnableGPUBasedValidation(TRUE);
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

bool Device::InitializeDevice(const DeviceInitParams* params)
{
    HRESULT hr;

    // Enable the D3D12 debug layer
    if (!InitDebugLayer(params->debugLevel))
    {
        return false;
    }

    hr = D3D_CALL_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(mDXGIFactory.GetPtr())));
    if (FAILED(hr))
        return false;

    int preferredCardId = params != nullptr ? params->preferredCardId : -1;
    if (!DetectVideoCards(preferredCardId))
    {
        NFE_LOG_ERROR("Failed to detect video cards");
        return false;
    }

    hr = D3D_CALL_CHECK(D3D12CreateDevice(mAdapters[mAdapterInUse].Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(mDevice.GetPtr())));
    if (FAILED(hr))
        return false;

    return true;
}

bool Device::PrepareD3DDebugLayer()
{
    HRESULT hr = D3D_CALL_CHECK(mDevice->QueryInterface(IID_PPV_ARGS(mDebugDevice.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("D3D12 device debugging won't be supported");
        return false;
    }

    mDebugLayerEnabled = true;
    if (FAILED(mDevice->QueryInterface(IID_PPV_ARGS(mInfoQueue.GetPtr()))))
    {
        NFE_LOG_ERROR("Failed to query ID3D12InfoQueue interface");
        return false;
    }

    D3D12_MESSAGE_ID messagesToHide[] =
    {
        // performance warning - let's ignore it for now
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
    };

    bool success = true;

    D3D12_INFO_QUEUE_FILTER filter;
    memset(&filter, 0, sizeof(filter));
    filter.DenyList.NumIDs = _countof(messagesToHide);
    filter.DenyList.pIDList = messagesToHide;
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueue->AddStorageFilterEntries(&filter)));
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE)));
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE)));
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE)));

    return success;
}

bool Device::PrepareDXGIDebugLayer()
{
    D3DPtr<IDXGIInfoQueue> dxgiInfoQueue;

    typedef HRESULT(WINAPI* LPDXGIGETDEBUGINTERFACE)(REFIID, void**);

    HMODULE dxgidebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!dxgidebug)
    {
        NFE_LOG_ERROR("Failed to load DXGIDebug library");
        return false;
    }

    auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
        reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));
    if (FAILED(dxgiGetDebugInterface(IID_PPV_ARGS(dxgiInfoQueue.GetPtr()))))
    {
        NFE_LOG_ERROR("Failed to obtain DXGIGetDebugInterface");
        return false;
    }

    bool success = true;

    success &= SUCCEEDED(D3D_CALL_CHECK(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true)));
    success &= SUCCEEDED(D3D_CALL_CHECK(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true)));

    return true;
}

bool Device::CreateResources()
{
    if (!mCbvSrvUavHeapAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for CBV, SRV and UAV");
        return false;
    }

    if (!mRtvHeapAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for RTV");
        return false;
    }

    if (!mDsvHeapAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for DSV");
        return false;
    }

    if (!mShaderCompiler.Init())
    {
        NFE_LOG_ERROR("Failed to initialize DXC shader compiler");
        return false;
    }

    return true;
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
            NFE_LOG_ERROR("EnumAdapters1 failed for id=%u", i);
            continue;
        }

        DXGI_ADAPTER_DESC1 adapterDesc;
        adapter->GetDesc1(&adapterDesc);

        // get GPU description
        Common::Utf16String wideDesc = adapterDesc.Description;
        Common::String descString;
        Common::UTF16ToUTF8(wideDesc, descString);

        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            descString += " [software adapter]";
        else if (mAdapterInUse < 0)
            mAdapterInUse = i;

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

bool Device::DetectFeatureLevel()
{
    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_9_1,  D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_12_1,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelsInfo;
    featureLevelsInfo.NumFeatureLevels = static_cast<UINT>(ArraySize(featureLevels));
    featureLevelsInfo.pFeatureLevelsRequested = featureLevels;

    if (SUCCEEDED(mDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelsInfo, sizeof(featureLevelsInfo))))
    {
        const char* featureLevelStr = "unknown";
        mFeatureLevel = featureLevelsInfo.MaxSupportedFeatureLevel;
        switch (mFeatureLevel)
        {
        case D3D_FEATURE_LEVEL_9_1:     featureLevelStr = "9_1";    break;
        case D3D_FEATURE_LEVEL_9_2:     featureLevelStr = "9_2";    break;
        case D3D_FEATURE_LEVEL_9_3:     featureLevelStr = "9_3";    break;
        case D3D_FEATURE_LEVEL_10_0:    featureLevelStr = "10_0";   break;
        case D3D_FEATURE_LEVEL_10_1:    featureLevelStr = "10_1";   break;
        case D3D_FEATURE_LEVEL_11_0:    featureLevelStr = "11_0";   break;
        case D3D_FEATURE_LEVEL_11_1:    featureLevelStr = "11_1";   break;
        case D3D_FEATURE_LEVEL_12_0:    featureLevelStr = "12_0";   break;
        case D3D_FEATURE_LEVEL_12_1:    featureLevelStr = "12_1";   break;
        }

        NFE_LOG_INFO("Direct3D 12 device created with %s feature level", featureLevelStr);
    }
    else
    {
        NFE_LOG_ERROR("Failed to obtain Direct3D feature level");
        mFeatureLevel = D3D_FEATURE_LEVEL_9_1;
        return false;
    }

    return true;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    using namespace Common;

    if (!mDevice)
        return false;

    HRESULT hr;
    DXGI_ADAPTER_DESC adapterDesc;
    hr = D3D_CALL_CHECK(mAdapters[mAdapterInUse]->GetDesc(&adapterDesc));
    if (FAILED(hr))
        return false;

    // get GPU description
    Common::Utf16String wideDesc = adapterDesc.Description;
    Common::UTF16ToUTF8(wideDesc, info.description);

    // get various GPU information
    info.misc =
        "Vendor ID: " + ToString(adapterDesc.VendorId) +
        ", Device ID: " + ToString(adapterDesc.DeviceId) +
        ", Sub System ID: " + ToString(adapterDesc.SubSysId) +
        ", Revision: " + ToString(adapterDesc.Revision) +
        ", Dedicated Video Memory: " + ToString(uint64(adapterDesc.DedicatedVideoMemory >> 10u)) + " KB"
        ", Dedicated System Memory: " + ToString(uint64(adapterDesc.DedicatedSystemMemory >> 10u)) + " KB"
        ", Shared System Memory: " + ToString(uint64(adapterDesc.SharedSystemMemory >> 10u)) + " KB";

    D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12options;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12options, sizeof(d3d12options));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to obtain D3D12 options info");
        return false;
    }

    info.features.PushBack("TiledResourcesTier=" + ToString(static_cast<uint32>(d3d12options.TiledResourcesTier)));
    info.features.PushBack("ResourceBindingTier=" + ToString(static_cast<uint32>(d3d12options.ResourceBindingTier)));
    info.features.PushBack("ResourceHeapTier=" + ToString(static_cast<uint32>(d3d12options.ResourceHeapTier)));

    info.features.PushBack("DoublePrecisionFloatShaderOps=" + ToString(d3d12options.DoublePrecisionFloatShaderOps));
    info.features.PushBack("OutputMergerLogicOp=" + ToString(d3d12options.OutputMergerLogicOp));
    info.features.PushBack("PSSpecifiedStencilRefSupported=" + ToString(d3d12options.PSSpecifiedStencilRefSupported));
    info.features.PushBack("TypedUAVLoadAdditionalFormats=" + ToString(d3d12options.TypedUAVLoadAdditionalFormats));
    info.features.PushBack("ROVsSupported=" + ToString(d3d12options.ROVsSupported));
    info.features.PushBack("ConservativeRasterizationTier=" + ToString(d3d12options.ConservativeRasterizationTier));
    info.features.PushBack("MaxGPUVirtualAddressBitsPerResource=" + ToString(d3d12options.MaxGPUVirtualAddressBitsPerResource));
    info.features.PushBack("StandardSwizzle64KBSupported=" + ToString(d3d12options.StandardSwizzle64KBSupported));
    info.features.PushBack("CrossAdapterRowMajorTextureSupported=" + ToString(d3d12options.CrossAdapterRowMajorTextureSupported));

    // minimum precision support
    {
        const char* minPrecissionSupportStr = "none";
        switch (d3d12options.MinPrecisionSupport)
        {
        case D3D12_SHADER_MIN_PRECISION_SUPPORT_10_BIT:
            minPrecissionSupportStr = "10 bit";
            break;
        case D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT:
            minPrecissionSupportStr = "16 bit";
            break;
        }
        info.features.PushBack(String("MinPrecisionSupport=") + minPrecissionSupportStr);
    }

    // cross-node sharing support
    {
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
        info.features.PushBack(String("CrossNodeSharingTier=") + crossNodeSharingStr);
    }

    return true;
}

bool Device::IsBackbufferFormatSupported(Format format)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatData;
    formatData.Format = TranslateElementFormat(format);

    HRESULT hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatData, sizeof(formatData));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to check format support info");
        return false;
    }

    return (formatData.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY) == D3D12_FORMAT_SUPPORT1_DISPLAY;
}

CommandRecorderPtr Device::CreateCommandRecorder()
{
    return Common::MakeSharedPtr<CommandRecorder>();
}

bool Device::Execute(const Common::ArrayView<ICommandList*> commandLists)
{
    uint64 fenceValue;
    {
        // force ordering of fence values passed to Signal()
        NFE_SCOPED_LOCK(mGraphicsQueueFence.lock);

        fenceValue = mGraphicsQueueFence.value++;
        mCommandListManager->ExecuteCommandList(commandLists, fenceValue);
        mGraphicsQueue->Signal(mGraphicsQueueFence.fenceObject.Get(), fenceValue);
    }

    mRingBuffer.FinishFrame(fenceValue);

    // TODO this should be done on separate thread
    {
        uint64 completedFenceValue = mGraphicsQueueFence.fenceObject->GetCompletedValue();
        if (completedFenceValue != UINT64_MAX)
        {
            mCommandListManager->OnFenveValueCompleted(completedFenceValue);
            mRingBuffer.OnFrameCompleted(completedFenceValue);
        }
    }

    return true;
}

bool Device::DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data)
{
    NFE_UNUSED(buffer);
    NFE_UNUSED(offset);
    NFE_UNUSED(size);
    NFE_UNUSED(data);
    return false;
}

bool Device::DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer)
{
    NFE_UNUSED(mipmap);
    NFE_UNUSED(layer);

    const Texture* texture = dynamic_cast<Texture*>(tex.Get());
    if (!texture)
    {
        NFE_LOG_ERROR("Invalid texture pointer");
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

bool Device::FinishFrame()
{
    return true;
}

bool Device::WaitForGPU()
{
    Common::Timer timer;
    timer.Start();

    bool result = mGraphicsQueueFence.Flush(mGraphicsQueue.Get());

    NFE_LOG_WARNING("Waiting for GPU took %.3f ms", 1000.0 * timer.Stop());

    mCommandListManager->OnFenveValueCompleted(mGraphicsQueueFence.value);
    mRingBuffer.OnFrameCompleted(mGraphicsQueueFence.value);

    return result;
}

} // namespace Renderer
} // namespace NFE
