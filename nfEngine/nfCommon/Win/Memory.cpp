/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Memory utilities definitions
 */

#include "../PCH.hpp"
#include "../Memory.hpp"

namespace NFE {
namespace Common {

bool MemoryCheck(const void* ptr, size_t size)
{
    MEMORY_BASIC_INFORMATION info;
    VirtualQuery(ptr, &info, sizeof(info));

    if (((size_t)info.BaseAddress + info.RegionSize) <= ((size_t)ptr + size))
        return false;

    return info.Protect == PAGE_EXECUTE_READ || info.Protect == PAGE_EXECUTE_READWRITE ||
           info.Protect == PAGE_READONLY || info.Protect == PAGE_READWRITE;
}


} // namespace Common
} // namespace NFE
