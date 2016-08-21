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
    : mCbvSrvUavHeapAllocator(HeapAllocator::Type::CbvSrvUav, 1024)
    , mRtvHeapAllocator(HeapAllocator::Type::Rtv, 64)
    , mDsvHeapAllocator(HeapAllocator::Type::Dsv, 16)
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
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            LOG_INFO("Enabling D3D12 debug layer");
            debugController->EnableDebugLayer();
        }
    }

    hr = D3D_CALL_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&mDXGIFactory)));
    if (FAILED(hr))
        return false;

    int preferredCardId = params != nullptr ? params->preferredCardId : -1;
    if (!DetectVideoCards(preferredCardId))
    {
        LOG_ERROR("Failed to detect video cards");
        return false;
    }

    hr = D3D_CALL_CHECK(D3D12CreateDevice(mAdapters[mAdapterInUse].get(),
                                          D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));
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

    // print device info
    // TODO: move to separate file (common for all renderers)
    {
        DeviceInfo deviceInfo;
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

    if (params->debugLevel > 0)
    {
        mDebugLayerEnabled = true;

        D3DPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(mDevice->QueryInterface(IID_PPV_ARGS(&infoQueue))))
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
            infoQueue->AddStorageFilterEntries(&filter);

            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        }
    }

    /// create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = D3D_CALL_CHECK(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
    if (FAILED(hr))
        return false;

    // create fence for frame synchronization
    hr = D3D_CALL_CHECK(gDevice->mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
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

    mDXGIFactory.reset();
    mAdapters.clear();
    mDevice.reset();

    ::CloseHandle(mFenceEvent);
}

ID3D12Device* Device::GetDevice() const
{
    return mDevice.get();
}

ID3D12CommandQueue* Device::GetCommandQueue() const
{
    return mCommandQueue.get();
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
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12options,
                                      sizeof(d3d12options));
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

ICommandBuffer* Device::CreateCommandBuffer()
{
    CommandBuffer* commandBuffer = new CommandBuffer;
    if (!commandBuffer->Init(mDevice.get()))
    {
        delete commandBuffer;
        return nullptr;
    }

    return commandBuffer;
}

bool Device::Execute(ICommandList* commandList)
{
    CommandList* list = dynamic_cast<CommandList*>(commandList);
    if (!list || !list->commandBuffer)
        return false;

    ID3D12CommandList* commandLists[] = { list->commandBuffer->mCommandList.get() };
    gDevice->mCommandQueue->ExecuteCommandLists(1, commandLists);

    return list->commandBuffer->MoveToNextFrame(gDevice->mCommandQueue.get());
}

bool Device::DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

bool Device::DownloadTexture(ITexture* tex, void* data, int mipmap, int layer)
{
    UNUSED(tex);
    UNUSED(data);
    UNUSED(mipmap);
    UNUSED(layer);
    return false;
}

ID3D12PipelineState* Device::GetFullPipelineState(const FullPipelineStateParts& parts)
{
    D3DPtr<ID3D12PipelineState>& fullState = mPipelineStateMap[parts];
    if (!fullState)
        fullState = PipelineState::CreateFullPipelineState(parts);

    return fullState.get();
}

void Device::OnShaderProgramDestroyed(IShaderProgram* program)
{
    // TODO this is extremely slow

    std::vector<FullPipelineStateParts> toRemove;
    for (const auto& pair : mPipelineStateMap)
        if (std::get<1>(pair.first) == program)
            toRemove.push_back(pair.first);

    WaitForGPU();

    for (const auto& parts : toRemove)
        mPipelineStateMap.erase(parts);
}

void Device::OnPipelineStateDestroyed(IPipelineState* pipelineState)
{
    // TODO this is extremely slow

    std::vector<FullPipelineStateParts> toRemove;
    for (const auto& pair : mPipelineStateMap)
        if (std::get<0>(pair.first) == pipelineState)
            toRemove.push_back(pair.first);

    WaitForGPU();

    for (const auto& parts : toRemove)
        mPipelineStateMap.erase(parts);
}

bool Device::WaitForGPU()
{
    HRESULT hr;

    // Signal and increment the fence value
    const uint64 prevFenceValue = mFenceValue++;
    hr = D3D_CALL_CHECK(mCommandQueue->Signal(mFence.get(), prevFenceValue));
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
