#include "stdafx.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"

using namespace NFE::Common;

TEST(FileTest, Simple)
{
    const std::string filePath = "test_file";
    const int testData = 0xAABBCCDD;
    int data;

    {
        File file(filePath, AccessMode::ReadWrite);
        ASSERT_EQ(true, file.IsOpened());

        ASSERT_EQ(sizeof(testData), file.Write(&testData, sizeof(testData)));
        EXPECT_EQ(sizeof(testData), file.GetPos());

        ASSERT_EQ(true, file.Seek(0, SeekMode::Begin));
        EXPECT_EQ(0, file.GetPos());

        ASSERT_EQ(sizeof(testData), file.Read(&data, sizeof(testData)));
        EXPECT_EQ(testData, data);

        ASSERT_EQ(true, file.Seek(0, SeekMode::End));
        EXPECT_EQ(4, file.GetPos());
    }

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(filePath));
}

TEST(FileTest, AccessShare)
{
    const std::string filePath = "access_share_test_file";

    // create the test file first
    {
        File file(filePath, AccessMode::Write);
        EXPECT_EQ(true, file.IsOpened());
    }

    {
        File fileA(filePath, AccessMode::Read);
        ASSERT_EQ(true, fileA.IsOpened());

        File fileB(filePath, AccessMode::Write);
        EXPECT_EQ(false, fileB.IsOpened());

        File fileC(filePath, AccessMode::Read);
        EXPECT_EQ(true, fileC.IsOpened());
    }

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(filePath));
}
