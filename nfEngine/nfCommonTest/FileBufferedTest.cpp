/**
* @file
* @author mkulagowski (mkkulagowski(at)gmail.com)
* @brief  Unit tests for FileBuffered class.
*/

#include "PCH.hpp"
#include "../nfCommon/FileBuffered.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/nfCommon.hpp"
#include <type_traits>


using namespace NFE::Common;

// Global variables for the tests
namespace {
const int bufferSize = 1000;                              //< Size of the test buffer
const NFE::uint8 operationsUpperLimit = 10;               //< Number of operations to perform on the buffer

} // namespace

class FileBufferedTest : public testing::Test
{
public:
    NFE::uint8 mBufferExpected[bufferSize];
    NFE::Math::Random mRand;
    const std::string mPath = "./testFile.buffered";

    void SetUp()
    {
        // Fill buffer with random data
        for (int i = 0; i < bufferSize; i++)
            mBufferExpected[i] = static_cast<NFE::uint8>(i);//mRand.GetInt());
    }

    void TearDown()
    {
        // Clean up after tests
        FileSystem::Remove(mPath);
    }
};

TEST_F(FileBufferedTest, Constructors)
{
    // Due to a bogus path, no file will be open for Read operation, but it shouldn't throw
    std::string path = "./some/path";
    ASSERT_NO_THROW(FileBuffered());
    ASSERT_NO_THROW(FileBuffered(path, AccessMode::Read));
    ASSERT_NO_THROW(FileBuffered(path, AccessMode::Read, true));
    ASSERT_NO_THROW(FileBuffered(FileBuffered(path, AccessMode::Read, true)));

    // Just to make sure...
    ASSERT_TRUE(std::is_move_constructible<FileBuffered>::value);
    ASSERT_FALSE(std::is_copy_constructible<FileBuffered>::value);
}

TEST_F(FileBufferedTest, Read)
{
    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    // Save values buffer to the file
    File testFile(mPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    testFile.Close();

    FileBuffered testBufferedFile(mPath, AccessMode::Read);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Do tha read, yo!
    size_t readSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * readSize;
        ASSERT_EQ(readSize, testBufferedFile.Read(bufferActual + shift, readSize));
    }

    // Check that data has been read successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testBufferedFile.Close();
}

TEST_F(FileBufferedTest, Write)
{
    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    FileBuffered testBufferedFile(mPath, AccessMode::Write, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // So for one last time nigga make some writes
    size_t writeSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * writeSize;
        ASSERT_EQ(writeSize, testBufferedFile.Write(mBufferExpected + shift, writeSize));
    }

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testBufferedFile.Close();

    File testFile(mPath, AccessMode::Read);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Read(bufferActual, bufferSize));
    testFile.Close();

    // Check that data has been written successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));
}

TEST_F(FileBufferedTest, OpenClose)
{
    // Make sure file is opened after constructor
    FileBuffered testBufferedFile(mPath, AccessMode::Write, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Make sure file is closed after Close() method
    testBufferedFile.Close();
    ASSERT_FALSE(testBufferedFile.IsOpened());

    // Make sure file is opened after Open() method
    testBufferedFile.Open(mPath, AccessMode::Write);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Make sure file is closed after Close() method
    testBufferedFile.Close();
    ASSERT_FALSE(testBufferedFile.IsOpened());
}

TEST_F(FileBufferedTest, OperationsOnClosed)
{
    // Make sure file is opened after constructor
    FileBuffered testBufferedFile(mPath, AccessMode::ReadWrite, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Make sure file is closed after Close() method
    testBufferedFile.Close();
    ASSERT_FALSE(testBufferedFile.IsOpened());

    // Make sure no operations may be performed on closed file
    char buffer[bufferSize];
    ASSERT_EQ(0, testBufferedFile.Write(buffer, bufferSize));
    ASSERT_EQ(0, testBufferedFile.Read(buffer, bufferSize));
}

TEST_F(FileBufferedTest, InvalidOperations)
{
    char buffer[bufferSize];

    // Open file for writing, then try to read
    FileBuffered testBufferedFile(mPath, AccessMode::Write, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    ASSERT_EQ(0, testBufferedFile.Read(buffer, bufferSize));

    testBufferedFile.Close();

    // Reopen file for reading, then try to write
    testBufferedFile.Open(mPath, AccessMode::Read, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    ASSERT_EQ(0, testBufferedFile.Write(buffer, bufferSize));

    testBufferedFile.Close();
}
