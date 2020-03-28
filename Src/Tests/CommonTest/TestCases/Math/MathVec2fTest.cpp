#include "PCH.hpp"
#include "Engine/Common/Math/Vec2f.hpp"

using namespace NFE::Math;

namespace {

const Vec2f vecA = Vec2f(1.0f, 1.0f);
const Vec2f vecB = Vec2f(1.0f, 2.0f);
const Vec2f vecC = Vec2f(2.0f, 3.0f);
const Vec2f vecD = Vec2f(1.0f, 4.0f);
const Vec2f vecE = Vec2f(4.0f, 3.0f);

} // namespace

TEST(MathVec2f, Constructors)
{
    const Vec2f f2(1.0f);
    const Vec2f f3(1.0f, 2.0f);

    EXPECT_TRUE(Vec2f(f2) == Vec2f(1.0f, 1.0f));
    EXPECT_TRUE(Vec2f(f3) == Vec2f(1.0f, 2.0f));
    EXPECT_TRUE(Vec2f(123.0f, 123.0f) == Vec2f::Splat(123.0f));
    EXPECT_TRUE(Vec2f(1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Vec2f(2.0f, 2.0f) == vecB.SplatY());
}

TEST(MathVec2f, SelectBySign)
{
    Vec2f vA(1.0f, 2.0f);
    Vec2f vB(5.0f, 6.0f);

    EXPECT_TRUE(Vec2f(1.0f, 2.0f) == Vec2f::SelectBySign(vA, vB, Vec2f(1.0f, 1.0f)));
    EXPECT_TRUE(Vec2f(5.0f, 2.0f) == Vec2f::SelectBySign(vA, vB, Vec2f(-1.0f, 1.0f)));
    EXPECT_TRUE(Vec2f(1.0f, 6.0f) == Vec2f::SelectBySign(vA, vB, Vec2f(1.0f, -1.0f)));
    EXPECT_TRUE(Vec2f(5.0f, 6.0f) == Vec2f::SelectBySign(vA, vB, Vec2f(-1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec2f, VectorArithmetics)
{
    EXPECT_TRUE(Vec2f::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vec2f::AlmostEqual(vecA - vecB, Vec2f(0.0f, -1.0f)));
    EXPECT_TRUE(Vec2f::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vec2f::AlmostEqual(vecC / vecB, Vec2f(2.0f, 1.5f)));
    EXPECT_TRUE(Vec2f::AlmostEqual(vecB * 2.0f, Vec2f(2.0f, 4.0f)));
    EXPECT_TRUE(Vec2f::AlmostEqual(vecB / 2.0f, Vec2f(0.5f, 1.0f)));
    EXPECT_TRUE(Vec2f::Abs(Vec2f(-1.0f, -2.0f)) == Vec2f(1.0f, 2.0f));
}

TEST(MathVec2f, VectorLerp)
{
    EXPECT_TRUE(Vec2f::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Vec2f::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Vec2f::Lerp(vecA, vecB, 0.5f) == Vec2f(1.0f, 1.5f));
}

TEST(MathVec2f, VectorMinMax)
{
    EXPECT_TRUE(Vec2f::Min(vecB, vecE) == Vec2f(1.0f, 2.0f));
    EXPECT_TRUE(Vec2f::Min(vecE, vecB) == Vec2f(1.0f, 2.0f));
    EXPECT_TRUE(Vec2f::Max(vecB, vecE) == Vec2f(4.0f, 3.0f));
    EXPECT_TRUE(Vec2f::Max(vecB, vecE) == Vec2f(4.0f, 3.0f));
}

TEST(MathVec2f, VectorGeometrics)
{
    EXPECT_EQ(8.0f, Vec2f::Dot(vecB, vecC));
    EXPECT_TRUE(Vec2f::Cross(vecB, vecC) == -1.0f);
    EXPECT_TRUE(Vec2f::Cross(vecC, vecB) == 1.0f);
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec2f, VectorLess)
{
    EXPECT_TRUE(Vec2f(1.0f, 2.0f) < vecC);
    EXPECT_FALSE(Vec2f(10.0f, 2.0f) < vecC);
    EXPECT_FALSE(Vec2f(1.0f, 10.0f) < vecC);
    EXPECT_FALSE(Vec2f(2.0f, 2.0f) < vecC);
    EXPECT_FALSE(Vec2f(1.0f, 3.0f) < vecC);
}

TEST(MathVec2f, VectorLessOrEqual)
{
    EXPECT_TRUE(Vec2f(1.0f, 2.0f) <= vecC);
    EXPECT_FALSE(Vec2f(10.0f, 2.0f) <= vecC);
    EXPECT_FALSE(Vec2f(1.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Vec2f(2.0f, 2.0f) <= vecC);
    EXPECT_TRUE(Vec2f(1.0f, 3.0f) <= vecC);
}

TEST(MathVec2f, VectorGreater)
{
    EXPECT_TRUE(Vec2f(3.0f, 4.0f) > vecC);
    EXPECT_FALSE(Vec2f(1.0f, 4.0f) > vecC);
    EXPECT_FALSE(Vec2f(3.0f, 1.0f) > vecC);
    EXPECT_FALSE(Vec2f(2.0f, 4.0f) > vecC);
    EXPECT_FALSE(Vec2f(3.0f, 3.0f) > vecC);
}

TEST(MathVec2f, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Vec2f(3.0f, 4.0f) >= vecC);
    EXPECT_FALSE(Vec2f(1.0f, 4.0f) >= vecC);
    EXPECT_FALSE(Vec2f(3.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Vec2f(2.0f, 4.0f) >= vecC);
    EXPECT_TRUE(Vec2f(3.0f, 3.0f) >= vecC);
}

TEST(MathVec2f, VectorEqual)
{
    EXPECT_TRUE(Vec2f(1.0f, 2.0f) == Vec2f(1.0f, 2.0f));
    EXPECT_FALSE(Vec2f(10.0f, 2.0f) == Vec2f(1.0f, 2.0f));
    EXPECT_FALSE(Vec2f(1.0f, 20.0f) == Vec2f(1.0f, 2.0f));
}

TEST(MathVec2f, VectorNotEqual)
{
    EXPECT_TRUE(Vec2f(4.0f, 3.0f) != Vec2f(1.0f, 2.0f));
    EXPECT_FALSE(Vec2f(1.0f, 3.0f) != Vec2f(1.0f, 2.0f));
    EXPECT_FALSE(Vec2f(4.0f, 2.0f) != Vec2f(1.0f, 2.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec2f, VectorSwizzle)
{
    const Vec2f v(0.0f, 1.0f);

    EXPECT_TRUE(Vec2f(0.0f, 0.0f) == (v.Swizzle<0, 0>()));
    EXPECT_TRUE(Vec2f(0.0f, 1.0f) == (v.Swizzle<0, 1>()));
    EXPECT_TRUE(Vec2f(1.0f, 0.0f) == (v.Swizzle<1, 0>()));
    EXPECT_TRUE(Vec2f(1.0f, 1.0f) == (v.Swizzle<1, 1>()));
}

TEST(MathVec2f, Blend)
{
    const Vec2f a(1.0f, 2.0f);
    const Vec2f b(5.0f, 6.0f);

    EXPECT_TRUE(Vec2f(1.0f, 2.0f) == (Vec2f::Blend<0, 0>(a, b)));
    EXPECT_TRUE(Vec2f(1.0f, 6.0f) == (Vec2f::Blend<0, 1>(a, b)));
    EXPECT_TRUE(Vec2f(5.0f, 2.0f) == (Vec2f::Blend<1, 0>(a, b)));
    EXPECT_TRUE(Vec2f(5.0f, 6.0f) == (Vec2f::Blend<1, 1>(a, b)));
}

TEST(MathVec2f, ChangeSign)
{
    const Vec2f v(0.5f, 1.0f);

    EXPECT_TRUE(Vec2f(0.5f, 1.0f) == (v.ChangeSign<false, false>()));
    EXPECT_TRUE(Vec2f(0.5f, -1.0f) == (v.ChangeSign<false, true>()));
    EXPECT_TRUE(Vec2f(-0.5f, 1.0f) == (v.ChangeSign<true, false>()));
    EXPECT_TRUE(Vec2f(-0.5f, -1.0f) == (v.ChangeSign<true, true>()));
}
