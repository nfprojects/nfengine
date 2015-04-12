#include "stdafx.hpp"
#include "../nfCommon/Library.hpp"

using namespace NFE::Common;

// TODO: create custom dynamic library designed for this test only

#if defined(WIN32)
TEST(LibraryTest, Simple)
{
    Library lib;
    EXPECT_FALSE(lib.Open("this_file_does_not_exist"));
    ASSERT_TRUE(lib.Open("kernel32.dll"));

    typedef void (WINAPI *FuncType)(LPSYSTEM_INFO);
    FuncType getNativeSystemInfoPtr = static_cast<FuncType>(lib.GetSymbol("GetNativeSystemInfo"));
    ASSERT_TRUE(getNativeSystemInfoPtr != NULL);

    SYSTEM_INFO sysInfo;
    getNativeSystemInfoPtr(&sysInfo);
}
#endif // defined(WIN32)