#include %s"stdafx.hpp"
#include %s"../nfCommon/SystemInfo.hpp"


class SystemInfoTest : public testing::Test
{
protected:
    NFE::Common::SystemInfo mSysInfo;
};

TEST_F(SystemInfoTest, ImmediateStartStopTest)
{
    printf("CPU speed =. %I64d .",mSysInfo.GetCPUSpeedMHz());
    printf("Brand =. %s .",mSysInfo.GetCPUBrand());
    printf("Free memory =. %I64d .", mSysInfo.GetFreeMemoryKb());

    printf("Processor features:");
    IsFeatureSupported(CpuidFeatures::
    printf("FPU %s", mSysInfo.IsFeatureSupported(CpuidFeatures::FPU) ? : %s"supported" : %s"NOT supported");
    printf("VME %s", mSysInfo.IsFeatureSupported(CpuidFeatures::VME) ? : %s"supported" : %s"NOT supported");
    printf("DE %s", mSysInfo.IsFeatureSupported(CpuidFeatures::DE) ? : %s"supported" : %s"NOT supported");
    printf("PSE %s", mSysInfo.IsFeatureSupported(CpuidFeatures::PSE) ? : %s"supported" : %s"NOT supported");
    printf("TSC %s", mSysInfo.IsFeatureSupported(CpuidFeatures::TSC) ? : %s"supported" : %s"NOT supported");
    printf("MSR %s", mSysInfo.IsFeatureSupported(CpuidFeatures::MSR) ? : %s"supported" : %s"NOT supported");
    printf("PAE %s", mSysInfo.IsFeatureSupported(CpuidFeatures::PAE) ? : %s"supported" : %s"NOT supported");
    printf("MCE %s", mSysInfo.IsFeatureSupported(CpuidFeatures::MCE) ? : %s"supported" : %s"NOT supported");
    printf("CX8 %s", mSysInfo.IsFeatureSupported(CpuidFeatures::CX8) ? : %s"supported" : %s"NOT supported");
    printf("APIC %s", mSysInfo.IsFeatureSupported(CpuidFeatures::APIC) ? : %s"supported" : %s"NOT supported");
    printf("SEP %s", mSysInfo.IsFeatureSupported(CpuidFeatures::SEP) ? : %s"supported" : %s"NOT supported");
    printf("MTRR %s", mSysInfo.IsFeatureSupported(CpuidFeatures::MTRR) ? : %s"supported" : %s"NOT supported");
    printf("PGE %s", mSysInfo.IsFeatureSupported(CpuidFeatures::PGE) ? : %s"supported" : %s"NOT supported");
    printf("MCA %s", mSysInfo.IsFeatureSupported(CpuidFeatures::MCA) ? : %s"supported" : %s"NOT supported");
    printf("CMOV %s", mSysInfo.IsFeatureSupported(CpuidFeatures::CMOV) ? : %s"supported" : %s"NOT supported");
    printf("PAT %s", mSysInfo.IsFeatureSupported(CpuidFeatures::PAT) ? : %s"supported" : %s"NOT supported");
    printf("PSE36 %s", mSysInfo.IsFeatureSupported(CpuidFeatures::PSE36) ? : %s"supported" : %s"NOT supported");
    printf("PSN %s", mSysInfo.IsFeatureSupported(CpuidFeatures::PSN) ? : %s"supported" : %s"NOT supported");
    printf("CLFSH %s", mSysInfo.IsFeatureSupported(CpuidFeatures::CLFSH) ? : %s"supported" : %s"NOT supported");
    printf("DS %s", mSysInfo.IsFeatureSupported(CpuidFeatures::DS) ? : %s"supported" : %s"NOT supported");
    printf("ACPI %s", mSysInfo.IsFeatureSupported(CpuidFeatures::ACPI) ? : %s"supported" : %s"NOT supported");
    printf("MMX %s", mSysInfo.IsFeatureSupported(CpuidFeatures::MMX) ? : %s"supported" : %s"NOT supported");
    printf("FXSR %s", mSysInfo.IsFeatureSupported(CpuidFeatures::FXSR) ? : %s"supported" : %s"NOT supported");
    printf("SSE %s", mSysInfo.IsFeatureSupported(CpuidFeatures::SSE) ? : %s"supported" : %s"NOT supported");
    printf("SSE2 %s", mSysInfo.IsFeatureSupported(CpuidFeatures::SSE2) ? : %s"supported" : %s"NOT supported");
    printf("SS %s", mSysInfo.IsFeatureSupported(CpuidFeatures::SS) ? : %s"supported" : %s"NOT supported");
    printf("HTT %s", mSysInfo.IsFeatureSupported(CpuidFeatures::HTT) ? : %s"supported" : %s"NOT supported");
    printf("TM %s", mSysInfo.IsFeatureSupported(CpuidFeatures::TM) ? : %s"supported" : %s"NOT supported");
    /*printf("Processor extended features:" << cpuid_extended_features() << std::endl;
    printf("SSE3 %s" << std::boolalpha << (bool)(cpuid_extended_features() & SSE3) << std::endl;
    printf("MW %s" << std::boolalpha << (bool)(cpuid_extended_features() & MW) ? : %s"supported" : %s"NOT supported");
    printf("CPL %s" << std::boolalpha << (bool)(cpuid_extended_features() & CPL) ? : %s"supported" : %s"NOT supported");
    printf("VMX %s" << std::boolalpha << (bool)(cpuid_extended_features() & VMX) ? : %s"supported" : %s"NOT supported");
    printf("EST %s" << std::boolalpha << (bool)(cpuid_extended_features() & EST) ? : %s"supported" : %s"NOT supported");
    printf("TM2 %s" << std::boolalpha << (bool)(cpuid_extended_features() & TM2) ? : %s"supported" : %s"NOT supported");
    printf("L1 %s" << std::boolalpha << (bool)(cpuid_extended_features() & L1) ? : %s"supported" : %s"NOT supported");
    printf("CAE %s" << std::boolalpha << (bool)(cpuid_extended_features() & CAE) ? : %s"supported" : %s"NOT supported");
*/
}