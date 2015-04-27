/**
 * @file   SystemInfo.cpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API implementation
 */

#include "stdafx.hpp"
#include "SystemInfo.hpp"
#include <sstream>

namespace NFE {
    namespace Common {

SystemInfo::SystemInfo()
{
    GetCPUInfoCommon();
    GetCPUInfoPlatform();
    GetMemoryInfo();
}

void SystemInfo::GetCPUInfoCommon()
{
    // SystemInfo::Get extended ids.
    int CPUInfo[4] = { -1 };
    Cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get features and exfeatures
    Cpuid(CPUInfo, 1);
    mCpuidFeatures = CPUInfo[3];
    mCpuidExFeatures = CPUInfo[2];

    // SystemInfo::Get the information associated with each extended ID.
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

std::string SystemInfo::GetAllInfo()
{
    std::stringstream allInfoString;
    allInfoString << std::endl << "SYSTEM INFORMATION:" << std::endl;

    allInfoString << std::endl << "..::CPU::.." << std::endl;
    allInfoString << "Brand           = " << mCPUBrand << std::endl;
    allInfoString << "Frequency       = " << mCPUSpeedMHz << " MHz\n";
    allInfoString << "CPU cores no.   = " << mCPUCoreNo << std::endl;
    allInfoString << "Page size       = " << mPageSize << " B\n";
    allInfoString << "Cache line size = " << mCacheLineSize << " B\n";

    allInfoString << std::endl << "..::MEMORY::.." << std::endl;
    allInfoString << "Free     memory total=\t" << GetFreeMemoryKb() << " kB\n";
    allInfoString << "Physical memory total=\t" << mMemTotalPhysKb << " kB\n";
    allInfoString << "Physical memory avail=\t" << mMemFreePhysKb << " kB\n";
    allInfoString << "Virtual  memory total=\t" << mMemTotalVirtKb << " kB\n";
    allInfoString << "Virtual  memory avail=\t" << mMemFreeVirtKb << " kB\n";

    allInfoString << std::endl << "..::Processor features::.." << std::endl;
    allInfoString << "FPU\t" << (IsFeatureSupported(CpuidFeatures::FPU) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "VME\t" << (IsFeatureSupported(CpuidFeatures::VME) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "DE\t" << (IsFeatureSupported(CpuidFeatures::DE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "PSE\t" << (IsFeatureSupported(CpuidFeatures::PSE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "TSC\t" << (IsFeatureSupported(CpuidFeatures::TSC) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "MSR\t" << (IsFeatureSupported(CpuidFeatures::MSR) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "PAE\t" << (IsFeatureSupported(CpuidFeatures::PAE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "MCE\t" << (IsFeatureSupported(CpuidFeatures::MCE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "CX8\t" << (IsFeatureSupported(CpuidFeatures::CX8) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "APIC\t" << (IsFeatureSupported(CpuidFeatures::APIC) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "SEP\t" << (IsFeatureSupported(CpuidFeatures::SEP) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "MTRR\t" << (IsFeatureSupported(CpuidFeatures::MTRR) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "PGE\t" << (IsFeatureSupported(CpuidFeatures::PGE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "MCA\t" << (IsFeatureSupported(CpuidFeatures::MCA) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "CMOV\t" << (IsFeatureSupported(CpuidFeatures::CMOV) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "PAT\t" << (IsFeatureSupported(CpuidFeatures::PAT) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "PSE36\t" << (IsFeatureSupported(CpuidFeatures::PSE36) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "PSN\t" << (IsFeatureSupported(CpuidFeatures::PSN) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "CLFSH\t" << (IsFeatureSupported(CpuidFeatures::CLFSH) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "DS\t" << (IsFeatureSupported(CpuidFeatures::DS) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "ACPI\t" << (IsFeatureSupported(CpuidFeatures::ACPI) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "MMX\t" << (IsFeatureSupported(CpuidFeatures::MMX) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "FXSR\t" << (IsFeatureSupported(CpuidFeatures::FXSR) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "SSE\t" << (IsFeatureSupported(CpuidFeatures::SSE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "SSE2\t" << (IsFeatureSupported(CpuidFeatures::SSE2) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "SS\t" << (IsFeatureSupported(CpuidFeatures::SS) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "HTT\t" << (IsFeatureSupported(CpuidFeatures::HTT) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "TM\t" << (IsFeatureSupported(CpuidFeatures::TM) ? "    supported" : "NOT supported") << std::endl;
    //allInfoString << std::endl << "..::Processor extended features::.." << std::endl;
    allInfoString << "SSE3\t" << (IsFeatureSupported(CpuidExFeatures::SSE3) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "MW\t" << (IsFeatureSupported(CpuidExFeatures::MW) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "CPL\t" << (IsFeatureSupported(CpuidExFeatures::CPL) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "VMX\t" << (IsFeatureSupported(CpuidExFeatures::VMX) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "EST\t" << (IsFeatureSupported(CpuidExFeatures::EST) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "TM2\t" << (IsFeatureSupported(CpuidExFeatures::TM2) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "L1\t" << (IsFeatureSupported(CpuidExFeatures::L1) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << "CAE\t" << (IsFeatureSupported(CpuidExFeatures::CAE) ? "    supported" : "NOT supported") << std::endl;
    allInfoString << std::endl;

    return allInfoString.str();
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