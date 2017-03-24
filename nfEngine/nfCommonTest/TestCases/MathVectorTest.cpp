#include "PCH.hpp"
#include "nfCommon/Math/Vector.hpp"

using namespace NFE::Math;

namespace {

const Vector vecA = Vector(1.0f, 1.0f, 1.0f, 1.0f);
const Vector vecB = Vector(1.0f, 2.0f, 3.0f, 4.0f);
const Vector vecC = Vector(2.0f, 3.0f, 4.0f, 5.0f);
const Vector vecD = Vector(1.0f, 4.0f, 9.0f, 16.0f);
const Vector vecE = Vector(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathVector, VectorLoadAndStore)
{
    Float2 f2;
    Float3 f3;
    Float4 f4;

    /// load, store, splat, etc.
    vecB.Store(&f2);
    vecB.Store(&f3);
    vecB.Store(&f4);
    EXPECT_TRUE(f2.x == 1.0f && f2.y == 2.0f);
    EXPECT_TRUE(f3.x == 1.0f && f3.y == 2.0f && f3.z == 3.0f);
    EXPECT_TRUE(f4.x == 1.0f && f4.y == 2.0f && f4.z == 3.0f && f4.w == 4.0f);
    EXPECT_TRUE(Vector(f2) == Vector(1.0f, 2.0f, 0.0f, 0.0f));
    EXPECT_TRUE(Vector(f3) == Vector(1.0f, 2.0f, 3.0f, 0.0f));
    EXPECT_TRUE(Vector(f4) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vector(123.0f, 123.0f, 123.0f, 123.0f) == Vector::Splat(123.0f));
    EXPECT_TRUE(Vector(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Vector(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Vector(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ());
    EXPECT_TRUE(Vector(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW());
}

TEST(MathVector, VectorMisc)
{
    Vector vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vector vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) ==
                Vector::SelectBySign(vA, vB, Vector(1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector(5.0f, 2.0f, 3.0f, 4.0f) ==
                Vector::SelectBySign(vA, vB, Vector(-1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector(1.0f, 6.0f, 3.0f, 4.0f) ==
                Vector::SelectBySign(vA, vB, Vector(1.0f, -1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector(1.0f, 2.0f, 7.0f, 4.0f) ==
                Vector::SelectBySign(vA, vB, Vector(1.0f, 1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 8.0f) ==
                Vector::SelectBySign(vA, vB, Vector(1.0f, 1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Vector(5.0f, 6.0f, 7.0f, 8.0f) ==
                Vector::SelectBySign(vA, vB, Vector(-1.0f, -1.0f, -1.0f, -1.0f)));
}

TEST(MathVector, VectorArithmetics)
{
    EXPECT_TRUE(Vector::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vector::AlmostEqual(vecA - vecB, Vector(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vector::AlmostEqual(vecC / vecB, Vector(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vector::AlmostEqual(vecB * 2.0f, Vector(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(vecB / 2.0f, Vector(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE(Vector::Abs(Vector(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector(1.0f, 2.0f, 0.0f, 3.0f));
}

TEST(MathVector, VectorLerp)
{
    EXPECT_TRUE(Vector::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Vector::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Vector::Lerp(vecA, vecB, 0.5f) == Vector(1.0f, 1.5f, 2.0f, 2.5f));
}

TEST(MathVector, VectorMinMax)
{
    EXPECT_TRUE(Vector::Min(vecB, vecE) == Vector(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Vector::Min(vecE, vecB) == Vector(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Vector::Max(vecB, vecE) == Vector(4.0f, 3.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vector::Max(vecB, vecE) == Vector(4.0f, 3.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorGeometrics)
{
    EXPECT_EQ(20.0f, Vector::Dot3(vecB, vecC));
    EXPECT_EQ(40.0f, Vector::Dot4(vecB, vecC));
    EXPECT_TRUE(Vector::Dot3V(vecB, vecC) == Vector::Splat(20.0f));
    EXPECT_TRUE(Vector::Dot4V(vecB, vecC) == Vector::Splat(40.0f));
    EXPECT_TRUE(Vector::Cross3(vecB, vecC) == Vector(-1.0f, 2.0f, -1.0f, 0.0f));
    EXPECT_TRUE(Vector::Cross3(vecC, vecB) == Vector(1.0f, -2.0f, 1.0f, 0.0f));
}

TEST(MathVector, VectorLess)
{
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 10.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 10.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 10.0f) < vecC);
    EXPECT_FALSE(Vector(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathVector, VectorLessOrEqual)
{
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(1.0f, 10.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 10.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Vector(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathVector, VectorGreater)
{
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(1.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 1.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 1.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 1.0f) > vecC);
    EXPECT_FALSE(Vector(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathVector, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(1.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(3.0f, 1.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 1.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Vector(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathVector, VectorEqual)
{
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 20.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 30.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 40.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorNotEqual)
{
    EXPECT_TRUE(Vector(4.0f, 3.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 3.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 2.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 3.0f, 3.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 3.0f, 2.0f, 4.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorSwizzle)
{
    const Vector v(0.0f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vector(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>()));
    EXPECT_TRUE(Vector(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>()));
    EXPECT_TRUE(Vector(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>()));
    EXPECT_TRUE(Vector(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>()));
    EXPECT_TRUE(Vector(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>()));

    EXPECT_TRUE(Vector(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>()));
    EXPECT_TRUE(Vector(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>()));

    EXPECT_TRUE(Vector(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>()));
    EXPECT_TRUE(Vector(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>()));

    EXPECT_TRUE(Vector(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>()));
    EXPECT_TRUE(Vector(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>()));
    EXPECT_TRUE(Vector(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>()));
}

TEST(MathVector, ChangeSign)
{
    const Vector v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vector(0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, false, false, false>()));
    EXPECT_TRUE(Vector(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, false, false, true>()));
    EXPECT_TRUE(Vector(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, false, true, false>()));
    EXPECT_TRUE(Vector(0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, false, true, true>()));
    EXPECT_TRUE(Vector(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, true, false, false>()));
    EXPECT_TRUE(Vector(0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, true, false, true>()));
    EXPECT_TRUE(Vector(0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, true, true, false>()));
    EXPECT_TRUE(Vector(0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, true, true, true>()));
    EXPECT_TRUE(Vector(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, false, false, false>()));
    EXPECT_TRUE(Vector(-0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, false, false, true>()));
    EXPECT_TRUE(Vector(-0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, false, true, false>()));
    EXPECT_TRUE(Vector(-0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, false, true, true>()));
    EXPECT_TRUE(Vector(-0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, true, false, false>()));
    EXPECT_TRUE(Vector(-0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, true, false, true>()));
    EXPECT_TRUE(Vector(-0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, true, true, false>()));
    EXPECT_TRUE(Vector(-0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, true, true, true>()));
}

TEST(MathVector, FMA)
{
    const Vector a(0.5f, 1.0f, 2.0f, 3.0f);
    const Vector b(4.0f, 5.0f, 6.0f, 7.0f);
    const Vector c(1.5f, 1.5f, 1.5f, 1.5f);

    EXPECT_TRUE(Vector(3.5f, 6.5f, 13.5f, 22.5f) == Vector::MulAndAdd(a, b, c));
    EXPECT_TRUE(Vector(0.5f, 3.5f, 10.5f, 19.5f) == Vector::MulAndSub(a, b, c));
    EXPECT_TRUE(Vector(-0.5f, -3.5f, -10.5f, -19.5f) == Vector::NegMulAndAdd(a, b, c));
    EXPECT_TRUE(Vector(-3.5f, -6.5f, -13.5f, -22.5f) == Vector::NegMulAndSub(a, b, c));
}

