/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for FileAsync class.
 */

#include "PCH.hpp"

#include "Engine/Common/FileSystem/FileAsync.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/Utils/Latch.hpp"
#include "Engine/Common/Math/Random.hpp"


using namespace NFE;
using namespace NFE::Common;

// Global variables for the tests
namespace {
// For counting how many operations were done
std::atomic_uint readOperationsCounter;
std::atomic_uint writeOperationsCounter;

// For checking which operations succeeded
std::atomic_uint readOperations;
std::atomic_uint writeOperations;

const uint32 bufferSize = 1000;                      //< Size of the test buffer
const uint8 operationsUpperLimit = 10;               //< Number of operations to perform on the buffer
uint8 shiftArray[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; //< Data array to pass to callbacks
const uint32 timeLimitMiliseconds = 10000;           //< Timeout for all operations
Latch* operationsLatch;                              //< Latch to wait for operations to finish (or timeout)
const uint32 expectedOperations = 0x3FF;             //< Expected state of read/writeOperations variables
                                                     //  after all operations succeed

const StringView testPath("./testFile.async");

NFE::Math::Random random;

// Callback for read & write operations
void TestCallback(void* obj, FileAsync* filePtr, size_t bytesProcessed, bool isRead)
{
    NFE_UNUSED(bytesProcessed);
    NFE_UNUSED(filePtr);

    int shift = *reinterpret_cast<NFE::uint8*>(obj);

    if (isRead)
    {
        readOperationsCounter.fetch_add(1);
        readOperations.fetch_or(1 << shift);

        if (operationsUpperLimit == readOperationsCounter.load())
            operationsLatch->Set();
    }
    else
    {
        writeOperationsCounter.fetch_add(1);
        writeOperations.fetch_or(1 << shift);

        if (operationsUpperLimit == writeOperationsCounter.load())
            operationsLatch->Set();
    }
}

} // namespace

class FileAsyncTest : public testing::Test
{
public:
    NFE::uint8 mBufferExpected[bufferSize];

    void SetUp()
    {
        // Reset all counters
        writeOperationsCounter.store(0);
        readOperationsCounter.store(0);
        readOperations.store(0);
        writeOperations.store(0);

        // Fill buffer with random data
        for (int i = 0; i < bufferSize; i++)
            mBufferExpected[i] = static_cast<NFE::uint8>(random.GetInt());
    }

    void TearDown()
    {
        // Clean up after tests
        FileSystem::Remove(testPath);
    }
};

TEST_F(FileAsyncTest, Constructors)
{
    auto simpleCallback = [](void*, FileAsync*, size_t, bool) ->void { return; };

    // Due to a bogus path, no file will be open for Read operation, but it shouldn't throw
    const StringView path("./some/path");
    FileAsync();
    // FileAsync(simpleCallback);
    FileAsync(nullptr);
    FileAsync(path, AccessMode::Read, simpleCallback);
    FileAsync(path, AccessMode::Read, simpleCallback, true);
    FileAsync(FileAsync(path, AccessMode::Read, simpleCallback, true));

    // Just to make sure...
    ASSERT_TRUE(std::is_move_constructible<FileAsync>::value);
    ASSERT_FALSE(std::is_copy_constructible<FileAsync>::value);
}

TEST_F(FileAsyncTest, Read)
{
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

    // Reset latch ptr
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
    ASSERT_TRUE(operationsLatch->Wait(timeLimitMiliseconds)) << "Expected ops["
        << static_cast<int>(operationsUpperLimit) << "]: " << expectedOperations << std::endl << "Actual ops["
        << readOperationsCounter << "]:" << readOperations << std::endl;

    // Check that callback for every operation was called
    ASSERT_EQ(expectedOperations, readOperations.load());

    // Doublecheck number of operations done
    ASSERT_EQ(operationsUpperLimit, readOperationsCounter.load());

    // Check that data has been read successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testAsyncFile.Close();
}

TEST_F(FileAsyncTest, Write)
{
    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    FileAsync testAsyncFile(testPath, AccessMode::Write, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset latch ptr
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
    ASSERT_TRUE(operationsLatch->Wait(timeLimitMiliseconds)) << "Expected ops["
        << static_cast<int>(operationsUpperLimit) << "]: " << expectedOperations << std::endl << "Actual ops["
        << writeOperationsCounter << "]:" << writeOperations << std::endl;

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
}

TEST_F(FileAsyncTest, OpenClose)
{
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
}

TEST_F(FileAsyncTest, OperationsOnClosed)
{
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
}

TEST_F(FileAsyncTest, InvalidOperations)
{
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
}
