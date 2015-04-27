/**
 * @file   Win/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Windows system
 */

#include "../PCH.hpp"
#include "../SystemInfo.hpp"
#include "../Math/Math.hpp"

namespace NFE {
namespace Common {

void SystemInfo::InitCPUInfoPlatform()
{
    //get CPU information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    mCPUCoreNo = sysInfo.dwNumberOfProcessors;
    mPageSize = sysInfo.dwPageSize;
}

void SystemInfo::InitMemoryInfo()
{
    // get memory information
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemTotalPhysKb = memStatusEx.ullTotalPhys / MEMORY_BYTES_DIVISOR;
    mMemFreePhysKb = memStatusEx.ullAvailPhys / MEMORY_BYTES_DIVISOR;
    mMemTotalVirtKb = memStatusEx.ullTotalVirtual / MEMORY_BYTES_DIVISOR;
    mMemFreeVirtKb = memStatusEx.ullAvailVirtual / MEMORY_BYTES_DIVISOR;
    mMemTotalSwapKb = memStatusEx.ullTotalPageFile / MEMORY_BYTES_DIVISOR;
    mMemFreeSwapKb = memStatusEx.ullAvailPageFile / MEMORY_BYTES_DIVISOR;

    if (mMemTotalSwapKb <= mMemTotalPhysKb) // If swap is turned off, it will be less or equal RAM
    {
        mMemTotalSwapKb = 0;
        mMemFreeSwapKb = 0;
        return;
    }

    mMemTotalSwapKb -= mMemTotalPhysKb; // PageFile is swap + RAM
    mMemFreeSwapKb -= mMemFreePhysKb;
}

void SystemInfo::Cpuid(int cpuInfo[4], int function_id)
{
    // MSVC provides a __cpuid function
    __cpuid(cpuInfo, function_id);
}

uint64_t SystemInfo::Rdtsc()
{
    return __rdtsc();
}

uint64_t SystemInfo::GetFreeMemoryKb()
{
    // get RAM info
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemFreePhysKb = memStatusEx.ullAvailPhys / MEMORY_BYTES_DIVISOR;
    mMemFreeVirtKb = memStatusEx.ullAvailVirtual / MEMORY_BYTES_DIVISOR;
    mMemFreeSwapKb = memStatusEx.ullAvailPageFile / MEMORY_BYTES_DIVISOR; // PageFile is swap + RAM
    mMemFreeSwapKb -= mMemFreePhysKb;

    if (mMemTotalSwapKb == 0) // Null mMemFreeSwapKb if swap is turned off
        mMemFreeSwapKb = 0;

    return Math::Min(mMemFreeSwapKb + mMemFreePhysKb, mMemFreeVirtKb);
}

uint64_t SystemInfo::GetMemFreePhysKb()
{
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemFreePhysKb = memStatusEx.ullAvailPhys / MEMORY_BYTES_DIVISOR;
    return mMemFreePhysKb;
}

uint64_t SystemInfo::GetMemFreeVirtKb()
{
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemFreeVirtKb = memStatusEx.ullAvailVirtual / MEMORY_BYTES_DIVISOR;
    return mMemFreeVirtKb;
}

uint64_t SystemInfo::GetMemFreeSwapKb()
{
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    uint64_t memFreePhysKb = memStatusEx.ullAvailPhys / MEMORY_BYTES_DIVISOR;
    mMemFreeSwapKb = memStatusEx.ullAvailPageFile / MEMORY_BYTES_DIVISOR; // PageFile is swap + RAM
    mMemFreeSwapKb -= memFreePhysKb;

    if (mMemTotalSwapKb == 0) // Null mMemFreeSwapKb if swap is turned off
        mMemFreeSwapKb = 0;
    return mMemFreeSwapKb;
}
} // namespace Common
} // namespace NFE
