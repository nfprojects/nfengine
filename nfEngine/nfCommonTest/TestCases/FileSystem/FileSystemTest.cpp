#include "PCH.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/Containers/String.hpp"

using namespace NFE::Common;

TEST(FileSystemTest, GetParentDir)
{
    {
        const StringView path("");
        EXPECT_EQ(StringView(""), FileSystem::GetParentDir(path));
    }

    {
        const StringView path("aaa");
        EXPECT_EQ(StringView(""), FileSystem::GetParentDir(path));
    }

    {
        const StringView path("aaa/bbb");
        EXPECT_EQ(StringView("aaa"), FileSystem::GetParentDir(path));
    }

    {
        const StringView path("aaa\\bbb");
        EXPECT_EQ(StringView("aaa"), FileSystem::GetParentDir(path));
    }

    {
        const StringView path("aaa/bbb/");
        EXPECT_EQ(StringView("aaa"), FileSystem::GetParentDir(path));
    }

    {
        const StringView path("aaa\\bbb\\");
        EXPECT_EQ(StringView("aaa"), FileSystem::GetParentDir(path));
    }
}

TEST(FileSystemTest, ExtractExtension)
{
    {
        const StringView path("");
        EXPECT_EQ(StringView(""), FileSystem::GetExtension(path));
    }

    {
        const StringView path(".");
        EXPECT_EQ(StringView(""), FileSystem::GetExtension(path));
    }

    {
        const StringView path("aaa.ext");
        EXPECT_EQ(StringView("ext"), FileSystem::GetExtension(path));
    }

    {
        const StringView path("aaa.");
        EXPECT_EQ(StringView(""), FileSystem::GetExtension(path));
    }

    {
        const StringView path("aaa.bbb/ccc.ddd");
        EXPECT_EQ(StringView("ddd"), FileSystem::GetExtension(path));
    }

    {
        const StringView path("aaa.bbb/ccc");
        EXPECT_EQ(StringView(""), FileSystem::GetExtension(path));
    }
}

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
    ASSERT_EQ(true, FileSystem::CreateDir(root)) << "create directory '" << root.Str() << "'";
    for (const String& path : test_dirs)
        ASSERT_TRUE(FileSystem::CreateDir(path)) << "create directory '" << path.Str() << "'";
    for (const String& path : test_files)
        ASSERT_TRUE(FileSystem::TouchFile(path)) << "create file '" << path.Str() << "'";


    std::set<String> dirs;
    std::set<String> files;
    auto callback = [&](const String& path, PathType type)
    {
        if (type == PathType::Directory)
            dirs.insert(path);
        else if (type == PathType::File)
            files.insert(path);

        return true;
    };

    ASSERT_TRUE(FileSystem::Iterate(root, callback));

    for (const String& path : test_dirs)
        EXPECT_EQ(1u, dirs.count(path)) << "check directory '" << path.Str() << "'";
    for (const String& path : test_files)
        EXPECT_EQ(1u, files.count(path)) << "check file '" << path.Str() << "'";

    // cleanup
    ASSERT_TRUE(FileSystem::Remove(root, true));
}

TEST(FileSystemTest, GetExecutablePath)
{
    String path = FileSystem::GetExecutablePath();
    ASSERT_FALSE(path.Empty());
    ASSERT_EQ(PathType::File, FileSystem::GetPathType(path));
}
