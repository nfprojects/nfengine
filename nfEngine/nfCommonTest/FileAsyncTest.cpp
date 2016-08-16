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

class FileAsyncTest;

struct CallbackData
{
    FileAsyncTest* testObjPtr;
    NFE::uint8 shift;

    CallbackData(FileAsyncTest* obj, NFE::uint8 shft)
        : testObjPtr(obj)
        , shift(shft)
    {
    }

    CallbackData()
        : testObjPtr(nullptr)
        , shift(0)
    {
    }
};

class FileAsyncTest : public testing::Test
{
public:
    // For counting how many operations were done
    std::atomic_uint mReadOperationsCounter;
    std::atomic_uint mWriteOperationsCounter;

    // For checking which operations succeeded
    std::atomic_uint mReadOperations;
    std::atomic_uint mWriteOperations;

    unsigned char mShiftArray[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; //< Data array to pass to callbacks
    Latch *mOperationsLatchR, *mOperationsLatchW;                   //< Latch to wait for operations to finish (or timeout)
    static const int mBufferSize = 1000;                            //< Size of the test buffer
    static const NFE::uint8 mOperationsUpperLimit = 10;             //< Number of operations to perform on the buffer
    static const unsigned int mTimeLimitMiliseconds = 10000;        //< Timeout for all operations
    static const int mExpectedOperations = 0x3FF;                   //< Expected state of read/mWriteOperations variables
                                                                    //  after all operations succeed
    NFE::uint8 mBufferExpected[mBufferSize];
    NFE::Math::Random mRand;
    const std::string mPath = "./testFile.async";

    void SetUp()
    {
        // Reset all counters
        mWriteOperationsCounter.store(0);
        mReadOperationsCounter.store(0);
        mReadOperations.store(0);
        mWriteOperations.store(0);

        // Fill buffer with random data
        for (int i = 0; i < mBufferSize; i++)
            mBufferExpected[i] = static_cast<NFE::uint8>(mRand.GetInt());
    }

    void TearDown()
    {
        // Clean up after tests
        FileSystem::Remove(mPath);
    }

    // Callback for read & write operations
    static void TestCallback(void* obj, FileAsync* filePtr, size_t bytesProcessed, bool isRead)
    {
        UNUSED(bytesProcessed);
        UNUSED(filePtr);

        CallbackData* data = reinterpret_cast<CallbackData*>(obj);
        NFE::uint8 shift = data->shift;
        FileAsyncTest* objPtr = data->testObjPtr;

        if (isRead)
        {
            objPtr->mReadOperationsCounter.fetch_add(1);
            objPtr->mReadOperations.fetch_or(1 << shift);

            if (mOperationsUpperLimit == objPtr->mReadOperationsCounter.load())
                objPtr->mOperationsLatchR->Set();
        }
        else
        {
            objPtr->mWriteOperationsCounter.fetch_add(1);
            objPtr->mWriteOperations.fetch_or(1 << shift);

            if (mOperationsUpperLimit == objPtr->mWriteOperationsCounter.load())
                objPtr->mOperationsLatchW->Set();
        }
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
    NFE::uint8 bufferActual[mBufferSize];
    for (int i = 0; i < mBufferSize; i++)
        bufferActual[i] = 0;

    // Save values buffer to the file
    File testFile(mPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(mBufferSize, testFile.Write(mBufferExpected, mBufferSize));
    testFile.Close();

    FileAsync testAsyncFile(mPath, AccessMode::Read, TestCallback);
    ASSERT_TRUE(testAsyncFile.Init());
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset latch ptr
    Latch readLatch;
    mOperationsLatchR = &readLatch;

    // Prepare callbackData
    std::vector<CallbackData> cbDataArray(mOperationsUpperLimit);

    // Enqueue read jobs
    size_t readSize = mBufferSize / mOperationsUpperLimit;
    for (int i = 0; i < mOperationsUpperLimit; i++)
    {
        cbDataArray[i].shift = static_cast<NFE::uint8>(i);
        cbDataArray[i].testObjPtr = this;
        NFE::uint64 shift = i * readSize;
        ASSERT_TRUE(testAsyncFile.Read(bufferActual + shift, readSize, shift,
                                       reinterpret_cast<void*>(&cbDataArray[i])));
    }

    // Make sure all threads finish (before timeout)
    bool latchRes = mOperationsLatchR->Wait(mTimeLimitMiliseconds);
    mOperationsLatchR = nullptr;
    ASSERT_TRUE(latchRes);

    // Check that callback for every operation was called
    ASSERT_EQ(mExpectedOperations, mReadOperations.load());

    // Doublecheck number of operations done
    ASSERT_EQ(mOperationsUpperLimit, mReadOperationsCounter.load());

    // Check that data has been read successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, mBufferSize));

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testAsyncFile.Close();
}

TEST_F(FileAsyncTest, Write)
{
    NFE::uint8 bufferActual[mBufferSize];
    for (int i = 0; i < mBufferSize; i++)
        bufferActual[i] = 0;

    FileAsync testAsyncFile(mPath, AccessMode::Write, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.Init());
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Reset latch ptr
    Latch writeLatch;
    mOperationsLatchW = &writeLatch;

    // Prepare callbackData
    std::vector<CallbackData> cbDataArray(mOperationsUpperLimit);

    // Enqueue write jobs
    size_t writeSize = mBufferSize / mOperationsUpperLimit;
    for (int i = 0; i < mOperationsUpperLimit; i++)
    {
        cbDataArray[i].shift = static_cast<NFE::uint8>(i);
        cbDataArray[i].testObjPtr = this;
        NFE::uint64 shift = i * writeSize;
        testAsyncFile.Write(mBufferExpected + shift, writeSize, shift,
                            reinterpret_cast<void*>(&cbDataArray[i]));
    }

    // Make sure all threads finish (before timeout)
    bool latchRes = mOperationsLatchW->Wait(mTimeLimitMiliseconds);
    mOperationsLatchW = nullptr;
    ASSERT_TRUE(latchRes);

    // Check that callback for every operation was called
    ASSERT_EQ(mExpectedOperations, mWriteOperations.load());

    // Doublecheck number of operations done
    ASSERT_EQ(mOperationsUpperLimit, mWriteOperationsCounter.load());

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testAsyncFile.Close();

    File testFile(mPath, AccessMode::Read);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(mBufferSize, testFile.Read(bufferActual, mBufferSize));
    testFile.Close();

    // Check that data has been written successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, mBufferSize));
}

TEST_F(FileAsyncTest, OpenClose)
{
    // Make sure file is opened after constructor
    FileAsync testAsyncFile(mPath, AccessMode::Write, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());

    // Make sure file is opened after Open() method
    testAsyncFile.Open(mPath, AccessMode::Write);
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());
}

TEST_F(FileAsyncTest, OperationsOnClosed)
{
    // Make sure file is opened after constructor
    FileAsync testAsyncFile(mPath, AccessMode::ReadWrite, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.Init());
    ASSERT_TRUE(testAsyncFile.IsOpened());

    // Make sure file is closed after Close() method
    testAsyncFile.Close();
    ASSERT_FALSE(testAsyncFile.IsOpened());

    // Make sure no operations may be performed on closed file
    char buffer[mBufferSize];
    ASSERT_FALSE(testAsyncFile.Write(buffer, mBufferSize, 0, nullptr));
    ASSERT_FALSE(testAsyncFile.Read(buffer, mBufferSize, 0, nullptr));
}

TEST_F(FileAsyncTest, InvalidOperations)
{
    char buffer[mBufferSize];

    // Open file for writing, then try to read
    FileAsync testAsyncFile(mPath, AccessMode::Write, TestCallback, true);
    ASSERT_TRUE(testAsyncFile.Init());
    ASSERT_TRUE(testAsyncFile.IsOpened());

    ASSERT_FALSE(testAsyncFile.Read(buffer, mBufferSize, 0, nullptr));

    testAsyncFile.Close();

    // Reopen file for reading, then try to write
    testAsyncFile.Open(mPath, AccessMode::Read, true);
    ASSERT_TRUE(testAsyncFile.Init());
    ASSERT_TRUE(testAsyncFile.IsOpened());

    ASSERT_FALSE(testAsyncFile.Write(buffer, mBufferSize, 0, nullptr));

    testAsyncFile.Close();
}
