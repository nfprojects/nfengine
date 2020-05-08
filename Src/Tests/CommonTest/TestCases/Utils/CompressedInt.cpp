#include "PCH.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/Buffer.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"

using namespace NFE;
using namespace Common;

static const uint32 valuesToTest32[] = { 0u, 127u, 128u, 2345u, 396293u, 25601235u, 2109479126u, 0xFFFFFFFFu };
static const uint64 valuesToTest64[] = { 0llu, 127llu, 128llu, 2345llu, 396293llu, 25601235llu, 2109479126llu, 16491649651100927llu, 0xFFFFFFFFFFFFFFFFllu };

TEST(CompressedInt, Uint32)
{
    for (uint32 i = 0; i < ArraySize(valuesToTest32); ++i)
    {
        const uint32 refValue = valuesToTest32[i];

        CompressedUint<uint32>::BufferType buffer;

        const uint32 bufferSize = CompressedUint<uint32>::Encode(refValue, buffer);
        ASSERT_GT(bufferSize, 0u);

        uint32 readValue = 0x12345678;
        ASSERT_TRUE(CompressedUint<uint32>::Decode(buffer, bufferSize, readValue));

        EXPECT_EQ(refValue, readValue);
    }
}


TEST(CompressedInt, Stream_Uint32)
{
    for (uint32 i = 0; i < ArraySize(valuesToTest32); ++i)
    {
        const uint32 refValue = valuesToTest32[i];

        Buffer buffer;

        {
            BufferOutputStream stream(buffer);
            ASSERT_TRUE(stream.WriteCompressedUint(refValue));
        }

        {
            BufferInputStream stream(buffer);
            uint32 readValue = 0x12345678;
            ASSERT_TRUE(stream.ReadCompressedUint(readValue));
            EXPECT_EQ(refValue, readValue);
        }
    }
}

TEST(CompressedInt, Uint64)
{
    for (uint32 i = 0; i < ArraySize(valuesToTest64); ++i)
    {
        const uint64 refValue = valuesToTest64[i];

        CompressedUint<uint64>::BufferType buffer;

        const uint32 bufferSize = CompressedUint<uint64>::Encode(refValue, buffer);
        ASSERT_GT(bufferSize, 0u);

        uint64 readValue = 0x1234567812345678llu;
        ASSERT_TRUE(CompressedUint<uint64>::Decode(buffer, bufferSize, readValue));

        EXPECT_EQ(refValue, readValue);
    }
}


TEST(CompressedInt, Stream_Uint64)
{
    for (uint32 i = 0; i < ArraySize(valuesToTest64); ++i)
    {
        const uint64 refValue = valuesToTest64[i];

        Buffer buffer;

        {
            BufferOutputStream stream(buffer);
            ASSERT_TRUE(stream.WriteCompressedUint(refValue));
        }

        {
            BufferInputStream stream(buffer);
            uint64 readValue = 0x1234567812345678llu;
            ASSERT_TRUE(stream.ReadCompressedUint(readValue));
            EXPECT_EQ(refValue, readValue);
        }
    }
}
