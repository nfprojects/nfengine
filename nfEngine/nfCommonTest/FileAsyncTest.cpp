/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for FileAsync class.
 */

#include "PCH.hpp"
#include "../nfCommon/FileAsync.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Latch.hpp"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/nfCommon.hpp"
#include <type_traits>
#include <atomic>


using namespace NFE::Common;

// Global variables for the tests
namespace {
// For counting how many operations were done
std::atomic_uint readOperationsCounter = 0;
std::atomic_uint writeOperationsCounter = 0;

// For checking which operations succeeded
std::atomic_uint readOperations = 0;
std::atomic_uint writeOperations = 0;

const int bufferSize = 1000;                              //< Size of the test buffer
const NFE::uint8 operationsUpperLimit = 10;               //< Number of operations to perform on the buffer
NFE::uint8 shiftArray[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; //< Data array to pass to callbacks
const unsigned int timeLimitMiliseconds = 2500;           //< Timeout for all operations
Latch* operationsLatch;                                   //< Latch to wait for operations to finish (or timeout)
const int expectedOperations = 0x3FF;                     //< Expected state of read/writeOperations variables
                                                          //  after all operations succeed

// Callback for read & write operations
void TestCallback(void* obj, FileAsync* filePtr, size_t bytesProcessed, bool isRead)
{
    UNUSED(bytesProcessed);
    UNUSED(filePtr);

    int shift = *reinterpret_cast<NFE::uint8*>(obj);
    LOG_DEBUG("callback entered, isRead=%d, shift=%i\n", isRead, shift);
    if (isRead)
    {
        readOperationsCounter.fetch_add(1);
        readOperations.fetch_or(1 << shift);

        if (operationsUpperLimit - 1 == readOperationsCounter.load())
            operationsLatch->Set();
    }
    else
    {
        writeOperationsCounter.fetch_add(1);
        writeOperations.fetch_or(1 << shift);

        if (operationsUpperLimit - 1 == writeOperationsCounter.load())
            operationsLatch->Set();
    }
}

} // namespace

class FileAsyncTest : public testing::Test
{
public:
    NFE::uint8 mBufferExpected[bufferSize];
    NFE::Math::Random mRand;

    void SetUp()
    {
        for (int i = 0; i < bufferSize; i++)
            mBufferExpected[i] = static_cast<NFE::uint8>(mRand.GetInt());
    }
};

TEST_F(FileAsyncTest, Constructors)
{
    auto simpleCallback = [](void*, FileAsync*, size_t, bool) ->void { return; };

    // Due to a bogus path, no file will be open for Read operation, but it shouldn't throw
    std::string path = "./some/path";
    ASSERT_NO_THROW(FileAsync());
    // ASSERT_NO_THROW(FileAsync(simpleCallback));
    ASSERT_NO_THROW(FileAsync(nullptr));
    ASSERT_NO_THROW(FileAsync(path, AccessMode::Read, simpleCallback));
    ASSERT_NO_THROW(FileAsync(path, AccessMode::Read, simpleCallback, true));
    ASSERT_NO_THROW(FileAsync(FileAsync(path, AccessMode::Read, simpleCallback, true)));

    // Just to make sure...
    ASSERT_TRUE(std::is_move_constructible<FileAsync>::value);
    ASSERT_FALSE(std::is_copy_constructible<FileAsync>::value);
}

TEST_F(FileAsyncTest, Read)
{
    const std::string testPath = "./testFile.async";

    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    // Save values buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    testFile.Close();

    FileAsync testAsyncFile(testPath, AccessMode::Read, TestCallback);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset read operations flags and latch
    readOperations.store(0);
    readOperationsCounter.store(0);

    Latch readLatch;
    operationsLatch = &readLatch;

    // Enqueue read jobs
    size_t readSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * readSize;
        ASSERT_TRUE(testAsyncFile.Read(bufferActual + shift, readSize, shift,
                                       reinterpret_cast<void*>(&shiftArray[i])));
    }

    // Make sure all threads finish (before timeout)
    ASSERT_TRUE(operationsLatch->Wait(timeLimitMiliseconds)) << readOperations << " " << readOperationsCounter;

    // Check that callback for every operation was called
    ASSERT_EQ(expectedOperations, readOperations.load());

    // Doublecheck number of operations done
    ASSERT_EQ(operationsUpperLimit, readOperationsCounter.load());

    // Check that data has been read successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(testPath));
}

TEST_F(FileAsyncTest, Write)
{
    const std::string testPath = "./testFile.async";

    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    FileAsync testAsyncFile(testPath, AccessMode::Write, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset write operations flags and latch
    writeOperations.store(0);
    writeOperationsCounter.store(0);

    Latch writeLatch;
    operationsLatch = &writeLatch;

    // Enqueue write jobs
    size_t writeSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * writeSize;
        testAsyncFile.Write(mBufferExpected + shift, writeSize, shift,
                            reinterpret_cast<void*>(&shiftArray[i]));
    }

    // Make sure all threads finish (before timeout)
    ASSERT_TRUE(operationsLatch->Wait(timeLimitMiliseconds)) << writeOperations << " " << writeOperationsCounter;

    // Check that callback for every operation was called
    ASSERT_EQ(expectedOperations, writeOperations.load());

    // Doublecheck number of operations done
    ASSERT_EQ(operationsUpperLimit, writeOperationsCounter.load());

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testAsyncFile.Close();

    File testFile(testPath, AccessMode::Read);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Read(bufferActual, bufferSize));
    testFile.Close();

    // Check that data has been written successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(testPath));
}

TEST_F(FileAsyncTest, OpenClose)
{
    const std::string testPath = "./testFile.async";

    // Make sure file is opened after constructor
    FileAsync testAsyncFile(testPath, AccessMode::Write, TestCallback, true);
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

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(testPath));
}

TEST_F(FileAsyncTest, OperationsOnClosed)
{
    const std::string testPath = "./testFile.async";

    // Make sure file is opened after constructor
    FileAsync testAsyncFile(testPath, AccessMode::ReadWrite, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());

    // Make sure no operations may be performed on closed file
    char buffer[bufferSize];
    ASSERT_FALSE(testAsyncFile.Write(buffer, bufferSize, 0, nullptr));
    ASSERT_FALSE(testAsyncFile.Read(buffer, bufferSize, 0, nullptr));

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(testPath));
}

TEST_F(FileAsyncTest, InvalidOperations)
{
    const std::string testPath = "./testFile.async";
    char buffer[bufferSize];

    // Open file for writing, then try to read
    FileAsync testAsyncFile(testPath, AccessMode::Write, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    ASSERT_FALSE(testAsyncFile.Read(buffer, bufferSize, 0, nullptr));

    testAsyncFile.Close();

    // Reopen file for reading, then try to write
    testAsyncFile.Open(testPath, AccessMode::Read, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    ASSERT_FALSE(testAsyncFile.Write(buffer, bufferSize, 0, nullptr));

    testAsyncFile.Close();

    // cleanup
    EXPECT_EQ(true, FileSystem::Remove(testPath));
}