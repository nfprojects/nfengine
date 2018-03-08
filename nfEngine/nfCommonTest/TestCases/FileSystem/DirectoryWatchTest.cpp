#include "PCH.hpp"
#include "nfCommon/FileSystem/DirectoryWatch.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Utils/Latch.hpp"
#include "nfCommon/Containers/String.hpp"

using namespace NFE::Common;

namespace {

// latch timeout in milliseconds
const unsigned int LATCH_TIMEOUT = 200;

// not watched directory
const String TEST_DIR0 = "dirwatch_test_dir_notwatched";
// watched directories:
const String TEST_DIR1 = "dirwatch_test_dir";
const String TEST_DIR2 = "dirwatch_test_dir_2";

const String TEST_DIR0_FILE1 = TEST_DIR0 + "/file1";
const String TEST_DIR1_FILE1 = TEST_DIR1 + "/file1";
const String TEST_DIR1_FILE2 = TEST_DIR1 + "/file2";
const String TEST_DIR1_DIR1 = TEST_DIR1 + "/dir1";
const String TEST_DIR1_DIR2 = TEST_DIR1 + "/dir2";
const String TEST_DIR2_FILE1 = TEST_DIR2 + "/file1";

} // namespace

class DirectoryWatchTest : public testing::Test
{
protected:
    void SetUp()
    {
        EXPECT_TRUE(FileSystem::CreateDirIfNotExist(TEST_DIR0));
        EXPECT_TRUE(FileSystem::CreateDirIfNotExist(TEST_DIR1));
        EXPECT_TRUE(FileSystem::CreateDirIfNotExist(TEST_DIR2));
    }

    void TearDown()
    {
        EXPECT_TRUE(FileSystem::Remove(TEST_DIR0, true));
        EXPECT_TRUE(FileSystem::Remove(TEST_DIR1, true));
        EXPECT_TRUE(FileSystem::Remove(TEST_DIR2, true));
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
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::Create));
    // verify adding duplicate event filter
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::Create));
    // try watching not existing path
    ASSERT_FALSE(watch.WatchPath(StringView("not_existing_path"), DirectoryWatch::Event::Create));
}

TEST_F(DirectoryWatchTest, MultipleWatches)
{
    Latch dirCreateLatch, dirDeleteLatch;
    std::atomic<bool> wrongType(false);
    std::atomic<int> dirCreateCount(0);
    std::atomic<int> dirDeleteCount(0);

    auto callbackA = [&](const DirectoryWatch::EventData& event)
    {
        if (event.type != DirectoryWatch::Event::Create)
            wrongType.store(true);
        else if (event.path == TEST_DIR1_DIR1)
        {
            dirCreateCount++;
            dirCreateLatch.Set();
        }
    };

    auto callbackB = [&](const DirectoryWatch::EventData& event)
    {
        if (event.type != DirectoryWatch::Event::Delete)
            wrongType.store(true);
        else if (event.path == TEST_DIR1_DIR1)
        {
            dirDeleteCount++;
            dirDeleteLatch.Set();
        }
    };

    DirectoryWatch watchA;
    watchA.SetCallback(callbackA);
    ASSERT_TRUE(watchA.WatchPath(TEST_DIR1, DirectoryWatch::Event::Create));

    DirectoryWatch watchB;
    watchB.SetCallback(callbackB);
    ASSERT_TRUE(watchB.WatchPath(TEST_DIR1, DirectoryWatch::Event::Delete));

    // create file and directory
    ASSERT_TRUE(FileSystem::CreateDir(TEST_DIR1_DIR1));
    ASSERT_TRUE(FileSystem::Remove(TEST_DIR1_DIR1));

    // wait for DirectoryWatch event
    EXPECT_TRUE(dirCreateLatch.Wait(LATCH_TIMEOUT));
    EXPECT_TRUE(dirDeleteLatch.Wait(LATCH_TIMEOUT));

    EXPECT_FALSE(wrongType);
    EXPECT_EQ(1, dirCreateCount);
    EXPECT_EQ(1, dirDeleteCount);
}

TEST_F(DirectoryWatchTest, Create)
{
    Latch fileLatch, dirLatch;
    std::atomic<bool> wrongType(false);
    std::atomic<int> dirCreateCount(0);
    std::atomic<int> fileCreateCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (event.type != DirectoryWatch::Event::Create)
            wrongType.store(true);

        if (event.path == TEST_DIR1_DIR1)
        {
            dirCreateCount++;
            dirLatch.Set();

        }
        else if (event.path == TEST_DIR1_FILE1)
        {
            fileCreateCount++;
            fileLatch.Set();
        }
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::Create));

    // create file and directory
    ASSERT_TRUE(FileSystem::CreateDir(TEST_DIR1_DIR1));
    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR1_FILE1));

    // wait for DirectoryWatch event
    EXPECT_TRUE(fileLatch.Wait(LATCH_TIMEOUT));
    EXPECT_TRUE(dirLatch.Wait(LATCH_TIMEOUT));

    EXPECT_FALSE(wrongType);
    EXPECT_EQ(1, dirCreateCount);
    EXPECT_EQ(1, fileCreateCount);
}

TEST_F(DirectoryWatchTest, Delete)
{
    Latch fileLatch, dirLatch;
    std::atomic<bool> wrongType(false);
    std::atomic<int> dirDeleteCount(0);
    std::atomic<int> fileDeleteCount(0);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (event.type != DirectoryWatch::Event::Delete)
            wrongType.store(true);

        if (event.path == TEST_DIR1_DIR1)
        {
            dirDeleteCount++;
            dirLatch.Set();
        }
        else if (event.path == TEST_DIR1_FILE1)
        {
            fileDeleteCount++;
            fileLatch.Set();
        }
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::Delete));

    // create file and directory ...
    ASSERT_TRUE(FileSystem::CreateDir(TEST_DIR1_DIR1));
    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR1_FILE1));
    // ... and remove them
    ASSERT_TRUE(FileSystem::Remove(TEST_DIR1_DIR1));
    ASSERT_TRUE(FileSystem::Remove(TEST_DIR1_FILE1));

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
    std::atomic<bool> pathMatches(false), pathAfterMatches(false);
    std::atomic<int> moveFromCount(0);
    std::atomic<int> moveToCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type)
        {
            pathMatches.store(TEST_DIR1_FILE1 == event.path);
            moveFromCount++;
        }
        else if (DirectoryWatch::Event::MoveTo == event.type)
        {
            pathAfterMatches.store(TEST_DIR1_FILE2 == event.path);
            moveToCount++;
            latch.Set();
        }
        else
            latch.Set();
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1,
                                DirectoryWatch::Event::MoveFrom |
                                DirectoryWatch::Event::MoveTo |
                                DirectoryWatch::Event::Delete));

    // create file and rename it
    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR1_FILE1));
    ASSERT_TRUE(FileSystem::Move(TEST_DIR1_FILE1, TEST_DIR1_FILE2));
    // wait for DirectoryWatch event
    EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

    EXPECT_EQ(1, moveFromCount);
    EXPECT_EQ(1, moveToCount);
    EXPECT_TRUE(pathAfterMatches);
    EXPECT_TRUE(pathMatches);
}

// moves a file to another watched directory
TEST_F(DirectoryWatchTest, MoveToAnotherWatchedDir)
{
    std::atomic<bool> pathMatches(false), pathAfterMatches(false);
    std::atomic<int> moveFromCount(0);
    std::atomic<int> moveToCount(0);
    Latch latch;

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        if (DirectoryWatch::Event::MoveFrom == event.type ||
            DirectoryWatch::Event::Delete == event.type)
        {
            pathMatches.store(TEST_DIR1_FILE1 == event.path);
            moveFromCount++;
        }
        else if (DirectoryWatch::Event::MoveTo == event.type ||
                 DirectoryWatch::Event::Create == event.type)
        {
            pathAfterMatches.store(TEST_DIR2_FILE1 == event.path);
            moveToCount++;
            latch.Set();
        }
        else
            latch.Set();
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1,
                                DirectoryWatch::Event::MoveFrom |
                                DirectoryWatch::Event::MoveTo |
                                DirectoryWatch::Event::Delete));
    ASSERT_TRUE(watch.WatchPath(TEST_DIR2,
                                DirectoryWatch::Event::MoveFrom |
                                DirectoryWatch::Event::MoveTo |
                                DirectoryWatch::Event::Create |
                                DirectoryWatch::Event::Delete));

    // create file and rename it
    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR1_FILE1));
    ASSERT_TRUE(FileSystem::Move(TEST_DIR1_FILE1, TEST_DIR2_FILE1));
    // wait for DirectoryWatch event
    EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

    EXPECT_EQ(1, moveFromCount);
    EXPECT_EQ(1, moveToCount);
    EXPECT_TRUE(pathMatches);
    EXPECT_TRUE(pathAfterMatches);
}

// moves a file from not-watched directory
TEST_F(DirectoryWatchTest, MoveFromNotWatchedDir)
{
    Latch latch;
    std::atomic<bool> pathMatches(false);
    std::atomic<bool> typeMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        typeMatches.store(DirectoryWatch::Event::MoveTo == event.type ||
                          DirectoryWatch::Event::Create == event.type);
        pathMatches.store(TEST_DIR1_FILE1 == event.path);
        latch.Set();
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1,
                                DirectoryWatch::Event::MoveFrom |
                                DirectoryWatch::Event::MoveTo |
                                DirectoryWatch::Event::Create |
                                DirectoryWatch::Event::Delete));

    // create file and rename it
    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR0_FILE1));
    ASSERT_TRUE(FileSystem::Move(TEST_DIR0_FILE1, TEST_DIR1_FILE1));
    // wait for DirectoryWatch event
    EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

    EXPECT_TRUE(pathMatches);
    EXPECT_TRUE(typeMatches);
}

// moves a file to not-watched directory
TEST_F(DirectoryWatchTest, MoveToNotWatchedDir)
{
    Latch latch;
    std::atomic<bool> pathMatches(false);
    std::atomic<bool> typeMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        typeMatches.store(DirectoryWatch::Event::MoveFrom == event.type ||
                          DirectoryWatch::Event::Delete == event.type);
        pathMatches.store(TEST_DIR1_FILE1 == event.path);
        latch.Set();
    };

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1,
                                DirectoryWatch::Event::MoveFrom |
                                DirectoryWatch::Event::MoveTo |
                                DirectoryWatch::Event::Delete));

    // create file and rename it
    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR1_FILE1));
    ASSERT_TRUE(FileSystem::Move(TEST_DIR1_FILE1, TEST_DIR0_FILE1));
    // wait for DirectoryWatch event
    EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

    EXPECT_TRUE(pathMatches);
    EXPECT_TRUE(typeMatches);
}

TEST_F(DirectoryWatchTest, ModifyFile)
{
    Latch latch;
    std::atomic<int> modifyCount(0);
    std::atomic<bool> pathMatches(false);

    auto callback = [&](const DirectoryWatch::EventData& event)
    {
        pathMatches.store(TEST_DIR1_FILE2 == event.path);
        latch.Set();
        modifyCount++;
    };

    // random data to write to the file
    const int fileData = 1234;
    int data;

    // prepare files
    {
        data = fileData;
        File fileA(TEST_DIR1_FILE1, AccessMode::Write);
        ASSERT_EQ(sizeof(data), fileA.Write(&data, sizeof(data)));

        File fileB(TEST_DIR1_FILE2, AccessMode::Write);
        ASSERT_EQ(sizeof(data), fileB.Write(&data, sizeof(data)));
    }

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::Modify));

    {
        File fileA(TEST_DIR1_FILE1, AccessMode::Read);
        ASSERT_EQ(sizeof(data), fileA.Read(&data, sizeof(data)));

        File fileB(TEST_DIR1_FILE2, AccessMode::Write);
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

// add and remove watch
TEST_F(DirectoryWatchTest, RemoveWatch)
{
    std::atomic<bool> callbackCalled(false);
    auto callback = [&](const DirectoryWatch::EventData&)
    {
        callbackCalled.store(true);
    };

    ASSERT_TRUE(FileSystem::TouchFile(TEST_DIR1_FILE1));

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::Create |
                                           DirectoryWatch::Event::Delete));
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1, DirectoryWatch::Event::None));

    EXPECT_TRUE(FileSystem::Remove(TEST_DIR1_FILE1));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_FALSE(callbackCalled);
}


// on Windows, ReadDirectoryChangesW does not report changes to the watched directory itself
#ifndef WIN32

// moves watched directory
TEST_F(DirectoryWatchTest, MoveWatchedDir)
{
    Latch latch;
    std::atomic<bool> pathMatches(false);
    std::atomic<bool> typeMatches(false);

    auto callback = [&] (const DirectoryWatch::EventData& event)
    {
        typeMatches.store(DirectoryWatch::Event::MoveFrom == event.type ||
                          DirectoryWatch::Event::Delete == event.type);
        pathMatches.store(TEST_DIR1_DIR1 == event.path);
        latch.Set();
    };

    ASSERT_TRUE(FileSystem::CreateDir(TEST_DIR1_DIR1));

    DirectoryWatch watch;
    watch.SetCallback(callback);
    ASSERT_TRUE(watch.WatchPath(TEST_DIR1_DIR1,
                                DirectoryWatch::Event::MoveFrom |
                                DirectoryWatch::Event::MoveTo |
                                DirectoryWatch::Event::Delete));

    ASSERT_TRUE(FileSystem::Move(TEST_DIR1_DIR1, TEST_DIR1_DIR2));
    // wait for DirectoryWatch event
    EXPECT_TRUE(latch.Wait(LATCH_TIMEOUT));

    EXPECT_TRUE(pathMatches);
    EXPECT_TRUE(typeMatches);
}

#endif // ifndef WIN32
