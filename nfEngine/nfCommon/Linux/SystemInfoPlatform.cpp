/**
 * @file   Linux/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Linux system
 */

#include "../stdafx.hpp"
#include "../SystemInfo.hpp"

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
#if defined(__i386__) && defined(__PIC__)
/* %ebx may be the PIC register.  */
#if __GNUC__ >= 3
    __asm__ ("xchg{l}\t{%%}ebx, %1\n\t"
            "cpuid\n\t"
            "xchg{l}\t{%%}ebx, %1\n\t"
            : "=a" (cpuInfo[0]), "=r" (cpuInfo[1]), "=c" (cpuInfo[2]), "=d" (cpuInfo[3])
            : "0" (function_id));
#else
/* Host GCCs older than 3.0 weren't supporting Intel asm syntax
   nor alternatives in i386 code.  */
    __asm__ ("xchgl\t%%ebx, %1\n\t"
            "cpuid\n\t"
            "xchgl\t%%ebx, %1\n\t"
            : "=a" (cpuInfo[0]), "=r" (cpuInfo[1]), "=c" (cpuInfo[2]), "=d" (cpuInfo[3])
            : "0" (function_id));
#endif
#else
    __asm__ ("cpuid\n\t"
            : "=a" (cpuInfo[0]), "=b" (cpuInfo[1]), "=c" (cpuInfo[2]), "=d" (cpuInfo[3])
            : "0" (function_id));
#endif
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