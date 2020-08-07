/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Memory utilities definitions
 */

#include "PCH.hpp"
#include "../Memory.hpp"
#include "../../Logger/Logger.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace NFE {
namespace Common {

bool MemoryCheck(const void* ptr, size_t size)
{
    ::SetLastError(0);

    MEMORY_BASIC_INFORMATION info;
    if (0 == ::VirtualQuery(ptr, &info, sizeof(info)))
    {
        const DWORD errorCode = ::GetLastError();
        NFE_LOG_ERROR("VirtualQuery failed (ptr=%p, size=%zu), error code: %u", ptr, size, errorCode);
        return false;
    }

    if ((reinterpret_cast<size_t>(info.BaseAddress) + info.RegionSize) <=
        (reinterpret_cast<size_t>(ptr) + size))
        return false;

    return info.Protect == PAGE_EXECUTE_READ || info.Protect == PAGE_EXECUTE_READWRITE ||
           info.Protect == PAGE_READONLY || info.Protect == PAGE_READWRITE;
}


} // namespace Common
} // namespace NFE
