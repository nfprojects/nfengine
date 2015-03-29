#include "PCH.hpp"
#include "../nfCommon/DirectoryWatch.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Latch.hpp"

using namespace NFE::Common;

// latch timeout in milliseconds
const unsigned int gTimeout = 200;
// watched directories:
const std::string gTestDir = "dir_watch_test_dir";
const std::string gTestDir2 = "dir_watch_test_dir_2";
// not watched directory
const std::string gTestDirNotWatched = "dir_watch_test_dir_notwatched";

class DirectoryWatchTest : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        EXPECT_TRUE(FileSystem::CreateDir(gTestDir));
        EXPECT_TRUE(FileSystem::CreateDir(gTestDir2));
        EXPECT_TRUE(FileSystem::CreateDir(gTestDirNotWatched));
    }

    static void TearDownTestCase()
    {
        EXPECT_TRUE(FileSystem::Remove(gTestDir, true));
        EXPECT_TRUE(FileSystem::Remove(gTestDir2, true));
        EXPECT_TRUE(FileSystem::Remove(gTestDirNotWatched, true));
    }
};

TEST_F(DirectoryWatchTest, Constructor)
{
    DirectoryWatch watch;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST_F(DirectoryWatchTest, Simple)
{
    DirectoryWatch watch;
    ASSERT_TRUE(watch.WatchPath(gTestDir, DirectoryWatch::Event::Create));
    ASSERT_TRUE(watch.WatchPath(gTestDir, DirectoryWatch::Event::Create));
    ASSERT_FALSE(watch.WatchPath("not_existing_path", DirectoryWatch::Event::Create));
}

TEST_F(DirectoryWatchTest, Create)
{
    const std::string createdDirName = gTestDir + '/' + "a_create_dir";
    const std::string createdFileName = gTestDir + '/' + "a_create_file";

    Latch fileLatch, dirLatch;
    std::atomic<int> dirCreateCount(0);
    std::atomic<int> fileCreateCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        ASSERT_EQ(DirectoryWatch::Event::Create, event.type);
        if (event.path == createdDirName)
        {
            dirCreateCount++;
            dirLatch.Set();

        }
        else if (event.path == createdFileName)
        {
            fileCreateCount++;
            fileLatch.Set();
        }
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(gTestDir, DirectoryWatch::Event::Create));

        // create file and directory
        ASSERT_TRUE(FileSystem::CreateDir(createdDirName));
        ASSERT_TRUE(FileSystem::TouchFile(createdFileName));

        // wait for DirectoryWatch event
        EXPECT_TRUE(fileLatch.Wait(gTimeout));
        EXPECT_TRUE(dirLatch.Wait(gTimeout));
        EXPECT_EQ(1, dirCreateCount);
        EXPECT_EQ(1, fileCreateCount);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(createdDirName));
    EXPECT_TRUE(FileSystem::Remove(createdFileName));
}

TEST_F(DirectoryWatchTest, Delete)
{
    const std::string deletedDirName = gTestDir + '/' + "a_deleted_dir";
    const std::string deletedFileName = gTestDir + '/' + "a_deleted_file";

    Latch fileLatch, dirLatch;
    std::atomic<int> dirDeleteCount(0);
    std::atomic<int> fileDeleteCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        ASSERT_EQ(DirectoryWatch::Event::Delete, event.type);
        if (event.path == deletedDirName)
        {
            dirDeleteCount++;
            dirLatch.Set();
        }
        else if (event.path == deletedFileName)
        {
            fileDeleteCount++;
            fileLatch.Set();
        }
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(gTestDir, DirectoryWatch::Event::Delete));

    // create file and directory ...
    ASSERT_TRUE(FileSystem::CreateDir(deletedDirName));
    ASSERT_TRUE(FileSystem::TouchFile(deletedFileName));
    // ... and remove them
    ASSERT_TRUE(FileSystem::Remove(deletedDirName));
    ASSERT_TRUE(FileSystem::Remove(deletedFileName));

    // wait for DirectoryWatch event
    EXPECT_TRUE(fileLatch.Wait(gTimeout));
    EXPECT_TRUE(dirLatch.Wait(gTimeout));
    EXPECT_EQ(1, dirDeleteCount);
    EXPECT_EQ(1, fileDeleteCount);
}

// moves a file within a single watched directory
TEST_F(DirectoryWatchTest, Move_0)
{
    const std::string movedFileName = gTestDir + '/' + "a_moved_file";
    const std::string movedFileNameAfter = gTestDir + '/' + "a_moved_file_after";

    std::atomic<int> moveFromCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            ASSERT_EQ(movedFileName, event.path);
            moveFromCount++;
        }

        if (DirectoryWatch::Event::MoveTo == event.type)
        {
            ASSERT_EQ(movedFileNameAfter, event.path);
            latch.Set();
        }
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(gTestDir,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(gTimeout));
        EXPECT_EQ(1, moveFromCount);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves a file to another watched directory
TEST_F(DirectoryWatchTest, Move_1)
{
    const std::string movedFileName = gTestDir + '/' +"a_moved_file";
    const std::string movedFileNameAfter = gTestDir2 + '/' +"a_moved_file_after";

    std::atomic<int> moveFromCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            ASSERT_EQ(movedFileName, event.path);
            moveFromCount++;
        }

        if (DirectoryWatch::Event::MoveTo == event.type)
        {
            ASSERT_EQ(movedFileNameAfter, event.path);
            latch.Set();
        }
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(gTestDir,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));
        ASSERT_TRUE(watch.WatchPath(gTestDir2,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Create |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(gTimeout));
        EXPECT_EQ(1, moveFromCount);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves a file from not-watched directory
TEST_F(DirectoryWatchTest, Move_2)
{
    const std::string movedFileName = gTestDirNotWatched + '/' +"a_moved_file";
    const std::string movedFileNameAfter = gTestDir + '/' +"a_moved_file_after";

    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        ASSERT_EQ(DirectoryWatch::Event::MoveTo, event.type);
        ASSERT_EQ(movedFileNameAfter, event.path);
        latch.Set();
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(gTestDir,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(gTimeout));
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves a file to not-watched directory
TEST_F(DirectoryWatchTest, Move_3)
{
    const std::string movedFileName = gTestDir + '/' +"a_moved_file";
    const std::string movedFileNameAfter = gTestDirNotWatched + '/' +"a_moved_file_after";

    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        ASSERT_EQ(DirectoryWatch::Event::MoveFrom, event.type);
        ASSERT_EQ(movedFileName, event.path);
        latch.Set();
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(gTestDir,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(gTimeout));
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves watched directory
TEST_F(DirectoryWatchTest, MoveWatched)
{
    const std::string movedDir = gTestDir + '/' +"a_moved_dir";
    const std::string movedDirAfter = gTestDir + '/' +"a_moved_dir_after";

    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        ASSERT_EQ(DirectoryWatch::Event::MoveFrom, event.type);
        ASSERT_EQ(movedDir, event.path);
        latch.Set();
    };

    ASSERT_TRUE(FileSystem::CreateDir(movedDir));

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(movedDir,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        ASSERT_TRUE(FileSystem::Move(movedDir, movedDirAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(gTimeout));
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedDirAfter));
}

TEST_F(DirectoryWatchTest, Modify)
{
    const std::string readFileName = gTestDir + '/' + "a_read_file";
    const std::string writeFileName = gTestDir + '/' + "a_write_file";

    Latch latch;
    std::atomic<int> modifyCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        EXPECT_EQ(writeFileName, event.path);
        latch.Set();
        modifyCount++;
    };

    int data;

    // prepare files
    {
        data = 1234;

        File fileA(readFileName, AccessMode::Write);
        ASSERT_EQ(sizeof(data), fileA.Write(&data, sizeof(data)));

        File fileB(writeFileName, AccessMode::Write);
        ASSERT_EQ(sizeof(data), fileB.Write(&data, sizeof(data)));
    }

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(gTestDir, DirectoryWatch::Event::Modify));

        {
            File fileA(readFileName, AccessMode::Read);
            ASSERT_EQ(sizeof(data), fileA.Read(&data, sizeof(data)));

            File fileB(writeFileName, AccessMode::Write);
            data = 123;
            ASSERT_EQ(sizeof(data), fileB.Write(&data, sizeof(data)));
            ASSERT_EQ(sizeof(data), fileB.Write(&data, sizeof(data)));
        }

        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(gTimeout));
        // make sure that only 1 event was recorded
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_EQ(1, modifyCount);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(readFileName));
    EXPECT_TRUE(FileSystem::Remove(writeFileName));
}
