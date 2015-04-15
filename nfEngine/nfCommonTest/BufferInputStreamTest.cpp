#include "stdafx.hpp"
#include "../nfCommon/InputStream.hpp"

#include "test.hpp"

using namespace NFE::Common;


class BufferInputStreamTest : public testing::Test
{
protected:
    char *mBuffer;

    void SetUp()
    {
        mBuffer = (char*) malloc(TEXTSIZE + 1);
        mBuffer[TEXTSIZE] = '\0';
    }
    void TearDown()
    {
        free(mBuffer);
    }
};

TEST_F(BufferInputStreamTest, NullBufferTest)
{
    // Create stream pointing to nullptr
    BufferInputStream stream(nullptr, TEXTSIZE);

    // Size should be as in constructor, methods should all fail
    ASSERT_EQ(stream.GetSize(), TEXTSIZE);
    ASSERT_EQ(stream.Seek(1), false);
    ASSERT_EQ(stream.Read(1, nullptr), false);
}

TEST_F(BufferInputStreamTest, StringBufferTest)
{
    // Create stream pointing to our TEXT
    BufferInputStream stream(TEXT.data(), TEXTSIZE);

    // Write stream to our buffer and compare it with TEXT
    ASSERT_EQ(stream.Read(TEXTSIZE, mBuffer), TEXTSIZE);
    ASSERT_EQ(memcmp(mBuffer, TEXT.data(), TEXTSIZE), 0);
}

TEST_F(BufferInputStreamTest, StringBufferSeekTest)
{
    // Create stream pointing to our TEXT
    BufferInputStream stream(TEXT.data(), TEXTSIZE);

    // Seek to the middle of the stream (TEXT/2) and compare it with 2nd half of TEXT
    size_t halfSize = TEXTSIZE / 2;
    bool seekResult = stream.Seek(halfSize);
    ASSERT_EQ(seekResult, true);
    size_t readResult = stream.Read(halfSize, mBuffer);
    ASSERT_EQ(readResult, halfSize);
    ASSERT_EQ(memcmp(mBuffer, TEXT.data() + halfSize, halfSize), 0);
}