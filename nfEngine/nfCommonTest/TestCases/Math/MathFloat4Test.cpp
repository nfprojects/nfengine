#include "PCH.hpp"
#include "nfCommon/Math/Float4.hpp"

using namespace NFE::Math;

namespace {

const Float4 vecA = Float4(1.0f, 1.0f, 1.0f, 1.0f);
const Float4 vecB = Float4(1.0f, 2.0f, 3.0f, 4.0f);
const Float4 vecC = Float4(2.0f, 3.0f, 4.0f, 5.0f);
const Float4 vecD = Float4(1.0f, 4.0f, 9.0f, 16.0f);
const Float4 vecE = Float4(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathFloat4, Constructors)
{
    const Float2 f2(1.0f, 2.0f);
    const Float3 f3(1.0f, 2.0f, 3.0f);
    const Float4 f4(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(Float4(f2) == Float4(1.0f, 2.0f, 0.0f, 0.0f));
    EXPECT_TRUE(Float4(f3) == Float4(1.0f, 2.0f, 3.0f, 0.0f));
    EXPECT_TRUE(Float4(f4) == Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Float4(123.0f, 123.0f, 123.0f, 123.0f) == Float4::Splat(123.0f));
    EXPECT_TRUE(Float4(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Float4(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Float4(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ());
    EXPECT_TRUE(Float4(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW());
}

TEST(MathFloat4, SelectBySign)
{
    Float4 vA(1.0f, 2.0f, 3.0f, 4.0f);
    Float4 vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 4.0f) == Float4::SelectBySign(vA, vB, Float4(1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Float4(5.0f, 2.0f, 3.0f, 4.0f) == Float4::SelectBySign(vA, vB, Float4(-1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Float4(1.0f, 6.0f, 3.0f, 4.0f) == Float4::SelectBySign(vA, vB, Float4(1.0f, -1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Float4(1.0f, 2.0f, 7.0f, 4.0f) == Float4::SelectBySign(vA, vB, Float4(1.0f, 1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 8.0f) == Float4::SelectBySign(vA, vB, Float4(1.0f, 1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Float4(5.0f, 6.0f, 7.0f, 8.0f) == Float4::SelectBySign(vA, vB, Float4(-1.0f, -1.0f, -1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat4, VectorArithmetics)
{
    EXPECT_TRUE(Float4::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Float4::AlmostEqual(vecA - vecB, Float4(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Float4::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Float4::AlmostEqual(vecC / vecB, Float4(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Float4::AlmostEqual(vecB * 2.0f, Float4(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Float4::AlmostEqual(vecB / 2.0f, Float4(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE(Float4::Abs(Float4(-1.0f, -2.0f, 0.0f, 3.0f)) == Float4(1.0f, 2.0f, 0.0f, 3.0f));
}

TEST(MathFloat4, VectorLerp)
{
    EXPECT_TRUE(Float4::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Float4::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Float4::Lerp(vecA, vecB, 0.5f) == Float4(1.0f, 1.5f, 2.0f, 2.5f));
}

TEST(MathFloat4, VectorMinMax)
{
    EXPECT_TRUE(Float4::Min(vecB, vecE) == Float4(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Float4::Min(vecE, vecB) == Float4(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Float4::Max(vecB, vecE) == Float4(4.0f, 3.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Float4::Max(vecB, vecE) == Float4(4.0f, 3.0f, 3.0f, 4.0f));
}

TEST(MathFloat4, VectorGeometrics)
{
    EXPECT_EQ(40.0f, Float4::Dot(vecB, vecC));
    // TODO cross product
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat4, VectorLess)
{
    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(10.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(1.0f, 10.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(1.0f, 2.0f, 10.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(1.0f, 2.0f, 3.0f, 10.0f) < vecC);
    EXPECT_FALSE(Float4(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_FALSE(Float4(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathFloat4, VectorLessOrEqual)
{
    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Float4(10.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Float4(1.0f, 10.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Float4(1.0f, 2.0f, 10.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Float4(1.0f, 2.0f, 3.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Float4(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Float4(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Float4(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathFloat4, VectorGreater)
{
    EXPECT_TRUE(Float4(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(1.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(3.0f, 1.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(3.0f, 4.0f, 1.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(3.0f, 4.0f, 5.0f, 1.0f) > vecC);
    EXPECT_FALSE(Float4(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_FALSE(Float4(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathFloat4, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Float4(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Float4(1.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Float4(3.0f, 1.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Float4(3.0f, 4.0f, 1.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Float4(3.0f, 4.0f, 5.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Float4(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Float4(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Float4(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Float4(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathFloat4, VectorEqual)
{
    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 4.0f) == Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(10.0f, 2.0f, 3.0f, 4.0f) == Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(1.0f, 20.0f, 3.0f, 4.0f) == Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(1.0f, 2.0f, 30.0f, 4.0f) == Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(1.0f, 2.0f, 3.0f, 40.0f) == Float4(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathFloat4, VectorNotEqual)
{
    EXPECT_TRUE(Float4(4.0f, 3.0f, 2.0f, 1.0f) != Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(1.0f, 3.0f, 2.0f, 1.0f) != Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(4.0f, 2.0f, 2.0f, 1.0f) != Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(4.0f, 3.0f, 3.0f, 1.0f) != Float4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Float4(4.0f, 3.0f, 2.0f, 4.0f) != Float4(1.0f, 2.0f, 3.0f, 4.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat4, VectorSwizzle)
{
    const Float4 v(0.0f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Float4(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>()));
    EXPECT_TRUE(Float4(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>()));
    EXPECT_TRUE(Float4(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>()));
    EXPECT_TRUE(Float4(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>()));
    EXPECT_TRUE(Float4(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>()));

    EXPECT_TRUE(Float4(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>()));
    EXPECT_TRUE(Float4(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>()));

    EXPECT_TRUE(Float4(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>()));
    EXPECT_TRUE(Float4(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>()));

    EXPECT_TRUE(Float4(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>()));
    EXPECT_TRUE(Float4(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>()));
    EXPECT_TRUE(Float4(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>()));
}

TEST(MathFloat4, Blend)
{
    const Float4 a(1.0f, 2.0f, 3.0f, 4.0f);
    const Float4 b(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 4.0f) == (Float4::Blend<0, 0, 0, 0>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 2.0f, 3.0f, 8.0f) == (Float4::Blend<0, 0, 0, 1>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 2.0f, 7.0f, 4.0f) == (Float4::Blend<0, 0, 1, 0>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 2.0f, 7.0f, 8.0f) == (Float4::Blend<0, 0, 1, 1>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 6.0f, 3.0f, 4.0f) == (Float4::Blend<0, 1, 0, 0>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 6.0f, 3.0f, 8.0f) == (Float4::Blend<0, 1, 0, 1>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 6.0f, 7.0f, 4.0f) == (Float4::Blend<0, 1, 1, 0>(a, b)));
    EXPECT_TRUE(Float4(1.0f, 6.0f, 7.0f, 8.0f) == (Float4::Blend<0, 1, 1, 1>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 2.0f, 3.0f, 4.0f) == (Float4::Blend<1, 0, 0, 0>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 2.0f, 3.0f, 8.0f) == (Float4::Blend<1, 0, 0, 1>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 2.0f, 7.0f, 4.0f) == (Float4::Blend<1, 0, 1, 0>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 2.0f, 7.0f, 8.0f) == (Float4::Blend<1, 0, 1, 1>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 6.0f, 3.0f, 4.0f) == (Float4::Blend<1, 1, 0, 0>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 6.0f, 3.0f, 8.0f) == (Float4::Blend<1, 1, 0, 1>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 6.0f, 7.0f, 4.0f) == (Float4::Blend<1, 1, 1, 0>(a, b)));
    EXPECT_TRUE(Float4(5.0f, 6.0f, 7.0f, 8.0f) == (Float4::Blend<1, 1, 1, 1>(a, b)));
}

TEST(MathFloat4, ChangeSign)
{
    const Float4 v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Float4(0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, false, false, false>()));
    EXPECT_TRUE(Float4(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, false, false, true>()));
    EXPECT_TRUE(Float4(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, false, true, false>()));
    EXPECT_TRUE(Float4(0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, false, true, true>()));
    EXPECT_TRUE(Float4(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, true, false, false>()));
    EXPECT_TRUE(Float4(0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, true, false, true>()));
    EXPECT_TRUE(Float4(0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, true, true, false>()));
    EXPECT_TRUE(Float4(0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, true, true, true>()));
    EXPECT_TRUE(Float4(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, false, false, false>()));
    EXPECT_TRUE(Float4(-0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, false, false, true>()));
    EXPECT_TRUE(Float4(-0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, false, true, false>()));
    EXPECT_TRUE(Float4(-0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, false, true, true>()));
    EXPECT_TRUE(Float4(-0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, true, false, false>()));
    EXPECT_TRUE(Float4(-0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, true, false, true>()));
    EXPECT_TRUE(Float4(-0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, true, true, false>()));
    EXPECT_TRUE(Float4(-0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, true, true, true>()));
}
