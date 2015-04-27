/**
 * @file   SystemInfo.hpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API declarations.
 */

#pragma once

#include "nfCommon.hpp"
#include "../Timer.hpp"

#ifdef
#include "sys/sysinfo.h"
#include <unistd.h>
#endif


namespace NFE {
namespace Common {

enum CpuidFeatures
{
  FPU   = 1<< 0, // Floating-Point Unit on-chip
  VME   = 1<< 1, // Virtual Mode Extension
  DE    = 1<< 2, // Debugging Extension
  PSE   = 1<< 3, // Page Size Extension
  TSC   = 1<< 4, // Time Stamp Counter
  MSR   = 1<< 5, // Model Specific Registers
  PAE   = 1<< 6, // Physical Address Extension
  MCE   = 1<< 7, // Machine Check Exception
  CX8   = 1<< 8, // CMPXCHG8 Instruction
  APIC  = 1<< 9, // On-chip APIC hardware
  SEP   = 1<<11, // Fast System Call
  MTRR  = 1<<12, // Memory type Range Registers
  PGE   = 1<<13, // Page Global Enable
  MCA   = 1<<14, // Machine Check Architecture
  CMOV  = 1<<15, // Conditional MOVe Instruction
  PAT   = 1<<16, // Page Attribute Table
  PSE36 = 1<<17, // 36bit Page Size Extension
  PSN   = 1<<18, // Processor Serial Number
  CLFSH = 1<<19, // CFLUSH Instruction
  DS    = 1<<21, // Debug Store
  ACPI  = 1<<22, // Thermal Monitor & Software Controlled Clock
  MMX   = 1<<23, // MultiMedia eXtension
  FXSR  = 1<<24, // Fast Floating Point Save & Restore
  SSE   = 1<<25, // Streaming SIMD Extension 1
  SSE2  = 1<<26, // Streaming SIMD Extension 2
  SS    = 1<<27, // Self Snoop
  HTT   = 1<<28, // Hyper Threading Technology
  TM    = 1<<29, // Thermal Monitor
  PBE   = 1<<31, // Pend Break Enabled
};

enum CpuidExFeatures
{
  SSE3  = 1<< 0, // Streaming SIMD Extension 3
  MW    = 1<< 3, // Mwait instruction
  CPL   = 1<< 4, // CPL-qualified Debug Store
  VMX   = 1<< 5, // VMX
  EST   = 1<< 7, // Enhanced Speed Test
  TM2   = 1<< 8, // Thermal Monitor 2
  L1    = 1<<10, // L1 Context ID
  CAE   = 1<<13, // CompareAndExchange 16B
};

/**
 * Class used to gather information about hardware
 */
class NFCOMMON_API SystemInfo
{
private:
    std::string mCPUBrand;
    uint64_t mCPUCoreNo;
    uint64_t mPageSize;
    uint64_t mCacheLineSize;
    uint64_t mCPUSpeedMHz;
    uint64_t mMemTotalPhysKb;
    uint64_t mMemFreePhysKb;
    uint64_t mMemTotalVirtKb;
    uint64_t mMemFreeVirtKb;
    uint64_t mCpuidFeatures;
    uint64_t mCpuidExFeatures;

    void Cpuid(int cpuInfo[4], int function_id);
    uint64_t Rdtsc();

public:
    SystemInfo();

    /**
     * Update mMemFree fields
     * @return Total free memory in kb (physical + virtual)
     */
    uint64_t GetFreeMemoryKb();
    bool IsFeatureSupported(CpuidFeatures feature);
    bool IsFeatureSupported(CpuidExFeatures feature);
    /**
     * Get class fields
     */
    std::string GetCPUBrand();
    uint64_t GetCPUCoreNo();
    uint64_t GetPageSize();
    uint64_t GetCacheLineSize();
    uint64_t GetCPUSpeedMHz();
    uint64_t GetMemTotalPhysKb();
    uint64_t GetMemTotalVirtKb();
    uint64_t GetMemFreePhysKb();
    uint64_t GetMemFreeVirtKb();
};

} // namespace Common
} // namespace NFE