#include "PCH.hpp"
#include "nfCommon/Math/Matrix2.hpp"


using namespace NFE::Math;

namespace {

const Matrix2 matA = Matrix2(Float2(-10.0f, 8.0f),
                             Float2(-2.0f, 4.0f));

const Matrix2 matB = Matrix2(Float2(4.0f,   9.0f),
                             Float2(-10.0f, 6.0f));

const Matrix2 matI = Matrix2(Float2(1.0f, 0.0f),
                             Float2(0.0f, 1.0f));

} // namespace


TEST(MathMatrix2, Simple)
{
    const Matrix2 matTransposeA = Matrix2(Float2(-10.0f, -2.0f),
                                          Float2(8.0f, 4.0f));

    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix2());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix2, Arithmetics)
{
    const Matrix2 matAB = Matrix2(Float2(-120.0f, -42.0f),
                                  Float2(-48.0f,  6.0f));
    const Float2 vecA = Float2(1.0f, 2.0f);

    // expected product of matA and vecA
    const Float2 matAvecA = Float2(-20.0f, 39.0f);
    const Float2 matAvecAaffine = Float2(-11.0f, 42.0f);

    EXPECT_TRUE(matAB == matA * matB);
    EXPECT_TRUE(Matrix2::Equal(matI, matA * matA.Inverted(), NFE_MATH_EPSILON));
}
