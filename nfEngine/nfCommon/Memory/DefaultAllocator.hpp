/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator declaration.
 */

#pragma once
#include "../nfCommon.hpp"

#include <unordered_map>
#include <mutex>
#include <atomic>

namespace NFE {
namespace Common {


struct AllocationDebugInfo
{
    size_t size;
    const char* sourceFile;
    int sourceLine;
};

struct AllocatorStats
{
    size_t bytesAllocated;
    size_t allocationsNum;
};

class NFCOMMON_API DefaultAllocator
{
#ifdef _DEBUG
    std::mutex mMutex;
    std::unordered_map<void*, AllocationDebugInfo> mAllocationsDebugInfo;
#endif

    std::atomic<size_t> mAllocationsNum;
    std::atomic<size_t> mBytesAllocated;

    DefaultAllocator();
    ~DefaultAllocator();

public:
    /**
     * Get instance of default global memory allocator.
     */
    static DefaultAllocator& GetInstance();

    /**
     * Allocate memory.
     * @param size                  Size of memory block in bytes.
     * @param alignment             Memory block alignment in bytes. Must be power of 2.
     * @param sourceFile,sourceLine Source file path and line at which the allocation
     *                              is performed. For debugging.
     * @return nullptr on error or valid memory block pointer.
     */
    void* Malloc(size_t size, size_t alignment, const char* sourceFile, int sourceLine);

    /**
     * Free the memory block allocated with @p Malloc method.
     * @param ptr Valid memory block.
     * @note A memory block can not be freed twice.
     */
    void Free(void* ptr);

    /**
     * Report all allocated memory blocks.
     */
    void ReportAllocations();

    /**
     * Get the default memory allocator statistics.
     */
    AllocatorStats GetStats() const;
};

} // namespace Common
} // namespace NFE


#define NFE_MALLOC(size, alignment) \
    NFE::Common::DefaultAllocator::GetInstance().Malloc(size, alignment, __FILE__, __LINE__)
#define NFE_FREE(ptr) \
    NFE::Common::DefaultAllocator::GetInstance().Free(ptr)