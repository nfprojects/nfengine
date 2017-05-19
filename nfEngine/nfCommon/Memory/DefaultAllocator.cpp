/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator definition.
 */

#include "PCH.hpp"
#include "DefaultAllocator.hpp"
#include "Logger/Logger.hpp"
#include "Math/Math.hpp"
#include "Utils/ScopedLock.hpp"


// TODO
// 1. Allocated size tracking in Release mode
// 2. Usage of custom memory allocators (depending on allocation size).

namespace NFE {
namespace Common {

DefaultAllocator::DefaultAllocator()
    : mAllocationsNum(0)
    , mBytesAllocated(0)
{
}

DefaultAllocator::~DefaultAllocator()
{
    ReportAllocations();
}

DefaultAllocator& DefaultAllocator::GetInstance()
{
    static DefaultAllocator allocator;
    return allocator;
}

void* DefaultAllocator::Malloc(size_t size, size_t alignment, const char* sourceFile, int sourceLine)
{
    if (size == 0)
    {
        LOG_ERROR("Allocated memory block must be greater than 0 bytes.");
        return nullptr;
    }

    if (!Math::PowerOfTwo(alignment))
    {
        LOG_ERROR("Invalid memory alignment: %zu. Must be power of two.", alignment);
        return nullptr;
    }

    void* ptr = nullptr;

#if defined(WIN32)
    ptr = _aligned_malloc(size, alignment);

#elif defined(__LINUX__) | defined(__linux__)
    alignment = std::max(alignment, sizeof(void*));
    int ret = posix_memalign(&ptr, alignment, size);
    if (ret != 0)
    {
        LOG_ERROR("posix_memalign() returned %i", ret);
        ptr = nullptr;
    }
#endif // defined(WIN32)

    if (ptr)
    {
        mAllocationsNum++;

#ifdef _DEBUG
        ScopedMutexLock lock(mMutex);
        AllocationDebugInfo info;
        info.size = size;
        info.sourceFile = sourceFile;
        info.sourceLine = sourceLine;
        mAllocationsDebugInfo[ptr] = info;
        mBytesAllocated += size;
#endif // _DEBUG
    }
    else
        LOG_ERROR("Memory allocation requested from %s:%i failed, size=%zu, alignment=%zu, errno=%i",
                  sourceFile, sourceLine, size, alignment, errno);

    return ptr;
}

void DefaultAllocator::Free(void* ptr)
{
    if (ptr == nullptr)
        return;

#ifdef _DEBUG
    {
        ScopedMutexLock lock(mMutex);
        const auto iter = mAllocationsDebugInfo.find(ptr);

        if (iter == mAllocationsDebugInfo.end())
            LOG_FATAL("Trying to free already freed memory block");

        mBytesAllocated -= iter->second.size;
        mAllocationsDebugInfo.erase(iter);
    }
#endif // _DEBUG

    mAllocationsNum--;

#if defined(WIN32)
    _aligned_free(ptr);

#elif defined(__LINUX__) | defined(__linux__)
    free(ptr);
#endif // defined(WIN32)
}

void DefaultAllocator::ReportAllocations()
{
#ifdef _DEBUG
    ScopedMutexLock lock(mMutex);

    LOG_INFO("Allocated blocks: %zu (%zu bytes)", mAllocationsNum.load(), mBytesAllocated.load());
    for (const auto& it : mAllocationsDebugInfo)
    {
        LOG_INFO("Allocated block (ptr=%p, size=%zu) at %s:%i", it.first, it.second.size,
                 it.second.sourceFile, it.second.sourceLine);
    }

#else
    LOG_INFO("Allocated blocks: %zu", mAllocationsNum.load());

#endif // _DEBUG
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
