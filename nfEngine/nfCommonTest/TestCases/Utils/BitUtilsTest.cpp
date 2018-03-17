#include "PCH.hpp"
#include "nfCommon/Utils/BitUtils.hpp"
#include "nfCommon/Math/Random.hpp"


using namespace NFE;
using namespace Common;

TEST(BitUtils, CountTrailingZeros8)
{
    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0xFF) << i;
        EXPECT_EQ(i, BitUtils<uint8>::CountTrailingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0x1) << i;
        EXPECT_EQ(i, BitUtils<uint8>::CountTrailingZeros(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountTrailingZeros16)
{
    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0xFFFF) << static_cast<uint16>(i);
        EXPECT_EQ(i, BitUtils<uint16>::CountTrailingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0x1) << static_cast<uint16>(i);
        EXPECT_EQ(i, BitUtils<uint16>::CountTrailingZeros(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountTrailingZeros32)
{
    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0xFFFFFFFF) << static_cast<uint32>(i);
        EXPECT_EQ(i, BitUtils<uint32>::CountTrailingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0x1) << static_cast<uint32>(i);
        EXPECT_EQ(i, BitUtils<uint32>::CountTrailingZeros(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountTrailingZeros64)
{
    for (uint8 i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0xFFFFFFFFFFFFFFFFull) << static_cast<uint64>(i);
        EXPECT_EQ(i, BitUtils<uint64>::CountTrailingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0x1) << static_cast<uint64>(i);
        EXPECT_EQ(i, BitUtils<uint64>::CountTrailingZeros(val)) << "val=" << val;
    }
}

//////////////////////////////////////////////////////////////////////////

TEST(BitUtils, CountLeadingZeros8)
{
    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0xFF) >> i;
        EXPECT_EQ(i, BitUtils<uint8>::CountLeadingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0x80) >> i;
        EXPECT_EQ(i, BitUtils<uint8>::CountLeadingZeros(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountLeadingZeros16)
{
    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0xFFFF) >> static_cast<uint16>(i);
        EXPECT_EQ(i, BitUtils<uint16>::CountLeadingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0x8000) >> static_cast<uint16>(i);
        EXPECT_EQ(i, BitUtils<uint16>::CountLeadingZeros(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountLeadingZeros32)
{
    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0xFFFFFFFF) >> static_cast<uint32>(i);
        EXPECT_EQ(i, BitUtils<uint32>::CountLeadingZeros(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0x80000000) >> static_cast<uint32>(i);
        EXPECT_EQ(i, BitUtils<uint32>::CountLeadingZeros(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountLeadingZeros64)
{
    for (size_t i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0xFFFFFFFFFFFFFFFFul) >> static_cast<uint64>(i);
        EXPECT_EQ(i, BitUtils<uint64>::CountLeadingZeros(val)) << "val=" << val;
    }

    for (size_t i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0x8000000000000000ul) >> static_cast<uint64>(i);
        EXPECT_EQ(i, BitUtils<uint64>::CountLeadingZeros(val)) << "val=" << val;
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
size_t CountBitsNaive(T x)
{
    // count bits the naive way
    size_t c = 0;
    for (size_t j = 0; j < 8 * sizeof(T); ++j)
    {
        c += ((static_cast<T>(1) << static_cast<T>(j)) & x) >> j;
    }

    return c;
}

TEST(BitUtils, CountBits8)
{
    ASSERT_EQ(0u, BitUtils<uint8>::CountBits(0));
    ASSERT_EQ(8u, BitUtils<uint8>::CountBits(0xFF));

    const size_t maxIterations = 100;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint8 num = static_cast<uint8>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint8>::CountBits(num)) << "num=" << num;
    }
}

TEST(BitUtils, CountBits16)
{
    ASSERT_EQ(0u, BitUtils<uint16>::CountBits(0));
    ASSERT_EQ(16u, BitUtils<uint16>::CountBits(0xFFFF));

    const size_t maxIterations = 200;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint16 num = static_cast<uint16>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint16>::CountBits(num)) << "num=" << num;
    }
}

TEST(BitUtils, CountBits32)
{
    ASSERT_EQ(0u, BitUtils<uint32>::CountBits(0));
    ASSERT_EQ(32u, BitUtils<uint32>::CountBits(0xFFFFFFFF));

    const size_t maxIterations = 400;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint32 num = static_cast<uint32>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint32>::CountBits(num)) << "num=" << num;
    }
}

TEST(BitUtils, CountBits64)
{
    ASSERT_EQ(0u, BitUtils<uint64>::CountBits(0));
    ASSERT_EQ(64u, BitUtils<uint64>::CountBits(0xFFFFFFFFFFFFFFFFul));

    const size_t maxIterations = 800;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint32 num = static_cast<uint32>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint64>::CountBits(num)) << "num=" << num;
    }

    size_t n = 123;
    for (size_t i = 0; i < 100000000; ++i)
        n += BitUtils<uint64>::CountBits(static_cast<uint64>(i));
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
T ReverseBitsNaive(T v)
{
    T r = v; // r will be reversed bits of v; first get LSB of v
    int s = sizeof(T) * 8 - 1; // extra shift needed at end

    for (v >>= 1; v; v >>= 1)
    {
        r <<= 1;
        r |= v & 1;
        s--;
    }

    return r << s;
}

TEST(BitUtils, ReverseBits8)
{
    ASSERT_EQ(0u, BitUtils<uint8>::ReverseBits(0));
    ASSERT_EQ(0xFFu, BitUtils<uint8>::ReverseBits(0xFF));

    const size_t maxIterations = 100;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint8 num = static_cast<uint8>(random.GetInt());
        uint8 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint8>::ReverseBits(num)) << "num=" << num;
    }
}

TEST(BitUtils, ReverseBits16)
{
    ASSERT_EQ(0u, BitUtils<uint16>::ReverseBits(0));
    ASSERT_EQ(0xFFFFu, BitUtils<uint16>::ReverseBits(0xFFFF));

    const size_t maxIterations = 200;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint16 num = static_cast<uint16>(random.GetInt());
        uint16 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint16>::ReverseBits(num)) << "num=" << num;
    }
}

TEST(BitUtils, ReverseBits32)
{
    ASSERT_EQ(0u, BitUtils<uint32>::ReverseBits(0));
    ASSERT_EQ(0xFFFFFFFFu, BitUtils<uint32>::ReverseBits(0xFFFFFFFF));

    const size_t maxIterations = 400;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint32 num = static_cast<uint32>(random.GetInt());
        uint32 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint32>::ReverseBits(num)) << "num=" << num;
    }
}

TEST(BitUtils, ReverseBits64)
{
    ASSERT_EQ(0u, BitUtils<uint64>::ReverseBits(0));
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFull, BitUtils<uint64>::ReverseBits(0xFFFFFFFFFFFFFFFFull));

    const size_t maxIterations = 800;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint64 num = static_cast<uint64>(random.GetInt());
        uint64 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, BitUtils<uint64>::ReverseBits(num)) << "num=" << num;
    }
}

//////////////////////////////////////////////////////////////////////////

TEST(BitUtils, SwapEndianness)
{
    EXPECT_EQ(0x2301u, BitUtils<uint16>::SwapEndianness(0x0123));
    EXPECT_EQ(0x67452301u, BitUtils<uint32>::SwapEndianness(0x01234567));
    EXPECT_EQ(0xEFCDAB8967452301u, BitUtils<uint64>::SwapEndianness(0x0123456789ABCDEFull));
}

//////////////////////////////////////////////////////////////////////////

TEST(BitUtils, CreateBitmask8)
{
    EXPECT_EQ(0x00u, BitUtils<uint8>::CreateBitMask(0));
    EXPECT_EQ(0x01u, BitUtils<uint8>::CreateBitMask(1));
    EXPECT_EQ(0x03u, BitUtils<uint8>::CreateBitMask(2));
    EXPECT_EQ(0x07u, BitUtils<uint8>::CreateBitMask(3));
    EXPECT_EQ(0x0Fu, BitUtils<uint8>::CreateBitMask(4));
    EXPECT_EQ(0x1Fu, BitUtils<uint8>::CreateBitMask(5));
    EXPECT_EQ(0x3Fu, BitUtils<uint8>::CreateBitMask(6));
    EXPECT_EQ(0x7Fu, BitUtils<uint8>::CreateBitMask(7));
    EXPECT_EQ(0xFFu, BitUtils<uint8>::CreateBitMask(8));
    EXPECT_EQ(0xFFu, BitUtils<uint8>::CreateBitMask(9));
}

TEST(BitUtils, CreateBitmask16)
{
    EXPECT_EQ(0x00u, BitUtils<uint16>::CreateBitMask(0));
    EXPECT_EQ(0x01u, BitUtils<uint16>::CreateBitMask(1));
    EXPECT_EQ(0x03u, BitUtils<uint16>::CreateBitMask(2));
    EXPECT_EQ(0x07u, BitUtils<uint16>::CreateBitMask(3));
    EXPECT_EQ(0x0Fu, BitUtils<uint16>::CreateBitMask(4));
    EXPECT_EQ(0x1Fu, BitUtils<uint16>::CreateBitMask(5));
    EXPECT_EQ(0x3Fu, BitUtils<uint16>::CreateBitMask(6));
    EXPECT_EQ(0x7Fu, BitUtils<uint16>::CreateBitMask(7));
    EXPECT_EQ(0x00FFu, BitUtils<uint16>::CreateBitMask(8));
    EXPECT_EQ(0x01FFu, BitUtils<uint16>::CreateBitMask(9));
    EXPECT_EQ(0x03FFu, BitUtils<uint16>::CreateBitMask(10));
    EXPECT_EQ(0x07FFu, BitUtils<uint16>::CreateBitMask(11));
    EXPECT_EQ(0x0FFFu, BitUtils<uint16>::CreateBitMask(12));
    EXPECT_EQ(0x1FFFu, BitUtils<uint16>::CreateBitMask(13));
    EXPECT_EQ(0x3FFFu, BitUtils<uint16>::CreateBitMask(14));
    EXPECT_EQ(0x7FFFu, BitUtils<uint16>::CreateBitMask(15));
    EXPECT_EQ(0xFFFFu, BitUtils<uint16>::CreateBitMask(16));
    EXPECT_EQ(0xFFFFu, BitUtils<uint16>::CreateBitMask(17));
}