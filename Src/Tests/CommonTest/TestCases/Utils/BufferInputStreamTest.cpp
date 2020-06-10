#include "PCH.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"

#include "Constants.hpp"



using namespace NFE::Common;


class BufferInputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<char[]> mBuffer;

    void SetUp()
    {
        mBuffer.reset(new char[gTextSize + 1]);
        mBuffer.get()[gTextSize] = '\0';
    }
};

TEST_F(BufferInputStreamTest, NullBufferTest)
{
    // Create stream pointing to nullptr
    BufferInputStream stream(nullptr, gTextSize);

    // Size should be as in constructor, methods should all fail
    ASSERT_EQ(stream.GetSize(), gTextSize);
    ASSERT_EQ(stream.Seek(1), false);
    ASSERT_EQ(stream.Read(nullptr, 1), 0u);
}

TEST_F(BufferInputStreamTest, StringBufferTest)
{
    // Create stream pointing to our gText
    BufferInputStream stream(gText.Str(), gTextSize);

    // Write stream to our buffer and compare it with gText
    ASSERT_EQ(stream.Read(mBuffer.get(), gTextSize), gTextSize);
    ASSERT_EQ(memcmp(mBuffer.get(), gText.Str(), gTextSize), 0);
}

TEST_F(BufferInputStreamTest, StringBufferSeekTest)
{
    // Create stream pointing to our gText
    BufferInputStream stream(gText.Str(), gTextSize);

    // Seek to the middle of the stream (gText/2) and compare it with 2nd half of gText
    size_t halfSize = gTextSize / 2;
    bool seekResult = stream.Seek(halfSize);
    ASSERT_EQ(seekResult, true);
    size_t readResult = stream.Read(mBuffer.get(), halfSize);
    ASSERT_EQ(readResult, halfSize);
    ASSERT_EQ(memcmp(mBuffer.get(), gText.Str() + halfSize, halfSize), 0);
}
