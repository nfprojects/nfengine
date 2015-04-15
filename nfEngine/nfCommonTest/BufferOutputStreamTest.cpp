#include "stdafx.hpp"
#include "../nfCommon/OutputStream.hpp"

using namespace NFE::Common;

namespace {
    const std::string TEXT = "The quick brown fox jumps over the lazy dog";
    const int TEXTSIZE = TEXT.length();
} // namespace


TEST(BufferOutputStreamTest, ConstructorTest)
{
    BufferOutputStream stream;
    ASSERT_EQ(stream.GetSize(), 0);
    ASSERT_EQ(stream.GetData(), nullptr);
    ASSERT_EQ(stream.Write(TEXT.c_str(), 1), 1);
}

TEST(BufferOutputStreamTest, WriteTest)
{
    BufferOutputStream stream;
    stream.Write(TEXT.c_str(), TEXTSIZE);
    size_t streamSize = stream.GetSize();
    const void* streamData = stream.GetData();
    ASSERT_EQ(streamSize, TEXTSIZE);
    ASSERT_EQ(memcmp(streamData, TEXT.c_str(), TEXTSIZE), 0);

    stream.Write(TEXT.c_str(), TEXTSIZE);
    streamSize = stream.GetSize();
    streamData = stream.GetData();
    std::string doubleStr = TEXT + TEXT;
    ASSERT_EQ(streamSize, 2 * TEXTSIZE);
    ASSERT_EQ(memcmp(streamData, doubleStr.c_str(), 2 * TEXTSIZE), 0);
}

TEST(BufferOutputStreamTest, ClearTest)
{
    BufferOutputStream stream;
    size_t writeResult = stream.Write(TEXT.c_str(), TEXTSIZE);

    stream.Clear();
    size_t streamSize = stream.GetSize();
    const void* streamData = stream.GetData();
    ASSERT_EQ(streamSize, 0);
    ASSERT_EQ(streamData, nullptr);
}