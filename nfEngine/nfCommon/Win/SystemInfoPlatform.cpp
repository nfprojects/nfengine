/**
 * @file   Win/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API implementation for Windows system
 */

#include "../stdafx.hpp"
#include "../SystemInfo.hpp"

namespace NFE {
namespace Common {

void SystemInfo::GetCPUInfoPlatform()
{
    //get CPU information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    mCPUCoreNo = sysInfo.dwNumberOfProcessors;
    mPageSize = sysInfo.dwPageSize;
}

void SystemInfo::GetMemoryInfo()
{
    // get memory information
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemTotalPhysKb = memStatusEx.ullTotalPhys / 1024;
    mMemFreePhysKb = memStatusEx.ullAvailPhys / 1024;
    mMemTotalVirtKb = memStatusEx.ullTotalPageFile / 1024; // PageFile is swap + RAM
    mMemFreeVirtKb = memStatusEx.ullAvailPageFile / 1024; // PageFile is swap + RAM
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
    mMemFreePhysKb = memStatusEx.ullAvailPhys / 1024;
    mMemFreeVirtKb = memStatusEx.ullAvailPageFile / 1024; // PageFile is swap + RAM

    return mMemFreeVirtKb + mMemFreePhysKb;
}
} // namespace Common
} // namespace NFE