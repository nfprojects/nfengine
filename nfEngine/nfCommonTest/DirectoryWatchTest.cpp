#include "PCH.hpp"
#include "../nfCommon/DirectoryWatch.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Latch.hpp"

using namespace NFE::Common;

namespace {

// latch timeout in milliseconds
const unsigned int LATCH_TIMEOUT = 200;
// watched directories:
const std::string TEST_DIR = "dirwatch_test_dir";
const std::string TEST_DIR2 = "dirwatch_test_dir_2";
// not watched directory
const std::string TEST_DIR_NOT_WATCHED = "dirwatch_test_dir_notwatched";

const std::string FILE_NAME = "a_file";
const std::string FILE_NAME2 = "a_file2";
const std::string DIR_NAME = "a_dir";
const std::string DIR_NAME2 = "a_dir2";

} // namespace

class DirectoryWatchTest : public testing::Test
{
protected:
    void SetUp()
    {
        EXPECT_TRUE(FileSystem::CreateDir(TEST_DIR));
        EXPECT_TRUE(FileSystem::CreateDir(TEST_DIR2));
        EXPECT_TRUE(FileSystem::CreateDir(TEST_DIR_NOT_WATCHED));
    }

    void TearDown()
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
    // verify adding duplicate event filter
    ASSERT_TRUE(watch.WatchPath(TEST_DIR, DirectoryWatch::Event::Create));
    // try watching not existing path
    ASSERT_FALSE(watch.WatchPath("not_existing_path", DirectoryWatch::Event::Create));
}

TEST_F(DirectoryWatchTest, Create)
{
    const std::string createdDirName = TEST_DIR + '/' + DIR_NAME;
    const std::string createdFileName = TEST_DIR + '/' + FILE_NAME;

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
}

TEST_F(DirectoryWatchTest, Delete)
{
    const std::string deletedDirName = TEST_DIR + '/' + DIR_NAME;
    const std::string deletedFileName = TEST_DIR + '/' + FILE_NAME;

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
    std::atomic<int> moveToCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            pathMatches.store(movedFileName == event.path);
            moveFromCount++;
        }
        else if (DirectoryWatch::Event::MoveTo == event.type)
        {
            pathAfterMatches.store(movedFileNameAfter == event.path);
            moveToCount++;
            latch.Set();
        }
        else
            latch.Set();
    };

    {
        DirectoryWatch watch;
        watch.SetCallback(callback);
        ASSERT_TRUE(watch.WatchPath(TEST_DIR,
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
        EXPECT_EQ(1, moveToCount);
        EXPECT_TRUE(pathAfterMatches);
        EXPECT_TRUE(pathMatches);
    }
}

// moves a file to another watched directory
TEST_F(DirectoryWatchTest, MoveToAnotherWatchedDir)
{
    const std::string movedFileName = TEST_DIR + '/' + FILE_NAME;
    const std::string movedFileNameAfter = TEST_DIR2 + '/' + FILE_NAME2;

    std::atomic<bool> pathMatches(false), pathAfterMatches(false);
    std::atomic<int> moveFromCount(0);
    std::atomic<int> moveToCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            pathMatches.store(movedFileName == event.path);
            moveFromCount++;
        }
        else if (DirectoryWatch::Event::MoveTo == event.type)
        {
            pathAfterMatches.store(movedFileNameAfter == event.path);
            moveToCount++;
            latch.Set();
        }
        else
            latch.Set();
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
        EXPECT_EQ(1, moveToCount);
        EXPECT_TRUE(pathMatches);
        EXPECT_TRUE(pathAfterMatches);
    }
}

// moves a file from not-watched directory
TEST_F(DirectoryWatchTest, MoveFromNotWatchedDir)
{
    const std::string movedFileName = TEST_DIR_NOT_WATCHED + '/' + FILE_NAME;
    const std::string movedFileNameAfter = TEST_DIR + '/' + FILE_NAME2;

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
}

// moves a file to not-watched directory
TEST_F(DirectoryWatchTest, MoveToNotWatchedDir)
{
    const std::string movedFileName = TEST_DIR + '/' + FILE_NAME;
    const std::string movedFileNameAfter = TEST_DIR_NOT_WATCHED + '/' + FILE_NAME2;

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
}

// moves watched directory
TEST_F(DirectoryWatchTest, MoveWatchedDir)
{
    const std::string movedDir = TEST_DIR + '/' + DIR_NAME;
    const std::string movedDirAfter = TEST_DIR + '/' + DIR_NAME2;

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
}

TEST_F(DirectoryWatchTest, ModifyFile)
{
    const std::string readFileName = TEST_DIR + '/' + FILE_NAME;
    const std::string writeFileName = TEST_DIR + '/' + FILE_NAME2;

    Latch latch;
    std::atomic<int> modifyCount(0);
    std::atomic<bool> pathMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        pathMatches.store(writeFileName == event.path);
        latch.Set();
        modifyCount++;
    };

    const int fileData = 1234;
    int data;

    // prepare files
    {
        data = fileData;
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
}

// add and remove watch
TEST_F(DirectoryWatchTest, RemoveWatch)
{
    const std::string createdFileName = TEST_DIR + '/' + FILE_NAME;

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
