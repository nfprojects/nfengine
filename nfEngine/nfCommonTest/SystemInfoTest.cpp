#include "stdafx.hpp"
#include "../nfCommon/SystemInfo.hpp"

using namespace NFE::Common;

TEST(SystemInfoTest, RunTest)
{
    SystemInfo mSysInfo;
    std::cout << std::endl << "SYSTEM INFORMATION:" << std::endl;
    
    std::cout << std::endl << "..::CPU::.." << std::endl;
    std::cout << "Brand           = " << mSysInfo.GetCPUBrand() << std::endl;
    std::cout << "Frequency       = " << mSysInfo.GetCPUSpeedMHz() << " MHz\n";
    std::cout << "CPU cores no.   = " << mSysInfo.GetCPUCoreNo() << std::endl;
    std::cout << "Page size       = " << mSysInfo.GetPageSize() << " B\n";
    std::cout << "Cache line size = " << mSysInfo.GetCacheLineSize() << " B\n";
    
    std::cout << std::endl << "..::MEMORY::.." << std::endl;
    std::cout << "Physical memory total=\t" << mSysInfo.GetMemTotalPhysKb() << " kB\n";
    std::cout << "Physical memory avail=\t" << mSysInfo.GetMemFreePhysKb() << " kB\n";
    std::cout << "Virtual  memory total=\t" << mSysInfo.GetMemTotalVirtKb() << " kB\n";
    std::cout << "Virtual  memory avail=\t" << mSysInfo.GetMemFreeVirtKb() << " kB\n";
    std::cout << "Free     memory total=\t" << mSysInfo.GetFreeMemoryKb() << " kB\n";
    
    std::cout <<  std::endl << "..::Processor features::.." << std::endl;
    std::cout << "FPU\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::FPU) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "VME\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::VME) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "DE\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::DE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "PSE\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::PSE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "TSC\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::TSC) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "MSR\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::MSR) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "PAE\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::PAE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "MCE\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::MCE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "CX8\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::CX8) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "APIC\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::APIC) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "SEP\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::SEP) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "MTRR\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::MTRR) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "PGE\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::PGE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "MCA\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::MCA) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "CMOV\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::CMOV) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "PAT\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::PAT) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "PSE36\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::PSE36) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "PSN\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::PSN) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "CLFSH\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::CLFSH) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "DS\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::DS) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "ACPI\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::ACPI) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "MMX\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::MMX) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "FXSR\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::FXSR) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "SSE\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::SSE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "SSE2\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::SSE2) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "SS\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::SS) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "HTT\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::HTT) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "TM\t" << (mSysInfo.IsFeatureSupported(CpuidFeatures::TM) ? "    supported" : "NOT supported") << std::endl;
    //std::cout << std::endl << "..::Processor extended features::.." << std::endl;
    std::cout << "SSE3\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::SSE3) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "MW\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::MW) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "CPL\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::CPL) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "VMX\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::VMX) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "EST\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::EST) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "TM2\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::TM2) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "L1\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::L1) ? "    supported" : "NOT supported") << std::endl;
    std::cout << "CAE\t" << (mSysInfo.IsFeatureSupported(CpuidExFeatures::CAE) ? "    supported" : "NOT supported") << std::endl;
    std::cout << std::endl;
}