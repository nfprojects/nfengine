#include "PCH.hpp"
#include "Engine/Common/Math/Vec16f.hpp"

using namespace NFE;
using namespace NFE::Math;

TEST(MathTest, Vec16f_Constructor0)
{
    const Vec16f v;
}

TEST(MathTest, Vec16f_ConstructorList)
{
    const Vec16f v(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);

    EXPECT_EQ(1.0f, v[0]);
    EXPECT_EQ(2.0f, v[1]);
    EXPECT_EQ(3.0f, v[2]);
    EXPECT_EQ(4.0f, v[3]);
    EXPECT_EQ(5.0f, v[4]);
    EXPECT_EQ(6.0f, v[5]);
    EXPECT_EQ(7.0f, v[6]);
    EXPECT_EQ(8.0f, v[7]);

    EXPECT_EQ(9.0f, v[8]);
    EXPECT_EQ(10.0f, v[9]);
    EXPECT_EQ(11.0f, v[10]);
    EXPECT_EQ(12.0f, v[11]);
    EXPECT_EQ(13.0f, v[12]);
    EXPECT_EQ(14.0f, v[13]);
    EXPECT_EQ(15.0f, v[14]);
    EXPECT_EQ(16.0f, v[15]);
}

TEST(MathTest, Vec16f_ConstructorScalar)
{
    const Vec16f v(7.0f);

    for (uint32 i = 0; i < 16; ++i)
    {
        EXPECT_EQ(7.0f, v[i]);
    }
}

TEST(MathTest, Vec16f_ConstructorHiLow)
{
    const Vec8f vA(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    const Vec8f vB(9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);

    const Vec16f v(vA, vB);

    EXPECT_EQ(1.0f, v[0]);
    EXPECT_EQ(2.0f, v[1]);
    EXPECT_EQ(3.0f, v[2]);
    EXPECT_EQ(4.0f, v[3]);
    EXPECT_EQ(5.0f, v[4]);
    EXPECT_EQ(6.0f, v[5]);
    EXPECT_EQ(7.0f, v[6]);
    EXPECT_EQ(8.0f, v[7]);

    EXPECT_EQ(9.0f, v[8]);
    EXPECT_EQ(10.0f, v[9]);
    EXPECT_EQ(11.0f, v[10]);
    EXPECT_EQ(12.0f, v[11]);
    EXPECT_EQ(13.0f, v[12]);
    EXPECT_EQ(14.0f, v[13]);
    EXPECT_EQ(15.0f, v[14]);
    EXPECT_EQ(16.0f, v[15]);
}

TEST(MathTest, VecBool16_Get)
{
    const VecBool16 vec(
        true, false, false, false, true, true, false, true,
        false, true, true, true, false, false, false, true);

    EXPECT_EQ(true,     vec.Get<0>());
    EXPECT_EQ(false,    vec.Get<1>());
    EXPECT_EQ(false,    vec.Get<2>());
    EXPECT_EQ(false,    vec.Get<3>());
    EXPECT_EQ(true,     vec.Get<4>());
    EXPECT_EQ(true,     vec.Get<5>());
    EXPECT_EQ(false,    vec.Get<6>());
    EXPECT_EQ(true,     vec.Get<7>());

    EXPECT_EQ(false,    vec.Get<8>());
    EXPECT_EQ(true,     vec.Get<9>());
    EXPECT_EQ(true,     vec.Get<10>());
    EXPECT_EQ(true,     vec.Get<11>());
    EXPECT_EQ(false,    vec.Get<12>());
    EXPECT_EQ(false,    vec.Get<13>());
    EXPECT_EQ(false,    vec.Get<14>());
    EXPECT_EQ(true,     vec.Get<15>());
}

/*
TEST(MathTest, Vec16f_Arithmetics)
{
    EXPECT_TRUE((Vec16f(11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f)
        == Vec16f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f) +
        Vec16f(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f)).All());

    EXPECT_TRUE((Vec16f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)
        == Vec16f(11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f) - Vec16f(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f)).All());

    EXPECT_TRUE((Vec16f(10.0f, 40.0f, 90.0f, 160.0f, 250.0f, 360.0f, 490.0f, 640.0f)
        == Vec16f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f) * Vec16f(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f)).All());

    EXPECT_TRUE((Vec16f(2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f)
        == Vec16f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f) * 2.0f).All());

    EXPECT_TRUE((Vec16f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)
        == Vec16f(2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f) / 2.0f).All());

    EXPECT_TRUE((Vec16f(-1.0f, -2.0f, -3.0f, -4.0f, -5.0f, -6.0f, -7.0f, -8.0f)
        == -Vec16f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)).All());
}
*/
