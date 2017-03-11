#include "PCH.hpp"
#include "nfCommon/Utils/InputStream.hpp"

#include "Constants.hpp"

#include <memory>


using namespace NFE::Common;


class BufferInputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<char[]> mBuffer;

    void SetUp()
    {
        mBuffer.reset(new char[TEXTSIZE + 1]);
        mBuffer.get()[TEXTSIZE] = '\0';
    }
};

TEST_F(BufferInputStreamTest, NullBufferTest)
{
    // Create stream pointing to nullptr
    BufferInputStream stream(nullptr, TEXTSIZE);

    // Size should be as in constructor, methods should all fail
    ASSERT_EQ(stream.GetSize(), TEXTSIZE);
    ASSERT_EQ(stream.Seek(1), false);
    ASSERT_EQ(stream.Read(nullptr, 1), 0);
}

TEST_F(BufferInputStreamTest, StringBufferTest)
{
    // Create stream pointing to our TEXT
    BufferInputStream stream(TEXT.data(), TEXTSIZE);

    // Write stream to our buffer and compare it with TEXT
    ASSERT_EQ(stream.Read(mBuffer.get(), TEXTSIZE), TEXTSIZE);
    ASSERT_EQ(memcmp(mBuffer.get(), TEXT.data(), TEXTSIZE), 0);
}

TEST_F(BufferInputStreamTest, StringBufferSeekTest)
{
    // Create stream pointing to our TEXT
    BufferInputStream stream(TEXT.data(), TEXTSIZE);

    // Seek to the middle of the stream (TEXT/2) and compare it with 2nd half of TEXT
    size_t halfSize = TEXTSIZE / 2;
    bool seekResult = stream.Seek(halfSize);
    ASSERT_EQ(seekResult, true);
    size_t readResult = stream.Read(mBuffer.get(), halfSize);
    ASSERT_EQ(readResult, halfSize);
    ASSERT_EQ(memcmp(mBuffer.get(), TEXT.data() + halfSize, halfSize), 0);
}
