#include "PCH.hpp"
#include "Engine/Common/Math/Vec4i.hpp"

using namespace NFE;
using namespace NFE::Math;

namespace {

static const Vec4i vecA = Vec4i(1, 1, 1, 1);
static const Vec4i vecB = Vec4i(1, 2, 3, 4);
static const Vec4i vecC = Vec4i(2, 3, 4, 5);
static const Vec4i vecD = Vec4i(1, 4, 9, 16);
static const Vec4i vecE = Vec4i(4, 3, 2, 1);

} // namespace

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4i_Constructor1)
{
    const Vec4i v(1, 2, 3, 4);
    EXPECT_EQ(1, v.x);
    EXPECT_EQ(2, v.y);
    EXPECT_EQ(3, v.z);
    EXPECT_EQ(4, v.w);
}

TEST(MathTest, Vec4i_Constructor2)
{
    const Vec4i v(7);
    EXPECT_EQ(7, v.x);
    EXPECT_EQ(7, v.y);
    EXPECT_EQ(7, v.z);
    EXPECT_EQ(7, v.w);
}

TEST(MathTest, Vec4i_Zero)
{
    const Vec4i v = Vec4i::Zero();
    EXPECT_EQ(0, v.x);
    EXPECT_EQ(0, v.y);
    EXPECT_EQ(0, v.z);
    EXPECT_EQ(0, v.w);
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4i_VectorArithmetics)
{
    EXPECT_TRUE((vecC == (vecA + vecB)).All());
    EXPECT_TRUE((Vec4i(0, -1, -2, -3) == (vecA - vecB)).All());
    EXPECT_TRUE((vecD == (vecB * vecB)).All());
    EXPECT_TRUE((Vec4i(2, 4, 6, 8) == (vecB * 2)).All());
    EXPECT_TRUE((Vec4i(-1, -2, -3, -4) == -Vec4i(1, 2, 3, 4)).All());
}

TEST(MathTest, Vec4i_ShiftLeftSharedCount)
{
    for (uint32 i = 0; i < 32; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        Vec4i v(0, 1, 123, INT32_MAX);
        const Vec4i expected(0, 1 << i, 123 << i, INT32_MAX << i);

        EXPECT_TRUE((expected == (v << i)).All());
        v <<= i;
        EXPECT_TRUE((expected == v).All());
    }
}

TEST(MathTest, Vec4i_ShiftRightSharedCount)
{
    for (uint32 i = 0; i < 32; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        Vec4i v(0, 1, 123456789, INT32_MAX);
        const Vec4i expected(0, 1 >> i, 123456789 >> i, INT32_MAX >> i);

        EXPECT_TRUE((expected == (v >> i)).All());
        v >>= i;
        EXPECT_TRUE((expected == v).All());
    }
}

TEST(MathTest, Vec4i_ShiftLeftGeneric)
{
    Vec4i v(1, 1, 1, 1);
    const Vec4i count(0, 1, 2, 3);
    const Vec4i expected(1 << 0, 1 << 1, 1 << 2, 1 << 3);

    EXPECT_TRUE((expected == (v << count)).All());
    v <<= count;
    EXPECT_TRUE((expected == v).All());
}

TEST(MathTest, Vec4i_VectorMinMax)
{
    EXPECT_TRUE((Vec4i::Min(vecB, vecE) == Vec4i(1, 2, 2, 1)).All());
    EXPECT_TRUE((Vec4i::Min(vecE, vecB) == Vec4i(1, 2, 2, 1)).All());
    EXPECT_TRUE((Vec4i::Max(vecB, vecE) == Vec4i(4, 3, 3, 4)).All());
    EXPECT_TRUE((Vec4i::Max(vecB, vecE) == Vec4i(4, 3, 3, 4)).All());
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4i_VectorLess)
{
    EXPECT_EQ(VecBool4i(0, 0, 0, 0), Vec4i(2, 3, 4, 5) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(1, 2, 3, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(9, 2, 3, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(1, 9, 3, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(1, 2, 9, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(1, 2, 3, 9) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(2, 2, 3, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(1, 3, 3, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(1, 2, 4, 4) < Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(1, 2, 3, 5) < Vec4i(2, 3, 4, 5));
}

TEST(MathTest, Vec4i_VectorLessOrEqual)
{
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(2, 3, 4, 5) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(1, 2, 3, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(9, 2, 3, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(1, 9, 3, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(1, 2, 9, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(1, 2, 3, 9) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(2, 2, 3, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(1, 3, 3, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(1, 2, 4, 4) <= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(1, 2, 3, 5) <= Vec4i(2, 3, 4, 5));
}

TEST(MathTest, Vec4i_VectorGreater)
{
    EXPECT_EQ(VecBool4i(0, 0, 0, 0), Vec4i(2, 3, 4, 5) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(3, 4, 5, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(0, 4, 5, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(3, 0, 5, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(3, 4, 0, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(3, 4, 5, 0) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(2, 4, 5, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(3, 3, 5, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(3, 4, 4, 6) > Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(3, 4, 5, 5) > Vec4i(2, 3, 4, 5));
}

TEST(MathTest, Vec4i_VectorGreaterOrEqual)
{
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(2, 3, 4, 5) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(3, 4, 5, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(0, 4, 5, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(3, 0, 5, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(3, 4, 0, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(3, 4, 5, 0) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(2, 4, 5, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(3, 3, 5, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(3, 4, 4, 6) >= Vec4i(2, 3, 4, 5));
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(3, 4, 5, 5) >= Vec4i(2, 3, 4, 5));
}

TEST(MathTest, Vec4i_VectorEqual)
{
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(1, 2, 3, 4) == Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(9, 2, 3, 4) == Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(1, 9, 3, 4) == Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(1, 2, 9, 4) == Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(1, 2, 3, 9) == Vec4i(1, 2, 3, 4));
}

TEST(MathTest, Vec4i_VectorNotEqual)
{
    EXPECT_EQ(VecBool4i(1, 1, 1, 1), Vec4i(4, 3, 2, 1) != Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(0, 1, 1, 1), Vec4i(1, 3, 2, 1) != Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(1, 0, 1, 1), Vec4i(4, 2, 2, 1) != Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(1, 1, 0, 1), Vec4i(4, 3, 3, 1) != Vec4i(1, 2, 3, 4));
    EXPECT_EQ(VecBool4i(1, 1, 1, 0), Vec4i(4, 3, 2, 4) != Vec4i(1, 2, 3, 4));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec4i_VectorSwizzle)
{
    const Vec4i v(0, 1, 2, 3);

    {
        EXPECT_TRUE((Vec4i(0, 0, 0, 0) == (v.Swizzle<0, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 0, 1) == (v.Swizzle<0, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 0, 2) == (v.Swizzle<0, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 0, 3) == (v.Swizzle<0, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 1, 0) == (v.Swizzle<0, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 1, 1) == (v.Swizzle<0, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 1, 2) == (v.Swizzle<0, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 1, 3) == (v.Swizzle<0, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 2, 0) == (v.Swizzle<0, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 2, 1) == (v.Swizzle<0, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 2, 2) == (v.Swizzle<0, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 2, 3) == (v.Swizzle<0, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 3, 0) == (v.Swizzle<0, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 3, 1) == (v.Swizzle<0, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 3, 2) == (v.Swizzle<0, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 0, 3, 3) == (v.Swizzle<0, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(0, 1, 0, 0) == (v.Swizzle<0, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 0, 1) == (v.Swizzle<0, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 0, 2) == (v.Swizzle<0, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 0, 3) == (v.Swizzle<0, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 1, 0) == (v.Swizzle<0, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 1, 1) == (v.Swizzle<0, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 1, 2) == (v.Swizzle<0, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 1, 3) == (v.Swizzle<0, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 2, 0) == (v.Swizzle<0, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 2, 1) == (v.Swizzle<0, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 2, 2) == (v.Swizzle<0, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 2, 3) == (v.Swizzle<0, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 3, 0) == (v.Swizzle<0, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 3, 1) == (v.Swizzle<0, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 3, 2) == (v.Swizzle<0, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 1, 3, 3) == (v.Swizzle<0, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(0, 2, 0, 0) == (v.Swizzle<0, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 0, 1) == (v.Swizzle<0, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 0, 2) == (v.Swizzle<0, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 0, 3) == (v.Swizzle<0, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 1, 0) == (v.Swizzle<0, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 1, 1) == (v.Swizzle<0, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 1, 2) == (v.Swizzle<0, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 1, 3) == (v.Swizzle<0, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 2, 0) == (v.Swizzle<0, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 2, 1) == (v.Swizzle<0, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 2, 2) == (v.Swizzle<0, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 2, 3) == (v.Swizzle<0, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 3, 0) == (v.Swizzle<0, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 3, 1) == (v.Swizzle<0, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 3, 2) == (v.Swizzle<0, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 2, 3, 3) == (v.Swizzle<0, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(0, 3, 0, 0) == (v.Swizzle<0, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 0, 1) == (v.Swizzle<0, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 0, 2) == (v.Swizzle<0, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 0, 3) == (v.Swizzle<0, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 1, 0) == (v.Swizzle<0, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 1, 1) == (v.Swizzle<0, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 1, 2) == (v.Swizzle<0, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 1, 3) == (v.Swizzle<0, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 2, 0) == (v.Swizzle<0, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 2, 1) == (v.Swizzle<0, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 2, 2) == (v.Swizzle<0, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 2, 3) == (v.Swizzle<0, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 3, 0) == (v.Swizzle<0, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 3, 1) == (v.Swizzle<0, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 3, 2) == (v.Swizzle<0, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(0, 3, 3, 3) == (v.Swizzle<0, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vec4i(1, 0, 0, 0) == (v.Swizzle<1, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 0, 1) == (v.Swizzle<1, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 0, 2) == (v.Swizzle<1, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 0, 3) == (v.Swizzle<1, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 1, 0) == (v.Swizzle<1, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 1, 1) == (v.Swizzle<1, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 1, 2) == (v.Swizzle<1, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 1, 3) == (v.Swizzle<1, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 2, 0) == (v.Swizzle<1, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 2, 1) == (v.Swizzle<1, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 2, 2) == (v.Swizzle<1, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 2, 3) == (v.Swizzle<1, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 3, 0) == (v.Swizzle<1, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 3, 1) == (v.Swizzle<1, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 3, 2) == (v.Swizzle<1, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 0, 3, 3) == (v.Swizzle<1, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(1, 1, 0, 0) == (v.Swizzle<1, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 0, 1) == (v.Swizzle<1, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 0, 2) == (v.Swizzle<1, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 0, 3) == (v.Swizzle<1, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 1, 0) == (v.Swizzle<1, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 1, 1) == (v.Swizzle<1, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 1, 2) == (v.Swizzle<1, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 1, 3) == (v.Swizzle<1, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 2, 0) == (v.Swizzle<1, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 2, 1) == (v.Swizzle<1, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 2, 2) == (v.Swizzle<1, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 2, 3) == (v.Swizzle<1, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 3, 0) == (v.Swizzle<1, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 3, 1) == (v.Swizzle<1, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 3, 2) == (v.Swizzle<1, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 1, 3, 3) == (v.Swizzle<1, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(1, 2, 0, 0) == (v.Swizzle<1, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 0, 1) == (v.Swizzle<1, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 0, 2) == (v.Swizzle<1, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 0, 3) == (v.Swizzle<1, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 1, 0) == (v.Swizzle<1, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 1, 1) == (v.Swizzle<1, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 1, 2) == (v.Swizzle<1, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 1, 3) == (v.Swizzle<1, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 2, 0) == (v.Swizzle<1, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 2, 1) == (v.Swizzle<1, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 2, 2) == (v.Swizzle<1, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 2, 3) == (v.Swizzle<1, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 3, 0) == (v.Swizzle<1, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 3, 1) == (v.Swizzle<1, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 3, 2) == (v.Swizzle<1, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 2, 3, 3) == (v.Swizzle<1, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(1, 3, 0, 0) == (v.Swizzle<1, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 0, 1) == (v.Swizzle<1, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 0, 2) == (v.Swizzle<1, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 0, 3) == (v.Swizzle<1, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 1, 0) == (v.Swizzle<1, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 1, 1) == (v.Swizzle<1, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 1, 2) == (v.Swizzle<1, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 1, 3) == (v.Swizzle<1, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 2, 0) == (v.Swizzle<1, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 2, 1) == (v.Swizzle<1, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 2, 2) == (v.Swizzle<1, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 2, 3) == (v.Swizzle<1, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 3, 0) == (v.Swizzle<1, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 3, 1) == (v.Swizzle<1, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 3, 2) == (v.Swizzle<1, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(1, 3, 3, 3) == (v.Swizzle<1, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vec4i(2, 0, 0, 0) == (v.Swizzle<2, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 0, 1) == (v.Swizzle<2, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 0, 2) == (v.Swizzle<2, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 0, 3) == (v.Swizzle<2, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 1, 0) == (v.Swizzle<2, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 1, 1) == (v.Swizzle<2, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 1, 2) == (v.Swizzle<2, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 1, 3) == (v.Swizzle<2, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 2, 0) == (v.Swizzle<2, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 2, 1) == (v.Swizzle<2, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 2, 2) == (v.Swizzle<2, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 2, 3) == (v.Swizzle<2, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 3, 0) == (v.Swizzle<2, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 3, 1) == (v.Swizzle<2, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 3, 2) == (v.Swizzle<2, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 0, 3, 3) == (v.Swizzle<2, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(2, 1, 0, 0) == (v.Swizzle<2, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 0, 1) == (v.Swizzle<2, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 0, 2) == (v.Swizzle<2, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 0, 3) == (v.Swizzle<2, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 1, 0) == (v.Swizzle<2, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 1, 1) == (v.Swizzle<2, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 1, 2) == (v.Swizzle<2, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 1, 3) == (v.Swizzle<2, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 2, 0) == (v.Swizzle<2, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 2, 1) == (v.Swizzle<2, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 2, 2) == (v.Swizzle<2, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 2, 3) == (v.Swizzle<2, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 3, 0) == (v.Swizzle<2, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 3, 1) == (v.Swizzle<2, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 3, 2) == (v.Swizzle<2, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 1, 3, 3) == (v.Swizzle<2, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(2, 2, 0, 0) == (v.Swizzle<2, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 0, 1) == (v.Swizzle<2, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 0, 2) == (v.Swizzle<2, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 0, 3) == (v.Swizzle<2, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 1, 0) == (v.Swizzle<2, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 1, 1) == (v.Swizzle<2, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 1, 2) == (v.Swizzle<2, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 1, 3) == (v.Swizzle<2, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 2, 0) == (v.Swizzle<2, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 2, 1) == (v.Swizzle<2, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 2, 2) == (v.Swizzle<2, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 2, 3) == (v.Swizzle<2, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 3, 0) == (v.Swizzle<2, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 3, 1) == (v.Swizzle<2, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 3, 2) == (v.Swizzle<2, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 2, 3, 3) == (v.Swizzle<2, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(2, 3, 0, 0) == (v.Swizzle<2, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 0, 1) == (v.Swizzle<2, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 0, 2) == (v.Swizzle<2, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 0, 3) == (v.Swizzle<2, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 1, 0) == (v.Swizzle<2, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 1, 1) == (v.Swizzle<2, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 1, 2) == (v.Swizzle<2, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 1, 3) == (v.Swizzle<2, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 2, 0) == (v.Swizzle<2, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 2, 1) == (v.Swizzle<2, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 2, 2) == (v.Swizzle<2, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 2, 3) == (v.Swizzle<2, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 3, 0) == (v.Swizzle<2, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 3, 1) == (v.Swizzle<2, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 3, 2) == (v.Swizzle<2, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(2, 3, 3, 3) == (v.Swizzle<2, 3, 3, 3>())).All());
    }

    {
        EXPECT_TRUE((Vec4i(3, 0, 0, 0) == (v.Swizzle<3, 0, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 0, 1) == (v.Swizzle<3, 0, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 0, 2) == (v.Swizzle<3, 0, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 0, 3) == (v.Swizzle<3, 0, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 1, 0) == (v.Swizzle<3, 0, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 1, 1) == (v.Swizzle<3, 0, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 1, 2) == (v.Swizzle<3, 0, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 1, 3) == (v.Swizzle<3, 0, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 2, 0) == (v.Swizzle<3, 0, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 2, 1) == (v.Swizzle<3, 0, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 2, 2) == (v.Swizzle<3, 0, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 2, 3) == (v.Swizzle<3, 0, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 3, 0) == (v.Swizzle<3, 0, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 3, 1) == (v.Swizzle<3, 0, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 3, 2) == (v.Swizzle<3, 0, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 0, 3, 3) == (v.Swizzle<3, 0, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(3, 1, 0, 0) == (v.Swizzle<3, 1, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 0, 1) == (v.Swizzle<3, 1, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 0, 2) == (v.Swizzle<3, 1, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 0, 3) == (v.Swizzle<3, 1, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 1, 0) == (v.Swizzle<3, 1, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 1, 1) == (v.Swizzle<3, 1, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 1, 2) == (v.Swizzle<3, 1, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 1, 3) == (v.Swizzle<3, 1, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 2, 0) == (v.Swizzle<3, 1, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 2, 1) == (v.Swizzle<3, 1, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 2, 2) == (v.Swizzle<3, 1, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 2, 3) == (v.Swizzle<3, 1, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 3, 0) == (v.Swizzle<3, 1, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 3, 1) == (v.Swizzle<3, 1, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 3, 2) == (v.Swizzle<3, 1, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 1, 3, 3) == (v.Swizzle<3, 1, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(3, 2, 0, 0) == (v.Swizzle<3, 2, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 0, 1) == (v.Swizzle<3, 2, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 0, 2) == (v.Swizzle<3, 2, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 0, 3) == (v.Swizzle<3, 2, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 1, 0) == (v.Swizzle<3, 2, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 1, 1) == (v.Swizzle<3, 2, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 1, 2) == (v.Swizzle<3, 2, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 1, 3) == (v.Swizzle<3, 2, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 2, 0) == (v.Swizzle<3, 2, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 2, 1) == (v.Swizzle<3, 2, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 2, 2) == (v.Swizzle<3, 2, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 2, 3) == (v.Swizzle<3, 2, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 3, 0) == (v.Swizzle<3, 2, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 3, 1) == (v.Swizzle<3, 2, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 3, 2) == (v.Swizzle<3, 2, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 2, 3, 3) == (v.Swizzle<3, 2, 3, 3>())).All());

        EXPECT_TRUE((Vec4i(3, 3, 0, 0) == (v.Swizzle<3, 3, 0, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 0, 1) == (v.Swizzle<3, 3, 0, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 0, 2) == (v.Swizzle<3, 3, 0, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 0, 3) == (v.Swizzle<3, 3, 0, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 1, 0) == (v.Swizzle<3, 3, 1, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 1, 1) == (v.Swizzle<3, 3, 1, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 1, 2) == (v.Swizzle<3, 3, 1, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 1, 3) == (v.Swizzle<3, 3, 1, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 2, 0) == (v.Swizzle<3, 3, 2, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 2, 1) == (v.Swizzle<3, 3, 2, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 2, 2) == (v.Swizzle<3, 3, 2, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 2, 3) == (v.Swizzle<3, 3, 2, 3>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 3, 0) == (v.Swizzle<3, 3, 3, 0>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 3, 1) == (v.Swizzle<3, 3, 3, 1>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 3, 2) == (v.Swizzle<3, 3, 3, 2>())).All());
        EXPECT_TRUE((Vec4i(3, 3, 3, 3) == (v.Swizzle<3, 3, 3, 3>())).All());
    }
}
