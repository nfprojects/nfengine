/**
 * @file
 * @author  Witek902
 * @brief   D3D12 implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"
#include "CommandRecorder.hpp"
#include "CommandListManager.hpp"
#include "CommandQueue.hpp"
#include "RendererD3D12.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "MemoryBlock.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "ComputePipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

#include "Engine/Common/Utils/StringUtils.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"
#include "Engine/Common/Utils/ScopedLock.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/System/Thread.hpp"

#include <dxgi1_5.h>


namespace NFE {
namespace Renderer {

using namespace Common;

namespace {

template<typename Type, typename... ArgTypes>
SharedPtr<Type> CreateGenericResource(const ArgTypes& ... args)
{
    auto resource = MakeSharedPtr<Type>();
    if (!resource)
    {
        return nullptr;
    }

    if (!resource->Init(args...))
    {
        return nullptr;
    }

    return resource;
}

} // namespace


Device::Device()
    : mCbvSrvUavHeapStagingAllocator(HeapAllocator::Type::CbvSrvUav, 1024)
    , mCbvSrvUavHeapAllocator(HeapAllocator::Type::CbvSrvUav, 1024, true)
    , mSamplerHeapStagingAllocator(HeapAllocator::Type::Sampler, 1024)
    , mSamplerHeapAllocator(HeapAllocator::Type::Sampler, 1024, true)
    , mRtvHeapAllocator(HeapAllocator::Type::Rtv, 256)
    , mDsvHeapAllocator(HeapAllocator::Type::Dsv, 256)
    , mDebugLayerEnabled(false)
{}

bool Device::Init(const DeviceInitParams* params)
{
    NFE_LOG_DEBUG("Started initializing renderer device");

    DeviceInitParams defaultParams;
    if (!params)
        params = &defaultParams;

    if (!InitializeDevice(params))
    {
        return false;
    }

    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = mDevice.Get();
        allocatorDesc.pAdapter = mAdapter.Get();

        if (FAILED(D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator)))
        {
            NFE_LOG_ERROR("Failed to initialize D3D12MemoryAllocator");
            return false;
        }
    }

    if (!DetectFeatureLevel())
    {
        return false;
    }

    if (params->debugLevel > 0)
    {
        if (!PrepareD3DDebugLayer())
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

            String features;
            for (uint32 i = 0; i < deviceInfo.features.Size(); ++i)
            {
                features += "\n    ";
                features += deviceInfo.features[i];
            }
            NFE_LOG_INFO("GPU features: %s", features.Str());
        }
    }

    mFenceManager.Initialize();

    mRingBuffer = MakeUniquePtr<RingBuffer>();
    if (!mRingBuffer->Init(64 * 1024 * 1024))
    {
        NFE_LOG_ERROR("Failed to initialize ring buffer");
        return false;
    }

    // create command lists manager
    mCommandListManager = MakeUniquePtr<CommandListManager>();
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

    NFE_LOG_DEBUG("Finished initializing renderer device");

    return true;
}

Device::~Device()
{
    const uint32 numQueues = ReleaseUnusedCommandQueues();
    NFE_ASSERT(numQueues == 0u, "There are still %u command queues alive", numQueues);

    mCommandListManager.Reset();

    mCbvSrvUavHeapStagingAllocator.Release();
    mCbvSrvUavHeapAllocator.Release();
    mSamplerHeapStagingAllocator.Release();
    mSamplerHeapAllocator.Release();
    mRtvHeapAllocator.Release();
    mDsvHeapAllocator.Release();

    mDXGIFactory.Reset();
    mAdapter.Reset();

    mFenceManager.Uninitialize();

    if (mAllocator)
    {
        mAllocator->Release();
        mAllocator = nullptr;
    }

    mDevice.Reset();

    if (mDebugDevice)
    {
        mInfoQueueD3D->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
        mInfoQueueD3D.Reset();

        mDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    }
}

bool Device::InitializeDevice(const DeviceInitParams* params)
{
    HRESULT hr;

    DWORD dxgiFactoryFlags = 0;

    if (params->debugLevel > 0)
    {
        D3DPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetPtr()))))
        {
            NFE_LOG_INFO("Enabling D3D12 debug layer");
            debugController->EnableDebugLayer();
        }
        else
        {
            NFE_LOG_WARNING("D3D12 Debug Device is not available");
        }

        if (debugController && params->debugLevel > 1)
        {
            D3DPtr<ID3D12Debug1> debugController1;
            if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(debugController1.GetPtr()))))
            {
                NFE_LOG_INFO("Enabling GPU-based validation");
                debugController1->SetEnableGPUBasedValidation(TRUE);
            }
            else
            {
                NFE_LOG_WARNING("D3D12 GPU-based validation is not available");
            }
        }

        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(mInfoQueueDXGI.GetPtr()))))
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
            mInfoQueueDXGI->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            mInfoQueueDXGI->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);
            mInfoQueueDXGI->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            NFE_LOG_INFO("Enabling DXGI debug layer");
        }
        else
        {
            NFE_LOG_WARNING("DXGI debugging is not available");
        }
    }

    hr = D3D_CALL_CHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(mDXGIFactory.GetPtr())));
    if (FAILED(hr))
    {
        return false;
    }

    // determine tearing support
    {
        D3DPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(mDXGIFactory->QueryInterface(IID_PPV_ARGS(factory5.GetPtr()))))
        {
            BOOL allowTearing = FALSE;
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            mCaps.tearingSupport = SUCCEEDED(hr) && allowTearing;
        }
    }

    int preferredCardId = params != nullptr ? params->preferredCardId : -1;
    if (!DetectVideoCards(preferredCardId))
    {
        NFE_LOG_ERROR("Failed to detect video cards");
        return false;
    }

    DetectMonitors();

    hr = D3D_CALL_CHECK(D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(mDevice.GetPtr())));
    if (FAILED(hr))
    {
        return false;
    }

    {
        mCaps.d3dOptions = {};
        hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &mCaps.d3dOptions, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
        if (FAILED(hr))
        {
            NFE_LOG_WARNING("Failed to query D3D12 device options");
        }
    }

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
    if (FAILED(mDevice->QueryInterface(IID_PPV_ARGS(mInfoQueueD3D.GetPtr()))))
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
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueueD3D->AddStorageFilterEntries(&filter)));
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueueD3D->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE)));
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueueD3D->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE)));
    success &= SUCCEEDED(D3D_CALL_CHECK(mInfoQueueD3D->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE)));

    return success;
}

bool Device::CreateResources()
{
    if (!mCbvSrvUavHeapStagingAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for CBV, SRV and UAV (non shader visible)");
        return false;
    }

    if (!mCbvSrvUavHeapAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for CBV, SRV and UAV (shader visible)");
        return false;
    }

    if (!mSamplerHeapStagingAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for samplers (non shader visible)");
        return false;
    }

    if (!mSamplerHeapAllocator.Init())
    {
        NFE_LOG_ERROR("Failed to initialize heap allocator for samplers (shader visible)");
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

    if (!CreateGraphicsRootSignature())
    {
        NFE_LOG_ERROR("Failed to initialize graphics root signature");
        return false;
    }

    if (!CreateComputeRootSignature())
    {
        NFE_LOG_ERROR("Failed to initialize compute root signature");
        return false;
    }

    if (!CreateIndirectDispatchCommandSignature())
    {
        NFE_LOG_ERROR("Failed to initialize command signature for indirect dispatch");
        return false;
    }

    return true;
}

bool Device::CreateGraphicsRootSignature()
{
    constexpr uint32_t NumRootParams = 7;

    Common::FixedArray<D3D12_ROOT_PARAMETER, NumRootParams> rootParameters;

    D3D12_DESCRIPTOR_RANGE vsCbRange = {};
    vsCbRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    vsCbRange.NumDescriptors = NFE_MAX_CBUFFER_SLOTS;
    vsCbRange.OffsetInDescriptorsFromTableStart = 0; // D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[0] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &vsCbRange;

    D3D12_DESCRIPTOR_RANGE vsSrvRange = {};
    vsSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    vsSrvRange.NumDescriptors = NFE_MAX_SHADER_RESOURCE_SLOTS;
    vsSrvRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[1] = {};
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &vsSrvRange;

    D3D12_DESCRIPTOR_RANGE psCbRange = {};
    psCbRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    psCbRange.NumDescriptors = NFE_MAX_CBUFFER_SLOTS;
    psCbRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[2] = {};
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &psCbRange;

    D3D12_DESCRIPTOR_RANGE psSrvRange = {};
    psSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    psSrvRange.NumDescriptors = NFE_MAX_SHADER_RESOURCE_SLOTS;
    psSrvRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[3] = {};
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &psSrvRange;

    D3D12_DESCRIPTOR_RANGE psUavRange = {};
    psUavRange = {};
    psUavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    psUavRange.NumDescriptors = NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS;
    psUavRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[4] = {};
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[4].DescriptorTable.pDescriptorRanges = &psUavRange;

    D3D12_DESCRIPTOR_RANGE vsSamplerRange = {};
    vsSamplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    vsSamplerRange.NumDescriptors = NFE_MAX_SAMPLER_SLOTS;
    vsSamplerRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[5] = {};
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[5].DescriptorTable.pDescriptorRanges = &vsSamplerRange;

    D3D12_DESCRIPTOR_RANGE psSamplerRange = {};
    psSamplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    psSamplerRange.NumDescriptors = NFE_MAX_SAMPLER_SLOTS;
    psSamplerRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[6] = {};
    rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[6].DescriptorTable.pDescriptorRanges = &psSamplerRange;

    D3D12_ROOT_SIGNATURE_DESC rsd = {};
    rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rsd.pParameters = rootParameters.Data();
    rsd.NumParameters = NumRootParams;
    rsd.NumStaticSamplers = 0;

    HRESULT hr;
    D3DPtr<ID3D10Blob> rootSignature, errorsBuffer;
    hr = D3D_CALL_CHECK(D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1,
                                                    rootSignature.GetPtr(), errorsBuffer.GetPtr()));
    if (FAILED(hr))
        return false;

    NFE_LOG_DEBUG("Graphics root signature blob size: %u bytes\n", rootSignature->GetBufferSize());

    hr = D3D_CALL_CHECK(mDevice->CreateRootSignature(
        0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(),
        IID_PPV_ARGS(mGraphicsRootSignature.GetPtr())));

    if (!SetDebugName(mGraphicsRootSignature.Get(), "Default Root Signature"))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }

    return true;
}

bool Device::CreateComputeRootSignature()
{
    constexpr uint32_t NumRootParams = 4;

    Common::FixedArray<D3D12_ROOT_PARAMETER, NumRootParams> rootParameters;

    D3D12_DESCRIPTOR_RANGE csCbRange = {};
    csCbRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    csCbRange.NumDescriptors = NFE_MAX_CBUFFER_SLOTS;
    csCbRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[0] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &csCbRange;

    D3D12_DESCRIPTOR_RANGE csSrvRange = {};
    csSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    csSrvRange.NumDescriptors = NFE_MAX_SHADER_RESOURCE_SLOTS;
    csSrvRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[1] = {};
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &csSrvRange;

    D3D12_DESCRIPTOR_RANGE csUavRange = {};
    csUavRange = {};
    csUavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    csUavRange.NumDescriptors = NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS;
    csUavRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[2] = {};
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &csUavRange;

    D3D12_DESCRIPTOR_RANGE csSamplerRange = {};
    csSamplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    csSamplerRange.NumDescriptors = NFE_MAX_SAMPLER_SLOTS;
    csSamplerRange.OffsetInDescriptorsFromTableStart = 0; //D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParameters[3] = {};
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &csSamplerRange;

    D3D12_ROOT_SIGNATURE_DESC rsd = {};
    rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rsd.pParameters = rootParameters.Data();
    rsd.NumParameters = NumRootParams;
    rsd.NumStaticSamplers = 0;

    HRESULT hr;
    D3DPtr<ID3D10Blob> rootSignature, errorsBuffer;
    hr = D3D_CALL_CHECK(D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1,
        rootSignature.GetPtr(), errorsBuffer.GetPtr()));
    if (FAILED(hr))
        return false;

    NFE_LOG_DEBUG("Compute root signature blob size: %u bytes\n", rootSignature->GetBufferSize());

    hr = D3D_CALL_CHECK(mDevice->CreateRootSignature(
        0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(),
        IID_PPV_ARGS(mComputeRootSignature.GetPtr())));

    if (!SetDebugName(mComputeRootSignature.Get(), "Compute Root Signature"))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }

    return true;
}

bool Device::CreateIndirectDispatchCommandSignature()
{
    D3D12_INDIRECT_ARGUMENT_DESC indirectArgDesc = {};
    indirectArgDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    commandSignatureDesc.NodeMask = 1;
    commandSignatureDesc.pArgumentDescs = &indirectArgDesc;
    commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
    commandSignatureDesc.NumArgumentDescs = 1;

    HRESULT hr = mDevice->CreateCommandSignature(
        &commandSignatureDesc, nullptr, IID_PPV_ARGS(mIndirectDispatchCommandSignature.GetPtr()));

    return SUCCEEDED(hr);
}

void* Device::GetHandle() const
{
    return mDevice.Get();
}

VertexLayoutPtr Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    return CreateGenericResource<VertexLayout, VertexLayoutDesc>(desc);
}

MemoryBlockPtr Device::CreateMemoryBlock(const MemoryBlockDesc& desc)
{
    return CreateGenericResource<MemoryBlock, MemoryBlockDesc>(desc);
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

CommandQueuePtr Device::CreateCommandQueue(CommandQueueType type, const char* debugName)
{
    SharedPtr<CommandQueue> queue = CreateGenericResource<CommandQueue, CommandQueueType, const char*>(type, debugName);
    {
        NFE_SCOPED_LOCK(mCommandQueuesLock);
        mCommandQueues.PushBack(queue);
    }
    return queue;
}

bool Device::DetectVideoCards(int preferredId)
{
    mAdapter.Reset();

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
        Utf16String wideDesc = adapterDesc.Description;
        String descString;
        UTF16ToUTF8(wideDesc, descString);

        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            descString += " [software adapter]";
        }

        if (!mAdapter || static_cast<uint32>(preferredId) == i)
        {
            mAdapter = D3DPtr<IDXGIAdapter>(adapter);
        }

        NFE_LOG_INFO("Adapter found at slot %u: %s", i, descString.Str());
    }

    return mAdapter;
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
        mCaps.featureLevel = featureLevelsInfo.MaxSupportedFeatureLevel;
        switch (mCaps.featureLevel)
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
        mCaps.featureLevel = D3D_FEATURE_LEVEL_9_1;
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
    hr = D3D_CALL_CHECK(mAdapter->GetDesc(&adapterDesc));
    if (FAILED(hr))
        return false;

    // get GPU description
    Utf16String wideDesc = adapterDesc.Description;
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

    D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12options;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12options, sizeof(d3d12options));
    if (SUCCEEDED(hr))
    {
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
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS1 d3d12options1;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &d3d12options1, sizeof(d3d12options1));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("WaveOps=" + ToString(d3d12options1.WaveOps));
        info.features.PushBack("WaveLaneCountMin=" + ToString(d3d12options1.WaveLaneCountMin));
        info.features.PushBack("WaveLaneCountMax=" + ToString(d3d12options1.WaveLaneCountMax));
        info.features.PushBack("TotalLaneCount=" + ToString(d3d12options1.TotalLaneCount));
        info.features.PushBack("ExpandedComputeResourceStates=" + ToString(d3d12options1.ExpandedComputeResourceStates));
        info.features.PushBack("Int64ShaderOps=" + ToString(d3d12options1.Int64ShaderOps));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS2 d3d12options2;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &d3d12options2, sizeof(d3d12options2));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("DepthBoundsTestSupported=" + ToString(d3d12options2.DepthBoundsTestSupported));
        info.features.PushBack("ProgrammableSamplePositionsTier=" + ToString(static_cast<uint32>(d3d12options2.ProgrammableSamplePositionsTier)));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS3 d3d12options3;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &d3d12options3, sizeof(d3d12options3));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("CopyQueueTimestampQueriesSupported=" + ToString(d3d12options3.CopyQueueTimestampQueriesSupported));
        info.features.PushBack("CastingFullyTypedFormatSupported=" + ToString(d3d12options3.CastingFullyTypedFormatSupported));
        info.features.PushBack("ViewInstancingTier=" + ToString(static_cast<uint32>(d3d12options3.ViewInstancingTier)));
        info.features.PushBack("BarycentricsSupported=" + ToString(d3d12options3.BarycentricsSupported));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS4 d3d12options4;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &d3d12options4, sizeof(d3d12options4));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("MSAA64KBAlignedTextureSupported=" + ToString(d3d12options4.MSAA64KBAlignedTextureSupported));
        info.features.PushBack("SharedResourceCompatibilityTier=" + ToString(static_cast<uint32>(d3d12options4.SharedResourceCompatibilityTier)));
        info.features.PushBack("Native16BitShaderOpsSupported=" + ToString(d3d12options4.Native16BitShaderOpsSupported));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 d3d12options5;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12options5, sizeof(d3d12options5));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("SRVOnlyTiledResourceTier3=" + ToString(d3d12options5.SRVOnlyTiledResourceTier3));
        info.features.PushBack("RenderPassesTier=" + ToString(static_cast<uint32>(d3d12options5.RenderPassesTier)));
        info.features.PushBack("RaytracingTier=" + ToString(static_cast<uint32>(d3d12options5.RaytracingTier)));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS6 d3d12options6;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &d3d12options6, sizeof(d3d12options6));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("AdditionalShadingRatesSupported=" + ToString(d3d12options6.AdditionalShadingRatesSupported));
        info.features.PushBack("PerPrimitiveShadingRateSupportedWithViewportIndexing=" + ToString(d3d12options6.PerPrimitiveShadingRateSupportedWithViewportIndexing));
        info.features.PushBack("VariableShadingRateTier=" + ToString(d3d12options6.VariableShadingRateTier));
        info.features.PushBack("ShadingRateImageTileSize=" + ToString(d3d12options6.ShadingRateImageTileSize));
        info.features.PushBack("BackgroundProcessingSupported=" + ToString(d3d12options6.BackgroundProcessingSupported));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 d3d12options7;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &d3d12options7, sizeof(d3d12options7));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("MeshShaderTier=" + ToString(d3d12options7.MeshShaderTier));
        info.features.PushBack("SamplerFeedbackTier=" + ToString(d3d12options7.SamplerFeedbackTier));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS8 d3d12options8;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS8, &d3d12options8, sizeof(d3d12options8));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("UnalignedBlockTexturesSupported=" + ToString(d3d12options8.UnalignedBlockTexturesSupported));
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS9 d3d12options9;
    hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &d3d12options9, sizeof(d3d12options9));
    if (SUCCEEDED(hr))
    {
        info.features.PushBack("MeshShaderPipelineStatsSupported=" + ToString(d3d12options9.MeshShaderPipelineStatsSupported));
        info.features.PushBack("MeshShaderSupportsFullRangeRenderTargetArrayIndex=" + ToString(d3d12options9.MeshShaderSupportsFullRangeRenderTargetArrayIndex));
        info.features.PushBack("AtomicInt64OnTypedResourceSupported=" + ToString(d3d12options9.AtomicInt64OnTypedResourceSupported));
        info.features.PushBack("AtomicInt64OnGroupSharedSupported=" + ToString(d3d12options9.AtomicInt64OnGroupSharedSupported));
        info.features.PushBack("DerivativesInMeshAndAmplificationShadersSupported=" + ToString(d3d12options9.DerivativesInMeshAndAmplificationShadersSupported));
        info.features.PushBack("WaveMMATier=" + ToString(d3d12options9.WaveMMATier));
    }

    return true;
}

DynArray<MonitorInfo> Device::GetMonitorsInfo() const
{
    DynArray<MonitorInfo> monitors;

    for (uint32 i = 0;; i++)
    {
        IDXGIOutput* pOutput = nullptr;
        if (mAdapter->EnumOutputs(i, &pOutput) == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

        if (!pOutput)
        {
            break;
        }

        DXGI_OUTPUT_DESC outputDesc;
        D3D_CALL_CHECK(pOutput->GetDesc(&outputDesc));
        pOutput->Release();

        MonitorInfo monitorInfo;
        if (!Common::UTF16ToUTF8(Utf16String(outputDesc.DeviceName), monitorInfo.name))
        {
            monitorInfo.name = "<unknown>";
        }

        monitorInfo.x = outputDesc.DesktopCoordinates.left;
        monitorInfo.y = outputDesc.DesktopCoordinates.top;
        monitorInfo.width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
        monitorInfo.height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
        monitorInfo.valid = outputDesc.AttachedToDesktop != 0;

        MONITORINFOEX mi = {};
        mi.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfoA(outputDesc.Monitor, &mi);


        monitors.PushBack(monitorInfo);
    }

    return monitors;
}

bool Device::DetectMonitors()
{
    const DynArray<MonitorInfo> monitors = GetMonitorsInfo();

    if (monitors.Empty())
    {
        NFE_LOG_WARNING("No monitors detected");
        return false;
    }

    for (uint32 i = 0; i < monitors.Size(); ++i)
    {
        const MonitorInfo& monitor = monitors[i];

        NFE_LOG_INFO("Monitor #%u detected", i);
        NFE_LOG_INFO("    Name: %s", monitor.name.Str());
        NFE_LOG_INFO("    Dimensions: %ux%u", monitor.width, monitor.height);
        NFE_LOG_INFO("    Offset: %d,%d", monitor.x, monitor.y);
        NFE_LOG_INFO("    Valid: %d", monitor.valid);
    }

    return true;
}

bool Device::IsBackbufferFormatSupported(Format format)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatData;
    formatData.Format = TranslateFormat(format);

    HRESULT hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatData, sizeof(formatData));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to check format support info");
        return false;
    }

    return (formatData.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY) == D3D12_FORMAT_SUPPORT1_DISPLAY;
}

bool Device::CalculateTexturePlacementInfo(Format format, uint32 width, uint32 height, uint32 depth, TexturePlacementInfo& outInfo) const
{
    if (width == 0 || height == 0 || depth == 0)
    {
        return false;
    }

    // TODO handle BC texture formats

    const uint32 bytesInRow = width * GetFormatSize(format);

    outInfo.alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    outInfo.rowPitch = Math::RoundUp<uint32>(bytesInRow, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    outInfo.totalSize = outInfo.rowPitch * height * depth;

    return true;
}

CommandRecorderPtr Device::CreateCommandRecorder()
{
    return MakeSharedPtr<CommandRecorder>();
}

uint32 Device::ReleaseUnusedCommandQueues()
{
    NFE_SCOPED_LOCK(mCommandQueuesLock);
    for (uint32 i = 0; i < mCommandQueues.Size(); ++i)
    {
        if (!mCommandQueues[i].Lock())
        {
            mCommandQueues[i] = std::move(mCommandQueues.Back());
            mCommandQueues.PopBack();
            i--;
        }
    }
    return mCommandQueues.Size();
}

bool Device::FinishFrame()
{
    return true;
}

} // namespace Renderer
} // namespace NFE
