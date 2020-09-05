/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "Translations.hpp"
#include "RendererD3D12.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Math/Math.hpp"


namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    if (desc.numBindings == 0)
    {
        NFE_LOG_ERROR("Binding set can not be empty");
        return false;
    }

    if (desc.shaderVisibility != ShaderType::Vertex &&
        desc.shaderVisibility != ShaderType::Hull &&
        desc.shaderVisibility != ShaderType::Domain &&
        desc.shaderVisibility != ShaderType::Geometry &&
        desc.shaderVisibility != ShaderType::Pixel &&
        desc.shaderVisibility != ShaderType::Compute &&
        desc.shaderVisibility != ShaderType::All)
    {
        NFE_LOG_ERROR("Invalid shader visibility");
        return false;
    }


    mBindings.Reserve(desc.numBindings);

    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        const ResourceBindingDesc& bindingDesc = desc.resourceBindings[i];

        if (bindingDesc.resourceType != ShaderResourceType::CBuffer &&
            bindingDesc.resourceType != ShaderResourceType::Texture &&
            bindingDesc.resourceType != ShaderResourceType::StructuredBuffer &&
            bindingDesc.resourceType != ShaderResourceType::WritableTexture &&
            bindingDesc.resourceType != ShaderResourceType::WritableStructuredBuffer)
        {
            NFE_LOG_ERROR("Invalid shader resource type at binding %i", i);
            return false;
        }

        mBindings.PushBack(desc.resourceBindings[i]);
    }

    mShaderVisibility = desc.shaderVisibility;
    return true;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    D3D12_STATIC_SAMPLER_DESC staticSamplers[NFE_RENDERER_MAX_RESOURCES_IN_SET];
    D3D12_DESCRIPTOR_RANGE descriptorRanges[NFE_RENDERER_MAX_RESOURCES_IN_SET];
    D3D12_ROOT_PARAMETER rootParameters[NFE_RENDERER_MAX_BINDING_SETS];

    D3D12_ROOT_SIGNATURE_DESC rsd;
    rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rsd.pParameters = rootParameters;

    if (desc.numBindingSets > NFE_RENDERER_MAX_BINDING_SETS)
    {
        NFE_LOG_ERROR("Too many sets in a binding layout");
        return false;
    }

    mBindingSets.Reserve(desc.numBindingSets);

    uint32 rootParamIndex = 0;
    uint32 samplerCounter = 0;
    uint32 rangeCounter = 0;
    for (uint32 i = 0; i < desc.numBindingSets; ++i, ++rootParamIndex)
    {
        InternalResourceBindingSetPtr bindingSet = Common::StaticCast<ResourceBindingSet>(desc.bindingSets[i]);
        if (!bindingSet)
        {
            NFE_LOG_ERROR("Invalid binding set");
            return false;
        }
        mBindingSets.PushBack(bindingSet);

        // set up root signature's parameter
        rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // TODO temporary
        rootParameters[rootParamIndex].DescriptorTable.NumDescriptorRanges = bindingSet->mBindings.Size();
        rootParameters[rootParamIndex].DescriptorTable.pDescriptorRanges = &descriptorRanges[rangeCounter];

        if (!TranslateShaderVisibility(bindingSet->mShaderVisibility, rootParameters[rootParamIndex].ShaderVisibility))
        {
            NFE_LOG_ERROR("Invalid shader visibility");
            return false;
        }

        // iterate through descriptors within the set
        for (uint32 j = 0; j < bindingSet->mBindings.Size(); ++j)
        {
            if (rangeCounter >= NFE_RENDERER_MAX_RESOURCES_IN_SET)
            {
                NFE_LOG_ERROR("Max supported number of bindings exceeded");
                return false;
            }

            const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[j];

            // set up root's signature descriptor table (range of descriptors)

            D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
            switch (bindingDesc.resourceType)
            {
            case ShaderResourceType::CBuffer:
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                break;
            case ShaderResourceType::Texture:
            case ShaderResourceType::StructuredBuffer:
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                break;
            case ShaderResourceType::WritableTexture:
            case ShaderResourceType::WritableStructuredBuffer:
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                break;
            default:
                NFE_LOG_ERROR("Invalid shader resource type");
                return false;
            }
            descriptorRanges[rangeCounter].RangeType = rangeType;
            descriptorRanges[rangeCounter].NumDescriptors = 1;  // TODO ranges
            descriptorRanges[rangeCounter].BaseShaderRegister = bindingDesc.slot;
            descriptorRanges[rangeCounter].RegisterSpace = 0;
            descriptorRanges[rangeCounter].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rangeCounter++;

            // fill static samplers
            if (bindingDesc.resourceType == ShaderResourceType::Texture && bindingDesc.staticSampler != nullptr)
            {
                Sampler* sampler = dynamic_cast<Sampler*>(bindingDesc.staticSampler.Get());
                if (!sampler)
                {
                    NFE_LOG_ERROR("Invalid static sampler in binding set %u at slot %u", i, j);
                    return false;
                }

                D3D12_STATIC_SAMPLER_DESC& targetSampler = staticSamplers[samplerCounter++];
                sampler->FillD3DStaticSampler(targetSampler);
                targetSampler.ShaderRegister = bindingDesc.slot;
                targetSampler.RegisterSpace = 0;
                targetSampler.ShaderVisibility = rootParameters[rootParamIndex].ShaderVisibility;
            }
        }
    }

    // initialize root parameters (root descriptors) for dynamic buffers bindings
    for (uint32 i = 0; i < desc.numVolatileCBuffers; ++i, ++rootParamIndex)
    {
        const VolatileCBufferBinding& bindingDesc = desc.volatileCBuffers[i];
        mDynamicBuffers.PushBack(bindingDesc);

        // set up root signature's parameter
        switch (desc.volatileCBuffers[i].resourceType)
        {
        case ShaderResourceType::CBuffer:
            rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            break;
        // TODO: UAVs, raw buffers, etc.
        default:
            NFE_LOG_ERROR("Unsupported shader resource type in dynamic buffer slot %u", i);
            return false;
        }

        // TODO check if shader slots are not overlapping
        rootParameters[rootParamIndex].Descriptor.RegisterSpace = 0;
        rootParameters[rootParamIndex].Descriptor.ShaderRegister = bindingDesc.slot;
        if (!TranslateShaderVisibility(bindingDesc.shaderVisibility, rootParameters[rootParamIndex].ShaderVisibility))
        {
            NFE_LOG_ERROR("Invalid shader visibility");
            return false;
        }
    }

    rsd.NumParameters = rootParamIndex;
    rsd.NumStaticSamplers = static_cast<UINT>(samplerCounter);
    rsd.pStaticSamplers = staticSamplers;

    HRESULT hr;
    D3DPtr<ID3D10Blob> rootSignature, errorsBuffer;
    hr = D3D_CALL_CHECK(D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1,
                                                    rootSignature.GetPtr(), errorsBuffer.GetPtr()));
    if (FAILED(hr))
        return false;

    NFE_LOG_DEBUG("Root signature blob size: %u bytes", rootSignature->GetBufferSize());

    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateRootSignature(
        0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetPtr())));

    if (desc.debugName && !SetDebugName(mRootSignature.Get(), Common::StringView(desc.debugName)))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }

    return true;
}

ResourceBindingInstance::ResourceBindingInstance()
    : mCpuDescriptorHeapOffset(UINT32_MAX)
    , mGpuDescriptorHeapOffset(UINT32_MAX)
    , mIsFinalized(false)
{ }

ResourceBindingInstance::~ResourceBindingInstance()
{
    if (mCpuDescriptorHeapOffset != UINT32_MAX)
    {
        gDevice->GetCbvSrvUavHeapStagingAllocator().Free({ mCpuDescriptorHeapOffset, mResources.Size() });
    }

    if (mGpuDescriptorHeapOffset != UINT32_MAX)
    {
        gDevice->GetCbvSrvUavHeapAllocator().Free({ mGpuDescriptorHeapOffset, mResources.Size() });
    }
}

bool ResourceBindingInstance::Init(const ResourceBindingSetPtr& bindingSet)
{
    const InternalResourceBindingSetPtr& set = Common::StaticCast<ResourceBindingSet>(bindingSet);
    if (!set)
    {
        NFE_LOG_ERROR("Invalid resource binding set");
        return false;
    }

    mResources.Resize(set->mBindings.Size());

    mCpuDescriptorHeapOffset = gDevice->GetCbvSrvUavHeapStagingAllocator().Allocate(set->mBindings.Size()).offset;
    mGpuDescriptorHeapOffset = gDevice->GetCbvSrvUavHeapAllocator().Allocate(set->mBindings.Size()).offset;

    return mCpuDescriptorHeapOffset != UINT32_MAX && mGpuDescriptorHeapOffset != UINT32_MAX;
}

bool ResourceBindingInstance::SetTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    NFE_ASSERT(!mIsFinalized, "Binding instance is finalized");

    Texture* tex = static_cast<Texture*>(texture.Get());
    if (!tex || !tex->mResource)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    mResources[slot].texture = texture;

    NFE_ASSERT(view.baseMip < tex->GetMipmapsNum(), "Invalid base mip: texture has %u mips, %u specified", tex->GetMipmapsNum(), view.baseMip);

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * static_cast<size_t>(mCpuDescriptorHeapOffset + slot);
    CreateTextureSRV(tex, view, handle);

    return true;
}

bool ResourceBindingInstance::SetBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    NFE_ASSERT(!mIsFinalized, "Binding instance is finalized");

    Buffer* buf = static_cast<Buffer*>(buffer.Get());
    if (!buf || !buf->mResource)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    mResources[slot].buffer = buffer;

    // TODO validate buffer view

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * static_cast<size_t>(mCpuDescriptorHeapOffset + slot);
    CreateBufferSRV(buf, view, handle);

    return true;
}

bool ResourceBindingInstance::SetCBufferView(uint32 slot, const BufferPtr& buffer)
{
    NFE_ASSERT(!mIsFinalized, "Binding instance is finalized");

    Buffer* cbuffer = static_cast<Buffer*>(buffer.Get());
    if (!buffer || !cbuffer->GetResource())
    {
        NFE_LOG_ERROR("Invalid buffer");
        return false;
    }

    mResources[slot].buffer = buffer;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbuffer->GetResource()->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = cbuffer->GetRealSize();

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE target = allocator.GetCpuHandle();
    target.ptr += allocator.GetDescriptorSize() * static_cast<size_t>(mCpuDescriptorHeapOffset + slot);
    gDevice->GetDevice()->CreateConstantBufferView(&cbvDesc, target);

    return true;
}

bool ResourceBindingInstance::SetWritableTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    NFE_ASSERT(!mIsFinalized, "Binding instance is finalized");

    Texture* tex = static_cast<Texture*>(texture.Get());
    if (!tex || !tex->mResource)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    NFE_ASSERT(tex->GetMode() == BufferMode::GPUOnly, "Invalid texture mode");

    mResources[slot].texture = texture;

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * static_cast<size_t>(mCpuDescriptorHeapOffset + slot);
    CreateTextureUAV(tex, view, handle);

    return true;
}

bool ResourceBindingInstance::SetWritableBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    NFE_ASSERT(!mIsFinalized, "Binding instance is finalized");

    Buffer* buf = static_cast<Buffer*>(buffer.Get());
    if (!buf || !buf->mResource)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    NFE_ASSERT(buf->GetMode() == BufferMode::GPUOnly, "Invalid buffer mode");

    mResources[slot].buffer = buffer;

    // TODO validate buffer view

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * static_cast<size_t>(mCpuDescriptorHeapOffset + slot);
    CreateBufferUAV(buf, view, handle);

    return true;
}

bool ResourceBindingInstance::Finalize()
{
    NFE_ASSERT(!mIsFinalized, "Binding instance is already finalized");

    // copy descriptors from non-shader visible to shader visible heap

    HeapAllocator& stagingAllocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = stagingAllocator.GetCpuHandle();
    srcHandle.ptr += stagingAllocator.GetDescriptorSize() * static_cast<size_t>(mCpuDescriptorHeapOffset);

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE destHandle = allocator.GetCpuHandle();
    destHandle.ptr += allocator.GetDescriptorSize() * static_cast<size_t>(mGpuDescriptorHeapOffset);

    gDevice->GetDevice()->CopyDescriptorsSimple(mResources.Size(), destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    mIsFinalized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////

void CreateTextureSRV(const Texture* tex, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = tex->GetSrvFormat();

    if (tex->GetType() == TextureType::Texture1D)
    {
        if (tex->GetLayersNum() > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.MostDetailedMip = view.baseMip;
            srvDesc.Texture1DArray.FirstArraySlice = view.baseLayer;
            srvDesc.Texture1DArray.MipLevels = view.numMips;
            srvDesc.Texture1DArray.ArraySize = view.numLayers;
            srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MostDetailedMip = view.baseMip;
            srvDesc.Texture1D.MipLevels = view.numMips;
            srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        }
    }
    else if (tex->GetType() == TextureType::Texture2D)
    {
        if (tex->GetSamplesNum() > 1)
        {
            if (tex->GetLayersNum() > 1)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srvDesc.Texture2DMSArray.FirstArraySlice = view.baseLayer;
                srvDesc.Texture2DMSArray.ArraySize = view.numLayers;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
        }
        else
        {
            if (tex->GetLayersNum() > 1)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip = view.baseMip;
                srvDesc.Texture2DArray.FirstArraySlice = view.baseLayer;
                srvDesc.Texture2DArray.MipLevels = view.numMips;
                srvDesc.Texture2DArray.ArraySize = view.numLayers;
                srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = view.baseMip;
                srvDesc.Texture2D.MipLevels = view.numMips;
                srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                srvDesc.Texture2D.PlaneSlice = 0;
            }
        }
    }
    else if (tex->GetType() == TextureType::TextureCube)
    {
        // TODO cube texture array
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = view.baseMip;
        srvDesc.TextureCube.MipLevels = view.numMips;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    }
    else if (tex->GetType() == TextureType::Texture3D)
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MostDetailedMip = view.baseMip;
        srvDesc.Texture3D.MipLevels = view.numMips;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
    }
    else
    {
        NFE_FATAL("Invalid texture type");
        return;
    }

    gDevice->GetDevice()->CreateShaderResourceView(tex->GetD3DResource(), &srvDesc, descriptorHandle);
}

void CreateTextureUAV(const Texture* tex, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = tex->GetSrvFormat();

    if (tex->GetType() == TextureType::Texture1D)
    {
        if (tex->GetLayersNum() > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.FirstArraySlice = view.baseLayer;
            uavDesc.Texture1DArray.MipSlice = view.baseMip;
            uavDesc.Texture1DArray.ArraySize = view.numLayers;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            uavDesc.Texture1D.MipSlice = view.baseMip;
        }
    }
    else if (tex->GetType() == TextureType::Texture2D)
    {
        if (tex->GetLayersNum() > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.FirstArraySlice = view.baseLayer;
            uavDesc.Texture2DArray.MipSlice = view.baseMip;
            uavDesc.Texture2DArray.ArraySize = view.numLayers;
            uavDesc.Texture2DArray.PlaneSlice = 0;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = view.baseMip;
            uavDesc.Texture2D.PlaneSlice = 0;
        }
    }
    else if (tex->GetType() == TextureType::Texture3D)
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.FirstWSlice = view.baseLayer;
        uavDesc.Texture3D.MipSlice = view.baseMip;
        uavDesc.Texture3D.WSize = view.numLayers;
    }
    else
    {
        NFE_FATAL("Invalid texture type");
        return;
    }

    gDevice->GetDevice()->CreateUnorderedAccessView(tex->GetD3DResource(), nullptr, &uavDesc, descriptorHandle);
}

void CreateBufferSRV(const Buffer* buf, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.FirstElement = view.firstElement;
    srvDesc.Buffer.NumElements = view.numElements;
    srvDesc.Buffer.StructureByteStride = 0; // TODO

    gDevice->GetDevice()->CreateShaderResourceView(buf->GetD3DResource(), &srvDesc, descriptorHandle);
}

void CreateBufferUAV(const Buffer* buf, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.FirstElement = view.firstElement;
    uavDesc.Buffer.NumElements = view.numElements;
    uavDesc.Buffer.StructureByteStride = 0; // TODO

    gDevice->GetDevice()->CreateUnorderedAccessView(buf->GetD3DResource(), nullptr, &uavDesc, descriptorHandle);
}

} // namespace Renderer
} // namespace NFE
