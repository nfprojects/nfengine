/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation.
 */

#include "PCH.hpp"
#include "SystemInfo.hpp"
#include "SystemInfoConstants.hpp"
#include <sstream>

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
            memcpy(CPUBrandString,
                CPUInfo,
                sizeof(CPUInfo));
        }
        else if (i == CPU_BRAND_STRING_2)
        {
            memcpy(CPUBrandString + 16,
                CPUInfo,
                sizeof(CPUInfo));
        }
        else if (i == CPU_BRAND_STRING_3)
        {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
    }
    mCPUBrand.assign(CPUBrandString);
    // Intel is known for a long space before CPU name
    {
        const size_t strBegin = mCPUBrand.find_first_not_of(" \t");
        if (strBegin != std::string::npos)
            mCPUBrand.erase(0, strBegin);
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
        mCompilerInfo += std::to_string((_MSC_VER / 100) - 6);
#elif defined NFE_COMPILER
    mCompilerInfo = NFE_COMPILER;
#endif
}

void SystemInfo::InitMap()
{
    mCpuidFeatureMap["FPU"]   = CpuidFeature(0, 1<< 0); // Floating-Point Unit on-chip
    mCpuidFeatureMap["VME"]   = CpuidFeature(0, 1<< 1); // Virtual Mode Extension
    mCpuidFeatureMap["DE"]    = CpuidFeature(0, 1<< 2); // Debugging Extension
    mCpuidFeatureMap["PSE"]   = CpuidFeature(0, 1<< 3); // Page Size Extension
    mCpuidFeatureMap["TSC"]   = CpuidFeature(0, 1<< 4); // Time Stamp Counter
    mCpuidFeatureMap["MSR"]   = CpuidFeature(0, 1<< 5); // Model Specific Registers
    mCpuidFeatureMap["PAE"]   = CpuidFeature(0, 1<< 6); // Physical Address Extension
    mCpuidFeatureMap["MCE"]   = CpuidFeature(0, 1<< 7); // Machine Check Exception
    mCpuidFeatureMap["CX8"]   = CpuidFeature(0, 1<< 8); // CMPXCHG8 Instruction
    mCpuidFeatureMap["APIC"]  = CpuidFeature(0, 1<< 9); // On-chip APIC hardware
    mCpuidFeatureMap["SEP"]   = CpuidFeature(0, 1<<11); // Fast System Call
    mCpuidFeatureMap["MTRR"]  = CpuidFeature(0, 1<<12); // Memory type Range Registers
    mCpuidFeatureMap["PGE"]   = CpuidFeature(0, 1<<13); // Page Global Enable
    mCpuidFeatureMap["MCA"]   = CpuidFeature(0, 1<<14); // Machine Check Architecture
    mCpuidFeatureMap["CMOV"]  = CpuidFeature(0, 1<<15); // Conditional MOVe Instruction
    mCpuidFeatureMap["PAT"]   = CpuidFeature(0, 1<<16); // Page Attribute Table
    mCpuidFeatureMap["PSE36"] = CpuidFeature(0, 1<<17); // 36bit Page Size Extension
    mCpuidFeatureMap["PSN"]   = CpuidFeature(0, 1<<18); // Processor Serial Number
    mCpuidFeatureMap["CLFSH"] = CpuidFeature(0, 1<<19); // CFLUSH Instruction
    mCpuidFeatureMap["DS"]    = CpuidFeature(0, 1<<21); // Debug Store
    mCpuidFeatureMap["ACPI"]  = CpuidFeature(0, 1<<22); // Thermal Monitor & Software Controlled Clock
    mCpuidFeatureMap["MMX"]   = CpuidFeature(0, 1<<23); // MultiMedia eXtension
    mCpuidFeatureMap["FXSR"]  = CpuidFeature(0, 1<<24); // Fast Floating Point Save & Restore
    mCpuidFeatureMap["SSE"]   = CpuidFeature(0, 1<<25); // Streaming SIMD Extension 1
    mCpuidFeatureMap["SSE2"]  = CpuidFeature(0, 1<<26); // Streaming SIMD Extension 2
    mCpuidFeatureMap["SS"]    = CpuidFeature(0, 1<<27); // Self Snoop
    mCpuidFeatureMap["HTT"]   = CpuidFeature(0, 1<<28); // Hyper Threading Technology
    mCpuidFeatureMap["TM"]    = CpuidFeature(0, 1<<29); // Thermal Monitor
    mCpuidFeatureMap["PBE"]   = CpuidFeature(0, 1<<31); // Pend Break Enabled

    mCpuidFeatureMap["SSE3"]  = CpuidFeature(1, 1<< 0); // Streaming SIMD Extension 3
    mCpuidFeatureMap["MW"]    = CpuidFeature(1, 1<< 3); // Mwait instruction
    mCpuidFeatureMap["CPL"]   = CpuidFeature(1, 1<< 4); // CPL-qualified Debug Store
    mCpuidFeatureMap["VMX"]   = CpuidFeature(1, 1<< 5); // VMX
    mCpuidFeatureMap["EST"]   = CpuidFeature(1, 1<< 7); // Enhanced Speed Test
    mCpuidFeatureMap["TM2"]   = CpuidFeature(1, 1<< 8); // Thermal Monitor 2
    mCpuidFeatureMap["SSSE3"] = CpuidFeature(1, 1<< 9); // Supplemental Streaming SIMD Extension 3
    mCpuidFeatureMap["L1"]    = CpuidFeature(1, 1<<10); // L1 Context ID
    mCpuidFeatureMap["FMA3"]  = CpuidFeature(1, 1<<12); // Fused Multiply/Add 3
    mCpuidFeatureMap["CAE"]   = CpuidFeature(1, 1<<13); // CompareAndExchange 16B
    mCpuidFeatureMap["SSE41"] = CpuidFeature(1, 1<<19); // Streaming SIMD Extensions 4.1
    mCpuidFeatureMap["SSE42"] = CpuidFeature(1, 1<<20); // Streaming SIMD Extensions 4.2
    mCpuidFeatureMap["AES"]   = CpuidFeature(1, 1<<25); // Advanced Encryption Standard
    mCpuidFeatureMap["AVX"]   = CpuidFeature(1, 1<<28); // Advanced Vector Extensions
    mCpuidFeatureMap["RDRAND"]= CpuidFeature(1, 1<<30); // RdRand instruction

    mCpuidFeatureMap["AVX2"]  = CpuidFeature(2, 1<< 2); // Advanced Vector Extensions 2
    mCpuidFeatureMap["BMI1"]  = CpuidFeature(2, 1<< 3); // Bit Manipulation Instruction set 1
    mCpuidFeatureMap["BMI2"]  = CpuidFeature(2, 1<< 8); // Bit Manipulation Instruction set 2
    mCpuidFeatureMap["ADX"]   = CpuidFeature(2, 1<<19); // Multi-Precision Add-Carry instruction Extensions
    mCpuidFeatureMap["SHA"]   = CpuidFeature(2, 1<<29); // Secure Hash Algorithm

    mCpuidFeatureMap["ABM"]   = CpuidFeature(3, 1<< 5); // Advanced Bit Manipulation
    mCpuidFeatureMap["SSE4a"] = CpuidFeature(3, 1<< 6); // Streaming SIMD Extensions 4a
    mCpuidFeatureMap["XOP"]   = CpuidFeature(3, 1<<11); // eXtended Operations
    mCpuidFeatureMap["FMA4"]  = CpuidFeature(3, 1<<16); // Fused Multiply/Add 4
    mCpuidFeatureMap["EM64T"] = CpuidFeature(4, 1<<29); // Support for 64bit OS
}

std::string SystemInfo::ConstructAllInfoString()
{
    // Lambda used for printing. Got 3 modes for our purposes only.
    auto cpuInfoPrinter = [](int mode, std::string name, std::string value, std::string units)
    {
        std::stringstream sStream;
        if (mode == 1)
        {
            sStream << std::setw(16) << std::left << name;
            sStream << "= " << value << " " << units << std::endl;
        }
        else if (mode == 2)
        {
            sStream << std::setw(9) << std::left << name;
            sStream << "memory " << units << "=";
            sStream << std::setw(12) << std::right << value;
            sStream << " kB\n";
        }
        else
        {
            sStream << std::setw(9) << std::left << name;
            sStream << std::setw(13) << std::right << value << std::endl;
        }

        return sStream.str();
    };

    // Building output string
    std::string outputStr;
    outputStr.append("\nSYSTEM INFORMATION:\n");

    outputStr.append("..::CPU::..\n");
    outputStr.append(cpuInfoPrinter(1, "Brand", mCPUBrand, ""));
    outputStr.append(cpuInfoPrinter(1, "CPU cores no.", std::to_string(mCPUCoreNo), ""));
    outputStr.append(cpuInfoPrinter(1, "Page size", std::to_string(mPageSize), "B"));
    outputStr.append(cpuInfoPrinter(1, "Cache line size", std::to_string(mCacheLineSize), "B"));

    outputStr.append("\n..::MEMORY::..\n");
    outputStr.append(cpuInfoPrinter(2, "Free", std::to_string(GetFreeMemoryKb()), "total"));
    outputStr.append(cpuInfoPrinter(2, "Physical", std::to_string(mMemTotalPhysKb), "total"));
    outputStr.append(cpuInfoPrinter(2, "Physical", std::to_string(mMemFreePhysKb), "avail"));
    outputStr.append(cpuInfoPrinter(2, "Swap", std::to_string(mMemTotalSwapKb), "total"));
    outputStr.append(cpuInfoPrinter(2, "Swap", std::to_string(mMemFreeSwapKb), "avail"));
    outputStr.append(cpuInfoPrinter(2, "Virtual", std::to_string(mMemTotalVirtKb), "total"));
    outputStr.append(cpuInfoPrinter(2, "Virtual", std::to_string(mMemFreeVirtKb), "avail"));

    // cast boolean to string lambda
    auto b2s = [](bool x){return x ? "supported" : "NOT supported"; };

    outputStr.append("\n..::Processor features::..\n");
    // iterate features map and print to the output string
    for(auto const &it : mCpuidFeatureMap)
        outputStr.append(cpuInfoPrinter(4, it.first, b2s(CheckFeature(it.second)), ""));

    outputStr.append("\n");
    return outputStr;
}

bool SystemInfo::CheckFeature(CpuidFeature feature) const
{
    return (mCpuidFeatures[feature.AddressNo] & feature.FeatureNo) != 0;
}

bool SystemInfo::IsFeatureSupported(const std::string& featureName) const
{
    std::map<std::string, CpuidFeature>::const_iterator it = mCpuidFeatureMap.find(featureName);
    if (it != mCpuidFeatureMap.end())
        return CheckFeature(it->second);
    return false;
}

const std::string& SystemInfo::GetCPUBrand() const
{
    return mCPUBrand;
}

const std::string& SystemInfo::GetOSVersion() const
{
    return mOSVersion;
}

const std::string& SystemInfo::GetCompilerInfo() const
{
    return mCompilerInfo;
}

uint64_t SystemInfo::GetCPUCoreNo() const
{
    return mCPUCoreNo;
}

uint64_t SystemInfo::GetPageSize() const
{
    return mPageSize;
}

uint64_t SystemInfo::GetCacheLineSize() const
{
    return mCacheLineSize;
}

uint64_t SystemInfo::GetMemTotalPhysKb() const
{
    return mMemTotalPhysKb;
}

uint64_t SystemInfo::GetMemTotalVirtKb() const
{
    return mMemTotalVirtKb;
}

uint64_t SystemInfo::GetMemTotalSwapKb() const
{
    return mMemTotalSwapKb;
}
} // namespace Common
} // namespace NFE
