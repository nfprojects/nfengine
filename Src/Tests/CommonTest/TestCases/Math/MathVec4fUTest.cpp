#include "PCH.hpp"
#include "Engine/Common/Math/Vec4fU.hpp"

using namespace NFE::Math;

namespace {

const Vec4fU vecA = Vec4fU(1.0f, 1.0f, 1.0f, 1.0f);
const Vec4fU vecB = Vec4fU(1.0f, 2.0f, 3.0f, 4.0f);
const Vec4fU vecC = Vec4fU(2.0f, 3.0f, 4.0f, 5.0f);
const Vec4fU vecD = Vec4fU(1.0f, 4.0f, 9.0f, 16.0f);
const Vec4fU vecE = Vec4fU(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathVec4fU, Constructors)
{
    const Vec2f f2(1.0f, 2.0f);
    const Vec3f f3(1.0f, 2.0f, 3.0f);
    const Vec4fU f4(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(Vec4fU(f2) == Vec4fU(1.0f, 2.0f, 0.0f, 0.0f));
    EXPECT_TRUE(Vec4fU(f3) == Vec4fU(1.0f, 2.0f, 3.0f, 0.0f));
    EXPECT_TRUE(Vec4fU(f4) == Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vec4fU(123.0f, 123.0f, 123.0f, 123.0f) == Vec4fU::Splat(123.0f));
    EXPECT_TRUE(Vec4fU(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Vec4fU(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Vec4fU(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ());
    EXPECT_TRUE(Vec4fU(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW());
}

TEST(MathVec4fU, SelectBySign)
{
    Vec4fU vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4fU vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 4.0f) == Vec4fU::SelectBySign(vA, vB, Vec4fU(1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vec4fU(5.0f, 2.0f, 3.0f, 4.0f) == Vec4fU::SelectBySign(vA, vB, Vec4fU(-1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vec4fU(1.0f, 6.0f, 3.0f, 4.0f) == Vec4fU::SelectBySign(vA, vB, Vec4fU(1.0f, -1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 7.0f, 4.0f) == Vec4fU::SelectBySign(vA, vB, Vec4fU(1.0f, 1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 8.0f) == Vec4fU::SelectBySign(vA, vB, Vec4fU(1.0f, 1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Vec4fU(5.0f, 6.0f, 7.0f, 8.0f) == Vec4fU::SelectBySign(vA, vB, Vec4fU(-1.0f, -1.0f, -1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec4fU, VectorArithmetics)
{
    EXPECT_TRUE(Vec4fU::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vec4fU::AlmostEqual(vecA - vecB, Vec4fU(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vec4fU::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vec4fU::AlmostEqual(vecC / vecB, Vec4fU(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vec4fU::AlmostEqual(vecB * 2.0f, Vec4fU(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vec4fU::AlmostEqual(vecB / 2.0f, Vec4fU(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE(Vec4fU::Abs(Vec4fU(-1.0f, -2.0f, 0.0f, 3.0f)) == Vec4fU(1.0f, 2.0f, 0.0f, 3.0f));
}

TEST(MathVec4fU, VectorLerp)
{
    EXPECT_TRUE(Vec4fU::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Vec4fU::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Vec4fU::Lerp(vecA, vecB, 0.5f) == Vec4fU(1.0f, 1.5f, 2.0f, 2.5f));
}

TEST(MathVec4fU, VectorMinMax)
{
    EXPECT_TRUE(Vec4fU::Min(vecB, vecE) == Vec4fU(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Vec4fU::Min(vecE, vecB) == Vec4fU(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Vec4fU::Max(vecB, vecE) == Vec4fU(4.0f, 3.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vec4fU::Max(vecB, vecE) == Vec4fU(4.0f, 3.0f, 3.0f, 4.0f));
}

TEST(MathVec4fU, VectorGeometrics)
{
    EXPECT_EQ(40.0f, Vec4fU::Dot(vecB, vecC));
    // TODO cross product
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec4fU, VectorLess)
{
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(10.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 10.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 10.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 3.0f, 10.0f) < vecC);
    EXPECT_FALSE(Vec4fU(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathVec4fU, VectorLessOrEqual)
{
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vec4fU(10.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 10.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 10.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 3.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Vec4fU(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vec4fU(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathVec4fU, VectorGreater)
{
    EXPECT_TRUE(Vec4fU(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 1.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 4.0f, 1.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 4.0f, 5.0f, 1.0f) > vecC);
    EXPECT_FALSE(Vec4fU(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathVec4fU, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Vec4fU(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vec4fU(1.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 1.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 4.0f, 1.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vec4fU(3.0f, 4.0f, 5.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Vec4fU(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vec4fU(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vec4fU(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vec4fU(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathVec4fU, VectorEqual)
{
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 4.0f) == Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(10.0f, 2.0f, 3.0f, 4.0f) == Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(1.0f, 20.0f, 3.0f, 4.0f) == Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 30.0f, 4.0f) == Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(1.0f, 2.0f, 3.0f, 40.0f) == Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathVec4fU, VectorNotEqual)
{
    EXPECT_TRUE(Vec4fU(4.0f, 3.0f, 2.0f, 1.0f) != Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(1.0f, 3.0f, 2.0f, 1.0f) != Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(4.0f, 2.0f, 2.0f, 1.0f) != Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(4.0f, 3.0f, 3.0f, 1.0f) != Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vec4fU(4.0f, 3.0f, 2.0f, 4.0f) != Vec4fU(1.0f, 2.0f, 3.0f, 4.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVec4fU, VectorSwizzle)
{
    const Vec4fU v(0.0f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vec4fU(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>()));
    EXPECT_TRUE(Vec4fU(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>()));
    EXPECT_TRUE(Vec4fU(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>()));
    EXPECT_TRUE(Vec4fU(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>()));
    EXPECT_TRUE(Vec4fU(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>()));

    EXPECT_TRUE(Vec4fU(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>()));

    EXPECT_TRUE(Vec4fU(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>()));

    EXPECT_TRUE(Vec4fU(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>()));
    EXPECT_TRUE(Vec4fU(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>()));
}

TEST(MathVec4fU, Blend)
{
    const Vec4fU a(1.0f, 2.0f, 3.0f, 4.0f);
    const Vec4fU b(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 4.0f) == (Vec4fU::Blend<0, 0, 0, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 3.0f, 8.0f) == (Vec4fU::Blend<0, 0, 0, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 7.0f, 4.0f) == (Vec4fU::Blend<0, 0, 1, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 2.0f, 7.0f, 8.0f) == (Vec4fU::Blend<0, 0, 1, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 6.0f, 3.0f, 4.0f) == (Vec4fU::Blend<0, 1, 0, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 6.0f, 3.0f, 8.0f) == (Vec4fU::Blend<0, 1, 0, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 6.0f, 7.0f, 4.0f) == (Vec4fU::Blend<0, 1, 1, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(1.0f, 6.0f, 7.0f, 8.0f) == (Vec4fU::Blend<0, 1, 1, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 2.0f, 3.0f, 4.0f) == (Vec4fU::Blend<1, 0, 0, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 2.0f, 3.0f, 8.0f) == (Vec4fU::Blend<1, 0, 0, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 2.0f, 7.0f, 4.0f) == (Vec4fU::Blend<1, 0, 1, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 2.0f, 7.0f, 8.0f) == (Vec4fU::Blend<1, 0, 1, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 6.0f, 3.0f, 4.0f) == (Vec4fU::Blend<1, 1, 0, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 6.0f, 3.0f, 8.0f) == (Vec4fU::Blend<1, 1, 0, 1>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 6.0f, 7.0f, 4.0f) == (Vec4fU::Blend<1, 1, 1, 0>(a, b)));
    EXPECT_TRUE(Vec4fU(5.0f, 6.0f, 7.0f, 8.0f) == (Vec4fU::Blend<1, 1, 1, 1>(a, b)));
}

TEST(MathVec4fU, ChangeSign)
{
    const Vec4fU v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vec4fU(0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, false, false, false>()));
    EXPECT_TRUE(Vec4fU(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, false, false, true>()));
    EXPECT_TRUE(Vec4fU(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, false, true, false>()));
    EXPECT_TRUE(Vec4fU(0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, false, true, true>()));
    EXPECT_TRUE(Vec4fU(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, true, false, false>()));
    EXPECT_TRUE(Vec4fU(0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, true, false, true>()));
    EXPECT_TRUE(Vec4fU(0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, true, true, false>()));
    EXPECT_TRUE(Vec4fU(0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, true, true, true>()));
    EXPECT_TRUE(Vec4fU(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, false, false, false>()));
    EXPECT_TRUE(Vec4fU(-0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, false, false, true>()));
    EXPECT_TRUE(Vec4fU(-0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, false, true, false>()));
    EXPECT_TRUE(Vec4fU(-0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, false, true, true>()));
    EXPECT_TRUE(Vec4fU(-0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, true, false, false>()));
    EXPECT_TRUE(Vec4fU(-0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, true, false, true>()));
    EXPECT_TRUE(Vec4fU(-0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, true, true, false>()));
    EXPECT_TRUE(Vec4fU(-0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, true, true, true>()));
}
