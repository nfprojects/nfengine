#include "PCH.hpp"
#include "nfCommon/Math/Vector.hpp"

using namespace NFE::Math;

namespace {

const Vector vecA = Vector(1.0f, 1.0f, 1.0f, 1.0f);
const Vector vecB = Vector(1.0f, 2.0f, 3.0f, 4.0f);
const Vector vecC = Vector(2.0f, 3.0f, 4.0f, 5.0f);
const Vector vecD = Vector(1.0f, 4.0f, 9.0f, 16.0f);
const Vector vecE = Vector(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathVector, VectorLoadAndStore)
{
    Float2 f2;
    Float3 f3;
    Float4 f4;

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
}

TEST(MathVector, VectorMisc)
{
    Vector vA(1.0f, 2.0f, 3.0f, 4.0f);
    Vector vB(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) ==
                VectorSelectBySign(vA, vB, Vector(1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector(5.0f, 2.0f, 3.0f, 4.0f) ==
                VectorSelectBySign(vA, vB, Vector(-1.0f, 1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector(1.0f, 6.0f, 3.0f, 4.0f) ==
                VectorSelectBySign(vA, vB, Vector(1.0f, -1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(Vector(1.0f, 2.0f, 7.0f, 4.0f) ==
                VectorSelectBySign(vA, vB, Vector(1.0f, 1.0f, -1.0f, 1.0f)));
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 8.0f) ==
                VectorSelectBySign(vA, vB, Vector(1.0f, 1.0f, 1.0f, -1.0f)));
    EXPECT_TRUE(Vector(5.0f, 6.0f, 7.0f, 8.0f) ==
                VectorSelectBySign(vA, vB, Vector(-1.0f, -1.0f, -1.0f, -1.0f)));
}

TEST(MathVector, VectorArithmetics)
{
    EXPECT_TRUE(vecA + vecB == vecC);
    EXPECT_TRUE(vecA - vecB == Vector(0.0f, -1.0f, -2.0f, -3.0f));
    EXPECT_TRUE(vecB * vecB == vecD);
    EXPECT_TRUE(vecC / vecB == Vector(2.0f, 1.5f, 4.0f / 3.0f, 1.25f));
    EXPECT_TRUE(vecB * 2.0f == Vector(2.0f, 4.0f, 6.0f, 8.0f));
    EXPECT_TRUE(vecB / 2.0f == Vector(0.5f, 1.0f, 1.5f, 2.0f));
    EXPECT_TRUE(VectorAbs(Vector(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector(1.0f, 2.0f, 0.0f, 3.0f));
}

TEST(MathVector, VectorLerp)
{
    EXPECT_TRUE(VectorLerp(vecA, vecB, 0.0f) == vecA);
    EXPECT_TRUE(VectorLerp(vecA, vecB, 1.0f) == vecB);
    EXPECT_TRUE(VectorLerp(vecA, vecB, 0.5f) == Vector(1.0f, 1.5f, 2.0f, 2.5f));
}

TEST(MathVector, VectorMinMax)
{
    EXPECT_TRUE(VectorMin(vecB, vecE) == Vector(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(VectorMin(vecE, vecB) == Vector(1.0f, 2.0f, 2.0f, 1.0f));
    EXPECT_TRUE(VectorMax(vecB, vecE) == Vector(4.0f, 3.0f, 3.0f, 4.0f));
    EXPECT_TRUE(VectorMax(vecB, vecE) == Vector(4.0f, 3.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorGeometrics)
{
    EXPECT_TRUE(VectorDot3(vecB, vecC) == VectorSplat(20.0f));
    EXPECT_TRUE(VectorDot4(vecB, vecC) == VectorSplat(40.0f));
    EXPECT_TRUE(VectorCross3(vecB, vecC) == Vector(-1.0f, 2.0f, -1.0f, 0.0f));
    EXPECT_TRUE(VectorCross3(vecC, vecB) == Vector(1.0f, -2.0f, 1.0f, 0.0f));
}

TEST(MathVector, VectorLess)
{
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 10.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 10.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 10.0f) < vecC);
    EXPECT_FALSE(Vector(2.0f, 2.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 3.0f, 3.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 4.0f, 4.0f) < vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 5.0f) < vecC);
}

TEST(MathVector, VectorLessOrEqual)
{
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(1.0f, 10.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 10.0f, 4.0f) <= vecC);
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 10.0f) <= vecC);
    EXPECT_TRUE(Vector(2.0f, 2.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector(1.0f, 3.0f, 3.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector(1.0f, 2.0f, 4.0f, 4.0f) <= vecC);
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 5.0f) <= vecC);
}

TEST(MathVector, VectorGreater)
{
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(1.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 1.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 1.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 1.0f) > vecC);
    EXPECT_FALSE(Vector(2.0f, 4.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 3.0f, 5.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 4.0f, 6.0f) > vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 5.0f) > vecC);
}

TEST(MathVector, VectorGreaterOrEqual)
{
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(1.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(3.0f, 1.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 1.0f, 6.0f) >= vecC);
    EXPECT_FALSE(Vector(3.0f, 4.0f, 5.0f, 1.0f) >= vecC);
    EXPECT_TRUE(Vector(2.0f, 4.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector(3.0f, 3.0f, 5.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector(3.0f, 4.0f, 4.0f, 6.0f) >= vecC);
    EXPECT_TRUE(Vector(3.0f, 4.0f, 5.0f, 5.0f) >= vecC);
}

TEST(MathVector, VectorEqual)
{
    EXPECT_TRUE(Vector(1.0f, 2.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(10.0f, 2.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 20.0f, 3.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 30.0f, 4.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 2.0f, 3.0f, 40.0f) == Vector(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(MathVector, VectorNotEqual)
{
    EXPECT_TRUE(Vector(4.0f, 3.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(1.0f, 3.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 2.0f, 2.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 3.0f, 3.0f, 1.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(Vector(4.0f, 3.0f, 2.0f, 4.0f) != Vector(1.0f, 2.0f, 3.0f, 4.0f));
}
