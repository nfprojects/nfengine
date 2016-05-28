/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator declaration.
 */

#pragma once
#include "../nfCommon.hpp"

#include <unordered_map>

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
    static DefaultAllocator& GetInstance();

    void* Malloc(size_t size, size_t alignment, const char* sourceFile, int sourceLine);
    void Free(void* ptr);

    void ReportAllocations() const;
    AllocatorStats GetStats() const;
};

} // namespace Common
} // namespace NFE


#define NFE_MALLOC(size, alignment) \
    NFE::Common::DefaultAllocator::GetInstance().Malloc(size, alignment, __FILE__, __LINE__)
#define NFE_FREE(ptr) \
    NFE::Common::DefaultAllocator::GetInstance().Free(ptr)
