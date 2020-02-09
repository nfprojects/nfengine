#include "PCH.hpp"
#include "nfCommon/Math/Box.hpp"

using namespace NFE::Math;


namespace {

const Box box1(Vector4(1.0f, 2.0f, 3.0f), Vector4(2.0f, 3.0f, 4.0f));
const Box box2(Vector4(1.0f, 2.0f, 3.0f), Vector4(2.0f, 3.0f, 5.0f));
const Box boxEmpty = Box::Empty();
const Box boxPoint(Vector4::Zero(), Vector4::Zero());

} // namespace


TEST(MathBox, Equality)
{
    // TODO
    // should be EXPECT_EQ and EXPECT_NE, but I get some weird
    // "actual parameter with requested alignment of 16 won't be aligned" warning in VS...

    EXPECT_TRUE(box1 == box1);
    EXPECT_TRUE(boxEmpty == boxEmpty);
    EXPECT_TRUE(boxPoint == boxPoint);

    EXPECT_TRUE(box1 != box2);
    EXPECT_TRUE(box2 != box1);

    EXPECT_TRUE(box1 != boxPoint);
    EXPECT_TRUE(boxPoint != box1);

    EXPECT_TRUE(boxEmpty != boxPoint);
    EXPECT_TRUE(boxPoint != boxEmpty);

    EXPECT_TRUE(box1 != boxEmpty);
    EXPECT_TRUE(boxEmpty != box1);
}

TEST(MathBox, Surface)
{
    EXPECT_FLOAT_EQ(6.0f, box1.SurfaceArea());
    EXPECT_FLOAT_EQ(10.0f, box2.SurfaceArea());
    //EXPECT_FLOAT_EQ(0.0f, boxEmpty.SurfaceArea());
    EXPECT_FLOAT_EQ(0.0f, boxPoint.SurfaceArea());
}

TEST(MathBox, Volume)
{
    EXPECT_FLOAT_EQ(1.0f, box1.Volume());
    EXPECT_FLOAT_EQ(2.0f, box2.Volume());
    //EXPECT_FLOAT_EQ(0.0f, boxEmpty.Volume());
    EXPECT_FLOAT_EQ(0.0f, boxPoint.Volume());
}