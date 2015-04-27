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
    Cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get features and exfeatures
    Cpuid(CPUInfo, 1);
    mCpuidFeatures = CPUInfo[3];
    mCpuidExFeatures = CPUInfo[2];

    // SystemInfo::Get the information associated with each extended ID.
    char CPUBrandString[0x40] = { 0 };
    for ( unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        Cpuid(CPUInfo, i);

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

    mCPUCoreNo = sysconf(_SC_NPROCESSORS_ONLN); // uint64_t 'ONLN' are currenty available,
                                                // 'CONF' are configured
    mPageSize = sysconf(_SC_PAGESIZE); // uint64_t may be 'PAGE_SIZE' on some systems

    //check cache line size
    unsigned int line_size = 0;
    Cpuid(CPUInfo, 0x80000006);
    line_size = CPUInfo[2] & 0xFF;
    mCacheLineSize = line_size; //uint64_t

    //check cpu freq
    Timer timer;
    double timerResult = 0;
    timer.Start();
    uint64_t cyclesNo = Rdtsc();
    do
    {
        timerResult = timer.Stop();
    } while(timerResult < 1);
    mCPUSpeedMHz = (Rdtsc() - cyclesNo) / 1000000.0;

    //get RAM info
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    mMemTotalVirtKb = sysInfo.totalram;
    mMemTotalVirtKb += sysInfo.totalswap;
    mMemTotalVirtKb *= sysInfo.mem_unit/1024; // Values are given as multiples of mem_unit bytes.

    mMemFreeVirtKb = sysInfo.freeram;
    mMemFreeVirtKb += sysInfo.freeswap;
    mMemFreeVirtKb *= sysInfo.mem_unit/1024;

    mMemTotalPhysKb = sysInfo.totalram;
    mMemTotalPhysKb *= sysInfo.mem_unit/1024;

    mMemFreePhysKb = sysInfo.freeram;
    mMemTotalPhysKb *= sysInfo.mem_unit/1024;
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
    mMemFreeVirtKb *= sysInfo.mem_unit/1024;

    mMemFreePhysKb = sysInfo.freeram;
    mMemTotalPhysKb *= sysInfo.mem_unit/1024;

    return mMemFreeVirtKb;
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures feature)
{
    return static_cast<bool>(mCpuidFeatures & feature);
}

bool SystemInfo::IsFeatureSupported(CpuidExFeatures feature)
{
    return static_cast<bool>(mCpuidExFeatures & feature);
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