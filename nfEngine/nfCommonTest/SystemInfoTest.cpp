#include "stdafx.hpp"
#include "../nfCommon/SystemInfo.hpp"

using namespace NFE::Common;

TEST(SystemInfoTest, RunTest)
{
    SystemInfo* sysInfoPtr = SystemInfo::Instance();
    std::string sysInfoStr = sysInfoPtr->GetAllInfoString();
    std::cout << sysInfoStr;
    ASSERT_EQ(sysInfoStr.compare("") > 0, true);
    ASSERT_EQ(sysInfoPtr->GetCPUBrand().compare("") > 0, true);
    ASSERT_EQ(sysInfoPtr->GetCPUCoreNo() >= 1, true);
    ASSERT_EQ(sysInfoPtr->GetPageSize() > 100, true);
    ASSERT_EQ(sysInfoPtr->GetCacheLineSize() >= 32, true);
    ASSERT_EQ(sysInfoPtr->GetCPUSpeedMHz() > 100, true);
    ASSERT_EQ(sysInfoPtr->GetMemTotalPhysKb() > 1000, true);
    ASSERT_EQ(sysInfoPtr->GetMemTotalVirtKb() > 1000, true);
    ASSERT_EQ(sysInfoPtr->GetMemFreePhysKb() > 1000, true);
    ASSERT_EQ(sysInfoPtr->GetMemFreeVirtKb() > 1000, true);
}