/**
 * @file
 * @author  Witek902
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's memory block.
 */

#include "PCH.hpp"
#include "MemoryBlock.hpp"


namespace NFE {
namespace Renderer {

MemoryBlock::MemoryBlock()
    : mMemory(VK_NULL_HANDLE)
    , mSize(0)
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

    return true;
}

} // namespace Renderer
} // namespace NFE
