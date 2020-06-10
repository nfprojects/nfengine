#include "PCH.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Memory/Buffer.hpp"
#include "Engine/Common/Containers/StringView.hpp"

#include "Constants.hpp"

using namespace NFE::Common;


TEST(BufferOutputStreamTest, ConstructorTest)
{
    // Perform class methods on an empty stream
    Buffer buffer;
    BufferOutputStream stream(buffer);
    ASSERT_EQ(stream.GetSize(), 0u);
    ASSERT_EQ(stream.GetData(), nullptr);
    ASSERT_EQ(stream.Write(gText.Str(), 1), 1u);
}

TEST(BufferOutputStreamTest, WriteTest)
{
    // Create stream and insert our gText
    Buffer buffer;
    BufferOutputStream stream(buffer);
    stream.Write(gText.Str(), gTextSize);

    // Check if our data (gText) was inserted correctly
    size_t streamSize = stream.GetSize();
    const void* streamData = stream.GetData();
    ASSERT_EQ(streamSize, gTextSize);
    ASSERT_EQ(memcmp(streamData, gText.Str(), gTextSize), 0);


    // Insert our gText for the second time, so the stream is 2*gText
    stream.Write(gText.Str(), gTextSize);

    // Check if our data (2*gText) was inserted correctly
    streamSize = stream.GetSize();
    streamData = stream.GetData();
    String doubleStr = gText + gText;
    ASSERT_EQ(streamSize, 2 * gTextSize);
    ASSERT_EQ(memcmp(streamData, doubleStr.Str(), 2 * gTextSize), 0);
}
