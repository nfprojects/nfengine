/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation.
 */

#include "PCH.hpp"
#include "SystemInfo.hpp"
#include "SystemInfoConstants.hpp"
#include "../Utils/StringUtils.hpp"


namespace NFE {
namespace Common {

SystemInfo::SystemInfo()
{
    InitCPUInfoCommon();
    InitCPUInfoPlatform();
    InitOSVersion();
    InitCompilerInfo();
    InitMemoryInfo();
    InitMap();
}

SystemInfo& SystemInfo::Instance()
{
    static SystemInfo instance;
    return instance;
}

void SystemInfo::InitCPUInfoCommon()
{
    // Get extended ids.
    int CPUInfo[4] = { -1 };
    Cpuid(CPUInfo, EXTENDED_ID_MAX_VALUE);
    unsigned int nExIds = CPUInfo[0];

    // Get features and exfeatures
    Cpuid(CPUInfo, CPUID_FEATURES_1_2);
    mCpuidFeatures[0] = CPUInfo[3];
    mCpuidFeatures[1] = CPUInfo[2];
    if (nExIds >= CPUID_FEATURES_3)
    {
        Cpuid(CPUInfo, CPUID_FEATURES_3);
        mCpuidFeatures[2] = CPUInfo[1];
    }
    if (nExIds >= CPUID_FEATURES_4_5)
    {
        Cpuid(CPUInfo, CPUID_FEATURES_4_5);
        mCpuidFeatures[3] = CPUInfo[2];
        mCpuidFeatures[4] = CPUInfo[3];
    }

    // Get the information associated with each extended ID.
    char CPUBrandString[CPU_BRAND_STRING_LENGTH] = { 0 };
    for (unsigned int i = CPU_BRAND_STRING_1; i <= nExIds; ++i)
    {
        Cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if (i == CPU_BRAND_STRING_1)
        {
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        }
        else if (i == CPU_BRAND_STRING_2)
        {
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        }
        else if (i == CPU_BRAND_STRING_3)
        {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
    }
    mCPUBrand = CPUBrandString;
    // Intel is known for a long space before CPU name
    {
        const StringView whiteCharacters(" \t");
        const uint32 strBegin = mCPUBrand.ToView().FindFirstNotOf(whiteCharacters);
        if (strBegin != StringView::END())
            mCPUBrand.Erase(0, strBegin);
    }

    // Get cache line size
    Cpuid(CPUInfo, CPU_CACHE_LINE_SIZE);
    mCacheLineSize = CPUInfo[2] & 0xFF;
}

void SystemInfo::InitCompilerInfo()
{
#ifdef _MSC_VER
    mCompilerInfo = "MSVC++ ";

    // All versions below the newest one can be calculated
    if (_MSC_VER == 1900)
        mCompilerInfo += "14";
    else
        mCompilerInfo += ToString((_MSC_VER / 100) - 6);
#elif defined NFE_COMPILER
    mCompilerInfo = NFE_COMPILER;
#endif
}

void SystemInfo::InitMap()
{
    mCpuidFeatureMap.Insert("FPU",   CpuidFeature(0, 1<< 0)); // Floating-Point Unit on-chip
    mCpuidFeatureMap.Insert("VME",   CpuidFeature(0, 1<< 1)); // Virtual Mode Extension
    mCpuidFeatureMap.Insert("DE",    CpuidFeature(0, 1<< 2)); // Debugging Extension
    mCpuidFeatureMap.Insert("PSE",   CpuidFeature(0, 1<< 3)); // Page Size Extension
    mCpuidFeatureMap.Insert("TSC",   CpuidFeature(0, 1<< 4)); // Time Stamp Counter
    mCpuidFeatureMap.Insert("MSR",   CpuidFeature(0, 1<< 5)); // Model Specific Registers
    mCpuidFeatureMap.Insert("PAE",   CpuidFeature(0, 1<< 6)); // Physical Address Extension
    mCpuidFeatureMap.Insert("MCE",   CpuidFeature(0, 1<< 7)); // Machine Check Exception
    mCpuidFeatureMap.Insert("CX8",   CpuidFeature(0, 1<< 8)); // CMPXCHG8 Instruction
    mCpuidFeatureMap.Insert("APIC",  CpuidFeature(0, 1<< 9)); // On-chip APIC hardware
    mCpuidFeatureMap.Insert("SEP",   CpuidFeature(0, 1<<11)); // Fast System Call
    mCpuidFeatureMap.Insert("MTRR",  CpuidFeature(0, 1<<12)); // Memory type Range Registers
    mCpuidFeatureMap.Insert("PGE",   CpuidFeature(0, 1<<13)); // Page Global Enable
    mCpuidFeatureMap.Insert("MCA",   CpuidFeature(0, 1<<14)); // Machine Check Architecture
    mCpuidFeatureMap.Insert("CMOV",  CpuidFeature(0, 1<<15)); // Conditional MOVe Instruction
    mCpuidFeatureMap.Insert("PAT",   CpuidFeature(0, 1<<16)); // Page Attribute Table
    mCpuidFeatureMap.Insert("PSE36", CpuidFeature(0, 1<<17)); // 36bit Page Size Extension
    mCpuidFeatureMap.Insert("PSN",   CpuidFeature(0, 1<<18)); // Processor Serial Number
    mCpuidFeatureMap.Insert("CLFSH", CpuidFeature(0, 1<<19)); // CFLUSH Instruction
    mCpuidFeatureMap.Insert("DS",    CpuidFeature(0, 1<<21)); // Debug Store
    mCpuidFeatureMap.Insert("ACPI",  CpuidFeature(0, 1<<22)); // Thermal Monitor & Software Controlled Clock
    mCpuidFeatureMap.Insert("MMX",   CpuidFeature(0, 1<<23)); // MultiMedia eXtension
    mCpuidFeatureMap.Insert("FXSR",  CpuidFeature(0, 1<<24)); // Fast Floating Point Save & Restore
    mCpuidFeatureMap.Insert("SSE",   CpuidFeature(0, 1<<25)); // Streaming SIMD Extension 1
    mCpuidFeatureMap.Insert("SSE2",  CpuidFeature(0, 1<<26)); // Streaming SIMD Extension 2
    mCpuidFeatureMap.Insert("SS",    CpuidFeature(0, 1<<27)); // Self Snoop
    mCpuidFeatureMap.Insert("HTT",   CpuidFeature(0, 1<<28)); // Hyper Threading Technology
    mCpuidFeatureMap.Insert("TM",    CpuidFeature(0, 1<<29)); // Thermal Monitor
    mCpuidFeatureMap.Insert("PBE",   CpuidFeature(0, 1<<31)); // Pend Break Enabled

    mCpuidFeatureMap.Insert("SSE3",  CpuidFeature(1, 1<< 0)); // Streaming SIMD Extension 3
    mCpuidFeatureMap.Insert("MW",    CpuidFeature(1, 1<< 3)); // Mwait instruction
    mCpuidFeatureMap.Insert("CPL",   CpuidFeature(1, 1<< 4)); // CPL-qualified Debug Store
    mCpuidFeatureMap.Insert("VMX",   CpuidFeature(1, 1<< 5)); // VMX
    mCpuidFeatureMap.Insert("EST",   CpuidFeature(1, 1<< 7)); // Enhanced Speed Test
    mCpuidFeatureMap.Insert("TM2",   CpuidFeature(1, 1<< 8)); // Thermal Monitor 2
    mCpuidFeatureMap.Insert("SSSE3", CpuidFeature(1, 1<< 9)); // Supplemental Streaming SIMD Extension 3
    mCpuidFeatureMap.Insert("L1",    CpuidFeature(1, 1<<10)); // L1 Context ID
    mCpuidFeatureMap.Insert("FMA3",  CpuidFeature(1, 1<<12)); // Fused Multiply/Add 3
    mCpuidFeatureMap.Insert("CAE",   CpuidFeature(1, 1<<13)); // CompareAndExchange 16B
    mCpuidFeatureMap.Insert("SSE41", CpuidFeature(1, 1<<19)); // Streaming SIMD Extensions 4.1
    mCpuidFeatureMap.Insert("SSE42", CpuidFeature(1, 1<<20)); // Streaming SIMD Extensions 4.2
    mCpuidFeatureMap.Insert("AES",   CpuidFeature(1, 1<<25)); // Advanced Encryption Standard
    mCpuidFeatureMap.Insert("AVX",   CpuidFeature(1, 1<<28)); // Advanced Vector Extensions
    mCpuidFeatureMap.Insert("RDRAND",CpuidFeature(1, 1<<30)); // RdRand instruction

    mCpuidFeatureMap.Insert("AVX2",  CpuidFeature(2, 1<< 2)); // Advanced Vector Extensions 2
    mCpuidFeatureMap.Insert("BMI1",  CpuidFeature(2, 1<< 3)); // Bit Manipulation Instruction set 1
    mCpuidFeatureMap.Insert("BMI2",  CpuidFeature(2, 1<< 8)); // Bit Manipulation Instruction set 2
    mCpuidFeatureMap.Insert("ADX",   CpuidFeature(2, 1<<19)); // Multi-Precision Add-Carry instruction Extensions
    mCpuidFeatureMap.Insert("SHA",   CpuidFeature(2, 1<<29)); // Secure Hash Algorithm

    mCpuidFeatureMap.Insert("ABM",   CpuidFeature(3, 1<< 5)); // Advanced Bit Manipulation
    mCpuidFeatureMap.Insert("SSE4a", CpuidFeature(3, 1<< 6)); // Streaming SIMD Extensions 4a
    mCpuidFeatureMap.Insert("XOP",   CpuidFeature(3, 1<<11)); // eXtended Operations
    mCpuidFeatureMap.Insert("FMA4",  CpuidFeature(3, 1<<16)); // Fused Multiply/Add 4
    mCpuidFeatureMap.Insert("EM64T", CpuidFeature(4, 1<<29)); // Support for 64bit OS
}

String SystemInfo::ConstructAllInfoString()
{
    // Building output string
    String outputStr;
    outputStr.Reserve(2048);

    outputStr += "\nSYSTEM INFORMATION:\n";

    outputStr += "..::CPU::..\n";
    outputStr += "Brand:           " + mCPUBrand + '\n';
    outputStr += "CPU cores no.:   " + ToString(mCPUCoreNo) + '\n';
    outputStr += "Page size:       " + ToString(mPageSize) + " bytes\n";
    outputStr += "Cache line size: " + ToString(mCacheLineSize) + "bytes\n";

    outputStr += "\n..::MEMORY::..\n";
    outputStr += "Free (total):         " + ToString(GetFreeMemoryKb()) + " kB\n";
    outputStr += "Physical (total):     " + ToString(mMemTotalPhysKb) + " kB\n";
    outputStr += "Physical (available): " + ToString(mMemFreePhysKb) + " kB\n";
    outputStr += "Swap (total):         " + ToString(mMemTotalSwapKb) + " kB\n";
    outputStr += "Swap (available):     " + ToString(mMemFreeSwapKb) + " kB\n";
    outputStr += "Virtual (total):      " + ToString(mMemTotalVirtKb) + " kB\n";
    outputStr += "Virtual (available):  " + ToString(mMemFreeVirtKb) + " kB\n";

    outputStr += "\n..::Processor features::..\n";
    // iterate features map and print to the output string
    for (auto const &it : mCpuidFeatureMap)
    {
        outputStr += it.first + (CheckFeature(it.second) ? " supported\n" : " NOT supported\n");
    }

    outputStr += '\n';
    return outputStr;
}

bool SystemInfo::CheckFeature(CpuidFeature feature) const
{
    return (mCpuidFeatures[feature.AddressNo] & feature.FeatureNo) != 0;
}

bool SystemInfo::IsFeatureSupported(const StringView featureName) const
{
    const auto it = mCpuidFeatureMap.Find(featureName);
    if (it != mCpuidFeatureMap.End())
        return CheckFeature(it->second);
    return false;
}

const String& SystemInfo::GetCPUBrand() const
{
    return mCPUBrand;
}

const String& SystemInfo::GetOSVersion() const
{
    return mOSVersion;
}

const String& SystemInfo::GetCompilerInfo() const
{
    return mCompilerInfo;
}

uint64 SystemInfo::GetCPUCoreNo() const
{
    return mCPUCoreNo;
}

uint64 SystemInfo::GetPageSize() const
{
    return mPageSize;
}

uint64 SystemInfo::GetCacheLineSize() const
{
    return mCacheLineSize;
}

uint64 SystemInfo::GetMemTotalPhysKb() const
{
    return mMemTotalPhysKb;
}

uint64 SystemInfo::GetMemTotalVirtKb() const
{
    return mMemTotalVirtKb;
}

uint64 SystemInfo::GetMemTotalSwapKb() const
{
    return mMemTotalSwapKb;
}

} // namespace Common
} // namespace NFE
