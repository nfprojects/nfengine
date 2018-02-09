#include "PCH.hpp"
#include "nfCommon/Math/Vector4.hpp"

using namespace NFE::Math;

namespace {

const Vector4 vecA = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 vecB = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
const Vector4 vecC = Vector4(2.0f, 3.0f, 4.0f, 5.0f);
const Vector4 vecD = Vector4(1.0f, 4.0f, 9.0f, 16.0f);
const Vector4 vecE = Vector4(4.0f, 3.0f, 2.0f, 1.0f);

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
    EXPECT_TRUE(Vector4(f2) == Vector4(1.0f, 2.0f, 0.0f, 0.0f));
    EXPECT_TRUE(Vector4(f3) == Vector4(1.0f, 2.0f, 3.0f, 0.0f));
    EXPECT_TRUE(Vector4(f4) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vector4(123.0f, 123.0f, 123.0f, 123.0f) == Vector4::Splat(123.0f));
    EXPECT_TRUE(Vector4(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Vector4(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Vector4(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ());
    EXPECT_TRUE(Vector4(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW());
}

TEST(MathVector, SelectBySign)
{
    Vector4 vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 4.0f) ==
                Vector4::SelectBySign(vA, vB, Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector4(5.0f, 2.0f, 3.0f, 4.0f) ==
                Vector4::SelectBySign(vA, vB, Vector4(-1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector4(1.0f, 6.0f, 3.0f, 4.0f) ==
                Vector4::SelectBySign(vA, vB, Vector4(1.0f, -1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 7.0f, 4.0f) ==
                Vector4::SelectBySign(vA, vB, Vector4(1.0f, 1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 8.0f) ==
                Vector4::SelectBySign(vA, vB, Vector4(1.0f, 1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Vector4(5.0f, 6.0f, 7.0f, 8.0f) ==
                Vector4::SelectBySign(vA, vB, Vector4(-1.0f, -1.0f, -1.0f, -1.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVector, VectorArithmetics)
{
    EXPECT_TRUE(Vector4::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vector4::AlmostEqual(vecA - vecB, Vector4(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vector4::AlmostEqual(vecC / vecB, Vector4(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB * 2.0f, Vector4(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(vecB / 2.0f, Vector4(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE(Vector4::Abs(Vector4(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector4(1.0f, 2.0f, 0.0f, 3.0f));
}

TEST(MathVector, VectorLerp)
{
    EXPECT_TRUE(Vector4::Lerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(Vector4::Lerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(Vector4::Lerp(vecA, vecB, 0.5f) == Vector4(1.0f, 1.5f, 2.0f, 2.5f));
}

TEST(MathVector, VectorMinMax)
{
    EXPECT_TRUE(Vector4::Min(vecB, vecE) == Vector4(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Vector4::Min(vecE, vecB) == Vector4(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(Vector4::Max(vecB, vecE) == Vector4(4.0f, 3.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vector4::Max(vecB, vecE) == Vector4(4.0f, 3.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorDotProduct)
{
    EXPECT_EQ(8.0f, Vector4::Dot2(vecB, vecC));
    EXPECT_EQ(20.0f, Vector4::Dot3(vecB, vecC));
    EXPECT_EQ(40.0f, Vector4::Dot4(vecB, vecC));

    EXPECT_TRUE(Vector4::Dot2V(vecB, vecC) == Vector4::Splat(8.0f));
    EXPECT_TRUE(Vector4::Dot3V(vecB, vecC) == Vector4::Splat(20.0f));
    EXPECT_TRUE(Vector4::Dot4V(vecB, vecC) == Vector4::Splat(40.0f));
}

TEST(MathVector, VectorCrossProduct)
{
    EXPECT_TRUE(Vector4::Cross3(vecB, vecC) == Vector4(-1.0f, 2.0f, -1.0f, 0.0f));
    EXPECT_TRUE(Vector4::Cross3(vecC, vecB) == Vector4(1.0f, -2.0f, 1.0f, 0.0f));
}

TEST(MathVector, VectorNormalized)
{
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized2() & VECTOR_MASK_XY,
                                    Vector4(1.0f / sqrtf(5.0f), 2.0f / sqrtf(5.0f), 0.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized3() & VECTOR_MASK_XYZ,
                                    Vector4(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(Vector4(1.0f, 2.0f, 3.0f, 4.0f).Normalized4(),
                                    Vector4(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathVector, VectorNormalize)
{
    Vector4 v2 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v3 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v4 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);

    v2.Normalize2();
    v3.Normalize3();
    v4.Normalize4();

    EXPECT_TRUE(Vector4::AlmostEqual(v2 & VECTOR_MASK_XY,
                                    Vector4(1.0f / sqrtf(5.0f), 2.0f / sqrtf(5.0f), 0.0f, 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(v3 & VECTOR_MASK_XYZ,
                                    Vector4(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
    EXPECT_TRUE(Vector4::AlmostEqual(v4,
                                    Vector4(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathVector, FusedMultiplyAndAdd)
{
    const Vector4 a(0.5f, 1.0f, 2.0f, 3.0f);
    const Vector4 b(4.0f, 5.0f, 6.0f, 7.0f);
    const Vector4 c(1.5f, 1.5f, 1.5f, 1.5f);

    EXPECT_TRUE(Vector4(3.5f, 6.5f, 13.5f, 22.5f) == Vector4::MulAndAdd(a, b, c));
    EXPECT_TRUE(Vector4(0.5f, 3.5f, 10.5f, 19.5f) == Vector4::MulAndSub(a, b, c));
    EXPECT_TRUE(Vector4(-0.5f, -3.5f, -10.5f, -19.5f) == Vector4::NegMulAndAdd(a, b, c));
    EXPECT_TRUE(Vector4(-3.5f, -6.5f, -13.5f, -22.5f) == Vector4::NegMulAndSub(a, b, c));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVector, VectorLess)
{
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(10.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(1.0f, 10.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 10.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 3.0f, 10.0f) < vecC);
    EXPECT_FALSE(Vector4(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathVector, VectorLessOrEqual)
{
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector4(10.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector4(1.0f, 10.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 10.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 3.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Vector4(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector4(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathVector, VectorGreater)
{
    EXPECT_TRUE(Vector4(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(1.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(3.0f, 1.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(3.0f, 4.0f, 1.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(3.0f, 4.0f, 5.0f, 1.0f) > vecC);
    EXPECT_FALSE(Vector4(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector4(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathVector, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Vector4(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector4(1.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector4(3.0f, 1.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector4(3.0f, 4.0f, 1.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector4(3.0f, 4.0f, 5.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Vector4(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector4(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector4(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector4(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathVector, VectorEqual)
{
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(10.0f, 2.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(1.0f, 20.0f, 3.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 30.0f, 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(1.0f, 2.0f, 3.0f, 40.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorNotEqual)
{
    EXPECT_TRUE(Vector4(4.0f, 3.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(1.0f, 3.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(4.0f, 2.0f, 2.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(4.0f, 3.0f, 3.0f, 1.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector4(4.0f, 3.0f, 2.0f, 4.0f) != Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVector, VectorLess3)
{
    EXPECT_TRUE(Vector4::Less3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 30.0f, 4.0f)));
    EXPECT_TRUE(Vector4::Less3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 30.0f, 0.0f)));
    EXPECT_FALSE(Vector4::Less3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 20.0f, 30.0f, 0.0f)));
    EXPECT_FALSE(Vector4::Less3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 2.0f, 30.0f, 0.0f)));
    EXPECT_FALSE(Vector4::Less3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 3.0f, 0.0f)));
}

TEST(MathVector, VectorGreater3)
{
    EXPECT_TRUE(Vector4::Greater3(Vector4(10.0f, 20.0f, 30.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_TRUE(Vector4::Greater3(Vector4(10.0f, 20.0f, 30.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 40.0f)));
    EXPECT_FALSE(Vector4::Greater3(Vector4(1.0f, 20.0f, 30.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 40.0f)));
    EXPECT_FALSE(Vector4::Greater3(Vector4(10.0f, 2.0f, 30.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 40.0f)));
    EXPECT_FALSE(Vector4::Greater3(Vector4(10.0f, 20.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 40.0f)));
}

TEST(MathVector, VectorLessEq3)
{
    EXPECT_TRUE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 30.0f, 4.0f)));
    EXPECT_TRUE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 30.0f, 40.0f)));
    EXPECT_TRUE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_TRUE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 0.0f)));
    EXPECT_FALSE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(0.0f, 20.0f, 30.0f, 40.0f)));
    EXPECT_FALSE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 0.0f, 30.0f, 40.0f)));
    EXPECT_FALSE(Vector4::LessEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 0.0f, 40.0f)));
}

TEST(MathVector, VectorGreaterEq3)
{
    EXPECT_TRUE(Vector4::GreaterEq3(Vector4(10.0f, 2.0f, 30.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_TRUE(Vector4::GreaterEq3(Vector4(10.0f, 20.0f, 30.0f, 40.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_TRUE(Vector4::GreaterEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_TRUE(Vector4::GreaterEq3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 111.0f)));
    EXPECT_FALSE(Vector4::GreaterEq3(Vector4(0.0f, 20.0f, 30.0f, 40.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::GreaterEq3(Vector4(10.0f, 0.0f, 30.0f, 40.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::GreaterEq3(Vector4(10.0f, 20.0f, 0.0f, 40.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
}


TEST(MathVector, VectorEqual3)
{
    EXPECT_TRUE(Vector4::Equal3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_TRUE(Vector4::Equal3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 111.0f)));
    EXPECT_FALSE(Vector4::Equal3(Vector4(111.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::Equal3(Vector4(1.0f, 222.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::Equal3(Vector4(1.0f, 2.0f, 333.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
}

TEST(MathVector, VectorNotEqual3)
{
    EXPECT_TRUE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 30.0f, 40.0f)));
    EXPECT_TRUE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(10.0f, 20.0f, 30.0f, 4.0f)));
    EXPECT_FALSE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 333.0f, 4.0f)));
    EXPECT_FALSE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 222.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(111.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FALSE(Vector4::NotEqual3(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(1.0f, 2.0f, 3.0f, 444.0f)));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathVector, VectorSwizzle)
{
    const Vector4 v(0.0f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vector4(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>()));
    EXPECT_TRUE(Vector4(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>()));
    EXPECT_TRUE(Vector4(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>()));
    EXPECT_TRUE(Vector4(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>()));
    EXPECT_TRUE(Vector4(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>()));

    EXPECT_TRUE(Vector4(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>()));

    EXPECT_TRUE(Vector4(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>()));

    EXPECT_TRUE(Vector4(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>()));
    EXPECT_TRUE(Vector4(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>()));
}

TEST(MathVector, Blend)
{
    const Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    const Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 4.0f) == (Vector4::Blend<0, 0, 0, 0>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 3.0f, 8.0f) == (Vector4::Blend<0, 0, 0, 1>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 7.0f, 4.0f) == (Vector4::Blend<0, 0, 1, 0>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 2.0f, 7.0f, 8.0f) == (Vector4::Blend<0, 0, 1, 1>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 6.0f, 3.0f, 4.0f) == (Vector4::Blend<0, 1, 0, 0>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 6.0f, 3.0f, 8.0f) == (Vector4::Blend<0, 1, 0, 1>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 6.0f, 7.0f, 4.0f) == (Vector4::Blend<0, 1, 1, 0>(a, b)));
    EXPECT_TRUE(Vector4(1.0f, 6.0f, 7.0f, 8.0f) == (Vector4::Blend<0, 1, 1, 1>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 2.0f, 3.0f, 4.0f) == (Vector4::Blend<1, 0, 0, 0>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 2.0f, 3.0f, 8.0f) == (Vector4::Blend<1, 0, 0, 1>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 2.0f, 7.0f, 4.0f) == (Vector4::Blend<1, 0, 1, 0>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 2.0f, 7.0f, 8.0f) == (Vector4::Blend<1, 0, 1, 1>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 6.0f, 3.0f, 4.0f) == (Vector4::Blend<1, 1, 0, 0>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 6.0f, 3.0f, 8.0f) == (Vector4::Blend<1, 1, 0, 1>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 6.0f, 7.0f, 4.0f) == (Vector4::Blend<1, 1, 1, 0>(a, b)));
    EXPECT_TRUE(Vector4(5.0f, 6.0f, 7.0f, 8.0f) == (Vector4::Blend<1, 1, 1, 1>(a, b)));
}

TEST(MathVector, ChangeSign)
{
    const Vector4 v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(Vector4(0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, false, false, false>()));
    EXPECT_TRUE(Vector4(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, false, false, true>()));
    EXPECT_TRUE(Vector4(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, false, true, false>()));
    EXPECT_TRUE(Vector4(0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, false, true, true>()));
    EXPECT_TRUE(Vector4(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, true, false, false>()));
    EXPECT_TRUE(Vector4(0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, true, false, true>()));
    EXPECT_TRUE(Vector4(0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, true, true, false>()));
    EXPECT_TRUE(Vector4(0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, true, true, true>()));
    EXPECT_TRUE(Vector4(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, false, false, false>()));
    EXPECT_TRUE(Vector4(-0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, false, false, true>()));
    EXPECT_TRUE(Vector4(-0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, false, true, false>()));
    EXPECT_TRUE(Vector4(-0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, false, true, true>()));
    EXPECT_TRUE(Vector4(-0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, true, false, false>()));
    EXPECT_TRUE(Vector4(-0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, true, false, true>()));
    EXPECT_TRUE(Vector4(-0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, true, true, false>()));
    EXPECT_TRUE(Vector4(-0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, true, true, true>()));
}
