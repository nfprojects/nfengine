#include "PCH.hpp"
#include "nfCommon/Math/Math.hpp"

using namespace NFE;
using namespace NFE::Math;


TEST(Math, MinMax_Int)
{
    EXPECT_EQ(1, Min(1, 2));
    EXPECT_EQ(1, Min(2, 1));
    EXPECT_EQ(2, Max(1, 2));
    EXPECT_EQ(2, Max(2, 1));

    EXPECT_EQ(1, Min(1, 2, 3));
    EXPECT_EQ(3, Max(1, 2, 3));
    EXPECT_EQ(1, Min(1, 3, 2));
    EXPECT_EQ(3, Max(1, 3, 2));
    EXPECT_EQ(1, Min(3, 1, 2));
    EXPECT_EQ(3, Max(3, 1, 2));
    EXPECT_EQ(1, Min(3, 2, 1));
    EXPECT_EQ(3, Max(3, 2, 1));
    EXPECT_EQ(1, Min(2, 1, 3));
    EXPECT_EQ(3, Max(2, 1, 3));
    EXPECT_EQ(1, Min(2, 3, 1));
    EXPECT_EQ(3, Max(2, 3, 1));
}

TEST(Math, MinMax_Float)
{
    EXPECT_EQ(1.0f, Min(1.0f, 2.0f));
    EXPECT_EQ(1.0f, Min(2.0f, 1.0f));
    EXPECT_EQ(2.0f, Max(1.0f, 2.0f));
    EXPECT_EQ(2.0f, Max(2.0f, 1.0f));

    EXPECT_EQ(1.0f, Min(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(3.0f, Max(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(1.0f, Min(1.0f, 3.0f, 2.0f));
    EXPECT_EQ(3.0f, Max(1.0f, 3.0f, 2.0f));
    EXPECT_EQ(1.0f, Min(3.0f, 1.0f, 2.0f));
    EXPECT_EQ(3.0f, Max(3.0f, 1.0f, 2.0f));
    EXPECT_EQ(1.0f, Min(3.0f, 2.0f, 1.0f));
    EXPECT_EQ(3.0f, Max(3.0f, 2.0f, 1.0f));
    EXPECT_EQ(1.0f, Min(2.0f, 1.0f, 3.0f));
    EXPECT_EQ(3.0f, Max(2.0f, 1.0f, 3.0f));
    EXPECT_EQ(1.0f, Min(2.0f, 3.0f, 1.0f));
    EXPECT_EQ(3.0f, Max(2.0f, 3.0f, 1.0f));
}

TEST(Math, MinMax_Double)
{
    EXPECT_EQ(1.0, Min(1.0, 2.0));
    EXPECT_EQ(1.0, Min(2.0, 1.0));
    EXPECT_EQ(2.0, Max(1.0, 2.0));
    EXPECT_EQ(2.0, Max(2.0, 1.0));

    EXPECT_EQ(1.0, Min(1.0, 2.0, 3.0));
    EXPECT_EQ(3.0, Max(1.0, 2.0, 3.0));
    EXPECT_EQ(1.0, Min(1.0, 3.0, 2.0));
    EXPECT_EQ(3.0, Max(1.0, 3.0, 2.0));
    EXPECT_EQ(1.0, Min(3.0, 1.0, 2.0));
    EXPECT_EQ(3.0, Max(3.0, 1.0, 2.0));
    EXPECT_EQ(1.0, Min(3.0, 2.0, 1.0));
    EXPECT_EQ(3.0, Max(3.0, 2.0, 1.0));
    EXPECT_EQ(1.0, Min(2.0, 1.0, 3.0));
    EXPECT_EQ(3.0, Max(2.0, 1.0, 3.0));
    EXPECT_EQ(1.0, Min(2.0, 3.0, 1.0));
    EXPECT_EQ(3.0, Max(2.0, 3.0, 1.0));
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, Abs_Int)
{
    EXPECT_EQ(0, Abs(0));
    EXPECT_EQ(123, Abs(123));
    EXPECT_EQ(123, Abs(-123));
}

TEST(Math, Abs_Float)
{
    EXPECT_EQ(0.0f, Abs(0.0f));
    EXPECT_EQ(0.0f, Abs(-0.0f));
    EXPECT_EQ(123.0f, Abs(123.0f));
    EXPECT_EQ(123.0f, Abs(-123.0f));
}

TEST(Math, Abs_Double)
{
    EXPECT_EQ(0.0, Abs(0.0));
    EXPECT_EQ(0.0, Abs(-0.0));
    EXPECT_EQ(123.0, Abs(123.0));
    EXPECT_EQ(123.0, Abs(-123.0));
}

//////////////////////////////////////////////////////////////////////////

TEST(Math, CopySign_Float)
{
    EXPECT_EQ(0.0f, CopySign(0.0f, 0.0f));
    EXPECT_EQ(1.0f, CopySign(1.0f, 123.0f));
    EXPECT_EQ(-1.0f, CopySign(1.0f, -123.0f));
    EXPECT_EQ(1.0f, CopySign(-1.0f, 123.0f));
    EXPECT_EQ(-1.0f, CopySign(-1.0f, -123.0f));
}

TEST(Math, CopySign_Double)
{
    EXPECT_EQ(0.0, CopySign(0.0, 0.0));
    EXPECT_EQ(1.0, CopySign(1.0, 123.0));
    EXPECT_EQ(-1.0, CopySign(1.0, -123.0));
    EXPECT_EQ(1.0, CopySign(-1.0, 123.0));
    EXPECT_EQ(-1.0, CopySign(-1.0, -123.0));
}
