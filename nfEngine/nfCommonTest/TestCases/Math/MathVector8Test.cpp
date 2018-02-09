#include "PCH.hpp"
#include "nfCommon/Math/Vector8.hpp"

using namespace NFE::Math;

namespace {

const Vector8 vecA = Vector8(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
const Vector8 vecB = Vector8(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
const Vector8 vecC = Vector8(2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
const Vector8 vecD = Vector8(1.0f, 4.0f, 9.0f, 16.0f);
const Vector8 vecE = Vector8(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathVector8, VectorLoadAndStore)
{
    EXPECT_TRUE(vecA == Vector8::Splat(1.0f));
}

TEST(MathVector8, VectorArithmetics)
{
    EXPECT_TRUE(Vector8::AlmostEqual(vecA + vecB, vecC));

    /*
    EXPECT_TRUE(Vector8::AlmostEqual(vecA - vecB, Vector4(0.0f, -1.0f, -2.0f, -3.0f)));
    EXPECT_TRUE(Vector8::AlmostEqual(vecB * vecB, vecD));
    EXPECT_TRUE(Vector8::AlmostEqual(vecC / vecB, Vector4(2.0f, 1.5f, 4.0f / 3.0f, 1.25f)));
    EXPECT_TRUE(Vector8::AlmostEqual(vecB * 2.0f, Vector4(2.0f, 4.0f, 6.0f, 8.0f)));
    EXPECT_TRUE(Vector8::AlmostEqual(vecB / 2.0f, Vector4(0.5f, 1.0f, 1.5f, 2.0f)));
    EXPECT_TRUE(Vector8::Abs(Vector4(-1.0f, -2.0f, 0.0f, 3.0f)) == Vector4(1.0f, 2.0f, 0.0f, 3.0f));
    */
}
