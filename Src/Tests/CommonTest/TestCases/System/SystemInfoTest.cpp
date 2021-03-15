#include "PCH.hpp"
#include "Engine/Common/System/SystemInfo.hpp"
#include "Engine/Common/Containers/String.hpp"

using namespace NFE::Common;

TEST(SystemInfoTest, CpuInfoTest)
{
    SystemInfo& sysInfoPtr = SystemInfo::Instance();
    EXPECT_FALSE(sysInfoPtr.GetCPUBrand().Empty());
    EXPECT_GE(sysInfoPtr.GetCPUCoreNo(), 1);
    EXPECT_GT(sysInfoPtr.GetPageSize(), 100);
    EXPECT_GE(sysInfoPtr.GetCacheLineSize(), 32);
}

TEST(SystemInfoTest, MemoryInfoTest)
{
    SystemInfo& sysInfoPtr = SystemInfo::Instance();
    EXPECT_GT(sysInfoPtr.GetMemTotalPhysKb(), 1000);
    EXPECT_GE(sysInfoPtr.GetMemTotalVirtKb(), 0);
    EXPECT_LT(sysInfoPtr.GetMemTotalSwapKb(), 1000000000);
    EXPECT_GT(sysInfoPtr.GetMemFreePhysKb(), 1000);
    EXPECT_GE(sysInfoPtr.GetMemFreeVirtKb(), 0);
    EXPECT_LT(sysInfoPtr.GetMemFreeSwapKb(), 1000000000);
    EXPECT_GT(sysInfoPtr.GetFreeMemoryKb(), 0);
}

TEST(SystemInfoTest, InfoStringTest)
{
    SystemInfo& sysInfoPtr = SystemInfo::Instance();
    const String sysInfoStr = sysInfoPtr.ConstructAllInfoString();
    std::cout << sysInfoStr.Str();
    ASSERT_FALSE(sysInfoStr.Empty());
}

TEST(SystemInfoTest, FeatureSupportTest)
{
    SystemInfo& sysInfoPtr = SystemInfo::Instance();
    // checking for TSC - it should be available on all CPUs since Pentium
    // + we use rdtsc, so we need it
    bool result = sysInfoPtr.IsFeatureSupported(StringView("TSC"));
    ASSERT_TRUE(result);

    result = sysInfoPtr.IsFeatureSupported(StringView("Should return false"));
    ASSERT_FALSE(result);
}
