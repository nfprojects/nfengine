/**
 * @file   Memory.hpp
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Memory utilities declarations.
 */

#pragma once

#include "nfCommon.hpp"

namespace NFE {
namespace Common {

//memory access rights
#define ACCESS_READ (1<<0)
#define ACCESS_WRITE (1<<1)
#define ACCESS_EXECUTE (1<<2)
#define ACCESS_READWRITE (ACCESS_READ|ACCESS_WRITE)

/**
 * Check memory block access privileges. This function is mainly used to verify pointers
 *        passed as input parameters to engine API to avoid access violations (segmentation faults).
 * @param ptr Pointer to the beginning of a block
 * @param size Block size
 * @return Bitfield describing access privileges to the memory block
 */
NFCOMMON_API uint32 MemoryCheck(const void* ptr, size_t size);

/**
 * Template version of MemoryCheck(). Useful to verify pointer to a single object.
 */
template <typename T>
uint32 MemoryCheck(const T* ptr)
{
    return MemoryCheck(ptr, sizeof(T));
}
} // namespace Common
} // namespace NFE
