/**
 * @file
 * @author  Witek902
 * @brief   Per-shader-stage descriptor set manager
 */

#include "PCH.hpp"
#include "DescriptorSet.hpp"
#include "RendererD3D12.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"


namespace NFE {
namespace Renderer {

DescriptorSet::DescriptorSet(ShaderType shaderStage)
    : mShaderStage(shaderStage)
{
    Reset();
}

void DescriptorSet::Reset()
{
    mDirtyCBVs = 0; // 0xFF;
    mDirtySRVs = 0; // 0xFFFFFFFF;
    mDirtyUAVs = 0; // SupportsUAV() ? 0xFFFF : 0;
    mDirtySamplers = 0; // 0xFFFF;

    std::fill(mDescriptorsCBV, mDescriptorsCBV + NFE_MAX_CBUFFER_SLOTS, UINT32_MAX);
    std::fill(mDescriptorsSRV, mDescriptorsSRV + NFE_MAX_SHADER_RESOURCE_SLOTS, UINT32_MAX);
    std::fill(mDescriptorsUAV, mDescriptorsUAV + NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS, UINT32_MAX);
    std::fill(mDescriptorsSampler, mDescriptorsSampler + NFE_MAX_SAMPLER_SLOTS, UINT32_MAX);
}

void DescriptorSet::SetConstantBuffer(uint32 slot, const BufferPtr& buffer)
{
    Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");
    NFE_ASSERT(slot < NFE_MAX_CBUFFER_SLOTS);

    const DescriptorID descriptor = bufferPtr->GetDescriptor(DescriptorType::CBV, {});
    if (mDescriptorsCBV[slot] != descriptor)
    {
        mDescriptorsCBV[slot] = descriptor;
        mDirtyCBVs |= (1 << slot);
    }

    // TODO null descriptor
}

void DescriptorSet::SetTexture(uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    Texture* texturePtr = static_cast<Texture*>(texture.Get());
    NFE_ASSERT(texturePtr, "Invalid texture");
    NFE_ASSERT(slot < NFE_MAX_SHADER_RESOURCE_SLOTS);

    const DescriptorID descriptor = texturePtr->GetDescriptor(DescriptorType::SRV, view);
    if (mDescriptorsSRV[slot] != descriptor)
    {
        mDescriptorsSRV[slot] = descriptor;
        mDirtySRVs |= (1 << slot);
    }

    // TODO null descriptor
}

void DescriptorSet::SetWritableTexture(uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    NFE_ASSERT(SupportsUAV(), "Shader stage does not support UAVs");

    Texture* texturePtr = static_cast<Texture*>(texture.Get());
    NFE_ASSERT(texturePtr, "Invalid texture");
    NFE_ASSERT(slot < NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS);

    const DescriptorID descriptor = texturePtr->GetDescriptor(DescriptorType::UAV, view);
    if (mDescriptorsUAV[slot] != descriptor)
    {
        mDescriptorsUAV[slot] = descriptor;
        mDirtyUAVs |= (1 << slot);
    }

    // TODO null descriptor
}

void DescriptorSet::SetBuffer(uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");
    NFE_ASSERT(slot < NFE_MAX_SHADER_RESOURCE_SLOTS);

    const DescriptorID descriptor = bufferPtr->GetDescriptor(DescriptorType::SRV, view);
    if (mDescriptorsSRV[slot] != descriptor)
    {
        mDescriptorsSRV[slot] = descriptor;
        mDirtySRVs |= (1 << slot);
    }

    // TODO null descriptor
}

void DescriptorSet::SetWritableBuffer(uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");
    NFE_ASSERT(slot < NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS);

    const DescriptorID descriptor = bufferPtr->GetDescriptor(DescriptorType::UAV, view);
    if (mDescriptorsUAV[slot] != descriptor)
    {
        mDescriptorsUAV[slot] = descriptor;
        mDirtyUAVs |= (1 << slot);
    }

    // TODO null descriptor
}

void  DescriptorSet::SetSampler(uint32 slot, const SamplerPtr& sampler)
{
    Sampler* samplerPtr = static_cast<Sampler*>(sampler.Get());
    NFE_ASSERT(samplerPtr, "Invalid sampler");
    NFE_ASSERT(slot < NFE_MAX_SAMPLER_SLOTS);

    const DescriptorID descriptor = samplerPtr->GetDescriptor();
    if (mDescriptorsSampler[slot] != descriptor)
    {
        mDescriptorsSampler[slot] = descriptor;
        mDirtySamplers |= (1 << slot);
    }
}

void DescriptorSet::ApplyChanges(
    ID3D12GraphicsCommandList* commandList,
    ReferencedDescriptorsRanges& outReferencedCbvSrvUavDescriptorRanges,
    ReferencedDescriptorsRanges& outReferencedSamplerDescriptorRanges)
{
    if (mDirtyCBVs)
    {
        UpdateDirtyCBVs(commandList, outReferencedCbvSrvUavDescriptorRanges);
    }

    if (mDirtySRVs)
    {
        UpdateDirtySRVs(commandList, outReferencedCbvSrvUavDescriptorRanges);
    }

    if (mDirtyUAVs)
    {
        UpdateDirtyUAVs(commandList, outReferencedCbvSrvUavDescriptorRanges);
    }

    if (mDirtySamplers)
    {
        UpdateDirtySamplers(commandList, outReferencedSamplerDescriptorRanges);
    }
}

void DescriptorSet::UpdateDirtyCBVs(
    ID3D12GraphicsCommandList* commandList,
    ReferencedDescriptorsRanges& outReferencedDescriptorRanges)
{
    // TODO instead of using generic allocator, maybe switch to ring buffer for temporary descriptor tables
    HeapAllocator& heapAllocator = gDevice->GetCbvSrvUavHeapAllocator();
    const HeapAllocator& stagingHeapAllocator = gDevice->GetCbvSrvUavHeapStagingAllocator();

    // TODO instead of always allocating full range, check what range is required by the shader
    const uint32 numDescriptors = NFE_MAX_CBUFFER_SLOTS;
    DescriptorID baseDescriptor = heapAllocator.Allocate(numDescriptors);
    NFE_ASSERT(baseDescriptor != UINT32_MAX, "Descriptor table allocation failed");

    // TODO use CopyDescriptors
    for (uint32_t i = 0; i < NFE_MAX_CBUFFER_SLOTS; ++i)
    {
        if (mDescriptorsCBV[i] != UINT32_MAX)
        {
            const D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = stagingHeapAllocator.GetCpuHandle(mDescriptorsCBV[i]);
            const D3D12_CPU_DESCRIPTOR_HANDLE destHandle = heapAllocator.GetCpuHandle(baseDescriptor + i);
            gDevice->GetDevice()->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        else
        {
            // TODO null descriptor
        }
    }

    // this must match Device::CreateGraphicsRootSignature
    uint32 rootParameterIndex = 0;
    switch (mShaderStage)
    {
    case ShaderType::Vertex:    rootParameterIndex = 0; break; // graphics root signature
    case ShaderType::Pixel:     rootParameterIndex = 2; break; // graphics root signature
    case ShaderType::Compute:   rootParameterIndex = 0; break; // compute root signature
    default:                    NFE_FATAL("Invalid shader stage"); break;
    }

    // update descriptor table
    if (mShaderStage == ShaderType::Compute)
    {
        commandList->SetComputeRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }
    else
    {
        commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }

    outReferencedDescriptorRanges.PushBack({ baseDescriptor, numDescriptors });

    mDirtyCBVs = 0;
}

void DescriptorSet::UpdateDirtySRVs(
    ID3D12GraphicsCommandList* commandList,
    ReferencedDescriptorsRanges& outReferencedDescriptorRanges)
{
    // TODO instead of using generic allocator, maybe switch to ring buffer for temporary descriptor tables
    HeapAllocator& heapAllocator = gDevice->GetCbvSrvUavHeapAllocator();
    const HeapAllocator& stagingHeapAllocator = gDevice->GetCbvSrvUavHeapStagingAllocator();

    // TODO instead of always allocating full range, check what range is required by the shader
    const uint32 numDescriptors = NFE_MAX_SHADER_RESOURCE_SLOTS;
    DescriptorID baseDescriptor = heapAllocator.Allocate(numDescriptors);
    NFE_ASSERT(baseDescriptor != UINT32_MAX, "Descriptor table allocation failed");

    // TODO use CopyDescriptors
    for (uint32_t i = 0; i < NFE_MAX_SHADER_RESOURCE_SLOTS; ++i)
    {
        if (mDescriptorsSRV[i] != UINT32_MAX)
        {
            const D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = stagingHeapAllocator.GetCpuHandle(mDescriptorsSRV[i]);
            const D3D12_CPU_DESCRIPTOR_HANDLE destHandle = heapAllocator.GetCpuHandle(baseDescriptor + i);
            gDevice->GetDevice()->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        else
        {
            // TODO null descriptor
        }
    }

    // this must match Device::CreateGraphicsRootSignature
    uint32 rootParameterIndex = 0;
    switch (mShaderStage)
    {
    case ShaderType::Vertex:    rootParameterIndex = 1; break; // graphics root signature
    case ShaderType::Pixel:     rootParameterIndex = 3; break; // graphics root signature
    case ShaderType::Compute:   rootParameterIndex = 1; break; // compute root signature
    default:                    NFE_FATAL("Invalid shader stage"); break;
    }

    // update descriptor table
    if (mShaderStage == ShaderType::Compute)
    {
        commandList->SetComputeRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }
    else
    {
        commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }

    outReferencedDescriptorRanges.PushBack({ baseDescriptor, numDescriptors });

    mDirtySRVs = 0;
}

void DescriptorSet::UpdateDirtyUAVs(
    ID3D12GraphicsCommandList* commandList,
    ReferencedDescriptorsRanges& outReferencedDescriptorRanges)
{
    // TODO instead of using generic allocator, maybe switch to ring buffer for temporary descriptor tables
    HeapAllocator& heapAllocator = gDevice->GetCbvSrvUavHeapAllocator();
    const HeapAllocator& stagingHeapAllocator = gDevice->GetCbvSrvUavHeapStagingAllocator();

    // TODO instead of always allocating full range, check what range is required by the shader
    const uint32 numDescriptors = NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS;
    DescriptorID baseDescriptor = heapAllocator.Allocate(numDescriptors);
    NFE_ASSERT(baseDescriptor != UINT32_MAX, "Descriptor table allocation failed");

    // TODO use CopyDescriptors
    for (uint32_t i = 0; i < NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS; ++i)
    {
        if (mDescriptorsUAV[i] != UINT32_MAX)
        {
            const D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = stagingHeapAllocator.GetCpuHandle(mDescriptorsUAV[i]);
            const D3D12_CPU_DESCRIPTOR_HANDLE destHandle = heapAllocator.GetCpuHandle(baseDescriptor + i);
            gDevice->GetDevice()->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        else
        {
            // TODO null descriptor
        }
    }

    // this must match Device::CreateGraphicsRootSignature
    uint32 rootParameterIndex = 0;
    switch (mShaderStage)
    {
    case ShaderType::Pixel:     rootParameterIndex = 4; break; // graphics root signature
    case ShaderType::Compute:   rootParameterIndex = 2; break; // compute root signature
    default:                    NFE_FATAL("Invalid shader stage"); break;
    }

    // update descriptor table
    if (mShaderStage == ShaderType::Compute)
    {
        commandList->SetComputeRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }
    else
    {
        commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }

    outReferencedDescriptorRanges.PushBack({ baseDescriptor, numDescriptors });

    mDirtyUAVs = 0;
}

void DescriptorSet::UpdateDirtySamplers(
    ID3D12GraphicsCommandList* commandList,
    ReferencedDescriptorsRanges& outReferencedDescriptorRanges)
{
    // TODO instead of using generic allocator, maybe switch to ring buffer for temporary descriptor tables
    HeapAllocator& heapAllocator = gDevice->GetSamplerHeapAllocator();
    const HeapAllocator& stagingHeapAllocator = gDevice->GetSamplerHeapStagingAllocator();

    // TODO instead of always allocating full range, check what range is required by the shader
    const uint32 numDescriptors = NFE_MAX_SAMPLER_SLOTS;
    DescriptorID baseDescriptor = heapAllocator.Allocate(numDescriptors);
    NFE_ASSERT(baseDescriptor != UINT32_MAX, "Descriptor table allocation failed");

    // TODO use CopyDescriptors
    for (uint32_t i = 0; i < NFE_MAX_SAMPLER_SLOTS; ++i)
    {
        if (mDescriptorsSampler[i] != UINT32_MAX)
        {
            const D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = stagingHeapAllocator.GetCpuHandle(mDescriptorsSampler[i]);
            const D3D12_CPU_DESCRIPTOR_HANDLE destHandle = heapAllocator.GetCpuHandle(baseDescriptor + i);
            gDevice->GetDevice()->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        }
        else
        {
            // TODO null descriptor
        }
    }

    // this must match Device::CreateGraphicsRootSignature
    uint32 rootParameterIndex = 0;
    switch (mShaderStage)
    {
    case ShaderType::Vertex:    rootParameterIndex = 5; break; // graphics root signature
    case ShaderType::Pixel:     rootParameterIndex = 6; break; // graphics root signature
    case ShaderType::Compute:   rootParameterIndex = 3; break; // compute root signature
    default:                    NFE_FATAL("Invalid shader stage"); break;
    }

    // update descriptor table
    if (mShaderStage == ShaderType::Compute)
    {
        commandList->SetComputeRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }
    else
    {
        commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, heapAllocator.GetGpuHandle(baseDescriptor));
    }

    outReferencedDescriptorRanges.PushBack({ baseDescriptor, numDescriptors });

    mDirtySamplers = 0;
}

} // namespace Renderer
} // namespace NFE
