#include "PCH.hpp"
#include "../nfCommon/Math/Geometry.hpp"
#include "../nfCommon/Math/Box.hpp"
#include "../nfCommon/Math/Frustum.hpp"
#include "../nfCommon/Math/Sphere.hpp"
#include "../nfCommon/Math/Triangle.hpp"
#include "../nfCommon/Math/Random.hpp"

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
    Frustum frustum;
    frustum.Construct(Vector(),
                      Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f),
                      1.0f, 10.0f);

    // boxes outside frustum near planes
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 0.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-1.0f, -1.0f, 11.0f), Vector(1.0f, 1.0f, 12.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(6.0f, -1.0f, 4.0f), Vector(7.0f, 1.0f, 5.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-7.0f, -1.0f, 4.0f), Vector(-6.0f, 1.0f, 5.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-1.0f, 6.0f, 4.0f), Vector(1.0f, 7.0f, 5.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-1.0f, -7.0f, 4.0f), Vector(1.0f, -6.0f, 5.0f))));

    // boxes intersecting with single frustum face
    EXPECT_TRUE(Intersect(frustum, Box(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 2.0f))));
    EXPECT_TRUE(Intersect(frustum, Box(Vector(-1.0f, -1.0f, 9.0f), Vector(1.0f, 1.0f, 12.0f))));
    EXPECT_TRUE(Intersect(frustum, Box(Vector(4.0f, -1.0f, 4.0f), Vector(7.0f, 1.0f, 5.0f))));
    EXPECT_TRUE(Intersect(frustum, Box(Vector(-7.0f, -1.0f, 4.0f), Vector(-4.0f, 1.0f, 5.0f))));
    EXPECT_TRUE(Intersect(frustum, Box(Vector(-1.0f, 4.0f, 4.0f), Vector(1.0f, 7.0f, 5.0f))));
    EXPECT_TRUE(Intersect(frustum, Box(Vector(-1.0f, -7.0f, 4.0f), Vector(1.0f, -4.0f, 5.0f))));

    // box inside / outside
    EXPECT_TRUE(Intersect(frustum, Box(Vector(6.0f, 6.0f, 7.0f), Vector(7.0f, 7.0f, 8.0f))));
    EXPECT_TRUE(Intersect(frustum, Box(Vector(-20.0f, -20.0f, -20.0f), Vector(20.0f, 20.0f, 20.0f))));

    // box with a corner
    EXPECT_TRUE(Intersect(frustum, Box(Vector(7.0f, 7.0f, 8.0f), Vector(11.0f, 11.0f, 11.0f))));

    // box outside, intersecting with multiple planes
    EXPECT_FALSE(Intersect(frustum, Box(Vector(11.0f, -20.0f, -20.0f), Vector(12.0f, 20.0f, 20.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-12.0f, -20.0f, -20.0f), Vector(-11.0f, 20.0f, 20.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-20.0f, 11.0f, -20.0f), Vector(20.0f, 12.0f, 20.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-20.0f, -12.0f, -20.0f), Vector(20.0f, -11.0f, 20.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-20.0f, -20.0f, 11.0f), Vector(20.0f, 20.0f, 12.0f))));
    EXPECT_FALSE(Intersect(frustum, Box(Vector(-20.0f, -20.0f, -1.0f), Vector(20.0f, 20.0f, 0.0f))));
}

TEST(MathGeometry, SphereFrustumIntersection)
{
    Frustum frustum;
    frustum.Construct(Vector(),
                      Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f),
                      1.0f, 10.0f);

    // spheres outside frustum near planes
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 0.0f), 0.5f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 11.0f), 0.5f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(6.0f, 0.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(-6.0f, 0.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 6.0f, 4.0f), 0.5f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 6.0f, 4.0f), 0.5f)));

    // spheres intersecting with single frustum face
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 1.0f), 0.5f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 10.0f), 0.5f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(5.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(-5.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 5.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 5.0f, 5.0f), 0.5f)));

    // spheres inside / outside
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 5.0f), 0.5f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 5.0f), 20.0f)));

    // sphere with a corner
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(9.0f, 9.0f, 9.0f), 3.0)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(11.0f, 11.0f, 11.0f), 1.7f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(-11.0f, -11.0f, 11.0f), 1.7f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(2.0f, 2.0f, 0.0f), 1.7f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(-2.0f, -2.0f, 0.0f), 1.6f)));

    // sphere with an edge
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(6.0f, 6.0f, 3.4f), 2.0f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(6.0f, 6.0f, 3.4f), 3.0f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 11.0f, 10.5f), 1.0f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 11.0f, 10.5f), 1.5f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(2.0f, 0.0f, 0.0f), 1.4f)));
    EXPECT_TRUE(Intersect(frustum, Sphere(Vector(0.0f, 2.0f, 0.0f), 1.42f)));

    // spheres outside, intersecting with multiple planes
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(1011.0f, 0.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(-1011.0f, 0.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 1011.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, -1011.0f, 0.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, -1000.0f), 1000.0f)));
    EXPECT_FALSE(Intersect(frustum, Sphere(Vector(0.0f, 0.0f, 1011.0f), 1000.0f)));
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
    Frustum frustum;
    frustum.Construct(Vector(),
                      Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f),
                      1.0f, 10.0f);

    /// Z axis
    EXPECT_FALSE(Intersect(Vector(0.0f, 0.0f, -1.0f), frustum));
    EXPECT_FALSE(Intersect(Vector(0.0f, 0.0f, 0.5f), frustum));
    EXPECT_TRUE(Intersect(Vector(0.0f, 0.0f, 1.5f), frustum));
    EXPECT_TRUE(Intersect(Vector(0.0f, 0.0f, 9.5f), frustum));
    EXPECT_FALSE(Intersect(Vector(0.0f, 0.0f, 10.5f), frustum));

    /// X axis
    EXPECT_FALSE(Intersect(Vector(-5.5f, 0.0f, 5.0f), frustum));
    EXPECT_TRUE(Intersect(Vector(-4.5f, 0.0f, 5.0f), frustum));
    EXPECT_TRUE(Intersect(Vector(4.5f, 0.0f, 5.0f), frustum));
    EXPECT_FALSE(Intersect(Vector(5.5f, 0.0f, 5.0f), frustum));

    /// Y axis
    EXPECT_FALSE(Intersect(Vector(0.0f, -5.5f, 5.0f), frustum));
    EXPECT_TRUE(Intersect(Vector(0.0f, -4.5f, 5.0f), frustum));
    EXPECT_TRUE(Intersect(Vector(0.0f, 4.5f, 5.0f), frustum));
    EXPECT_FALSE(Intersect(Vector(0.0f, 5.5f, 5.0f), frustum));

    EXPECT_FALSE(Intersect(Vector(-5.5f, -5.5f, 5.0f), frustum));
    EXPECT_TRUE(Intersect(Vector(-4.5f, -4.5f, 5.0f), frustum));
    EXPECT_FALSE(Intersect(Vector(10.5f, 10.5f, 10.0f), frustum));
    EXPECT_TRUE(Intersect(Vector(9.5f, 9.5f, 10.0f), frustum));
}

TEST(MathGeometry, TriangleFrustumIntersection)
{
    Frustum frustum;
    frustum.Construct(Vector(),
                      Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f),
                      1.0f, 10.0f);

    EXPECT_FALSE(Intersect(Triangle(Vector(-1.0f, 0.0f, 0.0f),
                                    Vector(1.0f, 0.0f, 0.0f),
                                    Vector(0.0f, 1.0f, 0.0f)), frustum));
    EXPECT_FALSE(Intersect(Triangle(Vector(-1.0f, 0.0f, 11.0f),
                                    Vector(1.0f, 0.0f, 11.0f),
                                    Vector(0.0f, 1.0f, 11.0f)), frustum));
    EXPECT_FALSE(Intersect(Triangle(Vector(-2.0f, 0.0f, 0.0f),
                                    Vector(2.0f, 0.0f, 0.0f),
                                    Vector(0.0f, 12.0f, 11.0f)), frustum));

    /*
     * TODO: edge cases - there is no straightforward separating plane.
     * Consider improving the intersection function.
     */
    /*
    EXPECT_FALSE(Intersect(Triangle(Vector(6.0f, 6.0f, 5.0f),
                                    Vector(5.0f, 6.0f, 5.0f),
                                    Vector(6.0f, 5.0f, 5.0f)), frustum));
    EXPECT_FALSE(Intersect(Triangle(Vector(0.0f, 0.0f, 0.0f),
                                    Vector(3.0f, 0.0f, 2.0f),
                                    Vector(3.0f, 0.0f, 0.0f)), frustum));
    EXPECT_FALSE(Intersect(Triangle(Vector(0.0f, 0.0f, 0.0f),
                                    Vector(0.0f, 3.0f, 2.0f),
                                    Vector(0.0f, 3.0f, 0.0f)), frustum));
    */

    EXPECT_TRUE(Intersect(Triangle(Vector(-2.0f, 0.0f, 0.0f),
                                   Vector(2.0f, 0.0f, 0.0f),
                                   Vector(0.0f, 0.0f, 2.0f)), frustum));
    EXPECT_TRUE(Intersect(Triangle(Vector(-5.0f, 1.0f, 6.0f),
                                   Vector(-4.0f, -2.0f, 5.0f),
                                   Vector(5.0f, 3.0f, 11.0f)), frustum));
    EXPECT_TRUE(Intersect(Triangle(Vector(-20.0f, 1.0f, 15.0f),
                                   Vector(20.0f, -1.0f, 15.0f),
                                   Vector(0.0f, 0.0f, -5.0f)), frustum));
}

TEST(MathGeometry, RayBoxIntersection)
{
    const Vector boxHalfExtent = Vector(1.0f, 2.0f, 3.0f);
    const int MAX_RAY_TESTS = 100;
    const int MAX_BOX_OFFSETS = 10;
    Random random(0);

    for (int i = 0; i < MAX_BOX_OFFSETS; ++i)
    {
        const Vector boxOffset = (2.0f * random.GetFloat3() - VECTOR_ONE) * 5.0f;
        SCOPED_TRACE("boxOffset = [" + std::to_string(boxOffset[0]) + ", " +
                     std::to_string(boxOffset[1]) + ", " + std::to_string(boxOffset[2]) + "]");

        // test box
        Box box(boxOffset - boxHalfExtent, boxOffset + boxHalfExtent);

        // check if a point 'p' is on the surface of the test box
        auto onBoxSurface = [&boxHalfExtent, &boxOffset](const Vector& p) -> bool
        {
            return (VectorLessEqMask(boxOffset + boxHalfExtent - p, VECTOR_EPSILON) != 0) ||
                   (VectorLessEqMask(boxOffset - boxHalfExtent - p, VECTOR_EPSILON) != 0);
        };

        EXPECT_TRUE(Intersect(Ray(Vector(-1.0f, 0.0f, 0.0f),
                                  boxOffset + Vector(2.0f, 0.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                  boxOffset + Vector(-2.0f, 0.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vector(0.0f, -1.0f, 0.0f),
                                  boxOffset + Vector(0.0f, 3.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vector(0.0f, 1.0f, 0.0f),
                                  boxOffset + Vector(0.0f, -3.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vector(0.0f, 0.0f, -1.0f),
                                  boxOffset + Vector(0.0f, 0.0f, 4.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f),
                                  boxOffset + Vector(0.0f, 0.0f, -4.0f)), box));

        EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                   boxOffset + Vector(2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(-1.0f, 0.0f, 0.0f),
                                   boxOffset + Vector(-2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 1.0f, 0.0f),
                                   boxOffset + Vector(0.0f, 3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(0.0f, -1.0f, 0.0f),
                                   boxOffset + Vector(0.0f, -3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f),
                                   boxOffset + Vector(0.0f, 0.0f, 4.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, -1.0f),
                                   boxOffset + Vector(0.0f, 0.0f, -4.0f)), box));

        // missing rays
        EXPECT_FALSE(Intersect(Ray(Vector(-1.0f, 2.1f, 0.0f),
                                   boxOffset + Vector(2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 2.1f, 0.0f),
                                   boxOffset + Vector(-2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(1.1f, -1.0f, 0.0f),
                                   boxOffset + Vector(0.0f, 3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(1.1f, 1.0f, 0.0f),
                                   boxOffset + Vector(0.0f, -3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(1.1f, 0.0f, -1.0f),
                                   boxOffset + Vector(0.0f, 0.0f, 4.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vector(1.1f, 0.0f, 1.0f),
                                   boxOffset + Vector(0.0f, 0.0f, 4.0f)), box));

        // rays with origin inside the box
        for (int j = 0; j < MAX_RAY_TESTS; ++j)
        {
            const Vector rayOrigin = boxOffset +
                boxHalfExtent * (Vector(random.GetFloat3()) * 2.0f - VECTOR_ONE);
            const Vector rayDir = Vector(random.GetFloat3()) * 2.0f - VECTOR_ONE;
            const Ray ray = Ray(rayDir, rayOrigin);
            EXPECT_TRUE(Intersect(ray, box));

            Vector dist;
            EXPECT_TRUE(Intersect(ray, box, dist));
            EXPECT_TRUE(onBoxSurface(ray.origin + ray.dir * dist));
        }
    }
}

TEST(MathGeometry, RayTriangleIntersection)
{
    // TODO
}

TEST(MathGeometry, RaySphereIntersection)
{
    EXPECT_TRUE(Intersect(Ray(Vector(0.4f, 1.0f, -0.2f), Vector(0.0f, -2.0f, 0.0f)),
                          Sphere(Vector(), 1.0f)));

    const int MAX_RAY_TESTS = 100;
    const int MAX_SPHERE_OFFSETS = 10;
    Random random(0);

    for (int i = 0; i < MAX_SPHERE_OFFSETS; ++i)
    {
        float radius = random.GetFloat();
        const Vector sphereOffset = (2.0f * random.GetFloat3() - VECTOR_ONE) * 5.0f;
        SCOPED_TRACE("sphereOffset = [" +
                     std::to_string(sphereOffset[0]) + ", " +
                     std::to_string(sphereOffset[1]) + ", " +
                     std::to_string(sphereOffset[2]) + "]");

        Sphere sphere(sphereOffset, radius);

        // check if a point 'p' is on the surface of the test sphere
        auto onSphereSurface = [&radius, &sphereOffset](const Vector& p) -> bool
        {
            float len = VectorLength3(sphereOffset - p).f[0];
            return fabsf(len - radius) < NFE_MATH_EPSILON;
        };

        Vector dist;
        EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                  Vector(-2.0f * radius, 0.5f * radius, 0.0f) + sphereOffset),
                              sphere));
        EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                  Vector(-2.0f * radius, -0.5f * radius, 0.0f) + sphereOffset),
                              sphere));
        EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                  Vector(-2.0f * radius, 0.5f * radius, 0.0f) + sphereOffset),
                              sphere));
        EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                  Vector(-2.0f * radius, -0.70710678f * radius, 0.0f) + sphereOffset),
                              sphere, dist));
        ASSERT_LT(fabsf(dist.f[0] - radius * (2.0f - 0.70710678f)), NFE_MATH_EPSILON);

        EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                   Vector(-2.0f * radius, 1.01f * radius, 0.0f) + sphereOffset),
                               sphere));
        EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                   Vector(-2.0f * radius, -1.01f * radius, 0.0f) + sphereOffset),
                               sphere));
        EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                   Vector(-2.0f * radius, 1.01f * radius, 0.0f) + sphereOffset),
                               sphere));
        EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                   Vector(-2.0f * radius, -1.01f * radius, 0.0f) + sphereOffset),
                               sphere));

        // rays with origin inside the sphere
        for (int j = 0; j < MAX_RAY_TESTS; ++j)
        {
            const Vector rayOrigin = sphereOffset + 3.0f * radius * (Vector(random.GetFloat3()) * 2.0f - VECTOR_ONE);
            const Vector rayDir = Vector(random.GetFloat3()) * 2.0f - VECTOR_ONE;
            const Ray ray = Ray(rayDir, rayOrigin);

            //Vector dist;
            if (VectorLength3(rayOrigin - sphereOffset).f[0] <= radius)
            {
                EXPECT_TRUE(Intersect(ray, sphere));
                EXPECT_TRUE(Intersect(ray, sphere, dist));
                EXPECT_TRUE(onSphereSurface(ray.origin + ray.dir * dist));  // WTF?
            }
            else if (Intersect(ray, sphere))
            {
                EXPECT_TRUE(Intersect(ray, sphere, dist));
                EXPECT_TRUE(onSphereSurface(ray.origin + ray.dir * dist));  // WTF?
            }
        }
    }

    // TODO
}

TEST(MathGeometry, RayFrustumIntersection)
{
    // TODO
}