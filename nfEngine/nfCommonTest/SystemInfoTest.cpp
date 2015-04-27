#include "stdafx.hpp"
#include "../nfCommon/SystemInfo.hpp"

using namespace NFE::Common;

TEST(SystemInfoTest, RunTest)
{
    SystemInfo mSysInfo;
    std::cout << mSysInfo.GetAllInfo();
}