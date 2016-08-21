#include "PCH.hpp"
#include "nfCommon/Math/Conversions.hpp"

using namespace NFE::Math;

TEST(Math, ConvertHalfFloat)
{
    // values that can be converted to half float and back to float without bits loss
    const float losslessValues[] =
    {
        0.0f,
        -1.0f, 1.0f,
        -0.25, 0.25f,
        48.0f, -48.0f,
        -1024.0f, 1024.0f,
    };

    for (size_t i = 0; i < NFE::ArraySize(losslessValues); ++i)
    {
        const float val = losslessValues[i];
        SCOPED_TRACE("Val = " + std::to_string(val));
        EXPECT_EQ(val, ToFloat(ToHalfFloat(val)));
    }


    const float testValues[] = { -0.21f, 15.31f, 135.02f, -1356.0f, 65504.0f, -65504.0f };
    for (size_t i = 0; i < NFE::ArraySize(testValues); ++i)
    {
        const float val = testValues[i];
        SCOPED_TRACE("Val = " + std::to_string(val));

        const float maxError = Abs(val) / 1000.0f;
        EXPECT_NEAR(val, ToFloat(ToHalfFloat(val)), maxError);
    }
}

TEST(Math, ConvertInt8)
{
    // Float to 8-bit signed integer
    EXPECT_EQ(0, ToInt8(0.0f));
    EXPECT_EQ(1, ToInt8(1.0f));
    EXPECT_EQ(-1, ToInt8(-1.0f));
    EXPECT_EQ((1 << 7) - 1, ToInt8(1000.0f));
    EXPECT_EQ(-(1 << 7), ToInt8(-1000.0f));

    // Float to 8-bit unsigned integer
    EXPECT_EQ(0, ToUint8(0.0f));
    EXPECT_EQ(1, ToUint8(1.0f));
    EXPECT_EQ(0, ToUint8(-1.0f));
    EXPECT_EQ((1 << 8) - 1, ToUint8(1000.0f));

    // Float to 8-bit signed integer (normalized)
    EXPECT_EQ(0, ToNormInt8(0.0f));
    EXPECT_EQ((1 << 6), ToNormInt8(0.5f));
    EXPECT_EQ(-(1 << 6), ToNormInt8(-0.5f));
    EXPECT_EQ(-(1 << 7) + 1, ToNormInt8(-1.0f));
    EXPECT_EQ((1 << 7) - 1, ToNormInt8(1.0f));
    EXPECT_EQ((1 << 7) - 1, ToNormInt8(1.0001f));
    EXPECT_EQ((1 << 7) - 1, ToNormInt8(100.0f));
    EXPECT_EQ(-(1 << 7), ToNormInt8(-100.0f));

    // Float to 8-bit unsigned integer (normalized)
    EXPECT_EQ(0, ToNormUint8(0.0f));
    EXPECT_EQ((1 << 7), ToNormUint8(0.5f));
    EXPECT_EQ((1 << 8) - 1, ToNormUint8(1.0f));
    EXPECT_EQ(0, ToNormUint8(-1.0f));
    EXPECT_EQ((1 << 8) - 1, ToNormUint8(100.0f));
}

TEST(Math, ConvertInt16)
{
    // Float to 16-bit signed integer
    EXPECT_EQ(0, ToInt16(0.0f));
    EXPECT_EQ(1, ToInt16(1.0f));
    EXPECT_EQ(-1, ToInt16(-1.0f));
    EXPECT_EQ((1 << 15) - 1, ToInt16(100000.0f));
    EXPECT_EQ(-(1 << 15), ToInt16(-100000.0f));

    // Float to 16-bit unsigned integer
    EXPECT_EQ(0, ToUint16(0.0f));
    EXPECT_EQ(1, ToUint16(1.0f));
    EXPECT_EQ(0, ToUint16(-1.0f));
    EXPECT_EQ((1 << 16) - 1, ToUint16(100000.0f));
    EXPECT_EQ((1 << 16) - 1, ToUint16(100000.0f));

    // Float to 16-bit signed integer (normalized)
    EXPECT_EQ(0, ToNormInt16(0.0f));
    EXPECT_EQ((1 << 14), ToNormInt16(0.5f));
    EXPECT_EQ(-(1 << 14), ToNormInt16(-0.5f));
    EXPECT_EQ(-(1 << 15) + 1, ToNormInt16(-1.0f));
    EXPECT_EQ((1 << 15) - 1, ToNormInt16(1.0f));
    EXPECT_EQ((1 << 15) - 1, ToNormInt16(1.0001f));
    EXPECT_EQ((1 << 15) - 1, ToNormInt16(100.0f));
    EXPECT_EQ(-(1 << 15), ToNormInt16(-100.0f));

    // Float to 8-bit unsigned integer (normalized)
    EXPECT_EQ(0, ToNormUint16(0.0f));
    EXPECT_EQ((1 << 15), ToNormUint16(0.5f));
    EXPECT_EQ((1 << 16) - 1, ToNormUint16(1.0f));
    EXPECT_EQ(0, ToNormUint16(-1.0f));
    EXPECT_EQ((1 << 16) - 1, ToNormUint16(100.0f));
}

TEST(Math, ConvertInt32)
{
    // Float to 32-bit signed integer
    EXPECT_EQ(0, ToInt32(0.0f));
    EXPECT_EQ(1, ToInt32(1.0f));
    EXPECT_EQ(-1, ToInt32(-1.0f));
    EXPECT_EQ((1LL << 31) - 1, ToInt32(2147483646.999f));
    EXPECT_EQ(-(1LL << 31), ToInt32(-2147483648.0f));
    EXPECT_EQ((1LL << 31) - 1, ToInt32(1.0e+10f));
    EXPECT_EQ(-(1LL << 31), ToInt32(-1.0e+10f));

    // Float to 32-bit unsigned integer
    EXPECT_EQ(0, ToUint32(0.0f));
    EXPECT_EQ(1, ToUint32(1.0f));
    EXPECT_EQ(0, ToUint32(-1.0f));
    EXPECT_EQ((1LL << 32) - 1, ToUint32(4294967295.0f));
    EXPECT_EQ((1LL << 32) - 1, ToUint32(1.0e+10f));
}
