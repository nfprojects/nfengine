#include "PCH.hpp"
#include "Engine/Common/Math/Matrix3.hpp"


using namespace NFE::Math;

namespace {

const Matrix3 matA = Matrix3(Float3(-10.0f, 8.0f, -8.0f),
                             Float3(-2.0f,  4.0f,  7.0f),
                             Float3( 2.0f,  9.0f, -1.0f));

const Matrix3 matB = Matrix3(Float3( 4.0f,  9.0f, -2.0f),
                             Float3(-10.0f, 6.0f, -9.0f),
                             Float3( 2.0f, -2.0f,  6.0f));

const Matrix3 matI = Matrix3(Float3(1.0f, 0.0f, 0.0f),
                             Float3(0.0f, 1.0f, 0.0f),
                             Float3(0.0f, 0.0f, 1.0f));

} // namespace


TEST(MathMatrix3, Simple)
{
    const Matrix3 matTransposeA = Matrix3(Float3(-10.0f, -2.0,   2.0f),
                                          Float3( 8.0f,   4.0f,  9.0f),
                                          Float3(-8.0f,   7.0f, -1.0f));

    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix3());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix3, Multiply)
{
    const Matrix3 matAB = Matrix3(Float3(-136.0f, -26.0f, -100.0f),
                                  Float3(-34.0f,  -8.0f,   10.0f),
                                  Float3(-84.0f,  74.0f,  -91.0f));
    EXPECT_TRUE(matAB == matA * matB);
}

TEST(MathMatrix3, MultiplyVector)
{
    const Float3 vecA = Float3(1.0f, 2.0f, 3.0f);

    // expected product of matA and vecA
    const Float3 matAvecA = Float3(-8.0f, 43.0f, 3.0f);

    EXPECT_TRUE(Float3::AlmostEqual(matAvecA, vecA * matA));
}

TEST(MathMatrix3, MultiplyVector_Affine)
{
    const Float3 vecA(1.0f, 2.0f, 0.0f);

    // expected product of matA and vecA
    const Float2 matAvecAaffine = Float2(-12.0f, 25.0f);

    EXPECT_TRUE(Float2::AlmostEqual(matAvecAaffine, matA.AffineTransform2(vecA)));
}

TEST(MathMatrix3, Inverse)
{
    Matrix3 inv;
    ASSERT_TRUE(matA.Invert(inv));
    EXPECT_TRUE(Matrix3::Equal(matI, matA * inv, NFE_MATH_EPSILON));
}

TEST(MathMatrix3, Inverse_Noninvertible)
{
    const Matrix3 mat(Float3(1.0f, 2.0f, 3.0f), Float3(2.0f, 4.0f, 6.0f), Float3(-5.0f, 2.0f, 1.0f));

    Matrix3 inv;
    ASSERT_FALSE(mat.Invert(inv));
}
