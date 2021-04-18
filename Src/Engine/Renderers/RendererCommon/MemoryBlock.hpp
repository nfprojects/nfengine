/**
 * @file
 * @author  Witek902
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of GPU memory block
 */
struct MemoryBlockDesc
{
    uint64 size;
    uint32 alignment;

    MemoryBlockDesc(uint64 size = 0u)
        : size(size)
        , alignment(0u)
    {}
};

/**
 * GPU memory block.
 */
class IMemoryBlock
{
public:
    virtual ~IMemoryBlock() {}
};

} // namespace Renderer
} // namespace NFE
