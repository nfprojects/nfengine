#include "PCH.hpp"
#include "Engine/Common/Math/Vec4f.hpp"
#include "Engine/Common/Math/Vec4i.hpp"

using namespace NFE;
using namespace NFE::Math;

namespace {

static const Vec4f vecA = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
static const Vec4f vecB = Vec4f(1.0f, 2.0f, 3.0f, 4.0f);
static const Vec4f vecC = Vec4f(2.0f, 3.0f, 4.0f, 5.0f);
static const Vec4f vecD = Vec4f(1.0f, 4.0f, 9.0f, 16.0f);
static const Vec4f vecE = Vec4f(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathTest, Vec4f_Constructor1)
{
    const Vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(1.0f, v.x);
    EXPECT_EQ(2.0f, v.y);
    EXPECT_EQ(3.0f, v.z);
    EXPECT_EQ(4.0f, v.w);
}

TEST(MathTest, Vec4f_Constructor2)
{
    const Vec4f v(7.0f);
    EXPECT_EQ(7.0f, v.x);
    EXPECT_EQ(7.0f, v.y);
    EXPECT_EQ(7.0f, v.z);
    EXPECT_EQ(7.0f, v.w);
}

TEST(MathTest, Vec4f_IsInfinite)
{
    EXPECT_FALSE(Vec4f(0.0f, 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_FALSE(Vec4f(1234.0f, 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_FALSE(Vec4f(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_FALSE(Vec4f(std::numeric_limits<float>::signaling_NaN(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_FALSE(Vec4f(std::numeric_limits<float>::max(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_FALSE(Vec4f(-std::numeric_limits<float>::max(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_FALSE(Vec4f(std::numeric_limits<float>::min(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_TRUE(Vec4f(-std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
    EXPECT_TRUE(Vec4f(-std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f).IsInfinite().Any());
}

TEST(MathTest, Vec4f_IsNAN)
{
    EXPECT_FALSE(Vec4f(0.0f, 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_FALSE(Vec4f(1234.0f, 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_TRUE(Vec4f(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_TRUE(Vec4f(std::numeric_limits<float>::signaling_NaN(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_FALSE(Vec4f(std::numeric_limits<float>::max(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_FALSE(Vec4f(-std::numeric_limits<float>::max(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_FALSE(Vec4f(std::numeric_limits<float>::min(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_FALSE(Vec4f(-std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
    EXPECT_FALSE(Vec4f(-std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f).IsNaN().Any());
}

TEST(MathTest, Vec4f_Invalid)
{
    EXPECT_TRUE(Vec4f(0.0f).IsValid());
    EXPECT_TRUE(Vec4f(-1.0f, 2.0f, 3.0f, 4.0f).IsValid());
    EXPECT_TRUE(Vec4f(-1.0f, 2.0f, 3.0f, std::numeric_limits<float>::max()).IsValid());
    EXPECT_TRUE(Vec4f(-1.0f, 2.0f, 3.0f, std::numeric_limits<float>::min()).IsValid());
    EXPECT_TRUE(Vec4f(-1.0f, 2.0f, 3.0f, -std::numeric_limits<float>::max()).IsValid());
    EXPECT_TRUE(Vec4f(-1.0f, 2.0f, 3.0f, -std::numeric_limits<float>::min()).IsValid());

    EXPECT_FALSE(Vec4f(-1.0f, std::numeric_limits<float>::quiet_NaN(), 3.0f, 4.0f).IsValid());
    EXPECT_FALSE(Vec4f(-1.0f, std::numeric_limits<float>::infinity(), 3.0f, 4.0f).IsValid());
    EXPECT_FALSE(Vec4f(-1.0f, -std::numeric_limits<float>::infinity(), 3.0f, 4.0f).IsValid());
}

TEST(MathTest, Vec4f_ToFromFloat2)
{
    const Vec2f f2 = vecB.ToVec2f();
    EXPECT_TRUE(f2.x == 1.0f && f2.y == 2.0f);
    EXPECT_TRUE((Vec4f(f2) == Vec4f(1.0f, 2.0f, 0.0f, 0.0f)).All());
}

TEST(MathTest, Vec4f_ToFromFloat3)
{
    const Vec3f f3 = vecB.ToVec3f();
    EXPECT_TRUE(f3.x == 1.0f && f3.y == 2.0f && f3.z == 3.0f);
    EXPECT_TRUE((Vec4f(f3) == Vec4f(1.0f, 2.0f, 3.0f, 0.0f)).All());
}

TEST(MathTest, Vec4f_Splat)
{
    EXPECT_TRUE((Vec4f(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX()).All());
    EXPECT_TRUE((Vec4f(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY()).All());
    EXPECT_TRUE((Vec4f(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ()).All());
    EXPECT_TRUE((Vec4f(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW()).All());
}

TEST(MathTest, Vec4f_Select_Variable)
{
    Vec4f vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4f vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 4.0f) == Vec4f::Select(vA, vB, VecBool4f(false, false, false, false))).All());
    EXPECT_TRUE((Vec4f(5.0f, 2.0f, 3.0f, 4.0f) == Vec4f::Select(vA, vB, VecBool4f(true, false, false, false))).All());
    EXPECT_TRUE((Vec4f(1.0f, 6.0f, 3.0f, 4.0f) == Vec4f::Select(vA, vB, VecBool4f(false, true, false, false))).All());
    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 7.0f, 4.0f) == Vec4f::Select(vA, vB, VecBool4f(false, false, true, false))).All());
    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 8.0f) == Vec4f::Select(vA, vB, VecBool4f(false, false, false, true))).All());
    EXPECT_TRUE((Vec4f(5.0f, 6.0f, 7.0f, 8.0f) == Vec4f::Select(vA, vB, VecBool4f(true, true, true, true))).All());
}

TEST(MathTest, Vec4f_Select_Immediate)
{
    const Vec4f a(1.0f, 2.0f, 3.0f, 4.0f);
    const Vec4f b(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 4.0f) == Vec4f::Select<0, 0, 0, 0>(a, b)).All());
    EXPECT_TRUE((Vec4f(5.0f, 6.0f, 7.0f, 8.0f) == Vec4f::Select<1, 1, 1, 1>(a, b)).All());

    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 8.0f) == Vec4f::Select<0, 0, 0, 1>(a, b)).All());
    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 7.0f, 4.0f) == Vec4f::Select<0, 0, 1, 0>(a, b)).All());
    EXPECT_TRUE((Vec4f(1.0f, 6.0f, 3.0f, 4.0f) == Vec4f::Select<0, 1, 0, 0>(a, b)).All());
    EXPECT_TRUE((Vec4f(5.0f, 2.0f, 3.0f, 4.0f) == Vec4f::Select<1, 0, 0, 0>(a, b)).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4f_Arithmetics)
{
    EXPECT_TRUE(Vec4f::AlmostEqual(vecA + vecB, vecC));
    EXPECT_TRUE(Vec4f::AlmostEqual(vecA - vecB, Vec4f(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vec4f::AlmostEqual(vecC / vecB, Vec4f(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(vecB * 2.0f, Vec4f(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(vecB / 2.0f, Vec4f(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE((Vec4f::Abs(Vec4f(-1.0f, -2.0f, 0.0f, 3.0f)) == Vec4f(1.0f, 2.0f, 0.0f, 3.0f)).All());
}

TEST(MathTest, Vec4f_Lerp)
{
    EXPECT_TRUE((Vec4f::Lerp(vecA, vecB, 0.0f) == vecA).All());
    EXPECT_TRUE((Vec4f::Lerp(vecA, vecB, 1.0f) == vecB).All());
    EXPECT_TRUE((Vec4f::Lerp(vecA, vecB, 0.5f) == Vec4f(1.0f, 1.5f, 2.0f, 2.5f)).All());
}

TEST(MathTest, Vec4f_MinMax)
{
    EXPECT_TRUE((Vec4f::Min(vecB, vecE) == Vec4f(1.0f, 2.0f, 2.0f, 1.0f)).All());
    EXPECT_TRUE((Vec4f::Min(vecE, vecB) == Vec4f(1.0f, 2.0f, 2.0f, 1.0f)).All());
    EXPECT_TRUE((Vec4f::Max(vecB, vecE) == Vec4f(4.0f, 3.0f, 3.0f, 4.0f)).All());
    EXPECT_TRUE((Vec4f::Max(vecB, vecE) == Vec4f(4.0f, 3.0f, 3.0f, 4.0f)).All());
}

TEST(MathTest, Vec4f_DotProduct)
{
    EXPECT_EQ(8.0f, Vec4f::Dot2(vecB, vecC));
    EXPECT_EQ(20.0f, Vec4f::Dot3(vecB, vecC));
    EXPECT_EQ(40.0f, Vec4f::Dot4(vecB, vecC));

    EXPECT_TRUE((Vec4f::Dot2V(vecB, vecC) == Vec4f(8.0f)).All());
    EXPECT_TRUE((Vec4f::Dot3V(vecB, vecC) == Vec4f(20.0f)).All());
    EXPECT_TRUE((Vec4f::Dot4V(vecB, vecC) == Vec4f(40.0f)).All());
}

TEST(MathTest, Vec4f_CrossProduct)
{
    EXPECT_TRUE((Vec4f::Cross3(vecB, vecC) == Vec4f(-1.0f, 2.0f, -1.0f, 0.0f)).All());
    EXPECT_TRUE((Vec4f::Cross3(vecC, vecB) == Vec4f(1.0f, -2.0f, 1.0f, 0.0f)).All());
    EXPECT_TRUE((Vec4f::Cross3(Vec4f(1.0f, 0.0f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f)) == Vec4f(0.0f, 0.0f, 1.0f)).All());
}

TEST(MathTest, Vec4f_Normalized)
{
    EXPECT_TRUE(Vec4f::AlmostEqual(Vec4f(1.0f, 2.0f, 3.0f, 4.0f).Normalized3() & Vec4f::MakeMask<1,1,1,0>(),
                                    Vec4f(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(Vec4f(1.0f, 2.0f, 3.0f, 4.0f).Normalized4(),
                                    Vec4f(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathTest, Vec4f_Normalize3)
{
    Vec4f v = Vec4f(1.0f, 2.0f, 3.0f, 4.0f);

    v.Normalize3();

    EXPECT_TRUE(Vec4f::AlmostEqual(v & Vec4f::MakeMask<1, 1, 1, 0>(),
                                    Vec4f(1.0f / sqrtf(14.0f), 2.0f / sqrtf(14.0f), 3.0f / sqrtf(14.0f), 0.0f)));
}

TEST(MathTest, Vec4f_Normalize4)
{
    Vec4f v = Vec4f(1.0f, 2.0f, 3.0f, 4.0f);

    v.Normalize4();

    EXPECT_TRUE(Vec4f::AlmostEqual(v,
        Vec4f(1.0f / sqrtf(30.0f), 2.0f / sqrtf(30.0f), 3.0f / sqrtf(30.0f), 4.0f / sqrtf(30.0f))));
}

TEST(MathTest, Vec4f_FusedMultiplyAndAdd)
{
    const Vec4f a(0.5f, 1.0f, 2.0f, 3.0f);
    const Vec4f b(4.0f, 5.0f, 6.0f, 7.0f);
    const Vec4f c(1.5f, 1.5f, 1.5f, 1.5f);

    EXPECT_TRUE((Vec4f(3.5f, 6.5f, 13.5f, 22.5f) == Vec4f::MulAndAdd(a, b, c)).All());
    EXPECT_TRUE((Vec4f(0.5f, 3.5f, 10.5f, 19.5f) == Vec4f::MulAndSub(a, b, c)).All());
    EXPECT_TRUE((Vec4f(-0.5f, -3.5f, -10.5f, -19.5f) == Vec4f::NegMulAndAdd(a, b, c)).All());
    EXPECT_TRUE((Vec4f(-3.5f, -6.5f, -13.5f, -22.5f) == Vec4f::NegMulAndSub(a, b, c)).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4f_Less)
{
    EXPECT_EQ(VecBool4f(false, false, false, false), Vec4f(2.0f, 3.0f, 4.0f, 5.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(9.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(1.0f, 9.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(1.0f, 2.0f, 9.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(1.0f, 2.0f, 3.0f, 9.0f) < vecC);
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathTest, Vec4f_LessOrEqual)
{
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(2.0f, 3.0f, 4.0f, 5.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(9.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(1.0f, 9.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(1.0f, 2.0f, 9.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(1.0f, 2.0f, 3.0f, 9.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathTest, Vec4f_Greater)
{
    EXPECT_EQ(VecBool4f(false, false, false, false), Vec4f(2.0f, 3.0f, 4.0f, 5.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(0.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(3.0f, 0.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(3.0f, 4.0f, 0.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(3.0f, 4.0f, 5.0f, 0.0f) > vecC);
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathTest, Vec4f_GreaterOrEqual)
{
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(2.0f, 3.0f, 4.0f, 5.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(0.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(3.0f, 0.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(3.0f, 4.0f, 0.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(3.0f, 4.0f, 5.0f, 0.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathTest, Vec4f_Equal)
{
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(1.0f, 2.0f, 3.0f, 4.0f) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(10.0f, 2.0f, 3.0f, 4.0f) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(1.0f, 20.0f, 3.0f, 4.0f) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(1.0f, 2.0f, 30.0f, 4.0f) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(1.0f, 2.0f, 3.0f, 40.0f) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathTest, Vec4f_NotEqual)
{
    EXPECT_EQ(VecBool4f(true, true, true, true), Vec4f(4.0f, 3.0f, 2.0f, 1.0f) != Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(false, true, true, true), Vec4f(1.0f, 3.0f, 2.0f, 1.0f) != Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(true, false, true, true), Vec4f(4.0f, 2.0f, 2.0f, 1.0f) != Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(true, true, false, true), Vec4f(4.0f, 3.0f, 3.0f, 1.0f) != Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(VecBool4f(true, true, true, false), Vec4f(4.0f, 3.0f, 2.0f, 4.0f) != Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4f_Swizzle_Variable)
{
    const Vec4f v(0.0f, 1.0f, 2.0f, 3.0f);

    for (int32 x = 0; x < 4; ++x)
    {
        for (int32 y = 0; y < 4; ++y)
        {
            for (int32 z = 0; z < 4; ++z)
            {
                for (int32 w = 0; w < 4; ++w)
                {
                    const Vec4f s = v.Swizzle(x, y, z, w);
                    EXPECT_EQ(v[x], s.x);
                    EXPECT_EQ(v[y], s.y);
                    EXPECT_EQ(v[z], s.z);
                    EXPECT_EQ(v[w], s.w);
                }
            }
        }
    }
}

TEST(MathTest, Vec4f_Swizzle_Immediate)
{
    const Vec4f v(0.0f, 1.0f, 2.0f, 3.0f);

    {
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<0, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<0, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<0, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<0, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<0, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<0, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<0, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<0, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<0, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<0, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<0, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<0, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<0, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<0, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<0, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<0, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<0, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<0, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<0, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<0, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<0, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<0, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<0, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<0, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<0, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<0, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<0, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<0, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<0, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<0, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<0, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<0, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<0, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<0, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<0, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<0, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<0, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<0, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<0, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<0, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<0, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<0, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<0, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<0, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<0, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<0, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<0, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<0, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<0, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<0, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<0, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<0, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<0, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<0, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<0, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<0, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<0, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<0, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<0, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<0, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<0, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<0, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<0, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(0.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<0, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<1, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<1, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<1, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<1, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<1, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<1, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<1, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<1, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<1, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<1, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<1, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<1, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<1, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<1, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<1, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<1, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<1, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<1, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<1, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<1, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<1, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<1, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<1, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<1, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<1, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<1, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<1, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<1, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<1, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<1, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<1, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<1, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<1, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<1, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<1, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<1, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<1, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<1, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<1, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<1, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<1, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<1, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<1, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<1, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<1, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<1, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<1, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<1, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<1, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<1, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<1, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<1, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<1, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<1, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<1, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<1, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<1, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<1, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<1, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<1, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<1, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<1, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<1, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(1.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<1, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<2, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<2, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<2, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<2, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<2, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<2, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<2, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<2, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<2, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<2, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<2, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<2, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<2, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<2, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<2, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<2, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<2, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<2, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<2, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<2, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<2, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<2, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<2, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<2, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<2, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<2, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<2, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<2, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<2, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<2, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<2, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<2, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<2, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<2, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<2, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<2, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<2, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<2, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<2, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<2, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<2, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<2, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<2, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<2, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<2, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<2, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<2, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<2, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<2, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<2, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<2, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<2, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<2, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<2, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<2, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<2, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<2, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<2, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<2, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<2, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<2, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<2, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<2, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(2.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<2, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 0.0f, 0.0f) == (v.Swizzle<3, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 0.0f, 1.0f) == (v.Swizzle<3, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 0.0f, 2.0f) == (v.Swizzle<3, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 0.0f, 3.0f) == (v.Swizzle<3, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 1.0f, 0.0f) == (v.Swizzle<3, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 1.0f, 1.0f) == (v.Swizzle<3, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 1.0f, 2.0f) == (v.Swizzle<3, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 1.0f, 3.0f) == (v.Swizzle<3, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 2.0f, 0.0f) == (v.Swizzle<3, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 2.0f, 1.0f) == (v.Swizzle<3, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 2.0f, 2.0f) == (v.Swizzle<3, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 2.0f, 3.0f) == (v.Swizzle<3, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 3.0f, 0.0f) == (v.Swizzle<3, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 3.0f, 1.0f) == (v.Swizzle<3, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 3.0f, 2.0f) == (v.Swizzle<3, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 0.0f, 3.0f, 3.0f) == (v.Swizzle<3, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 0.0f, 0.0f) == (v.Swizzle<3, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 0.0f, 1.0f) == (v.Swizzle<3, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 0.0f, 2.0f) == (v.Swizzle<3, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 0.0f, 3.0f) == (v.Swizzle<3, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 1.0f, 0.0f) == (v.Swizzle<3, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 1.0f, 1.0f) == (v.Swizzle<3, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 1.0f, 2.0f) == (v.Swizzle<3, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 1.0f, 3.0f) == (v.Swizzle<3, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 2.0f, 0.0f) == (v.Swizzle<3, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 2.0f, 1.0f) == (v.Swizzle<3, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 2.0f, 2.0f) == (v.Swizzle<3, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 2.0f, 3.0f) == (v.Swizzle<3, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 3.0f, 0.0f) == (v.Swizzle<3, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 3.0f, 1.0f) == (v.Swizzle<3, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 3.0f, 2.0f) == (v.Swizzle<3, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 1.0f, 3.0f, 3.0f) == (v.Swizzle<3, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 0.0f, 0.0f) == (v.Swizzle<3, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 0.0f, 1.0f) == (v.Swizzle<3, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 0.0f, 2.0f) == (v.Swizzle<3, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 0.0f, 3.0f) == (v.Swizzle<3, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 1.0f, 0.0f) == (v.Swizzle<3, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 1.0f, 1.0f) == (v.Swizzle<3, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 1.0f, 2.0f) == (v.Swizzle<3, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 1.0f, 3.0f) == (v.Swizzle<3, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 2.0f, 0.0f) == (v.Swizzle<3, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 2.0f, 1.0f) == (v.Swizzle<3, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 2.0f, 2.0f) == (v.Swizzle<3, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 2.0f, 3.0f) == (v.Swizzle<3, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 3.0f, 0.0f) == (v.Swizzle<3, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 3.0f, 1.0f) == (v.Swizzle<3, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 3.0f, 2.0f) == (v.Swizzle<3, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 2.0f, 3.0f, 3.0f) == (v.Swizzle<3, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 0.0f, 0.0f) == (v.Swizzle<3, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 0.0f, 1.0f) == (v.Swizzle<3, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 0.0f, 2.0f) == (v.Swizzle<3, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 0.0f, 3.0f) == (v.Swizzle<3, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 1.0f, 0.0f) == (v.Swizzle<3, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 1.0f, 1.0f) == (v.Swizzle<3, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 1.0f, 2.0f) == (v.Swizzle<3, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 1.0f, 3.0f) == (v.Swizzle<3, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 2.0f, 0.0f) == (v.Swizzle<3, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 2.0f, 1.0f) == (v.Swizzle<3, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 2.0f, 2.0f) == (v.Swizzle<3, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 2.0f, 3.0f) == (v.Swizzle<3, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 3.0f, 0.0f) == (v.Swizzle<3, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 3.0f, 1.0f) == (v.Swizzle<3, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 3.0f, 2.0f) == (v.Swizzle<3, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4f(3.0f, 3.0f, 3.0f, 3.0f) == (v.Swizzle<3, 3, 3, 3>())).All());
    }
}

TEST(MathTest, Vec4f_ChangeSign_Immediate)
{
    const Vec4f v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE((Vec4f(0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, false, false, false>())).All());
    EXPECT_TRUE((Vec4f(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, false, false, true>())).All());
    EXPECT_TRUE((Vec4f(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, false, true, false>())).All());
    EXPECT_TRUE((Vec4f(0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, false, true, true>())).All());
    EXPECT_TRUE((Vec4f(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<false, true, false, false>())).All());
    EXPECT_TRUE((Vec4f(0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<false, true, false, true>())).All());
    EXPECT_TRUE((Vec4f(0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<false, true, true, false>())).All());
    EXPECT_TRUE((Vec4f(0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<false, true, true, true>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, false, false, false>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, false, false, true>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, false, true, false>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, 1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, false, true, true>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign<true, true, false, false>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, -1.0f, 2.0f, -3.0f) == (v.ChangeSign<true, true, false, true>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, -1.0f, -2.0f, 3.0f) == (v.ChangeSign<true, true, true, false>())).All());
    EXPECT_TRUE((Vec4f(-0.5f, -1.0f, -2.0f, -3.0f) == (v.ChangeSign<true, true, true, true>())).All());
}

TEST(MathTest, Vec4f_ChangeSign)
{
    const Vec4f v(0.5f, 1.0f, 2.0f, 3.0f);

    EXPECT_TRUE((Vec4f(-0.5f, 1.0f, 2.0f, 3.0f) == (v.ChangeSign(Vec4f(1.0f, 0.0f, 0.0f, 0.0f) > Vec4f::Zero()))).All());
    EXPECT_TRUE((Vec4f(0.5f, -1.0f, 2.0f, 3.0f) == (v.ChangeSign(Vec4f(0.0f, 1.0f, 0.0f, 0.0f) > Vec4f::Zero()))).All());
    EXPECT_TRUE((Vec4f(0.5f, 1.0f, -2.0f, 3.0f) == (v.ChangeSign(Vec4f(0.0f, 0.0f, 1.0f, 0.0f) > Vec4f::Zero()))).All());
    EXPECT_TRUE((Vec4f(0.5f, 1.0f, 2.0f, -3.0f) == (v.ChangeSign(Vec4f(0.0f, 0.0f, 0.0f, 1.0f) > Vec4f::Zero()))).All());
}

TEST(MathTest, Vec4f_MakeMask)
{
    const Vec4f v(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE((Vec4f(0.0f, 0.0f, 0.0f, 4.0f) == (v & Vec4f::MakeMask<0,0,0,1>())).All());
    EXPECT_TRUE((Vec4f(0.0f, 0.0f, 3.0f, 0.0f) == (v & Vec4f::MakeMask<0,0,1,0>())).All());
    EXPECT_TRUE((Vec4f(0.0f, 0.0f, 3.0f, 4.0f) == (v & Vec4f::MakeMask<0,0,1,1>())).All());
    EXPECT_TRUE((Vec4f(0.0f, 2.0f, 0.0f, 0.0f) == (v & Vec4f::MakeMask<0,1,0,0>())).All());
    EXPECT_TRUE((Vec4f(0.0f, 2.0f, 0.0f, 4.0f) == (v & Vec4f::MakeMask<0,1,0,1>())).All());
    EXPECT_TRUE((Vec4f(0.0f, 2.0f, 3.0f, 0.0f) == (v & Vec4f::MakeMask<0,1,1,0>())).All());
    EXPECT_TRUE((Vec4f(0.0f, 2.0f, 3.0f, 4.0f) == (v & Vec4f::MakeMask<0,1,1,1>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 0.0f, 0.0f, 0.0f) == (v & Vec4f::MakeMask<1,0,0,0>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 0.0f, 0.0f, 4.0f) == (v & Vec4f::MakeMask<1,0,0,1>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 0.0f, 3.0f, 0.0f) == (v & Vec4f::MakeMask<1,0,1,0>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 0.0f, 3.0f, 4.0f) == (v & Vec4f::MakeMask<1,0,1,1>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 0.0f, 0.0f) == (v & Vec4f::MakeMask<1,1,0,0>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 0.0f, 4.0f) == (v & Vec4f::MakeMask<1,1,0,1>())).All());
    EXPECT_TRUE((Vec4f(1.0f, 2.0f, 3.0f, 0.0f) == (v & Vec4f::MakeMask<1,1,1,0>())).All());
}

TEST(MathTest, Vec4f_Transpose3)
{
    Vec4f a(1.0f, 2.0f, 3.0f, 0.0f);
    Vec4f b(4.0f, 5.0f, 6.0f, 0.0f);
    Vec4f c(7.0f, 8.0f, 9.0f, 0.0f);

    Vec4f::Transpose3(a, b, c);

    EXPECT_TRUE((Vec4f(1.0f, 4.0f, 7.0f, 0.0f) == (a & Vec4f::MakeMask<1,1,1,0>())).All());
    EXPECT_TRUE((Vec4f(2.0f, 5.0f, 8.0f, 0.0f) == (b & Vec4f::MakeMask<1,1,1,0>())).All());
    EXPECT_TRUE((Vec4f(3.0f, 6.0f, 9.0f, 0.0f) == (c & Vec4f::MakeMask<1,1,1,0>())).All());
}