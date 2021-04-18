/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Vulkan renderer's memory block.
 */

#pragma once

#include "../RendererCommon/MemoryBlock.hpp"

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

private:
    uint64 mSize;
    uint32 mAlignment;
};

} // namespace Renderer
} // namespace NFE
