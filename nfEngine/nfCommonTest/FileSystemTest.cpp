#include "stdafx.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"

using namespace NFE::Common;

namespace {

const std::string testDir = "FileSystemTestDir/";

} // namespace

TEST(FileSystemTest, Directory)
{
    const std::string testDir = "testDir1";

    // invalid path
    EXPECT_EQ(false, FileSystem::CreateDir("/\\:"));

    ASSERT_EQ(true, FileSystem::CreateDir(testDir));
    ASSERT_EQ(true, FileSystem::Remove(testDir));

    // remove the directory second time
    //EXPECT_EQ(false, FileSystem::Remove("testDir"));
}

/*
TEST(FileSystemTest, List)
{
    int i = 0;
    auto callback = [&i](const std::string& path, bool isDirectory)
    {
        if (isDirectory)
            std::cout << "[DIR] ";

        std::cout << path << std::endl;
        i++;
        return true;
    };

    FileSystem::List(".", callback);
    std::cout << "Found items: " << i << std::endl;
}
*/

TEST(FileSystemTest, FileTest)
{
    const int testData = 0xAABBCCDD;
    int data;

    File file("test_file", AccessMode::ReadWrite);
    ASSERT_EQ(true, file.IsOpened());

    ASSERT_EQ(sizeof(testData), file.Write(&testData, sizeof(testData)));
    EXPECT_EQ(sizeof(testData), file.GetPos());

    ASSERT_EQ(true, file.Seek(0, SeekMode::Begin));
    EXPECT_EQ(0, file.GetPos());

    ASSERT_EQ(sizeof(testData), file.Read(&data, sizeof(testData)));
    EXPECT_EQ(testData, data);
}

TEST(FileSystemTest, FileAccessSharing)
{
    const std::string filePath = "access_share_test_file";

    {
        File fileA(filePath, AccessMode::Read, AccessMode::None);
        ASSERT_EQ(true, fileA.IsOpened());
        File fileB(filePath, AccessMode::Write, AccessMode::ReadWrite);
        EXPECT_EQ(false, fileB.IsOpened());
        File fileC(filePath, AccessMode::Read, AccessMode::ReadWrite);
        EXPECT_EQ(false, fileC.IsOpened());
    }

    {
        File fileA(filePath, AccessMode::Read, AccessMode::Read);
        ASSERT_EQ(true, fileA.IsOpened());
        File fileB(filePath, AccessMode::Write, AccessMode::ReadWrite);
        EXPECT_EQ(false, fileB.IsOpened());
        File fileC(filePath, AccessMode::Read, AccessMode::ReadWrite);
        EXPECT_EQ(true, fileC.IsOpened());
    }

    {
        File fileA(filePath, AccessMode::Write, AccessMode::Write);
        ASSERT_EQ(true, fileA.IsOpened());
        File fileB(filePath, AccessMode::Write, AccessMode::ReadWrite);
        EXPECT_EQ(true, fileB.IsOpened());
        File fileC(filePath, AccessMode::Read, AccessMode::ReadWrite);
        EXPECT_EQ(false, fileC.IsOpened());
    }

    {
        File fileA(filePath, AccessMode::Write, AccessMode::ReadWrite);
        ASSERT_EQ(true, fileA.IsOpened());
        File fileB(filePath, AccessMode::Write, AccessMode::ReadWrite);
        EXPECT_EQ(true, fileB.IsOpened());
        File fileC(filePath, AccessMode::Read, AccessMode::ReadWrite);
        EXPECT_EQ(true, fileC.IsOpened());
    }
}
