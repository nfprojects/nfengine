/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for FileAsync class.
 */

#include "PCH.hpp"
#include "../nfCommon/FileAsync.hpp"
#include "../nfCommon/File.hpp"
#include <type_traits>
#include <atomic>
#include "../nfCommon/Timer.hpp"

using namespace NFE::Common;

// Global variables for the tests
std::atomic_uchar readOperations = 0;
std::atomic_uchar writeOperations = 0;
const uchar operationsUpperLimit = 4;
const double timeLimitSeconds = 10.0;
const int bufferSize = 100;

// Callback for read operations
void readCallbackfunc(void* obj)
{
    uchar pos = *reinterpret_cast<uchar*>(obj);
    readOperations.store(readOperations.load(std::memory_order_acquire) | pos,
                         std::memory_order_release);
}

// Callback for write operations
void writeCallbackfunc(void* obj)
{
    uchar pos = *reinterpret_cast<uchar*>(obj);
    writeOperations.store(writeOperations.load(std::memory_order_acquire) | pos,
                          std::memory_order_release);
}

TEST(FileAsyncTest, Constructors)
{
    auto simpleCallback = [](void*)->void{
        return;
    };

    // Due to a bogus path, no file will be open for Read operation, but it shouldn't throw
    std::string path = "./some/path";
    ASSERT_NO_THROW(FileAsync());
    ASSERT_NO_THROW(FileAsync(simpleCallback, simpleCallback));
    ASSERT_NO_THROW(FileAsync(nullptr, nullptr));
    ASSERT_NO_THROW(FileAsync(path, AccessMode::Read, simpleCallback, simpleCallback));
    ASSERT_NO_THROW(FileAsync(path, AccessMode::Read, simpleCallback, simpleCallback, true));
    ASSERT_NO_THROW(FileAsync(FileAsync(path, AccessMode::Read, simpleCallback, simpleCallback, true)));

    // Just to make sure...
    ASSERT_TRUE(std::is_move_constructible<FileAsync>::value);
    ASSERT_FALSE(std::is_copy_constructible<FileAsync>::value);
}

TEST(FileAsyncTest, Read)
{
    const std::string testPath = "./testFile.async";

    uchar buffer[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        buffer[i] = static_cast<uchar>(i);

    // Save filled buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    testFile.Write(buffer, bufferSize);
    testFile.Close();

    FileAsync testAsyncFile(testPath, AccessMode::Read, &readCallbackfunc,
                            &writeCallbackfunc);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset read operations flag
    readOperations.store(0);

    // Reset buffer and perform 4 async reads into it
    uchar readShiftArray[4] = {0, 1, 2, 3};
    memset(buffer, 0, bufferSize);
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        uint64 shift = i * (bufferSize / 4);
        testAsyncFile.Read(buffer + static_cast<uchar>(shift), bufferSize / 4, shift, &readShiftArray[i]);
    }

    Timer mTimer;
    bool flag = false;
    double timePassed = 0;
    mTimer.Start();
    // Wait for operations to finish. Timeout limit is set in @timeLimitSeconds.
    do
    {
        if (readOperations.load() == 0x1111)
            flag = true;

        timePassed += mTimer.Stop();
        ASSERT_LE(timePassed, timeLimitSeconds);
    } while (flag);

    // Check that data has been read successfully
    for (int i = 0; i < bufferSize; i++)
        ASSERT_EQ(static_cast<uchar>(i), buffer[i]);

    // Reset read operations flag
    readOperations.store(0);
}

TEST(FileAsyncTest, Write)
{
    const std::string testPath = "./testFile.async";

    uchar buffer[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        buffer[i] = static_cast<uchar>(i);

    FileAsync testAsyncFile(testPath, AccessMode::Write, &readCallbackfunc,
                            &writeCallbackfunc, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset write operations flag
    writeOperations.store(0);

    // Save buffer to file in 4 async writes
    uchar readShiftArray[4] = {0, 1, 2, 3};

    for (int i = 0; i < operationsUpperLimit; i++)
    {
        int shift = i * (bufferSize / 4);
        testAsyncFile.Write(buffer + shift, bufferSize / 4, shift, &readShiftArray[i]);
    }

    Timer mTimer;
    bool flag = false;
    double timePassed = 0;
    mTimer.Start();
    // Wait for operations to finish. Timeout limit is set in @timeLimitSeconds.
    do
    {
        if (writeOperations.load() == 0x1111)
            flag = true;

        timePassed += mTimer.Stop();
        ASSERT_LE(timePassed, timeLimitSeconds);
    } while (flag);

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testAsyncFile.Close();

    // Reset buffer and perform a read into it
    memset(buffer, 0, bufferSize);
    File testFile(testPath, AccessMode::Read);
    testFile.Read(buffer, bufferSize);
    testFile.Close();

    // Check that data has been read successfully
    for (int i = 0; i < bufferSize; i++)
        ASSERT_EQ(static_cast<uchar>(i), buffer[i]);

    // Reset write operations flag
    writeOperations.store(0);
}

TEST(FileAsyncTest, OpenClose)
{
    const std::string testPath = "./testFile.async";

    // Make sure file is opened after constructor
    FileAsync testAsyncFile(testPath, AccessMode::Write, &readCallbackfunc,
                            &writeCallbackfunc, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());

    // Make sure file is opened after Open() method
    testAsyncFile.Open(testPath, AccessMode::Write);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());
}

TEST(FileAsyncTest, OperationsOnClosed)
{
    const std::string testPath = "./testFile.async";

    // Make sure file is opened after constructor
    FileAsync testAsyncFile(testPath, AccessMode::ReadWrite, &readCallbackfunc,
                            &writeCallbackfunc, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());

    // Make sure no operations may be performed on closed file
    char buffer[bufferSize];
    ASSERT_FALSE(testAsyncFile.Write(buffer, bufferSize, 0, nullptr));
    ASSERT_FALSE(testAsyncFile.Read(buffer, bufferSize, 0, nullptr));
}

TEST(FileAsyncTest, InvalidOperations)
{
    const std::string testPath = "./testFile.async";
        char buffer[bufferSize];

        // Open file for reading, then try to write
    FileAsync testAsyncFile(testPath, AccessMode::Read, &readCallbackfunc,
                            &writeCallbackfunc, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    ASSERT_FALSE(testAsyncFile.Write(buffer, bufferSize, 0, nullptr));
    testAsyncFile.Close();

    // Reopen file for writing, then try to read
    testAsyncFile.Open(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    ASSERT_FALSE(testAsyncFile.Read(buffer, bufferSize, 0, nullptr));
}