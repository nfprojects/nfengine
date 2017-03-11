/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Memory utilities declarations.
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Check memory block access privileges. This function is mainly used to verify pointers
 * passed as input parameters to engine API to avoid access violations (segmentation faults).
 * @param ptr  Pointer to the beginning of a block
 * @param size Block size
 * @return True if given memory block can be read.
 */
NFCOMMON_API bool MemoryCheck(const void* ptr, size_t size);

/**
 * Template version of MemoryCheck(). Useful to verify pointer to a single object.
 */
template <typename T>
bool MemoryCheck(const T* ptr)
{
    return MemoryCheck(ptr, sizeof(T));
}
} // namespace Common
} // namespace NFE
