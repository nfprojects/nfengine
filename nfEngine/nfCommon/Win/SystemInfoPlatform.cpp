/**
 * @file   Win/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Windows system
 */

#include "../PCH.hpp"
#include "../SystemInfo.hpp"

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
    mMemTotalPhysKb = memStatusEx.ullTotalPhys / MemoryBytesDivisor;
    mMemFreePhysKb = memStatusEx.ullAvailPhys / MemoryBytesDivisor;
    mMemTotalVirtKb = memStatusEx.ullTotalVirtual / MemoryBytesDivisor;
    mMemFreeVirtKb = memStatusEx.ullAvailVirtual / MemoryBytesDivisor;
    mMemTotalSwapKb = memStatusEx.ullTotalPageFile / MemoryBytesDivisor;
    mMemFreeSwapKb = memStatusEx.ullAvailPageFile / MemoryBytesDivisor; 
    mMemTotalSwapKb -= mMemTotalPhysKb; // PageFile is swap + RAM
    mMemFreeSwapKb -= mMemFreePhysKb;
    if (mMemTotalSwapKb == 0)
        mMemFreeSwapKb = 0;
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
    mMemFreePhysKb = memStatusEx.ullAvailPhys / MemoryBytesDivisor;
    mMemFreeSwapKb = memStatusEx.ullAvailPageFile / MemoryBytesDivisor; // PageFile is swap + RAM
    mMemFreeSwapKb -= mMemFreePhysKb;
    if (mMemTotalSwapKb == 0)
        mMemFreeSwapKb = 0;

    return mMemFreeSwapKb + mMemFreePhysKb;
}
} // namespace Common
} // namespace NFE
