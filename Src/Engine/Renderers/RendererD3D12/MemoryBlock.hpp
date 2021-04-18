/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Direct3D 12 renderer's memory block.
 */

#pragma once

#include "../RendererCommon/MemoryBlock.hpp"
#include "D3D12MemAlloc.h"

#include "Common.hpp"


namespace NFE {
namespace Renderer {

class MemoryBlock : public IMemoryBlock
{
public:
    MemoryBlock();
    ~MemoryBlock();

    bool Init(const MemoryBlockDesc& desc);

    NFE_FORCE_INLINE uint64 GetSize() const
    {
        return mSize;
    }

    NFE_FORCE_INLINE uint32 GetAlignment() const
    {
        return mAlignment;
    }

    NFE_FORCE_INLINE D3D12MA::Allocation* GetAllocation() const
    {
        return mAllocation.Get();
    }

private:
    uint64 mSize;
    uint32 mAlignment;
    D3DPtr<D3D12MA::Allocation> mAllocation;
};

} // namespace Renderer
} // namespace NFE
