/**
 * @file   SystemInfo.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation.
 */

#include "stdafx.hpp"
#include "SystemInfo.hpp"

namespace NFE {
namespace Common {

void ReadHardwareInfo()
{
    // Get extended ids.
    int CPUInfo[4] = { -1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    char CPUBrandString[0x40] = { 0 };
    for ( unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
        {
            memcpy( CPUBrandString,
                    CPUInfo,
                    sizeof(CPUInfo));
        }
        else if ( i == 0x80000003 )
        {
            memcpy( CPUBrandString + 16,
                    CPUInfo,
                    sizeof(CPUInfo));
        }
        else if ( i == 0x80000004 )
        {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
    }

    LOG_INFO("CPU: %s", CPUBrandString);

    //get CPU info
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    LOG_INFO("CPU logical cores: %u", sysInfo.dwNumberOfProcessors);
    LOG_INFO("Memory page size: %u bytes", sysInfo.dwPageSize);

    //check cache line size
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = 0;
    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);
    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
    {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
        {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }
    free(buffer);
    LOG_INFO("CPU cache line: %u bytes", line_size);
}

} // namespace Common
} // namespace NFE