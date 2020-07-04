#pragma once

#include <Engine/Common/nfCommon.hpp>
#include <Engine/Common/System/Assertion.hpp>


namespace NFE {
namespace Renderer {

template<size_t MemorySize>
class LocalAllocator
{
    size_t mUsed;
    uint8_t mMemory[MemorySize];

public:
    LocalAllocator()
        : mUsed(0)
    {
    }

    ~LocalAllocator() = default;

    LocalAllocator(const LocalAllocator&) = delete;
    LocalAllocator(LocalAllocator&&) = delete;
    LocalAllocator operator=(const LocalAllocator&) = delete;
    LocalAllocator& operator=(LocalAllocator&&) = delete;

    /**
     * Allocate data of size @p size and return pointer to it.
     *
     * @p[in] size Size of memory to allocate on stack in bytes.
     *
     * Exceeding available memory size is provided only via an assertion, so it is compiled out on
     * Release configuration. It is application's duty to ensure allocations are not going to
     * perform out of bounds writes or reads on returned memory chunk.
     *
     * For simplicity and performance reasons it is impossible to free allocated pointers. Used
     * memory can be reclaimed via @ref Clear() call.
     */
    NFE_INLINE void* Allocate(size_t size)
    {
        NFE_ASSERT(size <= (MemorySize - mUsed), "Not enough space");
        void* ptr = &mMemory[mUsed];
        mUsed += size;
        return ptr;
    }

    /**
     * Clear the allocator, reclaiming all allocated memory for reuse.
     *
     * In fact, this call just zeroes the "used memory" variable. The entire memory block remains
     * intact, however it can now be overwritten with new allocations. It is best to assume, that
     * after this call all pointers acquired from @ref Allocate() are invalid.
     */
    NFE_INLINE void Clear()
    {
        mUsed = 0;
    }

    /**
     * Returns space consumed by objects on Allocator. For debugging/reporting purposes.
     */
    NFE_INLINE size_t GetUsedSpace() const
    {
        return mUsed;
    }
};

using CommandAllocator = LocalAllocator<NFE_VK_COMMAND_MEMORY_SPACE>;

} // namespace Renderer
} // namespace NFE
