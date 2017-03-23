#include "PCH.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"

using namespace NFE::Common;

TEST(FileSystemTest, DirectoryAndRemove)
{
    const String testDir = "testDir1";
    const String testDir2 = testDir + "/testDir2";

    ASSERT_TRUE(FileSystem::CreateDir(testDir));
    ASSERT_TRUE(FileSystem::CreateDir(testDir2));
    ASSERT_EQ(PathType::Directory, FileSystem::GetPathType(testDir));
    ASSERT_EQ(PathType::Directory, FileSystem::GetPathType(testDir2));
    ASSERT_FALSE(FileSystem::Remove(testDir)); // non-recursive remove will fail
    ASSERT_TRUE(FileSystem::Remove(testDir2));
    ASSERT_TRUE(FileSystem::Remove(testDir));
    ASSERT_FALSE(FileSystem::Remove(testDir));

    ASSERT_TRUE(FileSystem::CreateDir(testDir));
    ASSERT_TRUE(FileSystem::CreateDir(testDir2));
    ASSERT_TRUE(FileSystem::Remove(testDir, true));
    ASSERT_FALSE(FileSystem::Remove(testDir, true));
}

TEST(FileSystemTest, TouchFile)
{
    const String filePath = "touch_test_file";

    ASSERT_EQ(PathType::Invalid, FileSystem::GetPathType(filePath));
    ASSERT_TRUE(FileSystem::TouchFile(filePath));
    ASSERT_FALSE(FileSystem::TouchFile(filePath));
    ASSERT_EQ(PathType::File, FileSystem::GetPathType(filePath));
    ASSERT_TRUE(FileSystem::Remove(filePath));
}

TEST(FileSystemTest, Iterate)
{
    const String root = "test_dir";

    const std::vector<String> test_dirs =
    {
        root + "/a",
        root + "/b",
        root + "/a/b"
    };

    const std::vector<String> test_files =
    {
        root + "/a_file",
        root + "/b_file",
        root + "/a/a_file",
        root + "/a/b/a_file"
    };

    // prepare test files structure
    ASSERT_EQ(true, FileSystem::CreateDir(root)) << "create directory '" << root << "'";
    for (const String& path : test_dirs)
        ASSERT_TRUE(FileSystem::CreateDir(path)) << "create directory '" << path << "'";
    for (const String& path : test_files)
        ASSERT_TRUE(FileSystem::TouchFile(path)) << "create file '" << path << "'";


    std::set<String> dirs;
    std::set<String> files;
    auto callback = [&](const String & path, bool isDirectory)
    {
        if (isDirectory)
            dirs.insert(path);
        else
            files.insert(path);

        return true;
    };

    ASSERT_TRUE(FileSystem::Iterate(root, callback));

    for (const String& path : test_dirs)
        EXPECT_EQ(1, dirs.count(path)) << "check directory '" << path << "'";
    for (const String& path : test_files)
        EXPECT_EQ(1, files.count(path)) << "check file '" << path << "'";

    // cleanup
    ASSERT_TRUE(FileSystem::Remove(root, true));
}
TEST(FileSystemTest, GetExecutablePath)
{
    String path = FileSystem::GetExecutablePath();
    ASSERT_FALSE(path.empty());
    ASSERT_EQ(PathType::File, FileSystem::GetPathType(path));
}
