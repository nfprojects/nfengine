/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator definition.
 */

#include "PCH.hpp"
#include "DefaultAllocator.hpp"
#include "Logger/Logger.hpp"
#include "Math/Math.hpp"


// TODO
// 1. Allocated size tracking in Release mode
// 2. Usage of custom memory allocators (depending on allocation size).

namespace NFE {
namespace Common {


DefaultAllocator::DefaultAllocator()
    : mAllocationsNum(0)
    , mBytesAllocated(0)
    , mInitialized(true)
{
}

DefaultAllocator::~DefaultAllocator()
{
}

DefaultAllocator& DefaultAllocator::GetInstance()
{
    static DefaultAllocator allocator;
    return allocator;
}

void DefaultAllocator::Shutdown()
{
    ReportAllocations();

    mInitialized = false;
}

void* DefaultAllocator::Malloc(size_t size, size_t alignment, const char* sourceFile, int sourceLine)
{
    NFE_ASSERT(mInitialized, "Default memory allocator is not initialized");

    if (!mInitialized)
    {
        return nullptr;
    }

    if (size == 0)
    {
        NFE_LOG_ERROR("Allocated memory block must be greater than 0 bytes.");
        return nullptr;
    }

    if (!Math::IsPowerOfTwo(alignment))
    {
        NFE_LOG_ERROR("Invalid memory alignment: %zu. Must be power of two.", alignment);
        return nullptr;
    }

    void* ptr = nullptr;

#if defined(WIN32)
    ptr = _aligned_malloc_dbg(size, alignment, sourceFile, sourceLine);

#elif defined(__LINUX__) | defined(__linux__)
    alignment = std::max(alignment, sizeof(void*));
    int ret = posix_memalign(&ptr, alignment, size);
    if (ret != 0)
    {
        NFE_LOG_ERROR("posix_memalign() returned %i", ret);
        ptr = nullptr;
    }
#endif // defined(WIN32)

    if (ptr)
    {
        mAllocationsNum++;
    }
    else
    {
        NFE_LOG_ERROR("Memory allocation requested from %s:%i failed, size=%zu, alignment=%zu, errno=%i",
            sourceFile, sourceLine, size, alignment, errno);
    }

    return ptr;
}

void DefaultAllocator::Free(void* ptr)
{
    if (ptr == nullptr)
    {
        return;
    }

    mAllocationsNum--;

#if defined(WIN32)
    _aligned_free(ptr);

#elif defined(__LINUX__) | defined(__linux__)
    free(ptr);
#endif // defined(WIN32)
}

void DefaultAllocator::ReportAllocations()
{
    NFE_LOG_INFO("Allocated blocks: %zu", mAllocationsNum.load());
}

AllocatorStats DefaultAllocator::GetStats() const
{
    AllocatorStats stats;
    stats.allocationsNum = mAllocationsNum;
    stats.bytesAllocated = mBytesAllocated;
    return stats;
}


} // namespace Common
} // namespace NFE
