#include "stdafx.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Memory.hpp"

using namespace NFE::Common;

class MemoryTest : public testing::Test
{
protected:
    std::unique_ptr<File> file;
    uint32 resultAccess=0;
    std::string readPath="TimerTest.cpp";
    std::string writePath="newFileDont.Touch";
};

TEST(MemoryTest, ObjAccessSimple)
{
   file = std::make_unique<File>(readPath, Read);
   resultAccess = MemoryCheck(file);
   ASSERT_NE(resultAccess, 0);
}

TEST(MemoryTest, ObjAccessNone)
{
   file = std::make_unique<File>(readPath, None);
   resultAccess = MemoryCheck(file);
   // ? co przy none
}

TEST(MemoryTest, ObjAccessRead)
{
   file = std::make_unique<File>(readPath, Read);
   resultAccess = MemoryCheck(file);
   ASSERT_EQ(ACCESS_READ, resultAccess);
}

TEST(MemoryTest, ObjAccessWrite)
{
   file = std::make_unique<File>(writePath, Write);
   resultAccess = MemoryCheck(file);
   ASSERT_EQ(ACCESS_WRITE, resultAccess);
}

TEST(MemoryTest, ObjAccessReadWrite)
{
   file = std::make_unique<File>(writePath, ReadWrite);
   resultAccess = MemoryCheck(file);
   ASSERT_EQ(ACCESS_READWRITE, resultAccess);
}