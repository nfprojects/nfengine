/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Linux system
 */

#include "../PCH.hpp"
#include "../SystemInfo.hpp"
#include "../Math/Math.hpp"
#include "../Logger.hpp"
#include <vector>
#include <streambuf>
#include <fstream>
#include <sys/utsname.h>


namespace NFE {
namespace Common {

void SystemInfo::InitCPUInfoPlatform()
{
    //get CPU information
    mCPUCoreNo = sysconf(_SC_NPROCESSORS_ONLN); // 'ONLN' are currently available,
                                                // 'CONF' are configured
    mPageSize = sysconf(_SC_PAGESIZE); // may be 'PAGE_SIZE' on some systems
}

void SystemInfo::InitOSVersion()
{
    static const std::vector<std::string> osFiles = { "/etc/redhat-release",
                                                      "/etc/issue",
    };

    for (const auto& i : osFiles)
    {
        std::ifstream sysFile(i.c_str());

        if (sysFile)
        {
            std::getline(sysFile, mOSVersion);

            // There often is a newline char at the end
            if (mOSVersion.back() == '\n')
                mOSVersion.pop_back();

            break;
        }
    }

    struct utsname ver;
    uname(&ver);

    if (mOSVersion.empty())
        mOSVersion = std::string(ver.sysname);
    mOSVersion += " Build: " + std::string(ver.release);
}

void SystemInfo::InitMemoryInfo()
{
    // get memory information
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    /* TODO Implement checking total virtual memory for the current process
     * then we will be able to calculate the second one
     *     std:string line;
     *     uint64_t memUsedVirtKb;
     *     std::ifstream statmStream("/proc/self/statm");
     *     std::getline(statmStream, line);
     *     sscanf(line.c_str(), "%*s %ul", memUsedVirtKb);
     *     memstatmStreamInfo.close();
     *     mMemTotalVirtKb = <?????>
     *     mMemFreeVirtKb = <?????>
    */
    // 2 line "hotfix" waiting for the to-do above
    mMemTotalVirtKb = 0;
    mMemFreeVirtKb = 0;

    uint64_t memUnit = sysInfo.mem_unit;

    mMemTotalSwapKb = sysInfo.totalswap * memUnit;
    mMemTotalSwapKb /= MEMORY_BYTES_DIVISOR;

    mMemFreeSwapKb = sysInfo.freeswap * memUnit;
    mMemFreeSwapKb /= MEMORY_BYTES_DIVISOR;

    mMemTotalPhysKb = sysInfo.totalram * memUnit;
    mMemTotalPhysKb /= MEMORY_BYTES_DIVISOR;

    mMemFreePhysKb = sysInfo.freeram * memUnit;
    mMemFreePhysKb /= MEMORY_BYTES_DIVISOR;
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

    mMemFreeSwapKb = sysInfo.freeswap * memUnit;
    mMemFreeSwapKb /= MEMORY_BYTES_DIVISOR;

    mMemFreePhysKb = sysInfo.freeram * memUnit;
    mMemFreePhysKb /= MEMORY_BYTES_DIVISOR;

    /* TODO Implement checking total virtual memory for the current process
     * then we will be able to calculate the second one
     *     std:string line;
     *     uint64_t memUsedVirtKb;
     *     std::ifstream statmStream("/proc/self/statm");
     *     std::getline(statmStream, line);
     *     sscanf(line.c_str(), "%*s %ul", memUsedVirtKb);
     *     memstatmStreamInfo.close();
     *     mMemTotalVirtKb = <?????>
     *     mMemFreeVirtKb = <?????>
    */

    // TODO When FreeVirtualMemory is correctly calculated uncomment line below, delete obsolete one
    // return Math::Min(mMemFreeSwapKb + mMemFreePhysKb, mMemFreeVirtKb);
    return mMemFreeSwapKb + mMemFreePhysKb;
}

uint64_t SystemInfo::GetMemFreePhysKb()
{
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    uint64_t memUnit = sysInfo.mem_unit;

    mMemFreePhysKb = sysInfo.freeram * memUnit;
    mMemFreePhysKb /= MEMORY_BYTES_DIVISOR;
    return mMemFreePhysKb;
}

uint64_t SystemInfo::GetMemFreeVirtKb()
{
    /* TODO Implement checking total virtual memory for the current process
     * then we will be able to calculate the second one
     *     std:string line;
     *     uint64_t memUsedVirtKb;
     *     std::ifstream statmStream("/proc/self/statm");
     *     std::getline(statmStream, line);
     *     sscanf(line.c_str(), "%*s %ul", memUsedVirtKb);
     *     memstatmStreamInfo.close();
     *     mMemTotalVirtKb = <?????>
     *     mMemFreeVirtKb = <?????>
    */
    return mMemFreeVirtKb;
}

uint64_t SystemInfo::GetMemFreeSwapKb()
{
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    uint64_t memUnit = sysInfo.mem_unit;

    mMemFreeSwapKb = sysInfo.freeswap * memUnit;
    mMemFreeSwapKb /= MEMORY_BYTES_DIVISOR;
    return mMemFreeSwapKb;
}
} // namespace Common
} // namespace NFE
