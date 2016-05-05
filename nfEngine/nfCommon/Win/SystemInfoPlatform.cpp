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

void SystemInfo::InitOSVersion()
{
    OSVERSIONINFOEX osVersionEx;
    osVersionEx.dwOSVersionInfoSize = sizeof(osVersionEx);
    GetVersionEx((OSVERSIONINFO*) &osVersionEx);

    mOSVersion = "Microsoft Windows ";

    switch (osVersionEx.dwMajorVersion)
    {
        case 5:
            if (osVersionEx.dwMinorVersion == 0)
                mOSVersion += "2000";
            else if (osVersionEx.dwMinorVersion == 1)
                mOSVersion += "XP";
            else
            {
                if (osVersionEx.wProductType == VER_NT_WORKSTATION)
                    mOSVersion += "XP Professional x64 Edition";
                else if (osVersionEx.wSuiteMask & VER_SUITE_WH_SERVER)
                    mOSVersion += "Home Server";
                else if (GetSystemMetrics(SM_SERVERR2) == 0)
                    mOSVersion += "Server 2003";
                else
                    mOSVersion += "Server 2003 R2";
            }
            break;
        case 6:
            if (osVersionEx.dwMinorVersion == 0)
                if (osVersionEx.wProductType == VER_NT_WORKSTATION)
                    mOSVersion += "Vista";
                else
                    mOSVersion += "Server 2008";
            else if (osVersionEx.dwMinorVersion == 1)
                if (osVersionEx.wProductType == VER_NT_WORKSTATION)
                    mOSVersion += "7";
                else
                    mOSVersion += "Server 2008 R2";
            else if (osVersionEx.dwMinorVersion == 2)
                if (osVersionEx.wProductType == VER_NT_WORKSTATION)
                    mOSVersion += "8";
                else
                    mOSVersion += "Server 2012";
            else
                if (osVersionEx.wProductType == VER_NT_WORKSTATION)
                    mOSVersion += "8.1";
                else
                    mOSVersion += "Server 2012 R2";
            break;
        case 10:
            if (osVersionEx.wProductType == VER_NT_WORKSTATION)
                mOSVersion += "10";
            else
                mOSVersion += "Server 2016 Technical Preview";
    }
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
