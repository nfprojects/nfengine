#include "stdafx.hpp"
#include "../nfCommon/InputStream.hpp"


namespace {
    const std::string TEXT = "The quick brown fox jumps over the lazy dog";
} // namespace
using namespace NFE::Common;

class BufferInputStreamTest : public testing::Test
{
protected:
    void SetUp()
    {
        mStrSize = static_cast<size_t>(TEXT.length());
        mStream = std::make_unique<BufferInputStream>(TEXT.c_str(), mStrSize);
        ASSERT_EQ(static_cast<size_t>(mStream->GetSize()), mStrSize);
    }
    size_t mStrSize;
    std::unique_ptr<BufferInputStream> mStream;
};


TEST_F(BufferInputStreamTest, NullBufferTest)
{
    BufferInputStream stream(nullptr, 13);
    ASSERT_EQ(stream.GetSize(), 13);
    ASSERT_EQ(stream.Seek(1), false);
    ASSERT_EQ(stream.Read(1, nullptr), false);
}

TEST_F(BufferInputStreamTest, StringBufferTest)
{
    char charBuffer[50];
    ASSERT_EQ(mStream->Read(mStrSize, charBuffer), mStrSize);
    ASSERT_EQ(memcmp(charBuffer, TEXT.c_str(), mStrSize), 0);
}

TEST_F(BufferInputStreamTest, StringBufferSeekTest)
{
    size_t halfSize = mStrSize / 2;
    char charBuffer[50];
    bool seekResult = mStream->Seek(halfSize);
    ASSERT_EQ(seekResult, true);

    size_t readResult = mStream->Read(halfSize, charBuffer);
    ASSERT_EQ(readResult, mStrSize / 2);

    ASSERT_EQ(memcmp(charBuffer, &TEXT[halfSize], halfSize), 0);
}