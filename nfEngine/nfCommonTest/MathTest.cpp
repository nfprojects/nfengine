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
        EXPECT_EQ(val, ConvertHalfFloatToFloat(ConvertFloatToHalfFloat(val)));
    }


    const float testValues[] = { -0.21f, 15.31f, 135.02f, -1356.0f, 65504.0f, -65504.0f };
    for (size_t i = 0; i < NFE::ArraySize(testValues); ++i)
    {
        const float val = testValues[i];
        SCOPED_TRACE("Val = " + std::to_string(val));

        const float maxError = Abs(val) / 1000.0f;
        EXPECT_NEAR(val, ConvertHalfFloatToFloat(ConvertFloatToHalfFloat(val)), maxError);
    }
}

TEST(Math, ConvertInt8)
{
    // Float to 8-bit signed integer
    EXPECT_EQ(0, ConvertFloatToInt8(0.0f));
    EXPECT_EQ(1, ConvertFloatToInt8(1.0f));
    EXPECT_EQ(-1, ConvertFloatToInt8(-1.0f));
    EXPECT_EQ((1 << 7) - 1, ConvertFloatToInt8(1000.0f));
    EXPECT_EQ(-(1 << 7), ConvertFloatToInt8(-1000.0f));

    // Float to 8-bit unsigned integer
    EXPECT_EQ(0, ConvertFloatToUint8(0.0f));
    EXPECT_EQ(1, ConvertFloatToUint8(1.0f));
    EXPECT_EQ(0, ConvertFloatToUint8(-1.0f));
    EXPECT_EQ((1 << 8) - 1, ConvertFloatToUint8(1000.0f));

    // Float to 8-bit signed integer (normalized)
    EXPECT_EQ(0, ConvertFloatToNormInt8(0.0f));
    EXPECT_EQ((1 << 6), ConvertFloatToNormInt8(0.5f));
    EXPECT_EQ(-(1 << 6), ConvertFloatToNormInt8(-0.5f));
    EXPECT_EQ(-(1 << 7) + 1, ConvertFloatToNormInt8(-1.0f));
    EXPECT_EQ((1 << 7) - 1, ConvertFloatToNormInt8(1.0f));
    EXPECT_EQ((1 << 7) - 1, ConvertFloatToNormInt8(1.0001f));
    EXPECT_EQ((1 << 7) - 1, ConvertFloatToNormInt8(100.0f));
    EXPECT_EQ(-(1 << 7), ConvertFloatToNormInt8(-100.0f));

    // Float to 8-bit unsigned integer (normalized)
    EXPECT_EQ(0, ConvertFloatToNormUint8(0.0f));
    EXPECT_EQ((1 << 7), ConvertFloatToNormUint8(0.5f));
    EXPECT_EQ((1 << 8) - 1, ConvertFloatToNormUint8(1.0f));
    EXPECT_EQ(0, ConvertFloatToNormUint8(-1.0f));
    EXPECT_EQ((1 << 8) - 1, ConvertFloatToNormUint8(100.0f));
}

TEST(Math, ConvertInt16)
{
    // Float to 16-bit signed integer
    EXPECT_EQ(0, ConvertFloatToInt16(0.0f));
    EXPECT_EQ(1, ConvertFloatToInt16(1.0f));
    EXPECT_EQ(-1, ConvertFloatToInt16(-1.0f));
    EXPECT_EQ((1 << 15) - 1, ConvertFloatToInt16(100000.0f));
    EXPECT_EQ(-(1 << 15), ConvertFloatToInt16(-100000.0f));

    // Float to 16-bit unsigned integer
    EXPECT_EQ(0, ConvertFloatToUint16(0.0f));
    EXPECT_EQ(1, ConvertFloatToUint16(1.0f));
    EXPECT_EQ(0, ConvertFloatToUint16(-1.0f));
    EXPECT_EQ((1 << 16) - 1, ConvertFloatToUint16(100000.0f));
    EXPECT_EQ((1 << 16) - 1, ConvertFloatToUint16(100000.0f));

    // Float to 16-bit signed integer (normalized)
    EXPECT_EQ(0, ConvertFloatToNormInt16(0.0f));
    EXPECT_EQ((1 << 14), ConvertFloatToNormInt16(0.5f));
    EXPECT_EQ(-(1 << 14), ConvertFloatToNormInt16(-0.5f));
    EXPECT_EQ(-(1 << 15) + 1, ConvertFloatToNormInt16(-1.0f));
    EXPECT_EQ((1 << 15) - 1, ConvertFloatToNormInt16(1.0f));
    EXPECT_EQ((1 << 15) - 1, ConvertFloatToNormInt16(1.0001f));
    EXPECT_EQ((1 << 15) - 1, ConvertFloatToNormInt16(100.0f));
    EXPECT_EQ(-(1 << 15), ConvertFloatToNormInt16(-100.0f));

    // Float to 8-bit unsigned integer (normalized)
    EXPECT_EQ(0, ConvertFloatToNormUint16(0.0f));
    EXPECT_EQ((1 << 15), ConvertFloatToNormUint16(0.5f));
    EXPECT_EQ((1 << 16) - 1, ConvertFloatToNormUint16(1.0f));
    EXPECT_EQ(0, ConvertFloatToNormUint16(-1.0f));
    EXPECT_EQ((1 << 16) - 1, ConvertFloatToNormUint16(100.0f));
}

TEST(Math, ConvertInt32)
{
    // Float to 32-bit signed integer
    EXPECT_EQ(0, ConvertFloatToInt32(0.0f));
    EXPECT_EQ(1, ConvertFloatToInt32(1.0f));
    EXPECT_EQ(-1, ConvertFloatToInt32(-1.0f));
    EXPECT_EQ((1LL << 31) - 1, ConvertFloatToInt32(2147483646.999f));
    EXPECT_EQ(-(1LL << 31), ConvertFloatToInt32(-2147483648.0f));
    EXPECT_EQ((1LL << 31) - 1, ConvertFloatToInt32(1.0e+10f));
    EXPECT_EQ(-(1LL << 31), ConvertFloatToInt32(-1.0e+10f));

    // Float to 32-bit unsigned integer
    EXPECT_EQ(0, ConvertFloatToUint32(0.0f));
    EXPECT_EQ(1, ConvertFloatToUint32(1.0f));
    EXPECT_EQ(0, ConvertFloatToUint32(-1.0f));
    EXPECT_EQ((1LL << 32) - 1, ConvertFloatToUint32(4294967295.0f));
    EXPECT_EQ((1LL << 32) - 1, ConvertFloatToUint32(1.0e+10f));
}
