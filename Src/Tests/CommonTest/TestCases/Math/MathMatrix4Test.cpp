#include "PCH.hpp"
#include "Engine/Common/Math/Matrix4.hpp"

using namespace NFE::Math;

/*
 * TODO:
 * * tests for functions Matrix4::MakePerspective(), Matrix4::MakeTranslation3(), etc.
 */

namespace {

const Matrix4 matA = Matrix4(Vec4f(-10.0f, 8.0f, -8.0f,  9.0f),
                             Vec4f(-2.0f, 4.0f, 7.0f, 5.0f),
                             Vec4f(2.0f, 9.0f, -1.0f, 3.0f),
                             Vec4f(-3.0f, -1.0f, 8.0f, -9.0f));

const Matrix4 matB = Matrix4(Vec4f(4.0f,   9.0f, -2.0f,  2.0f),
                             Vec4f(-10.0f, 6.0f, -9.0f, -9.0f),
                             Vec4f(2.0f,  -2.0f,  6.0f, -8.0f),
                             Vec4f(-9.0f,  3.0f, -9.0f,  1.0f));

const Matrix4 matI = Matrix4(Vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                             Vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                             Vec4f(0.0f, 0.0f, 1.0f, 0.0f),
                             Vec4f(0.0f, 0.0f, 0.0f, 1.0f));

} // namespace

TEST(MathMatrix4, Simple)
{
    const Matrix4 matTransposeA = Matrix4(Vec4f(-10.0f, -2.0f, 2.0f, -3.0f),
                                          Vec4f(8.0f, 4.0f, 9.0f, -1.0f),
                                          Vec4f(-8.0f, 7.0f, -1.0f, 8.0f),
                                          Vec4f(9.0f, 5.0f, 3.0f, -9.0f));

    EXPECT_TRUE(matA[2][2] == -1.0f); // rows access
    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix4::Identity());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix4, Arithmetics)
{
    const Matrix4 matAB = Matrix4(Vec4f(-217.0f, 1.0f,  -181.0f, -19.0f),
                                  Vec4f(-79.0f,  7.0f,  -35.0f,  -91.0f),
                                  Vec4f(-111.0f, 83.0f, -118.0f, -66.0f),
                                  Vec4f(95.0f,  -76.0f,  144.0f, -70.0f));

    EXPECT_TRUE(matAB == matA * matB);
}

TEST(MathMatrix4, Determinant)
{
    EXPECT_EQ(1.0f, Matrix4::Identity().Determinant());
    EXPECT_EQ(-12039.0f, matA.Determinant());
}

TEST(MathMatrix4, Inverse)
{
    const Matrix4 result = matA * matA.Inverted();

    const float maxDiff = (result - Matrix4::Identity()).Abs().Max();
    EXPECT_GE(1.0e-6f, maxDiff);
}
