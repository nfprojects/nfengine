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
    mCPUBrand.assign(CPUBrandString);

    //get CPU information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    mCPUCoreNo = sysInfo.dwNumberOfProcessors;
    mPageSize = sysInfo.dwPageSize;

    //get cache line size
    unsigned int line_size = 0;
    Cpuid(CPUInfo, 0x80000006);
    line_size = CPUInfo[2] & 0xFF;
    mCacheLineSize = line_size;

    // check CPU frequency
    Timer timer;
    double timerResult = 0;
    timer.Start();
    uint64_t cyclesNo = Rdtsc();
    do
    {
        timerResult = timer.Stop();
    } while(timerResult < 1);
    mCPUSpeedMHz = (Rdtsc() - cyclesNo) / 1000000.0;

    // get memory information
    MEMORYSTATUSEX memStatusEx;
    memStatusEx.dwLength = sizeof(memStatusEx);
    GlobalMemoryStatusEx(&memStatusEx);
    mMemTotalPhysKb = memStatusEx.ullTotalPhys/1024;
    mMemFreePhysKb = memStatusEx.ullAvailPhys/1024;
    mMemTotalVirtKb = memStatusEx.ullTotalPageFile/1024; // PageFile is swap + RAM
    mMemFreeVirtKb = memStatusEx.ullAvailPageFile/1024; // PageFile is swap + RAM
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
    mMemFreePhysKb = memStatusEx.ullAvailPhys/1024; // uint64_t
    mMemFreeVirtKb = memStatusEx.ullAvailPageFile/1024; // uint64_t PageFile is swap + RAM

    return mMemFreeVirtKb + mMemFreePhysKb;
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