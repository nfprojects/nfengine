#include "PCH.hpp"
#include "nfCommon/Math/Matrix.hpp"

using namespace NFE::Math;

/*
 * TODO:
 * * tests for functions Matrix::MakePerspective(), Matrix::MakeTranslation3(), etc.
 */

namespace {

const Matrix matA = Matrix(Vector(-10.0f, 8.0f, -8.0f,  9.0f),
                           Vector(-2.0f,  4.0f,  7.0f,  5.0f),
                           Vector(2.0f,   9.0f, -1.0f,  3.0f),
                           Vector(-3.0f, -1.0f,  8.0f, -9.0f));

const Matrix matB = Matrix(Vector(4.0f,   9.0f, -2.0f,  2.0f),
                           Vector(-10.0f, 6.0f, -9.0f, -9.0f),
                           Vector(2.0f,  -2.0f,  6.0f, -8.0f),
                           Vector(-9.0f,  3.0f, -9.0f,  1.0f));

const Matrix matI = Matrix(Vector(1.0f, 0.0f, 0.0f, 0.0f),
                           Vector(0.0f, 1.0f, 0.0f, 0.0f),
                           Vector(0.0f, 0.0f, 1.0f, 0.0f),
                           Vector(0.0f, 0.0f, 0.0f, 1.0f));

} // namespace

TEST(MathMatrix, Simple)
{
    const Matrix matTransposeA = Matrix(Vector(-10.0f, -2.0f, 2.0f, -3.0f),
                                        Vector(8.0f, 4.0f, 9.0f, -1.0f),
                                        Vector(-8.0f, 7.0f, -1.0f, 8.0f),
                                        Vector(9.0f, 5.0f, 3.0f, -9.0f));

    EXPECT_TRUE(matA[2][2] == -1.0f); // rows access
    EXPECT_FALSE(matA == matB);
    EXPECT_TRUE(matI == Matrix());
    EXPECT_TRUE(matTransposeA == matA.Transposed());
}

TEST(MathMatrix, Arithmetics)
{
    const Matrix matAB = Matrix(Vector(-217.0f, 1.0f,  -181.0f, -19.0f),
                                Vector(-79.0f,  7.0f,  -35.0f,  -91.0f),
                                Vector(-111.0f, 83.0f, -118.0f, -66.0f),
                                Vector(95.0f,  -76.0f,  144.0f, -70.0f));
    const Vector vecA = Vector(1.0f, 2.0f, 3.0f, 4.0f);

    // expected product of matA and vecA
    const Vector matAvecA = Vector(-20.0f, 39.0f, 35.0f, -8.0f);
    const Vector matAvecAaffine = Vector(-11.0f, 42.0f, 11.0f, 19.0f);

    EXPECT_TRUE(matAB == matA * matB);
    EXPECT_TRUE(Matrix::Equal(matI, matA * matA.Inverted(), NFE_MATH_EPSILON));
    EXPECT_TRUE(Vector::AlmostEqual(matAvecA, vecA * matA));
    EXPECT_TRUE(Vector::Abs(matAvecAaffine - matA.LinearCombination3(vecA)) <
                Vector::Splat(NFE_MATH_EPSILON));
}
