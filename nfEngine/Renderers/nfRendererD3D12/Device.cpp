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

#include "../nfCommon/Win/Common.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

namespace {

// TODO dynamic heap expansion
const UINT INITIAL_CBV_SRV_UAV_HEAP_SIZE = 1024;
const UINT INITIAL_SAMPLER_HEAP_SIZE = 1024;

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
            featureLevelStr = "11_1";
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
        LOG_ERROR("Failed to obtained Direct3D feature level");
        mFeatureLevel = D3D_FEATURE_LEVEL_9_1;
    }

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

    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
    cbvSrvUavHeapDesc.NumDescriptors = INITIAL_CBV_SRV_UAV_HEAP_SIZE;
    cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = D3D_CALL_CHECK(mDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&mCbvSrvUavHeap)));
    if (FAILED(hr))
        return;

    mCbvSrvUavHeapMap.resize(INITIAL_CBV_SRV_UAV_HEAP_SIZE);
    for (size_t i = 0; i < mCbvSrvUavHeapMap.size(); ++i)
        mCbvSrvUavHeapMap[i] = false;

    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = INITIAL_SAMPLER_HEAP_SIZE;
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = D3D_CALL_CHECK(mDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&mSamplerHeap)));
    if (FAILED(hr))
        return;

    mSamplerHeapMap.resize(INITIAL_SAMPLER_HEAP_SIZE);
    for (size_t i = 0; i < mSamplerHeapMap.size(); ++i)
        mSamplerHeapMap[i] = false;

    // obtain descriptor sizes
    mCbvSrvUavDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    LOG_DEBUG("CBV/SRV/UAV descriptor heap handle increment: %u", mCbvSrvUavDescSize);
    mSamplerDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    LOG_DEBUG("Sampler descriptor heap handle increment: %u", mSamplerDescSize);
    mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    LOG_DEBUG("RTV descriptor heap handle increment: %u", mRtvDescSize);
    mDsvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    LOG_DEBUG("DSV descriptor heap handle increment: %u", mDsvDescSize);
}

Device::~Device()
{
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

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    if (!mDevice.get())
        return false;

    HRESULT hr;
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
    return new CommandBuffer(mDevice.get());
}

bool Device::Execute(ICommandList* commandList)
{
    CommandList* list = dynamic_cast<CommandList*>(commandList);
    if (!list || !list->commandBuffer)
        return false;

    ID3D12CommandList* commandLists[] = { list->commandBuffer->mCommandList.get() };
    gDevice->mCommandQueue->ExecuteCommandLists(1, commandLists);

    return true;
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

void Device::GetCbvSrvUavHeapInfo(UINT& descriptorSize, D3D12_CPU_DESCRIPTOR_HANDLE& ptr)
{
    descriptorSize = mCbvSrvUavDescSize;
    ptr = mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
}

size_t Device::AllocateCbvSrvUavHeap(size_t numDescriptors)
{
    size_t first = 0;
    size_t count = 0;
    for (size_t i = 0; i < mCbvSrvUavHeapMap.size(); ++i)
    {
        if (mCbvSrvUavHeapMap[i])
        {
            count = 0;
            first = i + 1;
            continue;
        }

        count++;
        if (count >= numDescriptors)
        {
            for (size_t j = first; j < first + count; ++j)
                mCbvSrvUavHeapMap[i] = true;
            return first;
        }
    }

    LOG_ERROR("Descriptor heap allocation failed");
    return static_cast<size_t>(-1);
}

void Device::FreeCbvSrvUavHeap(size_t offset, size_t numDescriptors)
{
    assert(offset + numDescriptors < INITIAL_CBV_SRV_UAV_HEAP_SIZE);

    for (size_t i = offset; i < offset + numDescriptors; ++i)
        mCbvSrvUavHeapMap[i] = true;
}

void Device::GetSamplerHeapInfo(UINT& descriptorSize, D3D12_CPU_DESCRIPTOR_HANDLE& ptr)
{
    descriptorSize = mSamplerDescSize;
    ptr = mSamplerHeap->GetCPUDescriptorHandleForHeapStart();
}

size_t Device::AllocateSamplerHeap()
{
    for (size_t i = 0; i < mSamplerHeapMap.size(); ++i)
    {
        if (!mSamplerHeapMap[i])
            return i;
    }

    LOG_ERROR("Descriptor heap allocation failed");
    return static_cast<size_t>(-1);
}

void Device::FreeSamplerHeap(size_t offset)
{
    assert(offset < INITIAL_SAMPLER_HEAP_SIZE);
    mSamplerHeapMap[offset] = false;
}

bool Device::WaitForGPU()
{
    UINT64 fenceValue = 1;
    D3DPtr<ID3D12Fence> fenceObject; // TODO this could be created once in the constructor
    HRESULT hr;

    if (FAILED(D3D_CALL_CHECK(gDevice->mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                                            IID_PPV_ARGS(&fenceObject)))))
        return false;

    // Create an event handle to use for frame synchronization.
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
    {
        LOG_ERROR("Failed to create fence event object");
        return false;
    }

    // Signal and increment the fence value.
    const UINT64 fence = fenceValue;
    hr = D3D_CALL_CHECK(mCommandQueue->Signal(fenceObject.get(), fence));
    if (FAILED(hr))
        return false;
    fenceValue++;

    // Wait until the previous frame is finished.
    if (fenceObject->GetCompletedValue() < fence)
    {
        hr = D3D_CALL_CHECK(fenceObject->SetEventOnCompletion(fence, fenceEvent));
        if (FAILED(hr))
            return false;
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
