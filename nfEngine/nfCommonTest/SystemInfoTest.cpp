#include "stdafx.hpp"
#include "../nfCommon/SystemInfo.hpp"

using namespace NFE::Common;

TEST(SystemInfoTest, CpuInfoTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    ASSERT_FALSE(sysInfoPtr->GetCPUBrand().empty());
    ASSERT_GE(sysInfoPtr->GetCPUCoreNo(), 1);
    ASSERT_GT(sysInfoPtr->GetPageSize(), 100);
    ASSERT_GE(sysInfoPtr->GetCacheLineSize(), 32);
    ASSERT_GT(sysInfoPtr->GetCPUSpeedMHz(), 100);
    
}
TEST(SystemInfoTest, MemoryInfoTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    ASSERT_GT(sysInfoPtr->GetMemTotalPhysKb(), 1000);
    ASSERT_GT(sysInfoPtr->GetMemTotalVirtKb(), 1000);
    ASSERT_GT(sysInfoPtr->GetMemFreePhysKb(), 1000);
    ASSERT_GT(sysInfoPtr->GetMemFreeVirtKb(), 1000);
}
TEST(SystemInfoTest, InfoStringTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    std::string sysInfoStr = sysInfoPtr->ConstructAllInfoString();
    std::cout << sysInfoStr;
    ASSERT_FALSE(sysInfoStr.empty());
}