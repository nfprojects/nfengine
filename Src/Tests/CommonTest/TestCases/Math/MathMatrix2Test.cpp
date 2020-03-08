#include "PCH.hpp"
#include "Engine/Common/Math/Matrix2.hpp"


using namespace NFE::Math;

namespace {

const Matrix2 matA = Matrix2(Float2(-10.0f, 8.0f),
                             Float2(-2.0f,  4.0f));

const Matrix2 matB = Matrix2(Float2( 4.0f,  9.0f),
                             Float2(-10.0f, 6.0f));

const Matrix2 matI = Matrix2(Float2(1.0f, 0.0f),
                             Float2(0.0f, 1.0f));

} // namespace


TEST(MathMatrix2, Simple)
{
    const Matrix2 matTransposeA = Matrix2(Float2(-10.0f, -2.0f),
                                          Float2( 8.0f,   4.0f));

    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix2());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix2, Multiply)
{
    const Matrix2 matAB = Matrix2(Float2(-120.0f, -42.0f),
                                  Float2(-48.0f,   6.0f));

    EXPECT_TRUE(matAB == matA * matB);
}

TEST(MathMatrix2, MultiplyVector)
{
    const Float2 vecA = Float2(1.0f, 2.0f);

    // expected product of matA and vecA
    const Float2 matAvecA = Float2(-14.0f, 16.0f);

    EXPECT_TRUE(Float2::AlmostEqual(matAvecA, vecA * matA));
}


TEST(MathMatrix2, Inverse)
{
    Matrix2 inv;
    ASSERT_TRUE(matA.Invert(inv));
    EXPECT_TRUE(Matrix2::Equal(matI, matA * inv, NFE_MATH_EPSILON));
}

TEST(MathMatrix2, Inverse_Noninvertible)
{
    const Matrix2 mat(Float2(1.0f, 2.0f), Float2(2.0f, 4.0f));

    Matrix2 inv;
    ASSERT_FALSE(mat.Invert(inv));
}
