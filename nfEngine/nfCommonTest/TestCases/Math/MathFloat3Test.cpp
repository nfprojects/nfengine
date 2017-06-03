#include "PCH.hpp"
#include "nfCommon/Math/Float3.hpp"

using namespace NFE::Math;

namespace {

const Float3 vecA = Float3(1.0f, 1.0f, 1.0f);
const Float3 vecB = Float3(1.0f, 2.0f, 3.0f);
const Float3 vecC = Float3(2.0f, 3.0f, 4.0f);
const Float3 vecD = Float3(1.0f, 4.0f, 9.0f);
const Float3 vecE = Float3(4.0f, 3.0f, 2.0f);

} // namespace

TEST(MathFloat3, Constructors)
{
    const Float2 f2(1.0f, 2.0f);
    const Float3 f3(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Float3(f2) == Float3(1.0f, 2.0f, 0.0f));
    EXPECT_TRUE(Float3(f3) == Float3(1.0f, 2.0f, 3.0f));
    EXPECT_TRUE(Float3(123.0f, 123.0f, 123.0f) == Float3::Splat(123.0f));
    EXPECT_TRUE(Float3(1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Float3(2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Float3(3.0f, 3.0f, 3.0f) == vecB.SplatZ());
}

TEST(MathFloat3, SelectBySign)
{
    Float3 vA(1.0f, 2.0f, 3.0f);
    Float3 vB(5.0f, 6.0f, 7.0f);

    EXPECT_TRUE(Float3(1.0f, 2.0f, 3.0f) == Float3::SelectBySign(vA, vB, Float3(1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Float3(5.0f, 2.0f, 3.0f) == Float3::SelectBySign(vA, vB, Float3(-1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Float3(1.0f, 6.0f, 3.0f) == Float3::SelectBySign(vA, vB, Float3(1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Float3(1.0f, 2.0f, 7.0f) == Float3::SelectBySign(vA, vB, Float3(1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Float3(5.0f, 6.0f, 7.0f) == Float3::SelectBySign(vA, vB, Float3(-1.0f, -1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat3, VectorArithmetics)
{
    EXPECT_TRUE(Float3::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Float3::AlmostEqual(vecA - vecB, Float3(0.0f, -1.0f, -2.0f)));
    EXPECT_TRUE(Float3::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Float3::AlmostEqual(vecC / vecB, Float3(2.0f, 1.5f, 4.0f / 3.0f)));
    EXPECT_TRUE(Float3::AlmostEqual(vecB * 2.0f, Float3(2.0f, 4.0f, 6.0f)));
    EXPECT_TRUE(Float3::AlmostEqual(vecB / 2.0f, Float3(0.5f, 1.0f, 1.5f)));
    EXPECT_TRUE(Float3::Abs(Float3(-1.0f, -2.0f, 0.0f)) == Float3(1.0f, 2.0f, 0.0f));
}

TEST(MathFloat3, VectorLerp)
{
    EXPECT_TRUE(Float3::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Float3::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Float3::Lerp(vecA, vecB, 0.5f) == Float3(1.0f, 1.5f, 2.0f));
}

TEST(MathFloat3, VectorMinMax)
{
    EXPECT_TRUE(Float3::Min(vecB, vecE) == Float3(1.0f, 2.0f, 2.0f));
    EXPECT_TRUE(Float3::Min(vecE, vecB) == Float3(1.0f, 2.0f, 2.0f));
    EXPECT_TRUE(Float3::Max(vecB, vecE) == Float3(4.0f, 3.0f, 3.0f));
    EXPECT_TRUE(Float3::Max(vecB, vecE) == Float3(4.0f, 3.0f, 3.0f));
}

TEST(MathFloat3, VectorGeometrics)
{
    EXPECT_EQ(20.0f, Float3::Dot(vecB, vecC));
    EXPECT_TRUE(Float3::Cross(vecB, vecC) == Float3(-1.0f, 2.0f, -1.0f));
    EXPECT_TRUE(Float3::Cross(vecC, vecB) == Float3(1.0f, -2.0f, 1.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat3, VectorLess)
{
    EXPECT_TRUE(Float3(1.0f, 2.0f, 3.0f) < vecC);
    EXPECT_FALSE(Float3(10.0f, 2.0f, 3.0f) < vecC);
    EXPECT_FALSE(Float3(1.0f, 10.0f, 3.0f) < vecC);
    EXPECT_FALSE(Float3(1.0f, 2.0f, 10.0f) < vecC);
    EXPECT_FALSE(Float3(2.0f, 2.0f, 3.0f) < vecC);
    EXPECT_FALSE(Float3(1.0f, 3.0f, 3.0f) < vecC);
    EXPECT_FALSE(Float3(1.0f, 2.0f, 4.0f) < vecC);
}

TEST(MathFloat3, VectorLessOrEqual)
{
    EXPECT_TRUE(Float3(1.0f, 2.0f, 3.0f) <= vecC);
    EXPECT_FALSE(Float3(10.0f, 2.0f, 3.0f) <= vecC);
    EXPECT_FALSE(Float3(1.0f, 10.0f, 3.0f) <= vecC);
    EXPECT_FALSE(Float3(1.0f, 2.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Float3(2.0f, 2.0f, 3.0f) <= vecC);
    EXPECT_TRUE(Float3(1.0f, 3.0f, 3.0f) <= vecC);
    EXPECT_TRUE(Float3(1.0f, 2.0f, 4.0f) <= vecC);
}

TEST(MathFloat3, VectorGreater)
{
    EXPECT_TRUE(Float3(3.0f, 4.0f, 5.0f) > vecC);
    EXPECT_FALSE(Float3(1.0f, 4.0f, 5.0f) > vecC);
    EXPECT_FALSE(Float3(3.0f, 1.0f, 5.0f) > vecC);
    EXPECT_FALSE(Float3(3.0f, 4.0f, 1.0f) > vecC);
    EXPECT_FALSE(Float3(2.0f, 4.0f, 5.0f) > vecC);
    EXPECT_FALSE(Float3(3.0f, 3.0f, 5.0f) > vecC);
    EXPECT_FALSE(Float3(3.0f, 4.0f, 4.0f) > vecC);
}

TEST(MathFloat3, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Float3(3.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_FALSE(Float3(1.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_FALSE(Float3(3.0f, 1.0f, 5.0f) >= vecC);
    EXPECT_FALSE(Float3(3.0f, 4.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Float3(2.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_TRUE(Float3(3.0f, 3.0f, 5.0f) >= vecC);
    EXPECT_TRUE(Float3(3.0f, 4.0f, 4.0f) >= vecC);
}

TEST(MathFloat3, VectorEqual)
{
    EXPECT_TRUE(Float3(1.0f, 2.0f, 3.0f) == Float3(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Float3(10.0f, 2.0f, 3.0f) == Float3(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Float3(1.0f, 20.0f, 3.0f) == Float3(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Float3(1.0f, 2.0f, 30.0f) == Float3(1.0f, 2.0f, 3.0f));
}

TEST(MathFloat3, VectorNotEqual)
{
    EXPECT_TRUE(Float3(4.0f, 3.0f, 2.0f) != Float3(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Float3(1.0f, 3.0f, 2.0f) != Float3(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Float3(4.0f, 2.0f, 2.0f) != Float3(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Float3(4.0f, 3.0f, 3.0f) != Float3(1.0f, 2.0f, 3.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathFloat3, VectorSwizzle)
{
    const Float3 v(0.0f, 1.0f, 2.0f);

    EXPECT_TRUE(Float3(0.0f, 1.0f, 2.0f) == (v.Swizzle<0, 1, 2>()));
    EXPECT_TRUE(Float3(0.0f, 2.0f, 1.0f) == (v.Swizzle<0, 2, 1>()));
    EXPECT_TRUE(Float3(1.0f, 0.0f, 2.0f) == (v.Swizzle<1, 0, 2>()));
    EXPECT_TRUE(Float3(1.0f, 2.0f, 0.0f) == (v.Swizzle<1, 2, 0>()));
    EXPECT_TRUE(Float3(2.0f, 0.0f, 1.0f) == (v.Swizzle<2, 0, 1>()));
    EXPECT_TRUE(Float3(2.0f, 1.0f, 0.0f) == (v.Swizzle<2, 1, 0>()));

    EXPECT_TRUE(Float3(1.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0>()));
    EXPECT_TRUE(Float3(0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 1, 0>()));
    EXPECT_TRUE(Float3(0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 1>()));
    EXPECT_TRUE(Float3(2.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0>()));
    EXPECT_TRUE(Float3(0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 2, 0>()));
    EXPECT_TRUE(Float3(0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 2>()));
}

TEST(MathFloat3, Blend)
{
    const Float3 a(1.0f, 2.0f, 3.0f);
    const Float3 b(5.0f, 6.0f, 7.0f);

    EXPECT_TRUE(Float3(1.0f, 2.0f, 3.0f) == (Float3::Blend<0, 0, 0>(a, b)));
    EXPECT_TRUE(Float3(1.0f, 2.0f, 7.0f) == (Float3::Blend<0, 0, 1>(a, b)));
    EXPECT_TRUE(Float3(1.0f, 6.0f, 3.0f) == (Float3::Blend<0, 1, 0>(a, b)));
    EXPECT_TRUE(Float3(1.0f, 6.0f, 7.0f) == (Float3::Blend<0, 1, 1>(a, b)));
    EXPECT_TRUE(Float3(5.0f, 2.0f, 3.0f) == (Float3::Blend<1, 0, 0>(a, b)));
    EXPECT_TRUE(Float3(5.0f, 2.0f, 7.0f) == (Float3::Blend<1, 0, 1>(a, b)));
    EXPECT_TRUE(Float3(5.0f, 6.0f, 3.0f) == (Float3::Blend<1, 1, 0>(a, b)));
    EXPECT_TRUE(Float3(5.0f, 6.0f, 7.0f) == (Float3::Blend<1, 1, 1>(a, b)));
}

TEST(MathFloat3, ChangeSign)
{
    const Float3 v(0.5f, 1.0f, 2.0f);

    EXPECT_TRUE(Float3(0.5f, 1.0f, 2.0f) == (v.ChangeSign<false, false, false>()));
    EXPECT_TRUE(Float3(0.5f, 1.0f, -2.0f) == (v.ChangeSign<false, false, true>()));
    EXPECT_TRUE(Float3(0.5f, -1.0f, 2.0f) == (v.ChangeSign<false, true, false>()));
    EXPECT_TRUE(Float3(0.5f, -1.0f, -2.0f) == (v.ChangeSign<false, true, true>()));
    EXPECT_TRUE(Float3(-0.5f, 1.0f, 2.0f) == (v.ChangeSign<true, false, false>()));
    EXPECT_TRUE(Float3(-0.5f, 1.0f, -2.0f) == (v.ChangeSign<true, false, true>()));
    EXPECT_TRUE(Float3(-0.5f, -1.0f, 2.0f) == (v.ChangeSign<true, true, false>()));
    EXPECT_TRUE(Float3(-0.5f, -1.0f, -2.0f) == (v.ChangeSign<true, true, true>()));
}
