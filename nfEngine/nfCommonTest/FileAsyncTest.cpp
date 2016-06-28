/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for FileAsync class.
 */

#include "PCH.hpp"
#include "../nfCommon/FileAsync.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Latch.hpp"
#include <type_traits>
#include <atomic>
#include <stdlib.h>
#include <time.h>


using namespace NFE::Common;

// Global variables for the tests
std::atomic_uint readOperations = 0;
std::atomic_uint writeOperations = 0;
const uchar operationsUpperLimit = 10;
const unsigned int timeLimitMiliseconds = 10000;
const int bufferSize = 1000;

class FileAsyncTest : public testing::Test
{
public:
    int mBufferExpected[bufferSize];

    static void SetUpTestCase()
    {
        srand(static_cast<unsigned int>(time(NULL)));
    }

    void SetUp()
    {
        for (int i = 0; i < bufferSize; i++)
            mBufferExpected[i] = rand();
    }
};

// Callback for read operations
void readCallbackfunc(void* obj)
{
    Latch* latch = reinterpret_cast<Latch*>(obj);
    readOperations.fetch_add(1);
    latch->Set();
}

// Callback for write operations
void writeCallbackfunc(void* obj)
{
    Latch* latch = reinterpret_cast<Latch*>(obj);
    writeOperations.fetch_add(1);
    latch->Set();
}

TEST_F(FileAsyncTest, Constructors)
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

TEST_F(FileAsyncTest, Read)
{
    const std::string testPath = "./testFile.async";

    int bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    // Save values buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    testFile.Close();

    FileAsync testAsyncFile(testPath, AccessMode::Read, &readCallbackfunc,
                            &writeCallbackfunc);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset read operations flag
    readOperations.store(0);

    // Create latches and enqueue read jobs
    Latch latchArray[operationsUpperLimit];
    size_t readSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        uint64 shift = i * readSize;
        ASSERT_TRUE(testAsyncFile.Read(bufferActual + shift, readSize * sizeof(int), shift, &latchArray[i]));
    }

    // Make sure all threads finish (before timeout)
    for (int i = 0; i < operationsUpperLimit; i++)
        ASSERT_TRUE(latchArray[i].Wait(timeLimitMiliseconds));

    // Check that data has been read successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize * sizeof(int)));
    // Check that callback for every operation was called
    ASSERT_EQ(operationsUpperLimit, readOperations.load());

    // Reset read operations flag
    readOperations.store(0);
}

TEST_F(FileAsyncTest, Write)
{
    const std::string testPath = "./testFile.async";

    int bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    FileAsync testAsyncFile(testPath, AccessMode::Write, &readCallbackfunc,
                            &writeCallbackfunc, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset write operations flag
    writeOperations.store(0);

    // Create latches and enqueue write jobs
    Latch latchArray[operationsUpperLimit];
    size_t writeSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        uint64 shift = i * writeSize;
        testAsyncFile.Write(mBufferExpected + shift, writeSize, shift, &latchArray[i]);
    }

    // Make sure all threads finish (before timeout)
    for (int i = 0; i < operationsUpperLimit; i++)
        ASSERT_TRUE(latchArray[i].Wait(timeLimitMiliseconds));

    // Check that callback for every operation was called
    ASSERT_EQ(operationsUpperLimit, writeOperations.load());

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testAsyncFile.Close();

    File testFile(testPath, AccessMode::Read);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Read(bufferActual, bufferSize));
    testFile.Close();

    // Check that data has been written successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize * sizeof(int)));

    // Reset write operations flag
    writeOperations.store(0);
}

TEST_F(FileAsyncTest, OpenClose)
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

TEST_F(FileAsyncTest, OperationsOnClosed)
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

TEST_F(FileAsyncTest, InvalidOperations)
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