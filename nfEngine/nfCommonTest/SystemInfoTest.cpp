#include "PCH.hpp"
#include "../nfCommon/SystemInfo.hpp"

using namespace NFE::Common;

TEST(SystemInfoTest, CpuInfoTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    ASSERT_FALSE(sysInfoPtr->GetCPUBrand().empty());
    ASSERT_GE(sysInfoPtr->GetCPUCoreNo(), 1u);
    ASSERT_GT(sysInfoPtr->GetPageSize(), 100u);
    ASSERT_GE(sysInfoPtr->GetCacheLineSize(), 32u);
    ASSERT_GT(sysInfoPtr->GetCPUSpeedMHz(), 100u);

}
TEST(SystemInfoTest, MemoryInfoTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    ASSERT_GT(sysInfoPtr->GetMemTotalPhysKb(), 1000u);
    ASSERT_GE(sysInfoPtr->GetMemTotalVirtKb(), 0u);
    ASSERT_LT(sysInfoPtr->GetMemTotalSwapKb(), 1000000000u);
    ASSERT_GT(sysInfoPtr->GetMemFreePhysKb(), 1000u);
    ASSERT_GE(sysInfoPtr->GetMemFreeVirtKb(), 0u);
    ASSERT_LT(sysInfoPtr->GetMemFreeSwapKb(), 1000000000u);
    ASSERT_GT(sysInfoPtr->GetFreeMemoryKb(), 0u);
}
TEST(SystemInfoTest, InfoStringTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    std::string sysInfoStr = sysInfoPtr->ConstructAllInfoString();
    std::cout << sysInfoStr;
    ASSERT_FALSE(sysInfoStr.empty());
}
TEST(SystemInfoTest, FeatureSupportTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    // checking for TSC - it should be available on all CPUs since Pentium
    // + we use rdtsc, so we need it
    bool result = sysInfoPtr->IsFeatureSupported("TSC");
    ASSERT_TRUE(result);

    result = sysInfoPtr->IsFeatureSupported("Should return false");
    ASSERT_FALSE(result);
}
