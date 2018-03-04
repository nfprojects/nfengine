#include "PCH.hpp"
#include "nfCommon/Utils/OutputStream.hpp"

#include "Constants.hpp"

using namespace NFE::Common;


TEST(BufferOutputStreamTest, ConstructorTest)
{
    // Perform class methods on an empty stream
    BufferOutputStream stream;
    ASSERT_EQ(stream.GetSize(), 0u);
    ASSERT_EQ(stream.GetData(), nullptr);
    ASSERT_EQ(stream.Write(TEXT.Str(), 1), 1u);
}

TEST(BufferOutputStreamTest, WriteTest)
{
    // Create stream and insert our TEXT
    BufferOutputStream stream;
    stream.Write(TEXT.Str(), TEXTSIZE);

    // Check if our data (TEXT) was inserted correctly
    size_t streamSize = stream.GetSize();
    const void* streamData = stream.GetData();
    ASSERT_EQ(streamSize, TEXTSIZE);
    ASSERT_EQ(memcmp(streamData, TEXT.Str(), TEXTSIZE), 0);


    // Insert our TEXT for the second time, so the stream is 2*TEXT
    stream.Write(TEXT.Str(), TEXTSIZE);

    // Check if our data (2*TEXT) was inserted correctly
    streamSize = stream.GetSize();
    streamData = stream.GetData();
    String doubleStr = TEXT + TEXT;
    ASSERT_EQ(streamSize, 2 * TEXTSIZE);
    ASSERT_EQ(memcmp(streamData, doubleStr.Str(), 2 * TEXTSIZE), 0);
}

TEST(BufferOutputStreamTest, ClearTest)
{
    // Create stream and insert our TEXT
    BufferOutputStream stream;
    size_t writeResult = stream.Write(TEXT.Str(), TEXTSIZE);
    ASSERT_EQ(TEXTSIZE, writeResult);

    // Clear and check if everything got cleared
    stream.Clear();
    size_t streamSize = stream.GetSize();
    const void* streamData = stream.GetData();
    ASSERT_EQ(streamSize, 0u);
    ASSERT_EQ(streamData, nullptr);
}