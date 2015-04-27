/**
 * @file   Win/SystemInfo.cpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API implementation for Windows system
 */

#include "../stdafx.hpp"
#include "../SystemInfo.hpp"

namespace NFE {
namespace Common {

SystemInfo::SystemInfo()
{
    // SystemInfo::Get extended ids.
    int CPUInfo[4] = { -1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // SystemInfo::Get the information associated with each extended ID.
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
    mCPUBrand = CPUBrandString; //string

    //get CPU info
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    mCPUCoreNo = sysInfo.dwNumberOfProcessors; //uint64_t
    mPageSize = sysInfo.dwPageSize; //uint64_t

    //check cache line size
    unsigned int line_size = 0;
    __cpuid(CPUInfo, 0x80000006);
    line_size = CPUInfo[2] & 0xFF;
    mCacheLineSize = line_size; //uint64_t

    //check cpu freq
    uint64_t CPUSpeedMHz = 0;
    LARGE_INTEGER qwWait, qwStart, qwCurrent;
    QueryPerformanceCounter(&qwStart);
    QueryPerformanceFrequency(&qwWait);
    qwWait.QuadPart >>= 5;
    unsigned __int64 Start = __rdtsc();
    do
    {
        QueryPerformanceCounter(&qwCurrent);
    }while(qwCurrent.QuadPart - qwStart.QuadPart < qwWait.QuadPart);
    CPUSpeedMHz = ((__rdtsc() - Start) << 5) / 1000000.0;
    mCPUSpeedMHz = CPUSpeedMHz; //uint64_t

    //get RAM info
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemTotalPhysKb = memStatusEx.ullTotalPhys/1024; //uint64_t
    mMemFreePhysKb = memStatusEx.ullAvailPhys/1024; //uint64_t
    mMemTotalVirtKb = memStatusEx.ullTotalVirtual/1024; //uint64_t
    mMemFreeVirtKb = memStatusEx.ullAvailVirtual/1024; //uint64_t
}

uint64_t SystemInfo::GetFreeMemoryKb()
{
    //get RAM info
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemFreePhysKb = memStatusEx.ullAvailPhys/1024; //uint64_t
    mMemFreeVirtKb = memStatusEx.ullAvailVirtual/1024; //uint64_t

    return mMemFreeVirtKb + mMemFreePhysKb;
}

std::string SystemInfo::GetCPUBrand()
{
    return mCPUBrand;
}

uint64_t SystemInfo::GetCPUCoreNo()
{
    return mCPUCoreNo;
}

uint64_t SystemInfo::GetPageSize()
{
    return mPageSize;
}

uint64_t SystemInfo::GetCacheLineSize()
{
    return mCacheLineSize;
}

uint64_t SystemInfo::GetCPUSpeedMHz()
{
    return mCPUSpeedMHz;
}

uint64_t SystemInfo::GetMemTotalPhysKb()
{
    return mMemTotalPhysKb;
}

uint64_t SystemInfo::GetMemTotalVirtKb()
{
    return mMemTotalVirtKb;
}

uint64_t SystemInfo::GetMemFreePhysKb()
{
    return mMemFreePhysKb;
}

uint64_t SystemInfo::GetMemFreeVirtKb()
{
    return mMemFreeVirtKb;
}

} // namespace Common
} // namespace NFE