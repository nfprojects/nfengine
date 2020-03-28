#include "PCH.hpp"
#include "Engine/Common/Math/Matrix3.hpp"


using namespace NFE::Math;

namespace {

const Matrix3 matA = Matrix3(Vec3f(-10.0f, 8.0f, -8.0f),
                             Vec3f(-2.0f,  4.0f,  7.0f),
                             Vec3f( 2.0f,  9.0f, -1.0f));

const Matrix3 matB = Matrix3(Vec3f( 4.0f,  9.0f, -2.0f),
                             Vec3f(-10.0f, 6.0f, -9.0f),
                             Vec3f( 2.0f, -2.0f,  6.0f));

const Matrix3 matI = Matrix3(Vec3f(1.0f, 0.0f, 0.0f),
                             Vec3f(0.0f, 1.0f, 0.0f),
                             Vec3f(0.0f, 0.0f, 1.0f));

} // namespace


TEST(MathMatrix3, Simple)
{
    const Matrix3 matTransposeA = Matrix3(Vec3f(-10.0f, -2.0,   2.0f),
                                          Vec3f( 8.0f,   4.0f,  9.0f),
                                          Vec3f(-8.0f,   7.0f, -1.0f));

    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix3());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix3, Multiply)
{
    const Matrix3 matAB = Matrix3(Vec3f(-136.0f, -26.0f, -100.0f),
                                  Vec3f(-34.0f,  -8.0f,   10.0f),
                                  Vec3f(-84.0f,  74.0f,  -91.0f));
    EXPECT_TRUE(matAB == matA * matB);
}

TEST(MathMatrix3, MultiplyVector)
{
    const Vec3f vecA = Vec3f(1.0f, 2.0f, 3.0f);

    // expected product of matA and vecA
    const Vec3f matAvecA = Vec3f(-8.0f, 43.0f, 3.0f);

    EXPECT_TRUE(Vec3f::AlmostEqual(matAvecA, vecA * matA));
}

TEST(MathMatrix3, MultiplyVector_Affine)
{
    const Vec3f vecA(1.0f, 2.0f, 0.0f);

    // expected product of matA and vecA
    const Vec2f matAvecAaffine = Vec2f(-12.0f, 25.0f);

    EXPECT_TRUE(Vec2f::AlmostEqual(matAvecAaffine, matA.AffineTransform2(vecA)));
}

TEST(MathMatrix3, Inverse)
{
    Matrix3 inv;
    ASSERT_TRUE(matA.Invert(inv));
    EXPECT_TRUE(Matrix3::Equal(matI, matA * inv, NFE_MATH_EPSILON));
}

TEST(MathMatrix3, Inverse_Noninvertible)
{
    const Matrix3 mat(Vec3f(1.0f, 2.0f, 3.0f), Vec3f(2.0f, 4.0f, 6.0f), Vec3f(-5.0f, 2.0f, 1.0f));

    Matrix3 inv;
    ASSERT_FALSE(mat.Invert(inv));
}
