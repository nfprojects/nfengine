/**
 * @file   Memory.cpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  Memory utilities definitions
 */

#include "stdafx.hpp"
#include "Memory.hpp"

namespace NFE {
namespace Common {

//check if memory block can be read and/or written
uint32 MemoryCheck(const void* ptr, size_t size)
{
    MEMORY_BASIC_INFORMATION info;
    VirtualQuery(ptr, &info, sizeof(info));

    if (((size_t)info.BaseAddress + info.RegionSize) <= ((size_t)ptr + size))
        return false;

    UINT flags = 0;
    switch (info.Protect & 0xFF)
    {
        case PAGE_EXECUTE_READ:
            flags = ACCESS_READ | ACCESS_EXECUTE;
            break;

        case PAGE_EXECUTE:
            flags = ACCESS_EXECUTE;
            break;

        case PAGE_EXECUTE_READWRITE:
            flags = ACCESS_WRITE | ACCESS_READ | ACCESS_EXECUTE;
            break;

        case PAGE_READONLY:
            flags = ACCESS_READ;
            break;

        case PAGE_READWRITE:
            flags = ACCESS_WRITE | ACCESS_READ;
            break;
    }

    return flags;
}


} // namespace Common
} // namespace NFE
