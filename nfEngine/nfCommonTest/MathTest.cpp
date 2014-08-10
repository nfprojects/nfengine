#include "stdafx.hpp"
#include "../nfCommon/Math/Math.hpp"

using namespace NFE::Math;

TEST(nfMathTest, VectorSimple)
{
    const Vector vecA = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    const Vector vecB = Vector(1.0f, 2.0f, 3.0f, 4.0f);
    const Vector vecC = Vector(2.0f, 3.0f, 4.0f, 5.0f);
    const Vector vecD = Vector(1.0f, 4.0f, 9.0f, 16.0f);
    const Vector vecE = Vector(4.0f, 3.0f, 2.0f, 1.0f);

    Float2 f2;
    Float3 f3;
    Float4 f4;
    Vector vec;

    /// load, store, splat, etc.
    VectorStore(vecB, &f2);
    VectorStore(vecB, &f3);
    VectorStore(vecB, &f4);
    EXPECT_TRUE(f2.x == 1.0f && f2.y == 2.0f);
    EXPECT_TRUE(f3.x == 1.0f && f3.y == 2.0f && f3.z == 3.0f);
    EXPECT_TRUE(f4.x == 1.0f && f4.y == 2.0f && f4.z == 3.0f && f4.w == 4.0f);
    EXPECT_TRUE(Vector(f2) == Vector(1.0f, 2.0f, 0.0f, 0.0f));
    EXPECT_TRUE(Vector(f3) == Vector(1.0f, 2.0f, 3.0f, 0.0f));
    EXPECT_TRUE(Vector(f4) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(Vector(123.0f, 123.0f, 123.0f, 123.0f) == VectorSplat(123.0f));
    EXPECT_TRUE(Vector(1.0f, 1.0f, 1.0f, 1.0f) == vecB.SplatX());
    EXPECT_TRUE(Vector(2.0f, 2.0f, 2.0f, 2.0f) == vecB.SplatY());
    EXPECT_TRUE(Vector(3.0f, 3.0f, 3.0f, 3.0f) == vecB.SplatZ());
    EXPECT_TRUE(Vector(4.0f, 4.0f, 4.0f, 4.0f) == vecB.SplatW());

    // simple arithmetic operations
    EXPECT_TRUE(vecA + vecB == Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(vecA - vecB == Vector(0.0f, -1.0f, -2.0f, -3.0f));
    EXPECT_TRUE(vecB * vecB == vecD);
    EXPECT_TRUE(vecC / vecB == Vector(2.0f, 1.5f, 4.0f / 3.0f, 1.25f));
    EXPECT_TRUE(vecB * 2.0f == Vector(2.0f, 4.0f, 6.0f, 8.0f));
    EXPECT_TRUE(vecB / 2.0f == Vector(0.5f, 1.0f, 1.5f, 2.0f));
    EXPECT_TRUE(VectorAbs(Vector(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector(1.0f, 2.0f, 0.0f, 3.0f));

    // linear interpolation
    EXPECT_TRUE(VectorLerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(VectorLerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(VectorLerp(vecA, vecB, 0.5f) == Vector(1.0f, 1.5f, 2.0f, 2.5f));

    /// min / max
    EXPECT_TRUE(VectorMin(vecB, vecE) == Vector(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(VectorMin(vecE, vecB) == Vector(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(VectorMax(vecB, vecE) == Vector(4.0f, 3.0f, 3.0f, 4.0f));
    EXPECT_TRUE(VectorMax(vecB, vecE) == Vector(4.0f, 3.0f, 3.0f, 4.0f));

    /// geometric operations
    EXPECT_TRUE(VectorDot3(vecB, vecC) == VectorSplat(20.0f));
    EXPECT_TRUE(VectorDot4(vecB, vecC) == VectorSplat(40.0f));
    EXPECT_TRUE(VectorCross3(vecB, vecC) == Vector(-1.0f, 2.0f, -1.0f, 0.0f));
    EXPECT_TRUE(VectorCross3(vecC, vecB) == Vector(1.0f, -2.0f, 1.0f, 0.0f));
}

TEST(nfMathTest, VectorCompare)
{
    // "less" tests
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 10.0f, 3.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 10.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 10.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(2.0f, 2.0f, 3.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 3.0f, 3.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 4.0f, 4.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 5.0f) < Vector(2.0f, 3.0f, 4.0f, 5.0f));

    // "less or equal" tests
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 10.0f, 3.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 10.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 10.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(2.0f, 2.0f, 3.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(1.0f, 3.0f, 3.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(1.0f, 2.0f, 4.0f, 4.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 5.0f) <= Vector(2.0f, 3.0f, 4.0f, 5.0f));

    // "greater" tests
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 4.0f, 5.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 1.0f, 5.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 4.0f, 1.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 1.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(2.0f, 4.0f, 5.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 3.0f, 5.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 4.0f, 4.0f, 6.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 5.0f) > Vector(2.0f, 3.0f, 4.0f, 5.0f));

    // "greater or equal" tests
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(1.0f, 4.0f, 5.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 1.0f, 5.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 4.0f, 1.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 1.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(2.0f, 4.0f, 5.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(3.0f, 3.0f, 5.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(3.0f, 4.0f, 4.0f, 6.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 5.0f) >= Vector(2.0f, 3.0f, 4.0f, 5.0f));

    // "equal" tests
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 20.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 30.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 40.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));

    // "not equal" tests
    EXPECT_TRUE(Vector(4.0f, 3.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 3.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 2.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 3.0f, 3.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 3.0f, 2.0f, 4.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(nfMathTest, ClosestPointOnSegment)
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

TEST(nfMathTest, BoxBoxIntersection)
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

TEST(nfMathTest, BoxFrustumIntersection)
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

TEST(nfMathTest, SphereFrustumIntersection)
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

TEST(nfMathTest, SphereBoxIntersection)
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

TEST(nfMathTest, SphereSphereIntersection)
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

TEST(nfMathTest, TriangleFrustumIntersection)
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