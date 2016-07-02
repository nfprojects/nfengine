/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API declarations.
 */

#pragma once

#include "nfCommon.hpp"
#include "Timer.hpp"

#if defined(__LINUX__) | defined(__linux__)
#include "sys/sysinfo.h"
#include <cpuid.h>
#include <unistd.h>
#endif // defined(__LINUX__) | defined(__linux__)


namespace NFE {
namespace Common {

const int MEMORY_BYTES_DIVISOR = 1024;

struct CpuidFeature
{
    int AddressNo;
    int FeatureNo;
    CpuidFeature() : AddressNo(0), FeatureNo(0)
    {};
    CpuidFeature(int x, int y) : AddressNo(x), FeatureNo(y)
    {}
};

/**
 * Class used to gather information about hardware
 */
class NFCOMMON_API SystemInfo
{
private:
    SystemInfo();

    std::map<std::string, CpuidFeature> mCpuidFeatureMap;
    std::string mCPUBrand;
    std::string mOSVersion;
    std::string mCompilerInfo;
    uint64_t mCPUCoreNo;
    uint64_t mPageSize;
    uint64_t mCacheLineSize;
    uint64_t mMemTotalPhysKb;
    uint64_t mMemFreePhysKb;
    uint64_t mMemTotalSwapKb;
    uint64_t mMemFreeSwapKb;
    uint64_t mMemTotalVirtKb;
    uint64_t mMemFreeVirtKb;
    uint64_t mCpuidFeatures[5];

    void Cpuid(int cpuInfo[4], int function_id);
    uint64_t Rdtsc();

    void InitCPUInfoCommon();
    void InitOSVersion();
    void InitCompilerInfo();
    void InitMap();
    void InitCPUInfoPlatform();
    void InitMemoryInfo();

    bool CheckFeature(CpuidFeature feature) const;

public:
    static SystemInfo& Instance();

    /**
     * Update mMemFree fields
     * @return Free memory in kB
     */
    uint64_t GetFreeMemoryKb();

    /**
     * Check if feature is supported. List of features at the bottom of this file.
     * @return True if feature is supported, False otherwise
     */
    bool IsFeatureSupported(const std::string& featureName) const;

    /**
     * Update free physical memory information
     * @return Free physical memory in kB
     */
    uint64_t GetMemFreePhysKb();

    /**
     * Update free virtual memory information
     * @return Free virtual memory in kB
     */
    uint64_t GetMemFreeVirtKb();

    /**
     * Update free swap memory information
     * @return Free swap memory in kB
     */
    uint64_t GetMemFreeSwapKb();

    /**
     * Print all gathered information
     * @return Formatted string with all information
     */
    std::string ConstructAllInfoString();

    /**
     * Get class fields
     */
    const std::string& GetCPUBrand() const;
    const std::string& GetOSVersion() const;
    const std::string& GetCompilerInfo() const;
    uint64_t GetCPUCoreNo() const;
    uint64_t GetPageSize() const;
    uint64_t GetCacheLineSize() const;
    uint64_t GetMemTotalPhysKb() const;
    uint64_t GetMemTotalVirtKb() const;
    uint64_t GetMemTotalSwapKb() const;
};

/**
 * List of features supported by IsFeatureSupported(std::string) function:
 *     ABM - Advanced Bit Manipulation
 *     ACPI - Thermal Monitor & Software Controlled Clock
 *     ADX - Multi-Precision Add-Carry instruction Extensions
 *     AES - Advanced Encryption Standard
 *     APIC - On-chip APIC hardware
 *     AVX - Advanced Vector Extensions
 *     AVX2 - Advanced Vector Extensions 2
 *     BMI1 - Bit Manipulation Instruction set 1
 *     BMI2 - Bit Manipulation Instruction set 2
 *     CAE - CompareAndExchange 16B
 *     CLFSH - CFLUSH Instruction
 *     CMOV - Conditional MOVe Instruction
 *     CPL - CPL-qualified Debug Store
 *     CX8 - CMPXCHG8 Instruction
 *     DE - Debugging Extension
 *     DS - Debug Store
 *     EM64T - Support for 64bit OS
 *     EST - Enhanced Speed Test
 *     FMA3 - Fused Multiply/Add 3
 *     FMA4 - Fused Multiply/Add 4
 *     FPU - Floating-Point Unit on-chip
 *     FXSR - Fast Floating Point Save & Restore
 *     HTT - Hyper Threading Technology
 *     L1 - L1 Context ID
 *     MCA - Machine Check Architecture
 *     MCE - Machine Check Exception
 *     MMX - MultiMedia eXtension
 *     MSR - Model Specific Registers
 *     MTRR - Memory type Range Registers
 *     MW - Mwait instruction
 *     PAE - Physical Address Extension
 *     PAT - Page Attribute Table
 *     PBE - Pend Break Enabled
 *     PGE - Page Global Enable
 *     PSE - Page Size Extension
 *     PSE36 - 36bit Page Size Extension
 *     PSN - Processor Serial Number
 *     RDRAND - RdRand instruction
 *     SEP - Fast System Call
 *     SHA - Secure Hash Algorithm
 *     SS - Self Snoop
 *     SSE - Streaming SIMD Extension 1
 *     SSE2 - Streaming SIMD Extension 2
 *     SSE3 - Streaming SIMD Extension 3
 *     SSE41 - Streaming SIMD Extensions 4.1
 *     SSE42 - Streaming SIMD Extensions 4.2
 *     SSE4a - Streaming SIMD Extensions 4a
 *     SSSE3 - Supplemental Streaming SIMD Extension 3
 *     TM - Thermal Monitor
 *     TM2 - Thermal Monitor 2
 *     TSC - Time Stamp Counter
 *     VME - Virtual Mode Extension
 *     VMX - VMX
 *     XOP - eXtended Operations
 */

} // namespace Common
} // namespace NFE