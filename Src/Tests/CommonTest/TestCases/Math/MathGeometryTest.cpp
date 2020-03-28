#include "PCH.hpp"
#include "Engine/Common/Math/Geometry.hpp"
#include "Engine/Common/Math/Frustum.hpp"
#include "Engine/Common/Math/Sphere.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Common/Math/SamplingHelpers.hpp"

/*
TODO:
* Frustum-Frustum
* Triangle-Triangle
* Point-Sphere
* Point-Box
* Sphere-Triangle
* Box-Triangle
*/

using namespace NFE;
using namespace NFE::Math;


namespace {

const Frustum TEST_FRUSTUM = Frustum::ConstructForPerspective(
    /* camera position */ Vec4f::Zero(),
    /* axes */ Vec4f(1.0f, 0.0f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f),
    /* near and far plane distances */ 1.0f, 10.0f);

} // namespace


TEST(MathGeometry, BuildOrthonormalBasis)
{
    const uint32 numIterations = 1000;

    Random random;

    for (uint32 i = 0; i < numIterations; ++i)
    {
        const Vec4f x = SamplingHelpers::GetSphere(random.GetVec2f());

        Vec4f u, v;
        BuildOrthonormalBasis(x, u, v);

        SCOPED_TRACE("x=[" + std::to_string(x.x) + ',' + std::to_string(x.y) + ',' + std::to_string(x.z) + ']');
        SCOPED_TRACE("u=[" + std::to_string(u.x) + ',' + std::to_string(u.y) + ',' + std::to_string(u.z) + ']');
        SCOPED_TRACE("v=[" + std::to_string(v.x) + ',' + std::to_string(v.y) + ',' + std::to_string(v.z) + ']');

        EXPECT_TRUE(Vec4f::AlmostEqual(x, Vec4f::Cross3(u, v), 0.00001f));
        EXPECT_TRUE(Vec4f::AlmostEqual(v, Vec4f::Cross3(x, u), 0.00001f));
        EXPECT_TRUE(Vec4f::AlmostEqual(u, Vec4f::Cross3(v, x), 0.00001f));

        EXPECT_NEAR(0.0f, Vec4f::Dot3(u, v), 0.00001f);
        EXPECT_NEAR(0.0f, Vec4f::Dot3(u, x), 0.00001f);
        EXPECT_NEAR(0.0f, Vec4f::Dot3(x, v), 0.00001f);
    }
}

TEST(MathGeometry, ClosestPointOnSegment)
{
    Vec4f p1(-1.0f, 0.0f, 0.0f);
    Vec4f p2(1.0f, 0.0f, 0.0f);
    float d;
    Vec4f p;

    d = ClosestPointOnSegment(Vec4f(-1.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(0.0f, d);
    EXPECT_TRUE((p == Vec4f(-1.0f, 0.0f, 0.0f)).All());

    d = ClosestPointOnSegment(Vec4f(-2.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(1.0f, d);
    EXPECT_TRUE((p == Vec4f(-1.0f, 0.0f, 0.0f)).All());

    d = ClosestPointOnSegment(Vec4f(1.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(0.0f, d);
    EXPECT_TRUE((p == Vec4f(1.0f, 0.0f, 0.0f)).All());

    d = ClosestPointOnSegment(Vec4f(2.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(1.0f, d);
    EXPECT_TRUE((p == Vec4f(1.0f, 0.0f, 0.0f)).All());

    d = ClosestPointOnSegment(Vec4f(0.5f, -1.0f, 1.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(sqrtf(2.0f), d);
    EXPECT_TRUE((p == Vec4f(0.5f, 0.0f, 0.0f)).All());

    d = ClosestPointOnSegment(Vec4f(-2.0f, 1.0f, 1.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(sqrtf(3.0f), d);
    EXPECT_TRUE((p == Vec4f(-1.0f, 0.0f, 0.0f)).All());
}

TEST(MathGeometry, BoxBoxIntersection)
{
    Box box(Vec4f(-1.0f, -2.0f, -3.0f), Vec4f(1.0f, 2.0f, 3.0f));

    EXPECT_FALSE(Intersect(box, Box(Vec4f(-3.0f, -1.0f, -1.0f), Vec4f(-2.0f, 1.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vec4f(2.0f, -1.0f, -1.0f), Vec4f(3.0f, 1.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vec4f(-1.0f, -4.0f, -1.0f), Vec4f(1.0f, -3.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vec4f(-1.0f, 3.0f, -1.0f), Vec4f(1.0f, 4.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vec4f(-1.0f, -1.0f, -5.0f), Vec4f(1.0f, 1.0f, -4.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vec4f(-1.0f, -1.0f, 4.0f), Vec4f(1.0f, 1.0f, 5.0f))));

    EXPECT_TRUE(Intersect(box, Box(Vec4f(-1.0f, -1.0f, -1.0f), Vec4f(1.0f, 1.0f, 1.0f))));
    EXPECT_TRUE(Intersect(box, Box(Vec4f(-10.0f, -1.0f, -1.0f), Vec4f(10.0f, 1.0f, 1.0f))));
    EXPECT_TRUE(Intersect(box, Box(Vec4f(-1.0f, -10.0f, -1.0f), Vec4f(1.0f, 10.0f, 1.0f))));
    EXPECT_TRUE(Intersect(box, Box(Vec4f(-1.0f, -1.0f, -10.0f), Vec4f(1.0f, 10.0f, 10.0f))));
}

TEST(MathGeometry, BoxFrustumIntersection)
{
    // boxes outside TEST_FRUSTUM near planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, -1.0f, -1.0f), Vec4f(1.0f, 1.0f, 0.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, -1.0f, 11.0f), Vec4f(1.0f, 1.0f, 12.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(6.0f, -1.0f, 4.0f), Vec4f(7.0f, 1.0f, 5.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-7.0f, -1.0f, 4.0f), Vec4f(-6.0f, 1.0f, 5.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, 6.0f, 4.0f), Vec4f(1.0f, 7.0f, 5.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, -7.0f, 4.0f), Vec4f(1.0f, -6.0f, 5.0f))));

    // boxes intersecting with single TEST_FRUSTUM face
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, -1.0f, -1.0f), Vec4f(1.0f, 1.0f, 2.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, -1.0f, 9.0f), Vec4f(1.0f, 1.0f, 12.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(4.0f, -1.0f, 4.0f), Vec4f(7.0f, 1.0f, 5.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(-7.0f, -1.0f, 4.0f), Vec4f(-4.0f, 1.0f, 5.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, 4.0f, 4.0f), Vec4f(1.0f, 7.0f, 5.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(-1.0f, -7.0f, 4.0f), Vec4f(1.0f, -4.0f, 5.0f))));

    // box inside / outside
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(6.0f, 6.0f, 7.0f), Vec4f(7.0f, 7.0f, 8.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(-20.0f, -20.0f, -20.0f), Vec4f(20.0f, 20.0f, 20.0f))));

    // box with a corner
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vec4f(7.0f, 7.0f, 8.0f), Vec4f(11.0f, 11.0f, 11.0f))));

    // box outside, intersecting with multiple planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(11.0f, -20.0f, -20.0f), Vec4f(12.0f, 20.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-12.0f, -20.0f, -20.0f), Vec4f(-11.0f, 20.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-20.0f, 11.0f, -20.0f), Vec4f(20.0f, 12.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-20.0f, -12.0f, -20.0f), Vec4f(20.0f, -11.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-20.0f, -20.0f, 11.0f), Vec4f(20.0f, 20.0f, 12.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vec4f(-20.0f, -20.0f, -1.0f), Vec4f(20.0f, 20.0f, 0.0f))));
}

TEST(MathGeometry, SphereFrustumIntersection)
{
    // spheres outside TEST_FRUSTUM near planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 0.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 11.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(6.0f, 0.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(-6.0f, 0.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 6.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 6.0f, 4.0f), 0.5f)));

    // spheres intersecting with single TEST_FRUSTUM face
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 1.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 10.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(5.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(-5.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 5.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 5.0f, 5.0f), 0.5f)));

    // spheres inside / outside
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 5.0f), 20.0f)));

    // sphere with a corner
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(9.0f, 9.0f, 9.0f), 3.0)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(11.0f, 11.0f, 11.0f), 1.7f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(-11.0f, -11.0f, 11.0f), 1.7f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(2.0f, 2.0f, 0.0f), 1.7f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(-2.0f, -2.0f, 0.0f), 1.6f)));

    // sphere with an edge
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(6.0f, 6.0f, 3.4f), 2.0f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(6.0f, 6.0f, 3.4f), 3.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 11.0f, 10.5f), 1.0f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 11.0f, 10.5f), 1.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(2.0f, 0.0f, 0.0f), 1.4f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 2.0f, 0.0f), 1.42f)));

    // spheres outside, intersecting with multiple planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(1011.0f, 0.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(-1011.0f, 0.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 1011.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, -1011.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, -1000.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vec4f(0.0f, 0.0f, 1011.0f), 1000.0f)));
}

TEST(MathGeometry, SphereBoxIntersection)
{
    Box box(Vec4f(-1.0f, -2.0f, -3.0f), Vec4f(1.0f, 2.0f, 3.0f));

    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, 0.0f, 0.0f), 0.1f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, 0.0f, 0.0f), 100.0f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(50.0f, 0.0f, 0.0f), 100.0f)));

    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(0.0f, 0.0f, 4.1f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(0.0f, 0.0f, -4.1f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(0.0f, 3.1f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(0.0f, -3.1f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(2.1f, 0.0f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(-2.1f, 0.0f, 0.0f), 1.0f)));

    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, 0.0f, 4.1f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, 0.0f, -4.1f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, 3.1f, 0.0f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, -3.1f, 0.0f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(2.1f, 0.0f, 0.0f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(-2.1f, 0.0f, 0.0f), 1.2f)));

    // edges
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(0.0f, 3.0f, 4.0f), 1.4f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(0.0f, 3.0f, 4.0f), 1.5f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(2.0f, 3.0f, 1.0f), 1.4f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(2.0f, 3.0f, 1.0f), 1.5f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(2.0f, -1.0f, 4.0f), 1.4f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(2.0f, -1.0f, 4.0f), 1.5f)));

    // corner
    EXPECT_FALSE(Intersect(box, Sphere(Vec4f(-2.0f, -3.0f, 4.0f), 1.7f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vec4f(-2.0f, -3.0f, 4.0f), 1.8f)));
}

TEST(MathGeometry, SphereSphereIntersection)
{
    EXPECT_TRUE(Intersect(Sphere(Vec4f(0.0f, 0.0f, 0.0f), 2.0f),
                          Sphere(Vec4f(0.0f, 0.0f, 0.0f), 1.0f)));
    EXPECT_TRUE(Intersect(Sphere(Vec4f(0.0f, 0.0f, 0.0f), 1.0f),
                          Sphere(Vec4f(0.0f, 0.0f, 0.0f), 2.0f)));

    EXPECT_FALSE(Intersect(Sphere(Vec4f(-1.0f, 0.0f, 0.0f), 0.5f),
                           Sphere(Vec4f(1.0f, 0.0f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(Sphere(Vec4f(1.0f, 1.0f, 1.0f), 1.5f),
                           Sphere(Vec4f(-1.0f, -1.0f, -1.0f), 1.5f)));
    EXPECT_TRUE(Intersect(Sphere(Vec4f(1.0f, 1.0f, 1.0f), 1.75f),
                          Sphere(Vec4f(-1.0f, -1.0f, -1.0f), 1.75f)));
}

TEST(MathGeometry, FrustumPointIntersection)
{
    /// Z axis
    EXPECT_FALSE(Intersect(Vec4f(0.0f, 0.0f, -1.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vec4f(0.0f, 0.0f, 0.5f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(0.0f, 0.0f, 1.5f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(0.0f, 0.0f, 9.5f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vec4f(0.0f, 0.0f, 10.5f), TEST_FRUSTUM));

    /// X axis
    EXPECT_FALSE(Intersect(Vec4f(-5.5f, 0.0f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(-4.5f, 0.0f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(4.5f, 0.0f, 5.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vec4f(5.5f, 0.0f, 5.0f), TEST_FRUSTUM));

    /// Y axis
    EXPECT_FALSE(Intersect(Vec4f(0.0f, -5.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(0.0f, -4.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(0.0f, 4.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vec4f(0.0f, 5.5f, 5.0f), TEST_FRUSTUM));

    EXPECT_FALSE(Intersect(Vec4f(-5.5f, -5.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(-4.5f, -4.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vec4f(10.5f, 10.5f, 10.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vec4f(9.5f, 9.5f, 10.0f) - VECTOR_EPSILON, TEST_FRUSTUM));
}

TEST(MathGeometry, TriangleFrustumIntersection)
{
    EXPECT_FALSE(Intersect(Triangle(Vec4f(-1.0f, 0.0f, 0.0f),
                                    Vec4f(1.0f, 0.0f, 0.0f),
                                    Vec4f(0.0f, 1.0f, 0.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vec4f(-1.0f, 0.0f, 11.0f),
                                    Vec4f(1.0f, 0.0f, 11.0f),
                                    Vec4f(0.0f, 1.0f, 11.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vec4f(-2.0f, 0.0f, 0.0f),
                                    Vec4f(2.0f, 0.0f, 0.0f),
                                    Vec4f(0.0f, 12.0f, 11.0f)), TEST_FRUSTUM));

    /*
     * TODO: edge cases - there is no straightforward separating plane.
     * Consider improving the intersection function.
     */
    /*
    EXPECT_FALSE(Intersect(Triangle(Vec4f(6.0f, 6.0f, 5.0f),
                                    Vec4f(5.0f, 6.0f, 5.0f),
                                    Vec4f(6.0f, 5.0f, 5.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vec4f(0.0f, 0.0f, 0.0f),
                                    Vec4f(3.0f, 0.0f, 2.0f),
                                    Vec4f(3.0f, 0.0f, 0.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vec4f(0.0f, 0.0f, 0.0f),
                                    Vec4f(0.0f, 3.0f, 2.0f),
                                    Vec4f(0.0f, 3.0f, 0.0f)), TEST_FRUSTUM));
    */

    EXPECT_TRUE(Intersect(Triangle(Vec4f(-2.0f, 0.0f, 0.0f),
                                   Vec4f(2.0f, 0.0f, 0.0f),
                                   Vec4f(0.0f, 0.0f, 2.0f)), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Triangle(Vec4f(-5.0f, 1.0f, 6.0f),
                                   Vec4f(-4.0f, -2.0f, 5.0f),
                                   Vec4f(5.0f, 3.0f, 11.0f)), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Triangle(Vec4f(-20.0f, 1.0f, 15.0f),
                                   Vec4f(20.0f, -1.0f, 15.0f),
                                   Vec4f(0.0f, 0.0f, -5.0f)), TEST_FRUSTUM));
}
