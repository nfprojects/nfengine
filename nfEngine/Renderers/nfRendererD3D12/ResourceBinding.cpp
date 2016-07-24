/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "Translations.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    if (desc.numBindings == 0)
    {
        LOG_ERROR("Binding set can not be empty");
        return false;
    }

    if (desc.shaderVisibility != ShaderType::Vertex &&
        desc.shaderVisibility != ShaderType::Hull &&
        desc.shaderVisibility != ShaderType::Domain &&
        desc.shaderVisibility != ShaderType::Geometry &&
        desc.shaderVisibility != ShaderType::Pixel &&
        desc.shaderVisibility != ShaderType::All)
    {
        LOG_ERROR("Invalid shader visibility");
        return false;
    }


    mBindings.reserve(desc.numBindings);

    for (size_t i = 0; i < desc.numBindings; ++i)
    {
        const ResourceBindingDesc& bindingDesc = desc.resourceBindings[i];

        if (bindingDesc.resourceType != ShaderResourceType::CBuffer &&
            bindingDesc.resourceType != ShaderResourceType::Texture)
        {
            LOG_ERROR("Invalid shader resource type at binding %i", i);
            return false;
        }

        mBindings.push_back(desc.resourceBindings[i]);
    }

    mShaderVisibility = desc.shaderVisibility;
    return true;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    const size_t maxSets = 16;
    const size_t maxBindings = 64;

    D3D12_STATIC_SAMPLER_DESC staticSamplers[maxBindings];
    D3D12_DESCRIPTOR_RANGE descriptorRanges[maxBindings];
    D3D12_ROOT_PARAMETER rootParameters[maxSets];

    D3D12_ROOT_SIGNATURE_DESC rsd;
    rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rsd.pParameters = rootParameters;

    mBindingSets.reserve(desc.numBindingSets);

    uint32 rootParamIndex = 0;
    size_t samplerCounter = 0;
    size_t rangeCounter = 0;
    for (size_t i = 0; i < desc.numBindingSets; ++i, ++rootParamIndex)
    {
        ResourceBindingSet* bindingSet = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i]);
        if (bindingSet == nullptr)
        {
            LOG_ERROR("Invalid binding set");
            return false;
        }
        mBindingSets.push_back(bindingSet);

        // set up root signature's parameter
        rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // TODO temporary
        rootParameters[rootParamIndex].DescriptorTable.NumDescriptorRanges =
            static_cast<UINT>(bindingSet->mBindings.size());
        rootParameters[rootParamIndex].DescriptorTable.pDescriptorRanges = &descriptorRanges[rangeCounter];

        if (!TranslateShaderVisibility(bindingSet->mShaderVisibility, rootParameters[rootParamIndex].ShaderVisibility))
        {
            LOG_ERROR("Invalid shader visibility");
            return false;
        }

        // iterate through descriptors within the set
        for (size_t j = 0; j < bindingSet->mBindings.size(); ++j)
        {
            if (rangeCounter >= maxBindings)
            {
                LOG_ERROR("Max supported number of bindings exceeded");
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
                rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                break;
            default:
                LOG_ERROR("Invalid shader resource type");
                return false;
            }
            descriptorRanges[rangeCounter].RangeType = rangeType;
            descriptorRanges[rangeCounter].NumDescriptors = 1;  // TODO ranges
            descriptorRanges[rangeCounter].BaseShaderRegister = bindingDesc.slot & SHADER_RES_SLOT_MASK;
            descriptorRanges[rangeCounter].RegisterSpace = 0;
            descriptorRanges[rangeCounter].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            rangeCounter++;

            // fill static samplers
            if (bindingDesc.resourceType == ShaderResourceType::Texture &&
                bindingDesc.staticSampler != nullptr)
            {
                Sampler* sampler = dynamic_cast<Sampler*>(bindingDesc.staticSampler);
                if (!sampler)
                {
                    LOG_ERROR("Invalid static sampler in binding set %zu at slot %zu", i, j);
                    return false;
                }

                D3D12_STATIC_SAMPLER_DESC& targetSampler = staticSamplers[samplerCounter++];
                sampler->FillD3DStaticSampler(targetSampler);
                targetSampler.ShaderRegister = bindingDesc.slot & SHADER_RES_SLOT_MASK;
                targetSampler.RegisterSpace = 0;
                targetSampler.ShaderVisibility = rootParameters[rootParamIndex].ShaderVisibility;
            }
        }
    }

    // initialize root parameters (root descriptors) for dynamic buffers bindings
    for (size_t i = 0; i < desc.numDynamicBuffers; ++i, ++rootParamIndex)
    {
        const DynamicBufferBindingDesc& bindingDesc = desc.dynamicBuffers[i];
        mDynamicBuffers.push_back(bindingDesc);

        // set up root signature's parameter
        switch (desc.dynamicBuffers[i].resourceType)
        {
        case ShaderResourceType::CBuffer:
            rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            break;
        // TODO: UAVs, raw buffers, etc.
        default:
            LOG_ERROR("Unsupported shader resource type in dynamic buffer slot %zu", i);
            return false;
        }

        // TODO check if shader slots are not overlapping
        rootParameters[rootParamIndex].Descriptor.RegisterSpace = 0;
        rootParameters[rootParamIndex].Descriptor.ShaderRegister = bindingDesc.slot & SHADER_RES_SLOT_MASK;
        if (!TranslateShaderVisibility(bindingDesc.shaderVisibility, rootParameters[rootParamIndex].ShaderVisibility))
        {
            LOG_ERROR("Invalid shader visibility");
            return false;
        }
    }

    rsd.NumParameters = rootParamIndex;
    rsd.NumStaticSamplers = static_cast<UINT>(samplerCounter);
    rsd.pStaticSamplers = staticSamplers;

    HRESULT hr;
    D3DPtr<ID3D10Blob> rootSignature, errorsBuffer;
    hr = D3D_CALL_CHECK(D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1,
                                                    &rootSignature, &errorsBuffer));
    if (FAILED(hr))
        return false;

    LOG_DEBUG("Root signature blob size: %u bytes", rootSignature->GetBufferSize());

    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateRootSignature(
        0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(),
        IID_PPV_ARGS(&mRootSignature)));

    return true;
}

ResourceBindingInstance::~ResourceBindingInstance()
{
    if (mSet)
    {
        HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
        allocator.Free(mDescriptorHeapOffset, static_cast<uint32>(mSet->mBindings.size()));
    }
}

bool ResourceBindingInstance::Init(IResourceBindingSet* bindingSet)
{
    mSet = dynamic_cast<ResourceBindingSet*>(bindingSet);
    if (!mSet)
    {
        LOG_ERROR("Invalid resource binding set");
        return false;
    }

    // TODO ranges support
    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    mDescriptorHeapOffset = allocator.Allocate(static_cast<uint32>(mSet->mBindings.size()));
    return mDescriptorHeapOffset != -1;
}

bool ResourceBindingInstance::WriteTextureView(size_t slot, ITexture* texture)
{
    // TODO this won't work if there are multiple buffers (frames) in the texture

    Texture* tex = dynamic_cast<Texture*>(texture);
    if (!tex || !tex->mBuffers[0])
    {
        LOG_ERROR("Invalid buffer");
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = tex->mSrvFormat;
    switch (tex->mType)
    {
    case TextureType::Texture1D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        srvDesc.Texture1D.MipLevels = tex->mMipmapsNum;
        srvDesc.Texture1D.MostDetailedMip = 0;
        srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        break;
    case TextureType::Texture2D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = tex->mMipmapsNum;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Texture2D.PlaneSlice = 0;
        break;
    case TextureType::TextureCube:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = tex->mMipmapsNum;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        break;
    case TextureType::Texture3D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = tex->mMipmapsNum;
        srvDesc.Texture3D.MostDetailedMip = 0;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
        break;
    // TODO multisampled and multilayered textures
    }

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
    handle.ptr += allocator.GetDescriptorSize() * (mDescriptorHeapOffset + slot);
    gDevice->GetDevice()->CreateShaderResourceView(tex->mBuffers[0].get(), &srvDesc, handle);

    return true;
}

bool ResourceBindingInstance::WriteCBufferView(size_t slot, IBuffer* buffer)
{
    UNUSED(slot);
    UNUSED(buffer);

    Buffer* cbuffer = dynamic_cast<Buffer*>(buffer);
    if (!buffer || !cbuffer->mResource)
    {
        LOG_ERROR("Invalid buffer");
        return false;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbuffer->mResource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(cbuffer->mSize);

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_CPU_DESCRIPTOR_HANDLE target = allocator.GetCpuHandle();
    target.ptr += allocator.GetDescriptorSize() * (mDescriptorHeapOffset + slot);
    gDevice->GetDevice()->CreateConstantBufferView(&cbvDesc, target);

    return true;
}

} // namespace Renderer
} // namespace NFE
