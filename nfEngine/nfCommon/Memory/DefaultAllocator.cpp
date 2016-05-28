/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator definition.
 */

#include "../PCH.hpp"
#include "DefaultAllocator.hpp"
#include "../Logger.hpp"

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
    void* ptr;
#if defined(WIN32)
    ptr = _aligned_malloc(size, alignment);
#elif defined(__LINUX__) | defined(__linux__)
    if (posix_memalign(&ptr, alignment, size) != 0)
        ptr = nullptr;
#endif

    if (ptr)
    {
        mAllocationsNum++;
        // mBytesAllocated += size;

#ifdef _DEBUG
        std::unique_lock<std::mutex> lock(mMutex);
        AllocationDebugInfo info;
        info.size = size;
        info.sourceFile = sourceFile;
        info.sourceLine = sourceLine;
        mAllocationsDebugInfo[ptr] = info;
#endif
    }

    return ptr;
}

void DefaultAllocator::Free(void* ptr)
{
    if (ptr == nullptr)
        return;

#ifdef _DEBUG
    {
        std::unique_lock<std::mutex> lock(mMutex);
        const auto iter = mAllocationsDebugInfo.find(ptr);

        if (iter == mAllocationsDebugInfo.end())
            LOG_FATAL("Trying to free alreay freed memory block");

        mAllocationsDebugInfo.erase(iter);
    }
#endif

    // mBytesAllocated -= size;
    mAllocationsNum--;

#if defined(WIN32)
    _aligned_free(ptr);
#elif defined(__LINUX__) | defined(__linux__)
    free(ptr);
#endif
}

void DefaultAllocator::ReportAllocations() const
{
    for (const auto& it : mAllocationsDebugInfo)
    {
        LOG_INFO("Allocated block (ptr=%p, size=%zu) at %s:%i", it.first, it.second.size,
                 it.second.sourceFile, it.second.sourceLine);
    }
}


} // namespace Common
} // namespace NFE
