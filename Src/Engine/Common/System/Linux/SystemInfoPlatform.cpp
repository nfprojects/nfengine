/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation for Linux system
 */

#include "PCH.hpp"
#include "../SystemInfo.hpp"
#include "Math/Math.hpp"
#include "Logger/Logger.hpp"
#include "Containers/DynArray.hpp"
#include "FileSystem/File.hpp"

#include <streambuf>
#include <fstream>
#include <sys/utsname.h>


namespace NFE {
namespace Common {

namespace {

bool ReadFileToString(const String& path, String& outString)
{
    File file(path, AccessMode::Read);

    int64 size = file.GetSize();
    if (size < 0)
    {
        return false;
    }

    if (size > static_cast<int64>(String::MaxInternalLength))
    {
        NFE_LOG_WARNING("File is too big to fit a String (%ull bytes)", size);
        size = static_cast<int64>(String::MaxInternalLength);
    }

    // TODO this buffer is redundant
    DynArray<char> buffer;
    buffer.Resize(static_cast<uint32>(size));
    const size_t bytesToRead = static_cast<size_t>(size);
    if (bytesToRead != file.Read(buffer.Data(), bytesToRead))
    {
        return false;
    }

    outString = StringView(buffer.Data(), buffer.Size());
    return true;
}

} // namespace

void SystemInfo::InitCPUInfoPlatform()
{
    //get CPU information
    mCPUCoreNo = sysconf(_SC_NPROCESSORS_ONLN); // 'ONLN' are currently available,
                                                // 'CONF' are configured
    mPageSize = sysconf(_SC_PAGESIZE); // may be 'PAGE_SIZE' on some systems
}

void SystemInfo::InitOSVersion()
{
    static const DynArray<String> osFiles = { "/etc/redhat-release", "/etc/issue" };

    for (const auto& path : osFiles)
    {
        String str;
        if (ReadFileToString(path, str))
        {
            if (!str.Empty())
            {
                if (str[str.Length() - 1])
                {
                    str.Erase(str.Length() - 1, 1);
                }

                mOSVersion = std::move(str);
                break;
            }
        }
    }

    struct utsname ver;
    uname(&ver);

    if (mOSVersion.Empty())
        mOSVersion = ver.sysname;

    mOSVersion += " Build: ";
    mOSVersion + ver.release;
}

void SystemInfo::InitMemoryInfo()
{
    // get memory information
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    /* TODO Implement checking total virtual memory for the current process
     * then we will be able to calculate the second one
     *     std:string line;
     *     uint64 memUsedVirtKb;
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

    uint64 memUnit = sysInfo.mem_unit;

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

uint64 SystemInfo::Rdtsc()
{
    unsigned int hi, lo;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64)hi << 32) | lo;
}

uint64 SystemInfo::GetFreeMemoryKb()
{
    //get RAM info
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    uint64 memUnit = sysInfo.mem_unit;

    mMemFreeSwapKb = sysInfo.freeswap * memUnit;
    mMemFreeSwapKb /= MEMORY_BYTES_DIVISOR;

    mMemFreePhysKb = sysInfo.freeram * memUnit;
    mMemFreePhysKb /= MEMORY_BYTES_DIVISOR;

    /* TODO Implement checking total virtual memory for the current process
     * then we will be able to calculate the second one
     *     std:string line;
     *     uint64 memUsedVirtKb;
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

uint64 SystemInfo::GetMemFreePhysKb()
{
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    uint64 memUnit = sysInfo.mem_unit;

    mMemFreePhysKb = sysInfo.freeram * memUnit;
    mMemFreePhysKb /= MEMORY_BYTES_DIVISOR;
    return mMemFreePhysKb;
}

uint64 SystemInfo::GetMemFreeVirtKb()
{
    /* TODO Implement checking total virtual memory for the current process
     * then we will be able to calculate the second one
     *     std:string line;
     *     uint64 memUsedVirtKb;
     *     std::ifstream statmStream("/proc/self/statm");
     *     std::getline(statmStream, line);
     *     sscanf(line.c_str(), "%*s %ul", memUsedVirtKb);
     *     memstatmStreamInfo.close();
     *     mMemTotalVirtKb = <?????>
     *     mMemFreeVirtKb = <?????>
    */
    return mMemFreeVirtKb;
}

uint64 SystemInfo::GetMemFreeSwapKb()
{
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);

    uint64 memUnit = sysInfo.mem_unit;

    mMemFreeSwapKb = sysInfo.freeswap * memUnit;
    mMemFreeSwapKb /= MEMORY_BYTES_DIVISOR;
    return mMemFreeSwapKb;
}
} // namespace Common
} // namespace NFE
