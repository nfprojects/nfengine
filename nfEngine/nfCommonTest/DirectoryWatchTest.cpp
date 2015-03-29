#include "PCH.hpp"
#include "../nfCommon/DirectoryWatch.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Latch.hpp"

using namespace NFE::Common;

// latch timeout in milliseconds
const unsigned int LATCH_TIMEOUT = 200;
// watched directories:
const std::string TEST_DIR = "dirwatch_test_dir";
const std::string TEST_DIR2 = "dirwatch_test_dir_2";
// not watched directory
const std::string TEST_DIR_NOT_WATCHED = "dirwatch_test_dir_notwatched";

class DirectoryWatchTest : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        EXPECT_TRUE(FileSystem::CreateDir(TEST_DIR));
        EXPECT_TRUE(FileSystem::CreateDir(TEST_DIR2));
        EXPECT_TRUE(FileSystem::CreateDir(TEST_DIR_NOT_WATCHED));
    }

    static void TearDownTestCase()
    {
        EXPECT_TRUE(FileSystem::Remove(TEST_DIR, true));
        EXPECT_TRUE(FileSystem::Remove(TEST_DIR2, true));
        EXPECT_TRUE(FileSystem::Remove(TEST_DIR_NOT_WATCHED, true));
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
    ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Create));
    ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Create));
    ASSERT_FALSE(watch.WatchPath("not_existing_path", DirectoryWatch::Event::Create));
}

TEST_F(DirectoryWatchTest, Create)
{
    const std::string createdDirName = TEST_DIR + '/' + "a_create_dir";
    const std::string createdFileName = TEST_DIR + '/' + "a_create_file";

    Latch fileLatch, dirLatch;
    std::atomic<bool> wrongType(false);
    std::atomic<int> dirCreateCount(0);
    std::atomic<int> fileCreateCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (event.type != DirectoryWatch::Event::Create)
            wrongType.store(true);

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
        ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Create));

        // create file and directory
        ASSERT_TRUE(FileSystem::CreateDir(createdDirName));
        ASSERT_TRUE(FileSystem::TouchFile(createdFileName));

        // wait for DirectoryWatch event
        EXPECT_TRUE(fileLatch.Wait(LATCH_TIMEOUT));
        EXPECT_TRUE(dirLatch.Wait(LATCH_TIMEOUT));

        EXPECT_FALSE(wrongType);
        EXPECT_EQ(1, dirCreateCount);
        EXPECT_EQ(1, fileCreateCount);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(createdDirName));
    EXPECT_TRUE(FileSystem::Remove(createdFileName));
}

TEST_F(DirectoryWatchTest, Delete)
{
    const std::string deletedDirName = TEST_DIR + '/' + "a_deleted_dir";
    const std::string deletedFileName = TEST_DIR + '/' + "a_deleted_file";

    Latch fileLatch, dirLatch;
    std::atomic<bool> wrongType(false);
    std::atomic<int> dirDeleteCount(0);
    std::atomic<int> fileDeleteCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (event.type != DirectoryWatch::Event::Delete)
            wrongType.store(true);

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
    ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Delete));

    // create file and directory ...
    ASSERT_TRUE(FileSystem::CreateDir(deletedDirName));
    ASSERT_TRUE(FileSystem::TouchFile(deletedFileName));
    // ... and remove them
    ASSERT_TRUE(FileSystem::Remove(deletedDirName));
    ASSERT_TRUE(FileSystem::Remove(deletedFileName));

    // wait for DirectoryWatch event
    EXPECT_TRUE(fileLatch.Wait(LATCH_TIMEOUT));
    EXPECT_TRUE(dirLatch.Wait(LATCH_TIMEOUT));

    EXPECT_FALSE(wrongType);
    EXPECT_EQ(1, dirDeleteCount);
    EXPECT_EQ(1, fileDeleteCount);
}

// moves a file within a single watched directory
TEST_F(DirectoryWatchTest, MoveWithinSingleWatchedDir)
{
    const std::string movedFileName = TEST_DIR + '/' + "a_moved_file";
    const std::string movedFileNameAfter = TEST_DIR + '/' + "a_moved_file_after";

    std::atomic<bool> pathMatches(false), pathAfterMatches(false);
    std::atomic<int> moveFromCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            pathMatches.store(movedFileName == event.path);
            moveFromCount++;
        }
        if (DirectoryWatch::Event::MoveTo == event.type)
        {
            pathAfterMatches.store(movedFileNameAfter == event.path);
            latch.Set();
        }
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(TEST_DIR,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

        EXPECT_EQ(1, moveFromCount);
        EXPECT_TRUE(pathAfterMatches);
        EXPECT_TRUE(pathMatches);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves a file to another watched directory
TEST_F(DirectoryWatchTest, MoveToAnotherWatchedDir)
{
    const std::string movedFileName = TEST_DIR + '/' +"a_moved_file";
    const std::string movedFileNameAfter = TEST_DIR2 + '/' +"a_moved_file_after";

    std::atomic<bool> pathMatches(false), pathAfterMatches(false);
    std::atomic<int> moveFromCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            pathMatches.store(movedFileName == event.path);
            moveFromCount++;
        }

        if (DirectoryWatch::Event::MoveTo == event.type)
        {
            pathAfterMatches.store(movedFileNameAfter == event.path);
            latch.Set();
        }
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(TEST_DIR,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));
        ASSERT_TRUE(watch.WatchPath(TEST_DIR2,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Create |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

        EXPECT_EQ(1, moveFromCount);
        EXPECT_TRUE(pathMatches);
        EXPECT_TRUE(pathAfterMatches);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves a file from not-watched directory
TEST_F(DirectoryWatchTest, MoveFromNotWatchedDir)
{
    const std::string movedFileName = TEST_DIR_NOT_WATCHED + '/' +"a_moved_file";
    const std::string movedFileNameAfter = TEST_DIR + '/' +"a_moved_file_after";

    Latch latch;
    std::atomic<bool> pathMatches(false);
    std::atomic<bool> typeMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        typeMatches.store(DirectoryWatch::Event::MoveTo == event.type);
        pathMatches.store(movedFileNameAfter == event.path);
        latch.Set();
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(TEST_DIR,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

        EXPECT_TRUE(pathMatches);
        EXPECT_TRUE(typeMatches);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves a file to not-watched directory
TEST_F(DirectoryWatchTest, MoveToNotWatchedDir)
{
    const std::string movedFileName = TEST_DIR + '/' +"a_moved_file";
    const std::string movedFileNameAfter = TEST_DIR_NOT_WATCHED + '/' +"a_moved_file_after";

    Latch latch;
    std::atomic<bool> pathMatches(false);
    std::atomic<bool> typeMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        typeMatches.store(DirectoryWatch::Event::MoveFrom == event.type);
        pathMatches.store(movedFileName == event.path);
        latch.Set();
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(TEST_DIR,
                                    DirectoryWatch::Event::MoveFrom |
                                    DirectoryWatch::Event::MoveTo |
                                    DirectoryWatch::Event::Delete));

        // create file and rename it
        ASSERT_TRUE(FileSystem::TouchFile(movedFileName));
        ASSERT_TRUE(FileSystem::Move(movedFileName, movedFileNameAfter));
        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

        EXPECT_TRUE(pathMatches);
        EXPECT_TRUE(typeMatches);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedFileNameAfter));
}

// moves watched directory
TEST_F(DirectoryWatchTest, MoveWatchedDir)
{
    const std::string movedDir = TEST_DIR + '/' +"a_moved_dir";
    const std::string movedDirAfter = TEST_DIR + '/' +"a_moved_dir_after";

    Latch latch;
    std::atomic<bool> pathMatches(false);
    std::atomic<bool> typeMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        typeMatches.store(DirectoryWatch::Event::MoveFrom == event.type);
        pathMatches.store(movedDir == event.path);
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
        EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

        EXPECT_TRUE(pathMatches);
        EXPECT_TRUE(typeMatches);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(movedDirAfter));
}

TEST_F(DirectoryWatchTest, ModifyFile)
{
    const std::string readFileName = TEST_DIR + '/' + "a_read_file";
    const std::string writeFileName = TEST_DIR + '/' + "a_write_file";

    Latch latch;
    std::atomic<int> modifyCount(0);
    std::atomic<bool> pathMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        pathMatches.store(writeFileName == event.path);
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
        ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Modify));

        {
            File fileA(readFileName, AccessMode::Read);
            ASSERT_EQ(sizeof(data), fileA.Read(&data, sizeof(data)));

            File fileB(writeFileName, AccessMode::Write);
            data = 123;
            ASSERT_EQ(sizeof(data), fileB.Write(&data, sizeof(data)));
            ASSERT_EQ(sizeof(data), fileB.Write(&data, sizeof(data)));
        }

        // wait for DirectoryWatch event
        EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));
        // make sure that only 1 event was recorded
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_EQ(1, modifyCount);
        EXPECT_TRUE(pathMatches);
    }

    // cleanup
    EXPECT_TRUE(FileSystem::Remove(readFileName));
    EXPECT_TRUE(FileSystem::Remove(writeFileName));
}

// add and remove watch
TEST_F(DirectoryWatchTest, RemoveWatch)
{
    const std::string createdFileName = TEST_DIR + '/' + "a_create_file";

    std::atomic<bool> callbackCalled(false);
    auto callback = [&](const DirectoryWatch::EventData&)
    {
        callbackCalled.store(true);
    };

    {
        ASSERT_TRUE(FileSystem::TouchFile(createdFileName));

        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Create |
                                              DirectoryWatch::Event::Delete));
        ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::None));

        EXPECT_TRUE(FileSystem::Remove(createdFileName));

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        EXPECT_FALSE(callbackCalled);
    }
}
