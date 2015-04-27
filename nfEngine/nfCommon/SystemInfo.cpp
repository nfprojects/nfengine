/**
 * @file   SystemInfo.cpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  System information API implementation.
 */

#include "stdafx.hpp"
#include "SystemInfo.hpp"
#include <sstream>

namespace NFE {
namespace Common {

SystemInfo* SystemInfo::mInstance = 0;

SystemInfo::SystemInfo()
{
    InitCPUInfoCommon();
    InitCPUInfoPlatform();
    InitMemoryInfo();
}

SystemInfo* SystemInfo::Instance()
{
    if (!mInstance)
        mInstance = new SystemInfo;
    return mInstance;
}

void SystemInfo::InitCPUInfoCommon()
{
    // const SystemInfo::Get extended ids.
    int CPUInfo[4] = { -1 };
    Cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get features and exfeatures
    Cpuid(CPUInfo, 1);
    mCpuidFeatures = CPUInfo[3];
    mCpuidExFeatures = CPUInfo[2];

    // const SystemInfo::Get the information associated with each extended ID.
    char CPUBrandString[0x40] = { 0 };
    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        Cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if (i == 0x80000002)
        {
            memcpy(CPUBrandString,
                CPUInfo,
                sizeof(CPUInfo));
        }
        else if (i == 0x80000003)
        {
            memcpy(CPUBrandString + 16,
                CPUInfo,
                sizeof(CPUInfo));
        }
        else if (i == 0x80000004)
        {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
    }
    mCPUBrand.assign(CPUBrandString);

    //get cache line size
    unsigned int line_size = 0;
    Cpuid(CPUInfo, 0x80000006);
    mCacheLineSize = CPUInfo[2] & 0xFF;

    // check CPU frequency
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

std::string const SystemInfo::GetAllInfoString()
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
            sStream << std::setw(13) << std::right;
            sStream << (value.compare("True") ? "NOT supported" : "supported") << std::endl;
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
    auto b2s = [](bool x) {return x ? "True" : "False"; };
    outputStr.append("\n..::Processor features::..\n");
    outputStr.append(cpuInfoPrinter(4, "FPU", b2s(IsFeatureSupported(CpuidFeatures::FPU)), ""));
    outputStr.append(cpuInfoPrinter(4, "VME", b2s(IsFeatureSupported(CpuidFeatures::VME)), ""));
    outputStr.append(cpuInfoPrinter(4, "DE", b2s(IsFeatureSupported(CpuidFeatures::DE)), ""));
    outputStr.append(cpuInfoPrinter(4, "PSE", b2s(IsFeatureSupported(CpuidFeatures::PSE)), ""));
    outputStr.append(cpuInfoPrinter(4, "TSC", b2s(IsFeatureSupported(CpuidFeatures::TSC)), ""));
    outputStr.append(cpuInfoPrinter(4, "MSR", b2s(IsFeatureSupported(CpuidFeatures::MSR)), ""));
    outputStr.append(cpuInfoPrinter(4, "PAE", b2s(IsFeatureSupported(CpuidFeatures::PAE)), ""));
    outputStr.append(cpuInfoPrinter(4, "MCE", b2s(IsFeatureSupported(CpuidFeatures::MCE)), ""));
    outputStr.append(cpuInfoPrinter(4, "CX8", b2s(IsFeatureSupported(CpuidFeatures::CX8)), ""));
    outputStr.append(cpuInfoPrinter(4, "APIC", b2s(IsFeatureSupported(CpuidFeatures::APIC)), ""));
    outputStr.append(cpuInfoPrinter(4, "SEP", b2s(IsFeatureSupported(CpuidFeatures::SEP)), ""));
    outputStr.append(cpuInfoPrinter(4, "MTRR", b2s(IsFeatureSupported(CpuidFeatures::MTRR)), ""));
    outputStr.append(cpuInfoPrinter(4, "PGE", b2s(IsFeatureSupported(CpuidFeatures::PGE)), ""));
    outputStr.append(cpuInfoPrinter(4, "MCA", b2s(IsFeatureSupported(CpuidFeatures::MCA)), ""));
    outputStr.append(cpuInfoPrinter(4, "CMOV", b2s(IsFeatureSupported(CpuidFeatures::CMOV)), ""));
    outputStr.append(cpuInfoPrinter(4, "PAT", b2s(IsFeatureSupported(CpuidFeatures::PAT)), ""));
    outputStr.append(cpuInfoPrinter(4, "PSE36", b2s(IsFeatureSupported(CpuidFeatures::PSE36)), ""));
    outputStr.append(cpuInfoPrinter(4, "PSN", b2s(IsFeatureSupported(CpuidFeatures::PSN)), ""));
    outputStr.append(cpuInfoPrinter(4, "CLFSH", b2s(IsFeatureSupported(CpuidFeatures::CLFSH)), ""));
    outputStr.append(cpuInfoPrinter(4, "DS", b2s(IsFeatureSupported(CpuidFeatures::DS)), ""));
    outputStr.append(cpuInfoPrinter(4, "ACPI", b2s(IsFeatureSupported(CpuidFeatures::ACPI)), ""));
    outputStr.append(cpuInfoPrinter(4, "MMX", b2s(IsFeatureSupported(CpuidFeatures::MMX)), ""));
    outputStr.append(cpuInfoPrinter(4, "FXSR", b2s(IsFeatureSupported(CpuidFeatures::FXSR)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE", b2s(IsFeatureSupported(CpuidFeatures::SSE)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE2", b2s(IsFeatureSupported(CpuidFeatures::SSE2)), ""));
    outputStr.append(cpuInfoPrinter(4, "SS", b2s(IsFeatureSupported(CpuidFeatures::SS)), ""));
    outputStr.append(cpuInfoPrinter(4, "HTT", b2s(IsFeatureSupported(CpuidFeatures::HTT)), ""));
    outputStr.append(cpuInfoPrinter(4, "TM", b2s(IsFeatureSupported(CpuidFeatures::TM)), ""));
    outputStr.append(cpuInfoPrinter(4, "SSE3", b2s(IsFeatureSupported(CpuidExFeatures::SSE3)), ""));
    outputStr.append(cpuInfoPrinter(4, "MW", b2s(IsFeatureSupported(CpuidExFeatures::MW)), ""));
    outputStr.append(cpuInfoPrinter(4, "CPL", b2s(IsFeatureSupported(CpuidExFeatures::CPL)), ""));
    outputStr.append(cpuInfoPrinter(4, "VMX", b2s(IsFeatureSupported(CpuidExFeatures::VMX)), ""));
    outputStr.append(cpuInfoPrinter(4, "EST", b2s(IsFeatureSupported(CpuidExFeatures::EST)), ""));
    outputStr.append(cpuInfoPrinter(4, "TM2", b2s(IsFeatureSupported(CpuidExFeatures::TM2)), ""));
    outputStr.append(cpuInfoPrinter(4, "L1", b2s(IsFeatureSupported(CpuidExFeatures::L1)), ""));
    outputStr.append(cpuInfoPrinter(4, "CAE", b2s(IsFeatureSupported(CpuidExFeatures::CAE)), ""));

    outputStr.append("\n");
    return outputStr;
}

bool SystemInfo::IsFeatureSupported(CpuidFeatures feature)
{
    return static_cast<bool>(mCpuidFeatures & feature);
}

bool SystemInfo::IsFeatureSupported(CpuidExFeatures feature)
{
    return static_cast<bool>(mCpuidExFeatures & feature);
}

std::string const SystemInfo::GetCPUBrand()
{
    return mCPUBrand;
}

uint64_t const SystemInfo::GetCPUCoreNo()
{
    return mCPUCoreNo;
}

uint64_t const SystemInfo::GetPageSize()
{
    return mPageSize;
}

uint64_t const SystemInfo::GetCacheLineSize()
{
    return mCacheLineSize;
}

uint64_t const SystemInfo::GetCPUSpeedMHz()
{
    return mCPUSpeedMHz;
}

uint64_t const SystemInfo::GetMemTotalPhysKb()
{
    return mMemTotalPhysKb;
}

uint64_t const SystemInfo::GetMemTotalVirtKb()
{
    return mMemTotalVirtKb;
}

uint64_t const SystemInfo::GetMemFreePhysKb()
{
    return mMemFreePhysKb;
}

uint64_t const SystemInfo::GetMemFreeVirtKb()
{
    return mMemFreeVirtKb;
}

} // namespace Common
} // namespace NFE