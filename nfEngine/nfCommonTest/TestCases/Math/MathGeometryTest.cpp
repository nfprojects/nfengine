#include "PCH.hpp"
#include "nfCommon/Math/Geometry.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Math/Frustum.hpp"
#include "nfCommon/Math/Sphere.hpp"
#include "nfCommon/Math/Triangle.hpp"

/*
TODO:
* Frustum-Frustum
* Triangle-Triangle
* Point-Sphere
* Point-Box
* Sphere-Triangle
* Box-Triangle
*/

using namespace NFE::Math;


namespace {

const Frustum TEST_FRUSTUM = Frustum::ConstructForPerspective(
    /* camera position */ Vector(),
    /* axes */ Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f),
    /* near and far plane distances */ 1.0f, 10.0f);

} // namespace


TEST(MathGeometry, ClosestPointOnSegment)
{
    Vector p1(-1.0f, 0.0f, 0.0f);
    Vector p2(1.0f, 0.0f, 0.0f);
    float d;
    Vector p;

    d = ClosestPointOnSegment(Vector(-1.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(0.0f, d);
    EXPECT_TRUE(p == Vector(-1.0f, 0.0f, 0.0f));

    d = ClosestPointOnSegment(Vector(-2.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(1.0f, d);
    EXPECT_TRUE(p == Vector(-1.0f, 0.0f, 0.0f));

    d = ClosestPointOnSegment(Vector(1.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(0.0f, d);
    EXPECT_TRUE(p == Vector(1.0f, 0.0f, 0.0f));

    d = ClosestPointOnSegment(Vector(2.0f, 0.0f, 0.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(1.0f, d);
    EXPECT_TRUE(p == Vector(1.0f, 0.0f, 0.0f));

    d = ClosestPointOnSegment(Vector(0.5f, -1.0f, 1.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(sqrtf(2.0f), d);
    EXPECT_TRUE(p == Vector(0.5f, 0.0f, 0.0f));

    d = ClosestPointOnSegment(Vector(-2.0f, 1.0f, 1.0f), p1, p2, p);
    EXPECT_FLOAT_EQ(sqrtf(3.0f), d);
    EXPECT_TRUE(p == Vector(-1.0f, 0.0f, 0.0f));
}

TEST(MathGeometry, BoxBoxIntersection)
{
    Box box(Vector(-1.0f, -2.0f, -3.0f), Vector(1.0f, 2.0f, 3.0f));

    EXPECT_FALSE(Intersect(box, Box(Vector(-3.0f, -1.0f, -1.0f), Vector(-2.0f, 1.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vector(2.0f, -1.0f, -1.0f), Vector(3.0f, 1.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vector(-1.0f, -4.0f, -1.0f), Vector(1.0f, -3.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vector(-1.0f, 3.0f, -1.0f), Vector(1.0f, 4.0f, 1.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vector(-1.0f, -1.0f, -5.0f), Vector(1.0f, 1.0f, -4.0f))));
    EXPECT_FALSE(Intersect(box, Box(Vector(-1.0f, -1.0f, 4.0f), Vector(1.0f, 1.0f, 5.0f))));

    EXPECT_TRUE(Intersect(box, Box(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 1.0f))));
    EXPECT_TRUE(Intersect(box, Box(Vector(-10.0f, -1.0f, -1.0f), Vector(10.0f, 1.0f, 1.0f))));
    EXPECT_TRUE(Intersect(box, Box(Vector(-1.0f, -10.0f, -1.0f), Vector(1.0f, 10.0f, 1.0f))));
    EXPECT_TRUE(Intersect(box, Box(Vector(-1.0f, -1.0f, -10.0f), Vector(1.0f, 10.0f, 10.0f))));
}

TEST(MathGeometry, BoxFrustumIntersection)
{
    // boxes outside TEST_FRUSTUM near planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 0.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, -1.0f, 11.0f), Vector(1.0f, 1.0f, 12.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(6.0f, -1.0f, 4.0f), Vector(7.0f, 1.0f, 5.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-7.0f, -1.0f, 4.0f), Vector(-6.0f, 1.0f, 5.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, 6.0f, 4.0f), Vector(1.0f, 7.0f, 5.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, -7.0f, 4.0f), Vector(1.0f, -6.0f, 5.0f))));

    // boxes intersecting with single TEST_FRUSTUM face
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 2.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, -1.0f, 9.0f), Vector(1.0f, 1.0f, 12.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(4.0f, -1.0f, 4.0f), Vector(7.0f, 1.0f, 5.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(-7.0f, -1.0f, 4.0f), Vector(-4.0f, 1.0f, 5.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, 4.0f, 4.0f), Vector(1.0f, 7.0f, 5.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(-1.0f, -7.0f, 4.0f), Vector(1.0f, -4.0f, 5.0f))));

    // box inside / outside
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(6.0f, 6.0f, 7.0f), Vector(7.0f, 7.0f, 8.0f))));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(-20.0f, -20.0f, -20.0f), Vector(20.0f, 20.0f, 20.0f))));

    // box with a corner
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Box(Vector(7.0f, 7.0f, 8.0f), Vector(11.0f, 11.0f, 11.0f))));

    // box outside, intersecting with multiple planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(11.0f, -20.0f, -20.0f), Vector(12.0f, 20.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-12.0f, -20.0f, -20.0f), Vector(-11.0f, 20.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-20.0f, 11.0f, -20.0f), Vector(20.0f, 12.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-20.0f, -12.0f, -20.0f), Vector(20.0f, -11.0f, 20.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-20.0f, -20.0f, 11.0f), Vector(20.0f, 20.0f, 12.0f))));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Box(Vector(-20.0f, -20.0f, -1.0f), Vector(20.0f, 20.0f, 0.0f))));
}

TEST(MathGeometry, SphereFrustumIntersection)
{
    // spheres outside TEST_FRUSTUM near planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 0.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 11.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(6.0f, 0.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(-6.0f, 0.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 6.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 6.0f, 4.0f), 0.5f)));

    // spheres intersecting with single TEST_FRUSTUM face
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 1.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 10.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(5.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(-5.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 5.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 5.0f, 5.0f), 0.5f)));

    // spheres inside / outside
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 5.0f), 20.0f)));

    // sphere with a corner
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(9.0f, 9.0f, 9.0f), 3.0)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(11.0f, 11.0f, 11.0f), 1.7f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(-11.0f, -11.0f, 11.0f), 1.7f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(2.0f, 2.0f, 0.0f), 1.7f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(-2.0f, -2.0f, 0.0f), 1.6f)));

    // sphere with an edge
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(6.0f, 6.0f, 3.4f), 2.0f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(6.0f, 6.0f, 3.4f), 3.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 11.0f, 10.5f), 1.0f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 11.0f, 10.5f), 1.5f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(2.0f, 0.0f, 0.0f), 1.4f)));
    EXPECT_TRUE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 2.0f, 0.0f), 1.42f)));

    // spheres outside, intersecting with multiple planes
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(1011.0f, 0.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(-1011.0f, 0.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 1011.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, -1011.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, -1000.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(TEST_FRUSTUM, Sphere(Vector(0.0f, 0.0f, 1011.0f), 1000.0f)));
}

TEST(MathGeometry, SphereBoxIntersection)
{
    Box box(Vector(-1.0f, -2.0f, -3.0f), Vector(1.0f, 2.0f, 3.0f));

    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, 0.0f, 0.0f), 0.1f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, 0.0f, 0.0f), 100.0f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(50.0f, 0.0f, 0.0f), 100.0f)));

    EXPECT_FALSE(Intersect(box, Sphere(Vector(0.0f, 0.0f, 4.1f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(0.0f, 0.0f, -4.1f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(0.0f, 3.1f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(0.0f, -3.1f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(2.1f, 0.0f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(-2.1f, 0.0f, 0.0f), 1.0f)));

    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, 0.0f, 4.1f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, 0.0f, -4.1f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, 3.1f, 0.0f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, -3.1f, 0.0f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(2.1f, 0.0f, 0.0f), 1.2f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(-2.1f, 0.0f, 0.0f), 1.2f)));

    // edges
    EXPECT_FALSE(Intersect(box, Sphere(Vector(0.0f, 3.0f, 4.0f), 1.4f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(0.0f, 3.0f, 4.0f), 1.5f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(2.0f, 3.0f, 1.0f), 1.4f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(2.0f, 3.0f, 1.0f), 1.5f)));
    EXPECT_FALSE(Intersect(box, Sphere(Vector(2.0f, -1.0f, 4.0f), 1.4f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(2.0f, -1.0f, 4.0f), 1.5f)));

    // corner
    EXPECT_FALSE(Intersect(box, Sphere(Vector(-2.0f, -3.0f, 4.0f), 1.7f)));
    EXPECT_TRUE(Intersect(box, Sphere(Vector(-2.0f, -3.0f, 4.0f), 1.8f)));
}

TEST(MathGeometry, SphereSphereIntersection)
{
    EXPECT_TRUE(Intersect(Sphere(Vector(0.0f, 0.0f, 0.0f), 2.0f),
                          Sphere(Vector(0.0f, 0.0f, 0.0f), 1.0f)));
    EXPECT_TRUE(Intersect(Sphere(Vector(0.0f, 0.0f, 0.0f), 1.0f),
                          Sphere(Vector(0.0f, 0.0f, 0.0f), 2.0f)));

    EXPECT_FALSE(Intersect(Sphere(Vector(-1.0f, 0.0f, 0.0f), 0.5f),
                           Sphere(Vector(1.0f, 0.0f, 0.0f), 1.0f)));
    EXPECT_FALSE(Intersect(Sphere(Vector(1.0f, 1.0f, 1.0f), 1.5f),
                           Sphere(Vector(-1.0f, -1.0f, -1.0f), 1.5f)));
    EXPECT_TRUE(Intersect(Sphere(Vector(1.0f, 1.0f, 1.0f), 1.75f),
                          Sphere(Vector(-1.0f, -1.0f, -1.0f), 1.75f)));
}

TEST(MathGeometry, FrustumPointIntersection)
{
    /// Z axis
    EXPECT_FALSE(Intersect(Vector(0.0f, 0.0f, -1.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vector(0.0f, 0.0f, 0.5f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(0.0f, 0.0f, 1.5f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(0.0f, 0.0f, 9.5f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vector(0.0f, 0.0f, 10.5f), TEST_FRUSTUM));

    /// X axis
    EXPECT_FALSE(Intersect(Vector(-5.5f, 0.0f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(-4.5f, 0.0f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(4.5f, 0.0f, 5.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vector(5.5f, 0.0f, 5.0f), TEST_FRUSTUM));

    /// Y axis
    EXPECT_FALSE(Intersect(Vector(0.0f, -5.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(0.0f, -4.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(0.0f, 4.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vector(0.0f, 5.5f, 5.0f), TEST_FRUSTUM));

    EXPECT_FALSE(Intersect(Vector(-5.5f, -5.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(-4.5f, -4.5f, 5.0f), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Vector(10.5f, 10.5f, 10.0f), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Vector(9.5f, 9.5f, 10.0f) - VECTOR_EPSILON, TEST_FRUSTUM));
}

TEST(MathGeometry, TriangleFrustumIntersection)
{
    EXPECT_FALSE(Intersect(Triangle(Vector(-1.0f, 0.0f, 0.0f),
                                    Vector(1.0f, 0.0f, 0.0f),
                                    Vector(0.0f, 1.0f, 0.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vector(-1.0f, 0.0f, 11.0f),
                                    Vector(1.0f, 0.0f, 11.0f),
                                    Vector(0.0f, 1.0f, 11.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vector(-2.0f, 0.0f, 0.0f),
                                    Vector(2.0f, 0.0f, 0.0f),
                                    Vector(0.0f, 12.0f, 11.0f)), TEST_FRUSTUM));

    /*
     * TODO: edge cases - there is no straightforward separating plane.
     * Consider improving the intersection function.
     */
    /*
    EXPECT_FALSE(Intersect(Triangle(Vector(6.0f, 6.0f, 5.0f),
                                    Vector(5.0f, 6.0f, 5.0f),
                                    Vector(6.0f, 5.0f, 5.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vector(0.0f, 0.0f, 0.0f),
                                    Vector(3.0f, 0.0f, 2.0f),
                                    Vector(3.0f, 0.0f, 0.0f)), TEST_FRUSTUM));
    EXPECT_FALSE(Intersect(Triangle(Vector(0.0f, 0.0f, 0.0f),
                                    Vector(0.0f, 3.0f, 2.0f),
                                    Vector(0.0f, 3.0f, 0.0f)), TEST_FRUSTUM));
    */

    EXPECT_TRUE(Intersect(Triangle(Vector(-2.0f, 0.0f, 0.0f),
                                   Vector(2.0f, 0.0f, 0.0f),
                                   Vector(0.0f, 0.0f, 2.0f)), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Triangle(Vector(-5.0f, 1.0f, 6.0f),
                                   Vector(-4.0f, -2.0f, 5.0f),
                                   Vector(5.0f, 3.0f, 11.0f)), TEST_FRUSTUM));
    EXPECT_TRUE(Intersect(Triangle(Vector(-20.0f, 1.0f, 15.0f),
                                   Vector(20.0f, -1.0f, 15.0f),
                                   Vector(0.0f, 0.0f, -5.0f)), TEST_FRUSTUM));
}
