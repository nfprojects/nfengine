#include "stdafx.hpp"
#include "../nfCommon/FileSystem.hpp"

using namespace NFE::Common;

namespace {

const std::string testDir = "FileSystemTestDir/";

} // namespace

TEST(FileSystemTest, DirectoryAndRemove)
{
    const std::string testDir = "testDir1";
    const std::string testDir2 = testDir + "/testDir2";

    ASSERT_EQ(true, FileSystem::CreateDir(testDir));
    ASSERT_EQ(true, FileSystem::CreateDir(testDir2));
    EXPECT_EQ(false, FileSystem::Remove(testDir)); // non-recursive remove will fail
    EXPECT_EQ(true, FileSystem::Remove(testDir2));
    ASSERT_EQ(true, FileSystem::Remove(testDir));
    EXPECT_EQ(false, FileSystem::Remove(testDir));

    ASSERT_EQ(true, FileSystem::CreateDir(testDir));
    ASSERT_EQ(true, FileSystem::CreateDir(testDir2));
    ASSERT_EQ(true, FileSystem::Remove(testDir, true));
    EXPECT_EQ(false, FileSystem::Remove(testDir, true));
}

TEST(FileSystemTest, TouchFile)
{
    const std::string filePath = "touch_test_file";

    ASSERT_EQ(PathType::Invalid, FileSystem::GetPathType(filePath));
    ASSERT_EQ(true, FileSystem::TouchFile(filePath));
    EXPECT_EQ(false, FileSystem::TouchFile(filePath));
    ASSERT_EQ(PathType::File, FileSystem::GetPathType(filePath));
    EXPECT_EQ(true, FileSystem::Remove(filePath));
}

TEST(FileSystemTest, List)
{
    const std::string root = "test_dir";

    const std::vector<std::string> test_dirs =
    {
        root + "/a",
        root + "/b",
        root + "/a/b"
    };

    const std::vector<std::string> test_files =
    {
        root + "/a_file",
        root + "/b_file",
        root + "/a/a_file",
        root + "/a/b/a_file"
    };

    // prepare test files structure
    ASSERT_EQ(true, FileSystem::CreateDir(root)) << "create directory '" << root << "'";
    for (const std::string& path : test_dirs)
        ASSERT_EQ(true, FileSystem::CreateDir(path)) << "create directory '" << path << "'";
    for (const std::string& path : test_files)
        ASSERT_EQ(true, FileSystem::TouchFile(path)) << "create file '" << path << "'";


    std::set<std::string> dirs;
    std::set<std::string> files;
    auto callback = [&](const std::string& path, bool isDirectory)
    {
        if (isDirectory)
            dirs.insert(path);
        else
            files.insert(path);

        return true;
    };

    EXPECT_EQ(true, FileSystem::List(root, callback));



    for (const std::string& path : test_dirs)
        EXPECT_EQ(1, dirs.count(path)) << "check directory '" << path << "'";
    for (const std::string& path : test_files)
        EXPECT_EQ(1, files.count(path)) << "check file '" << path << "'";

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(root, true));
}
