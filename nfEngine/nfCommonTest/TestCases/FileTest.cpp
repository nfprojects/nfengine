#include "PCH.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/FileSystem/File.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(FileTest, Simple)
{
    const std::string filePath = "test_file";
    const int testData = 0xAABBCCDD;
    int data;

    {
        File file;
        ASSERT_FALSE(file.IsOpened());
        ASSERT_EQ(AccessMode::No, file.GetFileMode());
    }

    {
        File file(filePath, AccessMode::ReadWrite);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::ReadWrite, file.GetFileMode());

        ASSERT_EQ(sizeof(testData), file.Write(&testData, sizeof(testData)));
        EXPECT_EQ(sizeof(testData), static_cast<size_t>(file.GetPos()));

        ASSERT_TRUE(file.Seek(0, SeekMode::Begin));
        EXPECT_EQ(0, file.GetPos());

        ASSERT_EQ(sizeof(testData), file.Read(&data, sizeof(testData)));
        EXPECT_EQ(testData, data);

        ASSERT_TRUE(file.Seek(0, SeekMode::End));
        EXPECT_EQ(4, file.GetPos());

        file.Close();
        ASSERT_FALSE(file.IsOpened());
        ASSERT_EQ(AccessMode::No, file.GetFileMode());
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(filePath));
}

TEST(FileTest, OpenNotExisting)
{
    const std::string filePath = "test_file";

    File file(filePath, AccessMode::Read);
    EXPECT_FALSE(file.IsOpened());
    ASSERT_EQ(AccessMode::No, file.GetFileMode());
    EXPECT_EQ(PathType::Invalid, FileSystem::GetPathType(filePath));

    // cleanup
    FileSystem::Remove(filePath);
}

TEST(FileTest, Overwrite)
{
    const std::string filePath = "test_file";
    const int testData = 0xAABBCCDD;
    int data;

    {
        File file(filePath, AccessMode::Write);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::Write, file.GetFileMode());
        ASSERT_EQ(sizeof(testData), file.Write(&testData, sizeof(testData)));
    }

    {
        // read without overwrite flag
        File file(filePath, AccessMode::Read);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::Read, file.GetFileMode());
        ASSERT_EQ(static_cast<int64>(sizeof(testData)), file.GetSize());
        ASSERT_EQ(sizeof(testData), file.Read(&data, sizeof(testData)));
        ASSERT_EQ(testData, data);
    }

    {
        // read with overwrite flag - should make no difference
        File file(filePath, AccessMode::Read, true);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::Read, file.GetFileMode());
        ASSERT_EQ(static_cast<int64>(sizeof(testData)), file.GetSize());
        ASSERT_EQ(sizeof(testData), file.Read(&data, sizeof(testData)));
        ASSERT_EQ(testData, data);
    }

    {
        // write without overwrite flag - file is not truncated
        File file(filePath, AccessMode::Write);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::Write, file.GetFileMode());
        ASSERT_EQ(static_cast<int64>(sizeof(testData)), file.GetSize());
    }

    {
        // read/write without overwrite flag - file is not truncated
        File file(filePath, AccessMode::ReadWrite);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::ReadWrite, file.GetFileMode());
        ASSERT_EQ(static_cast<int64>(sizeof(testData)), file.GetSize());
        ASSERT_EQ(sizeof(testData), file.Read(&data, sizeof(testData)));
        ASSERT_EQ(testData, data);
    }

    {
        // write with overwrite flag - file is truncated
        File file(filePath, AccessMode::Write, true);
        ASSERT_TRUE(file.IsOpened());
        ASSERT_EQ(AccessMode::Write, file.GetFileMode());
        ASSERT_EQ(0, file.GetSize());
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(filePath));
}

/*
 * This testcase is excluded form Linux build, because of different
 * file access sharing rules on this plarform.
 *
 * TODO: introduce "access share" flag in File::Open() method
 */
#ifdef WIN32
TEST(FileTest, AccessShare)
{
    const std::string filePath = "access_share_test_file";

    // create the test file first
    {
        File file(filePath, AccessMode::Write);
        EXPECT_TRUE(file.IsOpened());
    }

    {
        File fileA(filePath, AccessMode::Read);
        ASSERT_TRUE(fileA.IsOpened());

        File fileB(filePath, AccessMode::Write);
        EXPECT_FALSE(fileB.IsOpened());

        File fileC(filePath, AccessMode::Read);
        EXPECT_TRUE(fileC.IsOpened());
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(filePath));
}
#endif // WIN32
