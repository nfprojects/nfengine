/**
 * @file   Linux/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API implementation for Linux system
 */

#include "../stdafx.hpp"
#include "../SystemInfo.hpp"

namespace NFE {
    namespace Common {

void SystemInfo::GetCPUInfoPlatform()
{
    //get CPU information
    mCPUCoreNo = sysconf(_SC_NPROCESSORS_ONLN); // 'ONLN' are currently available,
                                                // 'CONF' are configured
    mPageSize = sysconf(_SC_PAGESIZE); // may be 'PAGE_SIZE' on some systems
}

void SystemInfo::GetMemoryInfo()
{
    // get memory information
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    mMemTotalVirtKb = sysInfo.totalram;
    mMemTotalVirtKb += sysInfo.totalswap;
    mMemTotalVirtKb *= sysInfo.mem_unit / 1024; // Values are given as multiples of mem_unit bytes.

    mMemFreeVirtKb = sysInfo.freeram;
    mMemFreeVirtKb += sysInfo.freeswap;
    mMemFreeVirtKb *= sysInfo.mem_unit / 1024;

    mMemTotalPhysKb = sysInfo.totalram;
    mMemTotalPhysKb *= sysInfo.mem_unit / 1024;

    mMemFreePhysKb = sysInfo.freeram;
    mMemTotalPhysKb *= sysInfo.mem_unit / 1024;
}

void SystemInfo::Cpuid(int cpuInfo[4], int function_id)
{
    // GCC won't allow us to clobber EBX since its used to store the GOT. So we need to
    // lie to GCC and backup/restore EBX without declaring it as clobbered.
    asm volatile( "pushl %%ebx   \n\t"
                "cpuid         \n\t"
                "movl %%ebx, %1\n\t"
                "popl %%ebx    \n\t"
                : "=a"(cpuInfo[0]), "=r"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
                : "a"(function_id)
                : "cc" );
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

    mMemFreeVirtKb = sysInfo.freeram;
    mMemFreeVirtKb += sysInfo.freeswap;
    mMemFreeVirtKb *= sysInfo.mem_unit / 1024;

    mMemFreePhysKb = sysInfo.freeram;
    mMemTotalPhysKb *= sysInfo.mem_unit / 1024;

    return mMemFreeVirtKb;
}
} // namespace Common
} // namespace NFE