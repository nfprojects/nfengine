/**
 * @file   SystemInfo.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation.
 */

#include "stdafx.hpp"
#include "SystemInfo.hpp"
#include "SystemInfoConstants.hpp"
#include <sstream>

namespace NFE {
namespace Common {

std::unique_ptr<SystemInfo> SystemInfo::mInstance = 0;

SystemInfo::SystemInfo()
{
    InitCPUInfoCommon();
    InitCPUInfoPlatform();
    InitMemoryInfo();
}

SystemInfo* SystemInfo::Instance()
{
    // TODO: This is not thread safe.
    if (!mInstance)
        mInstance.reset(new SystemInfo);
    return mInstance.get();
}

void SystemInfo::InitCPUInfoCommon()
{
    // Get extended ids.
    int CPUInfo[4] = { -1 };
    Cpuid(CPUInfo, EXTENDED_ID_MAX_VALUE);
    unsigned int nExIds = CPUInfo[0];

    // Get features and exfeatures
    Cpuid(CPUInfo, CPUID_FEATURES_1_2);
    mCpuidFeatures1 = CPUInfo[3];
    mCpuidFeatures2 = CPUInfo[2];
    if (nExIds >= CPUID_FEATURES_3)
    {
        Cpuid(CPUInfo, CPUID_FEATURES_3);
        mCpuidFeatures3 = CPUInfo[1];
    }
    if (nExIds >= CPUID_FEATURES_4_5)
    {
        Cpuid(CPUInfo, CPUID_FEATURES_4_5);
        mCpuidFeatures4 = CPUInfo[2];
        mCpuidFeatures5 = CPUInfo[3];
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

    // Get cache line size
    unsigned int line_size = 0;
    Cpuid(CPUInfo, CPU_CACHE_LINE_SIZE);
    mCacheLineSize = CPUInfo[2] & 0xFF;

    // Check CPU frequency
    Timer timer;
    double timerResult = 0;
    timer.Start();
    uint64_t cyclesNo = Rdtsc();
    do
    {
        timerResult = timer.Stop();
    } while (timerResult < 1);
    mCPUSpeedMHz = (Rdtsc() - cyclesNo) / 1000000.0;
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
    outputStr.append(cpuInfoPrinter(1, "Frequency", std::to_string(mCPUSpeedMHz), "MHz"));
    outputStr.append(cpuInfoPrinter(1, "CPU cores no.", std::to_string(mCPUCoreNo), ""));
    outputStr.append(cpuInfoPrinter(1, "Page size", std::to_string(mPageSize), "B"));
    outputStr.append(cpuInfoPrinter(1, "Cache line size", std::to_string(mCacheLineSize), "B"));

    outputStr.append("\n..::MEMORY::..\n");
    outputStr.append(cpuInfoPrinter(2, "Free", std::to_string(GetFreeMemoryKb()), "total"));
    outputStr.append(cpuInfoPrinter(2, "Physical", std::to_string(mMemTotalPhysKb), "total"));
    outputStr.append(cpuInfoPrinter(2, "Physical", std::to_string(mMemFreePhysKb), "avail"));
    outputStr.append(cpuInfoPrinter(2, "Virtual", std::to_string(mMemTotalVirtKb), "total"));
    outputStr.append(cpuInfoPrinter(2, "Virtual", std::to_string(mMemFreeVirtKb), "avail"));

    // cast boolean to string lambda
    auto b2s = [](bool x){return x ? "supported" : "NOT supported"; };

    outputStr.append("\n..::Processor features::..\n");
    outputStr.append(cpuInfoPrinter(4, "FPU", b2s(IsFeatureSupported(CpuidFeatures1::FPU)), ""));
    outputStr.append(cpuInfoPrinter(4, "VME", b2s(IsFeatureSupported(CpuidFeatures1::VME)), ""));
    outputStr.append(cpuInfoPrinter(4, "DE", b2s(IsFeatureSupported(CpuidFeatures1::DE)), ""));
    outputStr.append(cpuInfoPrinter(4, "PSE", b2s(IsFeatureSupported(CpuidFeatures1::PSE)), ""));
    outputStr.append(cpuInfoPrinter(4, "TSC", b2s(IsFeatureSupported(CpuidFeatures1::TSC)), ""));
    outputStr.append(cpuInfoPrinter(4, "MSR", b2s(IsFeatureSupported(CpuidFeatures1::MSR)), ""));
    outputStr.append(cpuInfoPrinter(4, "PAE", b2s(IsFeatureSupported(CpuidFeatures1::PAE)), ""));
    outputStr.append(cpuInfoPrinter(4, "MCE", b2s(IsFeatureSupported(CpuidFeatures1::MCE)), ""));
    outputStr.append(cpuInfoPrinter(4, "CX8", b2s(IsFeatureSupported(CpuidFeatures1::CX8)), ""));
    outputStr.append(cpuInfoPrinter(4, "APIC", b2s(IsFeatureSupported(CpuidFeatures1::APIC)), ""));
    outputStr.append(cpuInfoPrinter(4, "SEP", b2s(IsFeatureSupported(CpuidFeatures1::SEP)), ""));
    outputStr.append(cpuInfoPrinter(4, "MTRR", b2s(IsFeatureSupported(CpuidFeatures1::MTRR)), ""));
    outputStr.append(cpuInfoPrinter(4, "PGE", b2s(IsFeatureSupported(CpuidFeatures1::PGE)), ""));
    outputStr.append(cpuInfoPrinter(4, "MCA", b2s(IsFeatureSupported(CpuidFeatures1::MCA)), ""));
    outputStr.append(cpuInfoPrinter(4, "CMOV", b2s(IsFeatureSupported(CpuidFeatures1::CMOV)), ""));
    outputStr.append(cpuInfoPrinter(4, "PAT", b2s(IsFeatureSupported(CpuidFeatures1::PAT)), ""));
    outputStr.append(cpuInfoPrinter(4, "PSE36", b2s(IsFeatureSupported(CpuidFeatures1::PSE36)), ""));
    outputStr.append(cpuInfoPrinter(4, "PSN", b2s(IsFeatureSupported(CpuidFeatures1::PSN)), ""));
    outputStr.append(cpuInfoPrinter(4, "CLFSH", b2s(IsFeatureSupported(CpuidFeatures1::CLFSH)), ""));
    outputStr.append(cpuInfoPrinter(4, "DS", b2s(IsFeatureSupported(CpuidFeatures1::DS)), ""));
    outputStr.append(cpuInfoPrinter(4, "ACPI", b2s(IsFeatureSupported(CpuidFeatures1::ACPI)), ""));
    outputStr.append(cpuInfoPrinter(4, "MMX", b2s(IsFeatureSupported(CpuidFeatures1::MMX)), ""));
    outputStr.append(cpuInfoPrinter(4, "FXSR", b2s(IsFeatureSupported(CpuidFeatures1::FXSR)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE", b2s(IsFeatureSupported(CpuidFeatures1::SSE)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE2", b2s(IsFeatureSupported(CpuidFeatures1::SSE2)), ""));
    outputStr.append(cpuInfoPrinter(4, "SS", b2s(IsFeatureSupported(CpuidFeatures1::SS)), ""));
    outputStr.append(cpuInfoPrinter(4, "HTT", b2s(IsFeatureSupported(CpuidFeatures1::HTT)), ""));
    outputStr.append(cpuInfoPrinter(4, "TM", b2s(IsFeatureSupported(CpuidFeatures1::TM)), ""));
    outputStr.append(cpuInfoPrinter(4, "PBE", b2s(IsFeatureSupported(CpuidFeatures1::PBE)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE3", b2s(IsFeatureSupported(CpuidFeatures2::SSE3)), ""));
    outputStr.append(cpuInfoPrinter(4, "MW", b2s(IsFeatureSupported(CpuidFeatures2::MW)), ""));
    outputStr.append(cpuInfoPrinter(4, "CPL", b2s(IsFeatureSupported(CpuidFeatures2::CPL)), ""));
    outputStr.append(cpuInfoPrinter(4, "VMX", b2s(IsFeatureSupported(CpuidFeatures2::VMX)), ""));
    outputStr.append(cpuInfoPrinter(4, "EST", b2s(IsFeatureSupported(CpuidFeatures2::EST)), ""));
    outputStr.append(cpuInfoPrinter(4, "TM2", b2s(IsFeatureSupported(CpuidFeatures2::TM2)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSSE3", b2s(IsFeatureSupported(CpuidFeatures2::SSSE3)), ""));
    outputStr.append(cpuInfoPrinter(4, "L1", b2s(IsFeatureSupported(CpuidFeatures2::L1)), ""));
    outputStr.append(cpuInfoPrinter(4, "FMA3", b2s(IsFeatureSupported(CpuidFeatures2::FMA3)), ""));
    outputStr.append(cpuInfoPrinter(4, "CAE", b2s(IsFeatureSupported(CpuidFeatures2::CAE)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE41", b2s(IsFeatureSupported(CpuidFeatures2::SSE41)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE42", b2s(IsFeatureSupported(CpuidFeatures2::SSE42)), ""));
    outputStr.append(cpuInfoPrinter(4, "AES", b2s(IsFeatureSupported(CpuidFeatures2::AES)), ""));
    outputStr.append(cpuInfoPrinter(4, "AVX", b2s(IsFeatureSupported(CpuidFeatures2::AVX)), ""));
    outputStr.append(cpuInfoPrinter(4, "RDRAND", b2s(IsFeatureSupported(CpuidFeatures2::RDRAND)), ""));
    outputStr.append(cpuInfoPrinter(4, "AVX2", b2s(IsFeatureSupported(CpuidFeatures3::AVX2)), ""));
    outputStr.append(cpuInfoPrinter(4, "BMI1", b2s(IsFeatureSupported(CpuidFeatures3::BMI1)), ""));
    outputStr.append(cpuInfoPrinter(4, "BMI2", b2s(IsFeatureSupported(CpuidFeatures3::BMI2)), ""));
    outputStr.append(cpuInfoPrinter(4, "ADX", b2s(IsFeatureSupported(CpuidFeatures3::ADX)), ""));
    outputStr.append(cpuInfoPrinter(4, "SHA", b2s(IsFeatureSupported(CpuidFeatures3::SHA)), ""));
    outputStr.append(cpuInfoPrinter(4, "ABM", b2s(IsFeatureSupported(CpuidFeatures4::ABM)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE4a", b2s(IsFeatureSupported(CpuidFeatures4::SSE4a)), ""));
    outputStr.append(cpuInfoPrinter(4, "XOP", b2s(IsFeatureSupported(CpuidFeatures4::XOP)), ""));
    outputStr.append(cpuInfoPrinter(4, "FMA4", b2s(IsFeatureSupported(CpuidFeatures4::FMA4)), ""));
    outputStr.append(cpuInfoPrinter(4, "EM64T", b2s(IsFeatureSupported(CpuidFeatures5::EM64T)), ""));

    outputStr.append("\n");
    return outputStr;
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures1 feature) const
{
    return static_cast<bool>(mCpuidFeatures1 & feature);
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures2 feature) const
{
    return static_cast<bool>(mCpuidFeatures2 & feature);
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures3 feature) const
{
    return static_cast<bool>(mCpuidFeatures3 & feature);
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures4 feature) const
{
    return static_cast<bool>(mCpuidFeatures4 & feature);
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures5 feature) const
{
    return static_cast<bool>(mCpuidFeatures5 & feature);
}

const std::string& SystemInfo::GetCPUBrand() const
{
    return mCPUBrand;
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

uint64_t SystemInfo::GetCPUSpeedMHz() const
{
    return mCPUSpeedMHz;
}

uint64_t SystemInfo::GetMemTotalPhysKb() const
{
    return mMemTotalPhysKb;
}

uint64_t SystemInfo::GetMemTotalVirtKb() const
{
    return mMemTotalVirtKb;
}

uint64_t SystemInfo::GetMemFreePhysKb() const
{
    return mMemFreePhysKb;
}

uint64_t SystemInfo::GetMemFreeVirtKb() const
{
    return mMemFreeVirtKb;
}

} // namespace Common
} // namespace NFE