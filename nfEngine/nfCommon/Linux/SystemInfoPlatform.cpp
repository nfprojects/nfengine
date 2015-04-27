/**
 * @file   Linux/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Linux system
 */

#include "../stdafx.hpp"
#include "../SystemInfo.hpp"
#inlcude <errno.h>

namespace NFE {
namespace Common {

void SystemInfo::InitCPUInfoPlatform()
{
    //get CPU information
    mCPUCoreNo = sysconf(_SC_NPROCESSORS_ONLN); // 'ONLN' are currently available,
                                                // 'CONF' are configured
    mPageSize = sysconf(_SC_PAGESIZE); // may be 'PAGE_SIZE' on some systems
}

void SystemInfo::InitMemoryInfo()
{
    // get memory information
    struct sysinfo sysInfo;
    int retVal = sysinfo(&sysInfo);
    if (retVal != -1)
    {
        uint64_t memUnit = sysInfo.mem_unit / 1024; // Values are given as multiples
                                                    // of mem_unit bytes.
        mMemTotalVirtKb = sysInfo.totalswap;
        mMemTotalVirtKb *= memUnit

        mMemFreeVirtKb = sysInfo.freeswap;
        mMemFreeVirtKb *= memUnit;

        mMemTotalPhysKb = sysInfo.totalram;
        mMemTotalPhysKb *= memUnit;

        mMemFreePhysKb = sysInfo.freeram;
        mMemTotalPhysKb *= memUnit;
    } else
    {
        int errNo = errno;
        cout << "sysinfo read wrong! errno = " << errNo << std::endl;
    }
}

void SystemInfo::Cpuid(int cpuInfo[4], int function_id)
{
    __get_cpuid(function_id, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
}

uint64_t SystemInfo::Rdtsc()
{
    unsigned int hi, lo;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

uint64_t SystemInfo::GetFreeMemoryKb()
{
    //get RAM info
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    mMemFreeVirtKb = sysInfo.freeswap;
    mMemFreeVirtKb *= sysInfo.mem_unit / 1024;

    mMemFreePhysKb = sysInfo.freeram;
    mMemTotalPhysKb *= sysInfo.mem_unit / 1024;

    return mMemFreeVirtKb + mMemFreePhysKb;
}
} // namespace Common
} // namespace NFE