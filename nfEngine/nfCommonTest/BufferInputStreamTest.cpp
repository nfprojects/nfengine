#include "stdafx.hpp"
#include "../nfCommon/InputStream.hpp"

using namespace NFE::Common;

namespace {
    const std::string TEXT = "The quick brown fox jumps over the lazy dog";
	const int STRSIZE = 43;
} // namespace


TEST(BufferInputStreamTest, NullBufferTest)
{
	BufferInputStream stream(nullptr, STRSIZE);
	ASSERT_EQ(stream.GetSize(), STRSIZE);
	ASSERT_EQ(stream.Seek(1), false);
	ASSERT_EQ(stream.Read(1, nullptr), false);
}

TEST(BufferInputStreamTest, StringBufferTest)
{
	BufferInputStream stream(TEXT.c_str(), STRSIZE);
	char charBuffer[STRSIZE];
    ASSERT_EQ(stream.Read(static_cast<size_t>(STRSIZE), charBuffer), STRSIZE);
    ASSERT_EQ(memcmp(charBuffer, TEXT.c_str(), STRSIZE), 0);
}

TEST(BufferInputStreamTest, StringBufferSeekTest)
{
	BufferInputStream stream(TEXT.c_str(), STRSIZE);
	size_t halfSize = STRSIZE / 2;
    char charBuffer[STRSIZE];
    bool seekResult = stream.Seek(halfSize);
    ASSERT_EQ(seekResult, true);

    size_t readResult = stream.Read(halfSize, charBuffer);
    ASSERT_EQ(readResult, halfSize);

    ASSERT_EQ(memcmp(charBuffer, TEXT.c_str() + halfSize, halfSize), 0);
}