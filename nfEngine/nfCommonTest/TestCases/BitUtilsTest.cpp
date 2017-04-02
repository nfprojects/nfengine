#include "PCH.hpp"
#include "nfCommon/Utils/BitUtils.hpp"
#include "nfCommon/Math/Random.hpp"

// TODO remove
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Logger/Logger.hpp"


using namespace NFE;
using namespace Common;

TEST(BitUtils, CountTrailingZeros8)
{
    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0xFF) << i;
        EXPECT_EQ(i, CountTrailingZeros<uint8>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0x1) << i;
        EXPECT_EQ(i, CountTrailingZeros<uint8>(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountTrailingZeros16)
{
    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0xFFFF) << static_cast<uint16>(i);
        EXPECT_EQ(i, CountTrailingZeros<uint16>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0x1) << static_cast<uint16>(i);
        EXPECT_EQ(i, CountTrailingZeros<uint16>(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountTrailingZeros32)
{
    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0xFFFFFFFF) << static_cast<uint32>(i);
        EXPECT_EQ(i, CountTrailingZeros<uint32>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0x1) << static_cast<uint32>(i);
        EXPECT_EQ(i, CountTrailingZeros<uint32>(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountTrailingZeros64)
{
    for (uint8 i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0xFFFFFFFFFFFFFFFFull) << static_cast<uint64>(i);
        EXPECT_EQ(i, CountTrailingZeros<uint64>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0x1) << static_cast<uint64>(i);
        EXPECT_EQ(i, CountTrailingZeros<uint64>(val)) << "val=" << val;
    }

    Timer timer;
    timer.Start();

    uint64 n = 0;
    for (uint64 i = 0; i < 10000000; ++i)
        n += CountTrailingZeros<uint64>(i);

    LOG_INFO("COUNT TRAILING ZEROS: n = %llu, t = %f", n, (float)timer.Stop() * 1000.0f);
}

//////////////////////////////////////////////////////////////////////////

TEST(BitUtils, CountLeadingZeros8)
{
    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0xFF) >> i;
        EXPECT_EQ(i, CountLeadingZeros<uint8>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 8; ++i)
    {
        const uint8 val = static_cast<uint8>(0x80) >> i;
        EXPECT_EQ(i, CountLeadingZeros<uint8>(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountLeadingZeros16)
{
    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0xFFFF) >> static_cast<uint16>(i);
        EXPECT_EQ(i, CountLeadingZeros<uint16>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 16; ++i)
    {
        const uint16 val = static_cast<uint16>(0x8000) >> static_cast<uint16>(i);
        EXPECT_EQ(i, CountLeadingZeros<uint16>(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountLeadingZeros32)
{
    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0xFFFFFFFF) >> static_cast<uint32>(i);
        EXPECT_EQ(i, CountLeadingZeros<uint32>(val)) << "val=" << val;
    }

    for (uint8 i = 0; i < 32; ++i)
    {
        const uint32 val = static_cast<uint32>(0x80000000) >> static_cast<uint32>(i);
        EXPECT_EQ(i, CountLeadingZeros<uint32>(val)) << "val=" << val;
    }
}

TEST(BitUtils, CountLeadingZeros64)
{
    for (size_t i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0xFFFFFFFFFFFFFFFFul) >> static_cast<uint64>(i);
        EXPECT_EQ(i, CountLeadingZeros<uint64>(val)) << "val=" << val;
    }

    for (size_t i = 0; i < 64; ++i)
    {
        const uint64 val = static_cast<uint64>(0x8000000000000000ul) >> static_cast<uint64>(i);
        EXPECT_EQ(i, CountLeadingZeros<uint64>(val)) << "val=" << val;
    }

    Timer timer;
    timer.Start();

    uint64 n = 0;
    for (uint64 i = 0; i < 10000000; ++i)
        n += CountLeadingZeros<uint64>(i);

    LOG_INFO("COUNT LEADING ZEROS: n = %llu, t = %f", n, (float)timer.Stop() * 1000.0f);
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
size_t CountBitsNaive(T x)
{
    // count bits the naive way
    size_t c = 0;
    for (size_t j = 0; j < CHAR_BIT * sizeof(T); ++j)
    {
        c += ((static_cast<T>(1) << static_cast<T>(j)) & x) >> j;
    }

    return c;
}

TEST(BitUtils, CountBits8)
{
    ASSERT_EQ(0, CountBits<uint8>(0));
    ASSERT_EQ(8, CountBits<uint8>(0xFF));

    const size_t maxIterations = 100;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint8 num = static_cast<uint8>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, CountBits<uint8>(num)) << "num=" << num;
    }
}

TEST(BitUtils, CountBits16)
{
    ASSERT_EQ(0, CountBits<uint16>(0));
    ASSERT_EQ(16, CountBits<uint16>(0xFFFF));

    const size_t maxIterations = 200;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint16 num = static_cast<uint16>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, CountBits<uint16>(num)) << "num=" << num;
    }
}

TEST(BitUtils, CountBits32)
{
    ASSERT_EQ(0, CountBits<uint32>(0));
    ASSERT_EQ(32, CountBits<uint32>(0xFFFFFFFF));

    const size_t maxIterations = 400;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint32 num = static_cast<uint32>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, CountBits<uint32>(num)) << "num=" << num;
    }
}

TEST(BitUtils, CountBits64)
{
    ASSERT_EQ(0, CountBits<uint64>(0));
    ASSERT_EQ(64, CountBits<uint64>(0xFFFFFFFFFFFFFFFFul));

    const size_t maxIterations = 800;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint32 num = static_cast<uint32>(random.GetInt());
        size_t expected = CountBitsNaive(num);
        EXPECT_EQ(expected, CountBits<uint64>(num)) << "num=" << num;
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
T ReverseBitsNaive(T v)
{
    T r = v; // r will be reversed bits of v; first get LSB of v
    int s = sizeof(T) * CHAR_BIT - 1; // extra shift needed at end

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
    ASSERT_EQ(0, ReverseBits<uint8>(0));
    ASSERT_EQ(0xFF, ReverseBits<uint8>(0xFF));

    const size_t maxIterations = 100;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint8 num = static_cast<uint8>(random.GetInt());
        uint8 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, ReverseBits<uint8>(num)) << "num=" << num;
    }
}

TEST(BitUtils, ReverseBits16)
{
    ASSERT_EQ(0, ReverseBits<uint16>(0));
    ASSERT_EQ(0xFFFF, ReverseBits<uint16>(0xFFFF));

    const size_t maxIterations = 200;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint16 num = static_cast<uint16>(random.GetInt());
        uint16 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, ReverseBits<uint16>(num)) << "num=" << num;
    }
}

TEST(BitUtils, ReverseBits32)
{
    ASSERT_EQ(0, ReverseBits<uint32>(0));
    ASSERT_EQ(0xFFFFFFFF, ReverseBits<uint32>(0xFFFFFFFF));

    const size_t maxIterations = 400;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint32 num = static_cast<uint32>(random.GetInt());
        uint32 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, ReverseBits<uint32>(num)) << "num=" << num;
    }
}

TEST(BitUtils, ReverseBits64)
{
    ASSERT_EQ(0, ReverseBits<uint64>(0));
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFull, ReverseBits<uint64>(0xFFFFFFFFFFFFFFFFull));

    const size_t maxIterations = 800;

    Math::Random random;
    for (size_t i = 0; i < maxIterations; ++i)
    {
        uint64 num = static_cast<uint64>(random.GetInt());
        uint64 expected = ReverseBitsNaive(num);
        EXPECT_EQ(expected, ReverseBits<uint64>(num)) << "num=" << num;
    }
}

//////////////////////////////////////////////////////////////////////////

TEST(BitUtils, SwapEndianness)
{
    EXPECT_EQ(0x2301, SwapEndianness<uint16>(0x0123));
    EXPECT_EQ(0x67452301, SwapEndianness<uint32>(0x01234567));
    EXPECT_EQ(0xEFCDAB8967452301, SwapEndianness<uint64>(0x0123456789ABCDEFull));
}