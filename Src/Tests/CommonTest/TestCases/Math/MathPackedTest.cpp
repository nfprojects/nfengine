#include "PCH.hpp"
#include "Engine/Common/Math/Packed.hpp"
#include "Engine/Common/Math/Half.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Common/Math/SamplingHelpers.hpp"

using namespace NFE;
using namespace NFE::Math;


void TestUnitVector(const Vec4f& v, float maxError)
{
    PackedUnitVector3 packed;
    packed.FromVector(v);
    const Vec4f decompressed = packed.ToVector();

    SCOPED_TRACE("x=[" + std::to_string(v.x) + ',' + std::to_string(v.y) + ',' + std::to_string(v.z) + ']');

    ASSERT_NEAR(v.x, decompressed.x, maxError);
    ASSERT_NEAR(v.y, decompressed.y, maxError);
    ASSERT_NEAR(v.z, decompressed.z, maxError);
}

TEST(MathPacked, UnitVector)
{
    // test edge cases
    TestUnitVector(Vec4f( 1.0f,  0.0f,  0.0f), 0.0f);
    TestUnitVector(Vec4f(-1.0f,  0.0f,  0.0f), 0.0f);
    TestUnitVector(Vec4f( 0.0f,  1.0f,  0.0f), 0.0f);
    TestUnitVector(Vec4f( 0.0f, -1.0f,  0.0f), 0.0f);
    TestUnitVector(Vec4f( 0.0f,  0.0f,  1.0f), 0.0f);
    TestUnitVector(Vec4f( 0.0f,  0.0f, -1.0f), 0.0f);

    TestUnitVector(Vec4f( 1.0f,  1.0f,  1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f( 1.0f,  1.0f, -1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f( 1.0f, -1.0f,  1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f( 1.0f, -1.0f, -1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f,  1.0f,  1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f,  1.0f, -1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f, -1.0f,  1.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f, -1.0f, -1.0f).Normalized3(), 0.0002f);

    TestUnitVector(Vec4f( 1.0f,  2.0f,  3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f( 1.0f,  2.0f, -3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f( 1.0f, -2.0f,  3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f( 1.0f, -2.0f, -3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f,  2.0f,  3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f,  2.0f, -3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f, -2.0f,  3.0f).Normalized3(), 0.0002f);
    TestUnitVector(Vec4f(-1.0f, -2.0f, -3.0f).Normalized3(), 0.0002f);

    Random random;

    for (uint32 i = 0; i < 1000; ++i)
    {
        const Vec4f vec = SamplingHelpers::GetSphere(random.GetVec2f()).Normalized3();
        TestUnitVector(vec, 0.0002f);
    }
}


void TestColorRgbHdr(const Vec4f& color, float maxError)
{
    PackedColorRgbHdr packed;
    packed.FromVector(color);
    const Vec4f decompressed = packed.ToVector();

    SCOPED_TRACE("color=[" + std::to_string(color.x) + ',' + std::to_string(color.y) + ',' + std::to_string(color.z) + ']');

    ASSERT_NEAR(color.x, decompressed.x, maxError);
    ASSERT_NEAR(color.y, decompressed.y, maxError);
    ASSERT_NEAR(color.z, decompressed.z, maxError);
}

TEST(MathPacked, ColorRgbHdr)
{
    // test edge cases
    TestColorRgbHdr(Vec4f(0.0f, 0.0f, 0.0f), 0.0f);
    TestColorRgbHdr(Vec4f(1.0f, 0.0f, 0.0f), 0.0f);
    TestColorRgbHdr(Vec4f(0.0f, 1.0f, 0.0f), 0.0f);
    TestColorRgbHdr(Vec4f(0.0f, 0.0f, 1.0f), 0.0f);
    TestColorRgbHdr(Vec4f(1024.0f, 0.0f, 0.0f), 0.0f);
    TestColorRgbHdr(Vec4f(0.0f, 1024.0f, 0.0f), 0.0f);
    TestColorRgbHdr(Vec4f(0.0f, 0.0f, 1024.0f), 0.0f);

    Random random;

    for (uint32 i = 0; i < 1000; ++i)
    {
        const Vec4f vec = random.GetVec4f() * 0.001f;
        TestColorRgbHdr(vec, 0.0000002f);
    }

    for (uint32 i = 0; i < 1000; ++i)
    {
        const Vec4f vec = random.GetVec4f() * 1000.0f;
        TestColorRgbHdr(vec, 0.2f);
    }
}

TEST(MathPacked, Half)
{
    {
        Half half(0.0f);
        EXPECT_EQ(0.0f, half.ToFloat());
    }
    {
        Half half(1.0f);
        EXPECT_EQ(1.0f, half.ToFloat());
    }
    {
        Half half(-1.0f);
        EXPECT_EQ(-1.0f, half.ToFloat());
    }
    {
        Half half(123.0f);
        EXPECT_EQ(123.0f, half.ToFloat());
    }
    {
        Half half(-0.01074981689453125f);
        EXPECT_EQ(-0.01074981689453125f, half.ToFloat());
    }
    {
        // max value
        Half half(65504.0f);
        EXPECT_EQ(65504.0f, half.ToFloat());
    }
    {
        // max value (negative)
        Half half(-65504.0f);
        EXPECT_EQ(-65504.0f, half.ToFloat());
    }
    {
        // min normal value
        Half half(0.00006103515625f);
        EXPECT_EQ(0.00006103515625f, half.ToFloat());
    }
    {
        // min normal value (negative)
        Half half(-0.00006103515625f);
        EXPECT_EQ(-0.00006103515625f, half.ToFloat());
    }
}

