/**
 * @file
 * @author  Witek902
 * @brief   D3D12 implementation of renderer's memory block.
 */

#include "PCH.hpp"
#include "MemoryBlock.hpp"
#include "RendererD3D12.hpp"


namespace NFE {
namespace Renderer {

MemoryBlock::MemoryBlock()
    : mSize(0)
    , mAlignment(0)
{
}

MemoryBlock::~MemoryBlock()
{
}

bool MemoryBlock::Init(const MemoryBlockDesc& desc)
{
    if (desc.size == 0)
    {
        NFE_LOG_ERROR("Cannot create zero-sized memory block");
        return false;
    }

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    allocationDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
    allocationDesc.CustomPool = nullptr;

    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = {};
    allocationInfo.SizeInBytes = desc.size;
    allocationInfo.Alignment = desc.alignment;

    HRESULT hr = D3D_CALL_CHECK(gDevice->GetAllocator()->AllocateMemory(
        &allocationDesc,
        &allocationInfo,
        mAllocation.GetPtr()));

    if (FAILED(hr))
    {
        return false;
    }

    mSize = desc.size;
    mAlignment = desc.alignment;

    return true;
}

} // namespace Renderer
} // namespace NFE
