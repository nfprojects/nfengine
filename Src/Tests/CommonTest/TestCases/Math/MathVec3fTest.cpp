#include "PCH.hpp"
#include "Engine/Common/Math/Vec3f.hpp"

using namespace NFE::Math;

namespace {

const Vec3f vecA = Vec3f(1.0f, 1.0f, 1.0f);
const Vec3f vecB = Vec3f(1.0f, 2.0f, 3.0f);
const Vec3f vecC = Vec3f(2.0f, 3.0f, 4.0f);
const Vec3f vecD = Vec3f(1.0f, 4.0f, 9.0f);
const Vec3f vecE = Vec3f(4.0f, 3.0f, 2.0f);

} // namespace

TEST(MathVec3f, Constructors)
{
    const Vec2f f2(1.0f, 2.0f);
    const Vec3f f3(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vec3f(f2) == Vec3f(1.0f, 2.0f, 0.0f));
    EXPECT_TRUE(Vec3f(f3) == Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_TRUE(Vec3f(123.0f, 123.0f, 123.0f) == Vec3f::Splat(123.0f));
    EXPECT_TRUE(Vec3f(1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Vec3f(2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Vec3f(3.0f, 3.0f, 3.0f) == vecB.SplatZ());
}

TEST(MathVec3f, SelectBySign)
{
    Vec3f vA(1.0f, 2.0f, 3.0f);
    Vec3f vB(5.0f, 6.0f, 7.0f);

    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 3.0f) == Vec3f::SelectBySign(vA, vB, Vec3f(1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vec3f(5.0f, 2.0f, 3.0f) == Vec3f::SelectBySign(vA, vB, Vec3f(-1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vec3f(1.0f, 6.0f, 3.0f) == Vec3f::SelectBySign(vA, vB, Vec3f(1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 7.0f) == Vec3f::SelectBySign(vA, vB, Vec3f(1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Vec3f(5.0f, 6.0f, 7.0f) == Vec3f::SelectBySign(vA, vB, Vec3f(-1.0f, -1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec3f, VectorArithmetics)
{
    EXPECT_TRUE(Vec3f::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vec3f::AlmostEqual(vecA - vecB, Vec3f(0.0f, -1.0f, -2.0f)));
    EXPECT_TRUE(Vec3f::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vec3f::AlmostEqual(vecC / vecB, Vec3f(2.0f, 1.5f, 4.0f / 3.0f)));
    EXPECT_TRUE(Vec3f::AlmostEqual(vecB * 2.0f, Vec3f(2.0f, 4.0f, 6.0f)));
    EXPECT_TRUE(Vec3f::AlmostEqual(vecB / 2.0f, Vec3f(0.5f, 1.0f, 1.5f)));
    EXPECT_TRUE(Vec3f::Abs(Vec3f(-1.0f, -2.0f, 0.0f)) == Vec3f(1.0f, 2.0f, 0.0f));
}

TEST(MathVec3f, VectorLerp)
{
    EXPECT_TRUE(Vec3f::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Vec3f::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Vec3f::Lerp(vecA, vecB, 0.5f) == Vec3f(1.0f, 1.5f, 2.0f));
}

TEST(MathVec3f, VectorMinMax)
{
    EXPECT_TRUE(Vec3f::Min(vecB, vecE) == Vec3f(1.0f, 2.0f, 2.0f));
    EXPECT_TRUE(Vec3f::Min(vecE, vecB) == Vec3f(1.0f, 2.0f, 2.0f));
    EXPECT_TRUE(Vec3f::Max(vecB, vecE) == Vec3f(4.0f, 3.0f, 3.0f));
    EXPECT_TRUE(Vec3f::Max(vecB, vecE) == Vec3f(4.0f, 3.0f, 3.0f));
}

TEST(MathVec3f, VectorGeometrics)
{
    EXPECT_EQ(20.0f, Vec3f::Dot(vecB, vecC));
    EXPECT_TRUE(Vec3f::Cross(vecB, vecC) == Vec3f(-1.0f, 2.0f, -1.0f));
    EXPECT_TRUE(Vec3f::Cross(vecC, vecB) == Vec3f(1.0f, -2.0f, 1.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec3f, VectorLess)
{
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 3.0f) < vecC);
    EXPECT_FALSE(Vec3f(10.0f, 2.0f, 3.0f) < vecC);
    EXPECT_FALSE(Vec3f(1.0f, 10.0f, 3.0f) < vecC);
    EXPECT_FALSE(Vec3f(1.0f, 2.0f, 10.0f) < vecC);
    EXPECT_FALSE(Vec3f(2.0f, 2.0f, 3.0f) < vecC);
    EXPECT_FALSE(Vec3f(1.0f, 3.0f, 3.0f) < vecC);
    EXPECT_FALSE(Vec3f(1.0f, 2.0f, 4.0f) < vecC);
}

TEST(MathVec3f, VectorLessOrEqual)
{
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 3.0f) <= vecC);
    EXPECT_FALSE(Vec3f(10.0f, 2.0f, 3.0f) <= vecC);
    EXPECT_FALSE(Vec3f(1.0f, 10.0f, 3.0f) <= vecC);
    EXPECT_FALSE(Vec3f(1.0f, 2.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Vec3f(2.0f, 2.0f, 3.0f) <= vecC);
    EXPECT_TRUE(Vec3f(1.0f, 3.0f, 3.0f) <= vecC);
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 4.0f) <= vecC);
}

TEST(MathVec3f, VectorGreater)
{
    EXPECT_TRUE(Vec3f(3.0f, 4.0f, 5.0f) > vecC);
    EXPECT_FALSE(Vec3f(1.0f, 4.0f, 5.0f) > vecC);
    EXPECT_FALSE(Vec3f(3.0f, 1.0f, 5.0f) > vecC);
    EXPECT_FALSE(Vec3f(3.0f, 4.0f, 1.0f) > vecC);
    EXPECT_FALSE(Vec3f(2.0f, 4.0f, 5.0f) > vecC);
    EXPECT_FALSE(Vec3f(3.0f, 3.0f, 5.0f) > vecC);
    EXPECT_FALSE(Vec3f(3.0f, 4.0f, 4.0f) > vecC);
}

TEST(MathVec3f, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Vec3f(3.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_FALSE(Vec3f(1.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_FALSE(Vec3f(3.0f, 1.0f, 5.0f) >= vecC);
    EXPECT_FALSE(Vec3f(3.0f, 4.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Vec3f(2.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_TRUE(Vec3f(3.0f, 3.0f, 5.0f) >= vecC);
    EXPECT_TRUE(Vec3f(3.0f, 4.0f, 4.0f) >= vecC);
}

TEST(MathVec3f, VectorEqual)
{
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 3.0f) == Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Vec3f(10.0f, 2.0f, 3.0f) == Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Vec3f(1.0f, 20.0f, 3.0f) == Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Vec3f(1.0f, 2.0f, 30.0f) == Vec3f(1.0f, 2.0f, 3.0f));
}

TEST(MathVec3f, VectorNotEqual)
{
    EXPECT_TRUE(Vec3f(4.0f, 3.0f, 2.0f) != Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Vec3f(1.0f, 3.0f, 2.0f) != Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Vec3f(4.0f, 2.0f, 2.0f) != Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_FALSE(Vec3f(4.0f, 3.0f, 3.0f) != Vec3f(1.0f, 2.0f, 3.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec3f, VectorSwizzle)
{
    const Vec3f v(0.0f, 1.0f, 2.0f);

    EXPECT_TRUE(Vec3f(0.0f, 1.0f, 2.0f) == (v.Swizzle<0, 1, 2>()));
    EXPECT_TRUE(Vec3f(0.0f, 2.0f, 1.0f) == (v.Swizzle<0, 2, 1>()));
    EXPECT_TRUE(Vec3f(1.0f, 0.0f, 2.0f) == (v.Swizzle<1, 0, 2>()));
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 0.0f) == (v.Swizzle<1, 2, 0>()));
    EXPECT_TRUE(Vec3f(2.0f, 0.0f, 1.0f) == (v.Swizzle<2, 0, 1>()));
    EXPECT_TRUE(Vec3f(2.0f, 1.0f, 0.0f) == (v.Swizzle<2, 1, 0>()));

    EXPECT_TRUE(Vec3f(1.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0>()));
    EXPECT_TRUE(Vec3f(0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 1, 0>()));
    EXPECT_TRUE(Vec3f(0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 1>()));
    EXPECT_TRUE(Vec3f(2.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0>()));
    EXPECT_TRUE(Vec3f(0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 2, 0>()));
    EXPECT_TRUE(Vec3f(0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 2>()));
}

TEST(MathVec3f, Blend)
{
    const Vec3f a(1.0f, 2.0f, 3.0f);
    const Vec3f b(5.0f, 6.0f, 7.0f);

    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 3.0f) == (Vec3f::Blend<0, 0, 0>(a, b)));
    EXPECT_TRUE(Vec3f(1.0f, 2.0f, 7.0f) == (Vec3f::Blend<0, 0, 1>(a, b)));
    EXPECT_TRUE(Vec3f(1.0f, 6.0f, 3.0f) == (Vec3f::Blend<0, 1, 0>(a, b)));
    EXPECT_TRUE(Vec3f(1.0f, 6.0f, 7.0f) == (Vec3f::Blend<0, 1, 1>(a, b)));
    EXPECT_TRUE(Vec3f(5.0f, 2.0f, 3.0f) == (Vec3f::Blend<1, 0, 0>(a, b)));
    EXPECT_TRUE(Vec3f(5.0f, 2.0f, 7.0f) == (Vec3f::Blend<1, 0, 1>(a, b)));
    EXPECT_TRUE(Vec3f(5.0f, 6.0f, 3.0f) == (Vec3f::Blend<1, 1, 0>(a, b)));
    EXPECT_TRUE(Vec3f(5.0f, 6.0f, 7.0f) == (Vec3f::Blend<1, 1, 1>(a, b)));
}

TEST(MathVec3f, ChangeSign)
{
    const Vec3f v(0.5f, 1.0f, 2.0f);

    EXPECT_TRUE(Vec3f(0.5f, 1.0f, 2.0f) == (v.ChangeSign<false, false, false>()));
    EXPECT_TRUE(Vec3f(0.5f, 1.0f, -2.0f) == (v.ChangeSign<false, false, true>()));
    EXPECT_TRUE(Vec3f(0.5f, -1.0f, 2.0f) == (v.ChangeSign<false, true, false>()));
    EXPECT_TRUE(Vec3f(0.5f, -1.0f, -2.0f) == (v.ChangeSign<false, true, true>()));
    EXPECT_TRUE(Vec3f(-0.5f, 1.0f, 2.0f) == (v.ChangeSign<true, false, false>()));
    EXPECT_TRUE(Vec3f(-0.5f, 1.0f, -2.0f) == (v.ChangeSign<true, false, true>()));
    EXPECT_TRUE(Vec3f(-0.5f, -1.0f, 2.0f) == (v.ChangeSign<true, true, false>()));
    EXPECT_TRUE(Vec3f(-0.5f, -1.0f, -2.0f) == (v.ChangeSign<true, true, true>()));
}
