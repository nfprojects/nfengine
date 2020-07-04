#pragma once

#include <Engine/Common/nfCommon.hpp>
#include "LocalAllocator.hpp"


namespace NFE {
namespace Renderer {

template <size_t maxSize>
class LocalAllocatorObject
{
protected:
    static void* operator new(size_t) noexcept { return nullptr; };
    static void operator delete(void*) noexcept {};

public:
    NFE_INLINE static void* operator new(size_t size, LocalAllocator<maxSize>& allocator) noexcept
    {
        return allocator.Allocate(size);
    }

    NFE_INLINE static void operator delete(void* p, LocalAllocator<maxSize>& allocator) noexcept
    {
        NFE_UNUSED(p);
        NFE_UNUSED(allocator);
    }
};

using CommandAllocatorObject = LocalAllocatorObject<NFE_VK_COMMAND_MEMORY_SPACE>;

} // namespace Renderer
} // namespace NFE
