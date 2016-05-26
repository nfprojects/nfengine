/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator declaration.
 */

#pragma once
#include "../nfCommon.hpp"

namespace NFE {
namespace Common {


struct AllocationDebugInfo
{
    size_t size;
    const char* sourceFile;
    int sourceLine;
};

class NFCOMMON_API GenericAllocator
{
#ifdef _DEBUG
    std::mutex mMutex;
    std::map<void*, AllocationDebugInfo> mAllocationsDebugInfo;
#endif

    std::atomic<size_t> mAllocationsNum;
    std::atomic<size_t> mBytesAllocated;

    GenericAllocator();
    ~GenericAllocator();

public:
    static GenericAllocator& GetInstance();

    void* Malloc(size_t size, size_t alignment, const char* sourceFile, int sourceLine);

    void Free(void* ptr);

    void ReportAllocations() const;
};

} // namespace Common
} // namespace NFE


#define NFE_GENERIC_MALLOC(size, alignment) \
    NFE::Common::GenericAllocator::GetInstance().Malloc(size, alignment, __FILE__, __LINE__)
#define NFE_GENERIC_FREE(ptr) \
    NFE::Common::GenericAllocator::GetInstance().Free(ptr)
