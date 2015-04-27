/**
 * @file   Linux/SystemInfoPlatform.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Linux system
 */

#include "../PCH.hpp"
#include "../SystemInfo.hpp"
#include <errno.h>

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
    
    std::ifstream memInfo("/proc/meminfo");
    for (int i = 0; i < 19; i++)
        memInfo.ignore(256, '\n');
    
    auto getProcValue = [](std::ifstream stream) -> uint64_t
    {
        std:string line;
        std::getline(stream, line);
        size_type colonPosition = line.find(':');
        size_type kbPosition = line.rfind("kB");
        line = line.substr(colonPosition, kbPosition - colonPosition);
        return std::static_cast<uint64_t>(std::stoull(line));
    }
    
    mMemTotalVirtKb = getProcValue(memInfo);
    mMemFreeVirtKb = mMemTotalVirtKb;
    mMemFreeVirtKb -= getProcValue(memInfo);
    
    uint64_t memUnit = sysInfo.mem_unit;

    mMemTotalSwapKb = sysInfo.totalswap / MemoryBytesDivisor;
    mMemTotalSwapKb *= memUnit;

    mMemFreeSwapKb = sysInfo.freeswap / MemoryBytesDivisor;
    mMemFreeSwapKb *= memUnit;

    mMemTotalPhysKb = sysInfo.totalram / MemoryBytesDivisor;
    mMemTotalPhysKb *= memUnit;

    mMemFreePhysKb = sysInfo.freeram / MemoryBytesDivisor;
    mMemFreePhysKb *= memUnit;
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

    uint64_t memUnit = sysInfo.mem_unit;

    mMemFreeSwapKb = sysInfo.freeswap / MemoryBytesDivisor;
    mMemFreeSwapKb *= memUnit;

    mMemFreePhysKb = sysInfo.freeram / MemoryBytesDivisor;
    mMemFreePhysKb *= memUnit;

    return mMemFreeSwapKb + mMemFreePhysKb;
}
} // namespace Common
} // namespace NFE
