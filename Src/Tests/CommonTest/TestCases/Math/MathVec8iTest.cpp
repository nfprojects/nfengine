#include "PCH.hpp"
#include "Engine/Common/Math/Vec8i.hpp"

using namespace NFE;
using namespace NFE::Math;

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Vec8i_ConstructorFromValues)
{
    const Vec8i v(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
    EXPECT_EQ(4, v[3]);
    EXPECT_EQ(5, v[4]);
    EXPECT_EQ(6, v[5]);
    EXPECT_EQ(7, v[6]);
    EXPECT_EQ(8, v[7]);
}

TEST(MathTest, Vec8i_ConstructFromScalar)
{
    const Vec8i v(42);
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(42, v[1]);
    EXPECT_EQ(42, v[2]);
    EXPECT_EQ(42, v[3]);
    EXPECT_EQ(42, v[4]);
    EXPECT_EQ(42, v[5]);
    EXPECT_EQ(42, v[6]);
    EXPECT_EQ(42, v[7]);
}

TEST(MathTest, Vec8i_ConstructFromLoHi)
{
    const Vec4i lo(1, 2, 3, 4);
    const Vec4i hi(5, 6, 7, 8);

    const Vec8i v(lo, hi);
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
    EXPECT_EQ(4, v[3]);
    EXPECT_EQ(5, v[4]);
    EXPECT_EQ(6, v[5]);
    EXPECT_EQ(7, v[6]);
    EXPECT_EQ(8, v[7]);
}

TEST(MathTest, Vec8i_Zero)
{
    const Vec8i v = Vec8i::Zero();
    EXPECT_EQ(0, v[0]);
    EXPECT_EQ(0, v[1]);
    EXPECT_EQ(0, v[2]);
    EXPECT_EQ(0, v[3]);
    EXPECT_EQ(0, v[4]);
    EXPECT_EQ(0, v[5]);
    EXPECT_EQ(0, v[6]);
    EXPECT_EQ(0, v[7]);
}

TEST(MathTest, VecBool8i_All)
{
    EXPECT_TRUE(VecBool8i(true, true, true, true, true, true, true, true).All());
    EXPECT_FALSE(VecBool8i(false, true, true, true, true, true, true, true).All());
    EXPECT_FALSE(VecBool8i(true, false, true, true, true, true, true, true).All());
    EXPECT_FALSE(VecBool8i(true, true, false, true, true, true, true, true).All());
    EXPECT_FALSE(VecBool8i(true, true, true, false, true, true, true, true).All());
    EXPECT_FALSE(VecBool8i(true, true, true, true, false, true, true, true).All());
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, false, true, true).All());
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, true, false, true).All());
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, true, true, false).All());
    EXPECT_FALSE(VecBool8i(false, false, false, false, false, false, false, false).All());
}

TEST(MathTest, VecBool8i_None)
{
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, true, true, true).None());
    EXPECT_FALSE(VecBool8i(false, true, true, true, true, true, true, true).None());
    EXPECT_FALSE(VecBool8i(true, false, true, true, true, true, true, true).None());
    EXPECT_FALSE(VecBool8i(true, true, false, true, true, true, true, true).None());
    EXPECT_FALSE(VecBool8i(true, true, true, false, true, true, true, true).None());
    EXPECT_FALSE(VecBool8i(true, true, true, true, false, true, true, true).None());
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, false, true, true).None());
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, true, false, true).None());
    EXPECT_FALSE(VecBool8i(true, true, true, true, true, true, true, false).None());
    EXPECT_TRUE(VecBool8i(false, false, false, false, false, false, false, false).None());
}

TEST(MathTest, VecBool8i_Any)
{
    EXPECT_TRUE(VecBool8i(true, true, true, true, true, true, true, true).Any());
    EXPECT_TRUE(VecBool8i(false, true, true, true, true, true, true, true).Any());
    EXPECT_TRUE(VecBool8i(true, false, true, true, true, true, true, true).Any());
    EXPECT_TRUE(VecBool8i(true, true, false, true, true, true, true, true).Any());
    EXPECT_TRUE(VecBool8i(true, true, true, false, true, true, true, true).Any());
    EXPECT_TRUE(VecBool8i(true, true, true, true, false, true, true, true).Any());
    EXPECT_TRUE(VecBool8i(true, true, true, true, true, false, true, true).Any());
    EXPECT_TRUE(VecBool8i(true, true, true, true, true, true, false, true).Any());
    EXPECT_TRUE(VecBool8i(true, true, true, true, true, true, true, false).Any());
    EXPECT_FALSE(VecBool8i(false, false, false, false, false, false, false, false).Any());
}

TEST(MathTest, Vec8i_Equal)
{
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)).All());
    EXPECT_TRUE((Vec8i(10, 22, 33, 44, 55, 66, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, true, true, true, true, true, true, true));
    EXPECT_TRUE((Vec8i(11, 20, 33, 44, 55, 66, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, false, true, true, true, true, true, true));
    EXPECT_TRUE((Vec8i(11, 22, 30, 44, 55, 66, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, true, false, true, true, true, true, true));
    EXPECT_TRUE((Vec8i(11, 22, 33, 40, 55, 66, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, true, true, false, true, true, true, true));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 50, 66, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, true, true, true, false, true, true, true));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 60, 77, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, true, true, true, true, false, true, true));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 70, 88) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, true, true, true, true, true, false, true));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 77, 80) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, true, true, true, true, true, true, false));
}

TEST(MathTest, Vec8i_NotEqual)
{
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)).None());
    EXPECT_TRUE((Vec8i(10, 22, 33, 44, 55, 66, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(true, false, false, false, false, false, false, false));
    EXPECT_TRUE((Vec8i(11, 20, 33, 44, 55, 66, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, true, false, false, false, false, false, false));
    EXPECT_TRUE((Vec8i(11, 22, 30, 44, 55, 66, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, false, true, false, false, false, false, false));
    EXPECT_TRUE((Vec8i(11, 22, 33, 40, 55, 66, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, false, false, true, false, false, false, false));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 50, 66, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, false, false, false, true, false, false, false));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 60, 77, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, false, false, false, false, true, false, false));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 70, 88) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, false, false, false, false, false, true, false));
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 77, 80) != Vec8i(11, 22, 33, 44, 55, 66, 77, 88)) == VecBool8i(false, false, false, false, false, false, false, true));
}

TEST(MathTest, Vec8i_VectorArithmetics)
{
    EXPECT_TRUE((Vec8i(11, 22, 33, 44, 55, 66, 77, 88) == Vec8i(1, 2, 3, 4, 5, 6, 7, 8) + Vec8i(10, 20, 30, 40, 50, 60, 70, 80)).All());
    EXPECT_TRUE((Vec8i(11, 12, 13, 14, 15, 16, 17, 18) == Vec8i(1, 2, 3, 4, 5, 6, 7, 8) + 10).All());
    EXPECT_TRUE((Vec8i(1, 2, 3, 4, 5, 6, 7, 8) == Vec8i(11, 22, 33, 44, 55, 66, 77, 88) - Vec8i(10, 20, 30, 40, 50, 60, 70, 80)).All());
    EXPECT_TRUE((Vec8i(10, 40, 90, 160, 250, 360, 490, 640) == Vec8i(1, 2, 3, 4, 5, 6, 7, 8) * Vec8i(10, 20, 30, 40, 50, 60, 70, 80)).All());
    EXPECT_TRUE((Vec8i(2, 4, 6, 8, 10, 12, 14, 16) == Vec8i(1, 2, 3, 4, 5, 6, 7, 8) * 2).All());
    EXPECT_TRUE((Vec8i(-1, -2, -3, -4, -5, -6, -7, -8) == -Vec8i(1, 2, 3, 4, 5, 6, 7, 8)).All());
}
