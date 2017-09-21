#include "PCH.hpp"
#include "nfCommon/Math/Geometry.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Math/Frustum.hpp"
#include "nfCommon/Math/Sphere.hpp"
#include "nfCommon/Math/Triangle.hpp"
#include "nfCommon/Math/Random.hpp"

using namespace NFE::Math;

TEST(MathGeometry, RayBoxIntersection)
{
    const Vector boxHalfExtent = Vector(1.0f, 2.0f, 3.0f);
    const int MAX_RAY_TESTS = 100;
    const int MAX_BOX_OFFSETS = 10;
    Random random(0);

    for (int i = 0; i < MAX_BOX_OFFSETS; ++i)
    {
        const Vector boxOffset = (2.0f * Vector(random.GetFloat3()) - VECTOR_ONE) * 5.0f;
        SCOPED_TRACE("boxOffset = [" + std::to_string(boxOffset[0]) + ", " +
                     std::to_string(boxOffset[1]) + ", " + std::to_string(boxOffset[2]) + "]");

        // test box
        Box box(boxOffset - boxHalfExtent, boxOffset + boxHalfExtent);

        // check if a point 'p' is on the surface of the test box
        auto onBoxSurface = [&boxHalfExtent, &boxOffset](const Vector& p) -> bool
        {
            return (Vector::LessEqMask(boxOffset + boxHalfExtent - p, VECTOR_EPSILON) != 0) ||
                   (Vector::LessEqMask(boxOffset - boxHalfExtent - p, VECTOR_EPSILON) != 0);
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
    Triangle tri(Vector(-1.0f, -1.0f, 0.0f), Vector(1.0f, -1.0f, 0.0f), Vector(1.0f, 1.0f, 0.0f));

    // check if a point 'p' is on the surface of the test triangle's plane
    auto onTrianglePlane = [&tri](const Vector& p) -> bool
    {
        Vector plane = Vector::PlaneFromPoints(tri.v0, tri.v1, tri.v2);
        const float d = Vector::Dot3(plane, p) + plane.f[3];
        return fabsf(d) < NFE_MATH_EPSILON;
    };

    // negative checks
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector(-1.1f, -1.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector( 1.1f, -1.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector( 1.1f,  1.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector(-0.1f,  0.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector(-0.8f, -0.9f,  0.1f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector( 0.9f, -0.9f,  0.1f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector( 0.1f, -0.1f,  0.1f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vector(0.0f, 0.0f, 1.0f), Vector( 0.9f,  0.8f,  0.1f)), tri));

    // rays for positive checks
    const Ray rays[] =
    {
        Ray(Vector(0.0f,  0.0f, 1.0f), Vector(-0.8f, -0.9f, -1.0f)),
        Ray(Vector(0.0f,  0.0f, 1.0f), Vector( 0.9f, -0.9f, -1.0f)),
        Ray(Vector(0.0f,  0.0f, 1.0f), Vector( 0.1f, -0.1f, -1.0f)),
        Ray(Vector(0.0f,  0.0f, 1.0f), Vector( 0.9f,  0.8f, -1.0f)),
        Ray(Vector(1.0f, -1.0f, 0.7f), Vector(-0.5f,  0.5f, -1.0f)),
    };

    for (int i = 0; i < sizeof(rays) / sizeof(Ray); ++i)
    {
        SCOPED_TRACE("i = " + std::to_string(i));
        Vector d;
        EXPECT_TRUE(Intersect(rays[i], tri, d));
        EXPECT_TRUE(onTrianglePlane(rays[i].origin + d * rays[i].dir));
    }
}

TEST(MathGeometry, RaySphereIntersection)
{
    EXPECT_TRUE(Intersect(Ray(Vector(0.4f, 1.0f, -0.2f), Vector(0.0f, -2.0f, 0.0f)),
                          Sphere(Vector(), 1.0f)));

    const int MAX_RAY_TESTS = 100;
    Random random(0);

    float radius = random.GetFloat();
    Sphere sphere(Vector(), radius);

    // check if a point 'p' is on the surface of the test sphere
    auto onSphereSurface = [&radius](const Vector& p) -> bool
    {
        float len = p.Length3();
        // increased epsilon in order to handle corner cases
        return fabsf(len - radius) < 10.0f * NFE_MATH_EPSILON;
    };

    Vector dist;
    EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, 0.5f * radius, 0.0f)),
                            sphere));
    EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, -0.5f * radius, 0.0f)),
                            sphere));
    EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, 0.5f * radius, 0.0f)),
                            sphere));
    EXPECT_TRUE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, -0.70710678f * radius, 0.0f)),
                            sphere, dist));
    ASSERT_LT(fabsf(dist.f[0] - radius * (2.0f - 0.70710678f)), NFE_MATH_EPSILON);

    EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, 1.01f * radius, 0.0f)),
                            sphere));
    EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, -1.01f * radius, 0.0f)),
                            sphere));
    EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, 1.01f * radius, 0.0f)),
                            sphere));
    EXPECT_FALSE(Intersect(Ray(Vector(1.0f, 0.0f, 0.0f),
                                Vector(-2.0f * radius, -1.01f * radius, 0.0f)),
                            sphere));

    // rays with origin inside the sphere
    for (int j = 0; j < MAX_RAY_TESTS; ++j)
    {
        const Vector rayOrigin = 3.0f * radius * (Vector(random.GetFloat3()) * 2.0f - VECTOR_ONE);
        const Vector rayDir = Vector(random.GetFloat3()) * 2.0f - VECTOR_ONE;
        const Ray ray = Ray(rayDir, rayOrigin);

        //Vector dist;
        if (rayOrigin.Length3() <= radius)
        {
            EXPECT_TRUE(Intersect(ray, sphere));
            EXPECT_TRUE(Intersect(ray, sphere, dist));
            EXPECT_TRUE(onSphereSurface(ray.origin + ray.dir * dist));
        }
        else if (Intersect(ray, sphere))
        {
            EXPECT_TRUE(Intersect(ray, sphere, dist));
            EXPECT_TRUE(onSphereSurface(ray.origin + ray.dir * dist));
        }
    }
}
