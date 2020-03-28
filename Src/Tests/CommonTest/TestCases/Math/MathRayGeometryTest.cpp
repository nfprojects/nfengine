#include "PCH.hpp"
#include "Engine/Common/Math/Geometry.hpp"
#include "Engine/Common/Math/Frustum.hpp"
#include "Engine/Common/Math/Sphere.hpp"
#include "Engine/Common/Math/Random.hpp"

using namespace NFE::Math;

/*

TEST(MathGeometry, RayBoxIntersection)
{
    const Vec4f boxHalfExtent = Vec4f(1.0f, 2.0f, 3.0f);
    const int MAX_RAY_TESTS = 100;
    const int MAX_BOX_OFFSETS = 10;
    Random random;

    for (int i = 0; i < MAX_BOX_OFFSETS; ++i)
    {
        const Vec4f boxOffset = (2.0f * Vec4f(random.GetVec3f()) - VECTOR_ONE) * 5.0f;
        SCOPED_TRACE("boxOffset = [" + std::to_string(boxOffset[0]) + ", " +
                     std::to_string(boxOffset[1]) + ", " + std::to_string(boxOffset[2]) + "]");

        // test box
        Box box(boxOffset - boxHalfExtent, boxOffset + boxHalfExtent);

        // check if a point 'p' is on the surface of the test box
        auto onBoxSurface = [&boxHalfExtent, &boxOffset](const Vec4f& p) -> bool
        {
            return ((boxOffset + boxHalfExtent - p <= VECTOR_EPSILON).GetMask() != 0) ||
                   ((boxOffset - boxHalfExtent - p <= VECTOR_EPSILON).GetMask() != 0);
        };

        EXPECT_TRUE(Intersect(Ray(Vec4f(-1.0f, 0.0f, 0.0f),
                                  boxOffset + Vec4f(2.0f, 0.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                  boxOffset + Vec4f(-2.0f, 0.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vec4f(0.0f, -1.0f, 0.0f),
                                  boxOffset + Vec4f(0.0f, 3.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vec4f(0.0f, 1.0f, 0.0f),
                                  boxOffset + Vec4f(0.0f, -3.0f, 0.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vec4f(0.0f, 0.0f, -1.0f),
                                  boxOffset + Vec4f(0.0f, 0.0f, 4.0f)), box));
        EXPECT_TRUE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f),
                                  boxOffset + Vec4f(0.0f, 0.0f, -4.0f)), box));

        EXPECT_FALSE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                   boxOffset + Vec4f(2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(-1.0f, 0.0f, 0.0f),
                                   boxOffset + Vec4f(-2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 1.0f, 0.0f),
                                   boxOffset + Vec4f(0.0f, 3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, -1.0f, 0.0f),
                                   boxOffset + Vec4f(0.0f, -3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f),
                                   boxOffset + Vec4f(0.0f, 0.0f, 4.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, -1.0f),
                                   boxOffset + Vec4f(0.0f, 0.0f, -4.0f)), box));

        // missing rays
        EXPECT_FALSE(Intersect(Ray(Vec4f(-1.0f, 2.1f, 0.0f),
                                   boxOffset + Vec4f(2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(1.0f, 2.1f, 0.0f),
                                   boxOffset + Vec4f(-2.0f, 0.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(1.1f, -1.0f, 0.0f),
                                   boxOffset + Vec4f(0.0f, 3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(1.1f, 1.0f, 0.0f),
                                   boxOffset + Vec4f(0.0f, -3.0f, 0.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(1.1f, 0.0f, -1.0f),
                                   boxOffset + Vec4f(0.0f, 0.0f, 4.0f)), box));
        EXPECT_FALSE(Intersect(Ray(Vec4f(1.1f, 0.0f, 1.0f),
                                   boxOffset + Vec4f(0.0f, 0.0f, 4.0f)), box));

        // rays with origin inside the box
        for (int j = 0; j < MAX_RAY_TESTS; ++j)
        {
            const Vec4f rayOrigin = boxOffset +
                boxHalfExtent * (Vec4f(random.GetVec3f()) * 2.0f - VECTOR_ONE);
            const Vec4f rayDir = Vec4f(random.GetVec3f()) * 2.0f - VECTOR_ONE;
            const Ray ray = Ray(rayDir, rayOrigin);
            EXPECT_TRUE(Intersect(ray, box));

            Vec4f dist;
            EXPECT_TRUE(Intersect(ray, box, dist));
            EXPECT_TRUE(onBoxSurface(ray.origin + ray.dir * dist));
        }
    }
}

TEST(MathGeometry, RayTriangleIntersection)
{
    Triangle tri(Vec4f(-1.0f, -1.0f, 0.0f), Vec4f(1.0f, -1.0f, 0.0f), Vec4f(1.0f, 1.0f, 0.0f));

    // check if a point 'p' is on the surface of the test triangle's plane
    auto onTrianglePlane = [&tri](const Vec4f& p) -> bool
    {
        const Plane plane(tri.v0, tri.v1, tri.v2);
        const float d = plane.PointDistance(p);
        return fabsf(d) < NFE_MATH_EPSILON;
    };

    // negative checks
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f(-1.1f, -1.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f( 1.1f, -1.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f( 1.1f,  1.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f(-0.1f,  0.1f, -1.0f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f(-0.8f, -0.9f,  0.1f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f( 0.9f, -0.9f,  0.1f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f( 0.1f, -0.1f,  0.1f)), tri));
    EXPECT_FALSE(Intersect(Ray(Vec4f(0.0f, 0.0f, 1.0f), Vec4f( 0.9f,  0.8f,  0.1f)), tri));

    // rays for positive checks
    const Ray rays[] =
    {
        Ray(Vec4f(0.0f,  0.0f, 1.0f), Vec4f(-0.8f, -0.9f, -1.0f)),
        Ray(Vec4f(0.0f,  0.0f, 1.0f), Vec4f( 0.9f, -0.9f, -1.0f)),
        Ray(Vec4f(0.0f,  0.0f, 1.0f), Vec4f( 0.1f, -0.1f, -1.0f)),
        Ray(Vec4f(0.0f,  0.0f, 1.0f), Vec4f( 0.9f,  0.8f, -1.0f)),
        Ray(Vec4f(1.0f, -1.0f, 0.7f), Vec4f(-0.5f,  0.5f, -1.0f)),
    };

    for (int i = 0; i < sizeof(rays) / sizeof(Ray); ++i)
    {
        SCOPED_TRACE("i = " + std::to_string(i));
        Vec4f d;
        EXPECT_TRUE(Intersect(rays[i], tri, d));
        EXPECT_TRUE(onTrianglePlane(rays[i].origin + d * rays[i].dir));
    }
}

TEST(MathGeometry, RaySphereIntersection)
{
    EXPECT_TRUE(Intersect(Ray(Vec4f(0.4f, 1.0f, -0.2f), Vec4f(0.0f, -2.0f, 0.0f)),
                          Sphere(Vec4f::Zero(), 1.0f)));

    const int MAX_RAY_TESTS = 100;
    Random random;

    float radius = random.GetFloat();
    Sphere sphere(Vec4f::Zero(), radius);

    // check if a point 'p' is on the surface of the test sphere
    auto onSphereSurface = [&radius](const Vec4f& p) -> bool
    {
        float len = p.Length3();
        // increased epsilon in order to handle corner cases
        return fabsf(len - radius) < 10.0f * NFE_MATH_EPSILON;
    };

    Vec4f dist;
    EXPECT_TRUE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, 0.5f * radius, 0.0f)),
                            sphere));
    EXPECT_TRUE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, -0.5f * radius, 0.0f)),
                            sphere));
    EXPECT_TRUE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, 0.5f * radius, 0.0f)),
                            sphere));
    EXPECT_TRUE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, -0.70710678f * radius, 0.0f)),
                            sphere, dist));
    ASSERT_LT(fabsf(dist.f[0] - radius * (2.0f - 0.70710678f)), NFE_MATH_EPSILON);

    EXPECT_FALSE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, 1.01f * radius, 0.0f)),
                            sphere));
    EXPECT_FALSE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, -1.01f * radius, 0.0f)),
                            sphere));
    EXPECT_FALSE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, 1.01f * radius, 0.0f)),
                            sphere));
    EXPECT_FALSE(Intersect(Ray(Vec4f(1.0f, 0.0f, 0.0f),
                                Vec4f(-2.0f * radius, -1.01f * radius, 0.0f)),
                            sphere));

    // rays with origin inside the sphere
    for (int j = 0; j < MAX_RAY_TESTS; ++j)
    {
        const Vec4f rayOrigin = 3.0f * radius * (Vec4f(random.GetVec3f()) * 2.0f - VECTOR_ONE);
        const Vec4f rayDir = Vec4f(random.GetVec3f()) * 2.0f - VECTOR_ONE;
        const Ray ray = Ray(rayDir, rayOrigin);

        //Vec4f dist;
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

*/