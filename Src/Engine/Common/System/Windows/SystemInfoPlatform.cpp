/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Windows system
 */

#include "PCH.hpp"
#include "../SystemInfo.hpp"
#include "../Library.hpp"
#include "Common.hpp"
#include "../../Utils/StringUtils.hpp"

#include <intrin.h>

namespace NFE {
namespace Common {

namespace {

typedef void (WINAPI *RtlGetVersionFuncType)(OSVERSIONINFOEXW*);

bool GetVersion(OSVERSIONINFOEX* os)
{
    Library lib;
    RtlGetVersionFuncType func;

#ifdef UNICODE
    OSVERSIONINFOEXW* osw = os;
#else
    OSVERSIONINFOEXW o;
    OSVERSIONINFOEXW* osw = &o;
#endif

    if (lib.Open("ntdll.dll"))
    {
        if(!lib.GetSymbol("RtlGetVersion", func))
        {
            lib.Close();
            return false;
        }
        ZeroMemory(osw, sizeof(*osw));
        osw->dwOSVersionInfoSize = sizeof(*osw);
        func(osw);

#ifndef UNICODE
        os->dwBuildNumber = osw->dwBuildNumber;
        os->dwMajorVersion = osw->dwMajorVersion;
        os->dwMinorVersion = osw->dwMinorVersion;
        os->dwPlatformId = osw->dwPlatformId;
        os->dwOSVersionInfoSize = sizeof(*os);
        WCHAR* csdVerW = osw->szCSDVersion;
        unsigned char* csdVer = reinterpret_cast<unsigned char *>(os->szCSDVersion);
        while (*csdVerW)
            *csdVer++ = static_cast<unsigned char>(*csdVerW++);
        *csdVer = '\0';
#endif

    } else
        return false;

    lib.Close();
    return true;
}

} // namespace

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
    OSVERSIONINFOEX os;
    if (!GetVersion(&os))
    {
        NFE_LOG_ERROR("Failed to get OS version");
        return;
    }

    mOSVersion = "Microsoft Windows ";

    if (os.dwMajorVersion == 10)
    {
        if (IsWindowsServer())
            mOSVersion += "Server 2016 Technical Preview";
        else
            mOSVersion += "10";
    }
    else if (IsWindows8Point1OrGreater())
    {
        if (IsWindowsServer())
            mOSVersion += "Server 2012 R2";
        else
            mOSVersion += "8.1";
    }
    else if (IsWindows8OrGreater())
    {
        if (IsWindowsServer())
            mOSVersion += "Server 2012";
        else
            mOSVersion += "8";
    }
    else if (IsWindows7OrGreater())
    {
        if (IsWindowsServer())
            mOSVersion += "Server 2008 R2";
        else
            mOSVersion += "7";
    }
    else if (IsWindowsVistaOrGreater())
    {
        if (IsWindowsServer())
            mOSVersion += "Server 2008";
        else
            mOSVersion += "Vista";
    }
    else
    {
        mOSVersion += ToString(uint32(os.dwMajorVersion)) + '.' + ToString(uint32(os.dwMinorVersion));
    }

    String szCSDVer;
    if (UTF16ToUTF8(os.szCSDVersion, szCSDVer))
    {
        if (!szCSDVer.Empty())
        {
            mOSVersion += " " + szCSDVer;
        }
    }
    mOSVersion += " Build: ";
    mOSVersion += ToString(uint32(os.dwBuildNumber));
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
