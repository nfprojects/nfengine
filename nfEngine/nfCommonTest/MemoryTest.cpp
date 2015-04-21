#include "stdafx.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Memory.hpp"

using namespace NFE::Common;

TEST(MemoryTest, AccessReadWrite)
{
   int* x = new int;
   uint32 resultAccess = MemoryCheck(x);
   ASSERT_EQ(ACCESS_READWRITE, resultAccess);
   delete x;
}

TEST(MemoryTest, AccessNone)
{
	int* x = nullptr;
	uint32 resultAccess = MemoryCheck(x);
	ASSERT_EQ(0, resultAccess);
}
