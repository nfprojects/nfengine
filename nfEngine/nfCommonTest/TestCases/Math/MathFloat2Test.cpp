#include "PCH.hpp"
#include "nfCommon/Math/Float2.hpp"

using namespace NFE::Math;

namespace {

const Float2 vecA = Float2(1.0f, 1.0f);
const Float2 vecB = Float2(1.0f, 2.0f);
const Float2 vecC = Float2(2.0f, 3.0f);
const Float2 vecD = Float2(1.0f, 4.0f);
const Float2 vecE = Float2(4.0f, 3.0f);

} // namespace

TEST(MathFloat2, Constructors)
{
    const Float2 f2(1.0f);
    const Float2 f3(1.0f, 2.0f);

    EXPECT_TRUE(Float2(f2) == Float2(1.0f, 0.0f));
    EXPECT_TRUE(Float2(f3) == Float2(1.0f, 2.0f));
    EXPECT_TRUE(Float2(123.0f, 123.0f) == Float2::Splat(123.0f));
    EXPECT_TRUE(Float2(1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Float2(2.0f, 2.0f) == vecB.SplatY());
}

TEST(MathFloat2, SelectBySign)
{
    Float2 vA(1.0f, 2.0f);
    Float2 vB(5.0f, 6.0f);

    EXPECT_TRUE(Float2(1.0f, 2.0f) == Float2::SelectBySign(vA, vB, Float2(1.0f, 1.0f)));
    EXPECT_TRUE(Float2(5.0f, 2.0f) == Float2::SelectBySign(vA, vB, Float2(-1.0f, 1.0f)));
    EXPECT_TRUE(Float2(1.0f, 6.0f) == Float2::SelectBySign(vA, vB, Float2(1.0f, -1.0f)));
    EXPECT_TRUE(Float2(5.0f, 6.0f) == Float2::SelectBySign(vA, vB, Float2(-1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat2, VectorArithmetics)
{
    EXPECT_TRUE(Float2::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Float2::AlmostEqual(vecA - vecB, Float2(0.0f, -1.0f)));
    EXPECT_TRUE(Float2::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Float2::AlmostEqual(vecC / vecB, Float2(2.0f, 1.5f)));
    EXPECT_TRUE(Float2::AlmostEqual(vecB * 2.0f, Float2(2.0f, 4.0f)));
    EXPECT_TRUE(Float2::AlmostEqual(vecB / 2.0f, Float2(0.5f, 1.0f)));
    EXPECT_TRUE(Float2::Abs(Float2(-1.0f, -2.0f)) == Float2(1.0f, 2.0f));
}

TEST(MathFloat2, VectorLerp)
{
    EXPECT_TRUE(Float2::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Float2::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Float2::Lerp(vecA, vecB, 0.5f) == Float2(1.0f, 1.5f));
}

TEST(MathFloat2, VectorMinMax)
{
    EXPECT_TRUE(Float2::Min(vecB, vecE) == Float2(1.0f, 2.0f));
    EXPECT_TRUE(Float2::Min(vecE, vecB) == Float2(1.0f, 2.0f));
    EXPECT_TRUE(Float2::Max(vecB, vecE) == Float2(4.0f, 3.0f));
    EXPECT_TRUE(Float2::Max(vecB, vecE) == Float2(4.0f, 3.0f));
}

TEST(MathFloat2, VectorGeometrics)
{
    EXPECT_EQ(8.0f, Float2::Dot(vecB, vecC));
    EXPECT_TRUE(Float2::Cross(vecB, vecC) == -1.0f);
    EXPECT_TRUE(Float2::Cross(vecC, vecB) == 1.0f);
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat2, VectorLess)
{
    EXPECT_TRUE(Float2(1.0f, 2.0f) < vecC);
    EXPECT_FALSE(Float2(10.0f, 2.0f) < vecC);
    EXPECT_FALSE(Float2(1.0f, 10.0f) < vecC);
    EXPECT_FALSE(Float2(2.0f, 2.0f) < vecC);
    EXPECT_FALSE(Float2(1.0f, 3.0f) < vecC);
}

TEST(MathFloat2, VectorLessOrEqual)
{
    EXPECT_TRUE(Float2(1.0f, 2.0f) <= vecC);
    EXPECT_FALSE(Float2(10.0f, 2.0f) <= vecC);
    EXPECT_FALSE(Float2(1.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Float2(2.0f, 2.0f) <= vecC);
    EXPECT_TRUE(Float2(1.0f, 3.0f) <= vecC);
}

TEST(MathFloat2, VectorGreater)
{
    EXPECT_TRUE(Float2(3.0f, 4.0f) > vecC);
    EXPECT_FALSE(Float2(1.0f, 4.0f) > vecC);
    EXPECT_FALSE(Float2(3.0f, 1.0f) > vecC);
    EXPECT_FALSE(Float2(2.0f, 4.0f) > vecC);
    EXPECT_FALSE(Float2(3.0f, 3.0f) > vecC);
}

TEST(MathFloat2, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Float2(3.0f, 4.0f) >= vecC);
    EXPECT_FALSE(Float2(1.0f, 4.0f) >= vecC);
    EXPECT_FALSE(Float2(3.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Float2(2.0f, 4.0f) >= vecC);
    EXPECT_TRUE(Float2(3.0f, 3.0f) >= vecC);
}

TEST(MathFloat2, VectorEqual)
{
    EXPECT_TRUE(Float2(1.0f, 2.0f) == Float2(1.0f, 2.0f));
    EXPECT_FALSE(Float2(10.0f, 2.0f) == Float2(1.0f, 2.0f));
    EXPECT_FALSE(Float2(1.0f, 20.0f) == Float2(1.0f, 2.0f));
}

TEST(MathFloat2, VectorNotEqual)
{
    EXPECT_TRUE(Float2(4.0f, 3.0f) != Float2(1.0f, 2.0f));
    EXPECT_FALSE(Float2(1.0f, 3.0f) != Float2(1.0f, 2.0f));
    EXPECT_FALSE(Float2(4.0f, 2.0f) != Float2(1.0f, 2.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat2, VectorSwizzle)
{
    const Float2 v(0.0f, 1.0f);

    EXPECT_TRUE(Float2(0.0f, 0.0f) == (v.Swizzle<0, 0>()));
    EXPECT_TRUE(Float2(0.0f, 1.0f) == (v.Swizzle<0, 1>()));
    EXPECT_TRUE(Float2(1.0f, 0.0f) == (v.Swizzle<1, 0>()));
    EXPECT_TRUE(Float2(1.0f, 1.0f) == (v.Swizzle<1, 1>()));
}

TEST(MathFloat2, Blend)
{
    const Float2 a(1.0f, 2.0f);
    const Float2 b(5.0f, 6.0f);

    EXPECT_TRUE(Float2(1.0f, 2.0f) == (Float2::Blend<0, 0>(a, b)));
    EXPECT_TRUE(Float2(1.0f, 6.0f) == (Float2::Blend<0, 1>(a, b)));
    EXPECT_TRUE(Float2(5.0f, 2.0f) == (Float2::Blend<1, 0>(a, b)));
    EXPECT_TRUE(Float2(5.0f, 6.0f) == (Float2::Blend<1, 1>(a, b)));
}

TEST(MathFloat2, ChangeSign)
{
    const Float2 v(0.5f, 1.0f);

    EXPECT_TRUE(Float2(0.5f, 1.0f) == (v.ChangeSign<false, false>()));
    EXPECT_TRUE(Float2(0.5f, -1.0f) == (v.ChangeSign<false, true>()));
    EXPECT_TRUE(Float2(-0.5f, 1.0f) == (v.ChangeSign<true, false>()));
    EXPECT_TRUE(Float2(-0.5f, -1.0f) == (v.ChangeSign<true, true>()));
}
