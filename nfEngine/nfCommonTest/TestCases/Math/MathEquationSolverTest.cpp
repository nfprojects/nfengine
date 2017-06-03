#include "PCH.hpp"
#include "nfCommon/Math/EquationSolver.hpp"
#include "nfCommon/Math/Matrix3.hpp"


using namespace NFE::Math;


static const float maxError = 1.0e-5f;


TEST(MathEquationSolver, QuadraticEq_2_solutions)
{
    float x1, x2;
    ASSERT_EQ(2, EquationSolver::SolveQuadraticEquation(1.0f, 1.0f, -6.0f, x1, x2));
    EXPECT_FLOAT_EQ(-3.0f, Min(x1, x2));
    EXPECT_FLOAT_EQ(2.0f, Max(x1, x2));
}

TEST(MathEquationSolver, QuadraticEq_1_solution)
{
    float x1, x2;
    ASSERT_EQ(1, EquationSolver::SolveQuadraticEquation(3.0f, 30.0f, 75.0f, x1, x2));
    EXPECT_EQ(-5.0f, x1);
}

TEST(MathEquationSolver, QuadraticEq_0_solutions)
{
    float x1, x2;
    ASSERT_EQ(0, EquationSolver::SolveQuadraticEquation(1.0f, -1.0f, 3.0f, x1, x2));
    ASSERT_EQ(0, EquationSolver::SolveQuadraticEquation(1.0f, 0.0f, 3.0f, x1, x2));
    ASSERT_EQ(0, EquationSolver::SolveQuadraticEquation(-1.0f, 0.0f, -3.0f, x1, x2));
}

//////////////////////////////////////////////////////////////////////////

TEST(MathEquationSolver, CubicEq_3_real_solutions)
{
    float x0, x1, x2;

    // solve x^3 - 6x^2 + 11x - 6 = 0
    ASSERT_EQ(3, EquationSolver::SolveCubicEquation(1.0f, -6.0f, 11.0f, -6.0f, x0, x1, x2));

    // x0 = 1
    // x1 = 2
    // x2 = 3
    EXPECT_NEAR(1.0f, Min(x0, x1, x2), maxError);
    EXPECT_NEAR(2.0f, Median(x0, x1, x2), maxError);
    EXPECT_NEAR(3.0f, Max(x0, x1, x2), maxError);
}

TEST(MathEquationSolver, CubicEq_tripple_root)
{
    float x0, x1, x2;

    // solve 2x^2 - 6x^2 + 6x - 2 = 0
    ASSERT_LT(0u, EquationSolver::SolveCubicEquation(2.0f, -6.0f, 6.0f, -2.0f, x0, x1, x2));

    // x0 = x1 = x2 = 1
    EXPECT_NEAR(1.0f, x0, maxError);
    EXPECT_NEAR(1.0f, x1, maxError);
    EXPECT_NEAR(1.0f, x2, maxError);
}

TEST(MathEquationSolver, CubicEq_2_real_solutions)
{
    float x0, x1, x2;

    // solve x^3 - 5x^2 + 7x - 3 = 0
    ASSERT_LT(0u, EquationSolver::SolveCubicEquation(1.0f, -5.0f, 7.0f, -3.0f, x0, x1, x2));

    // x0 = 1
    // x1 = x2 = 3
    EXPECT_NEAR(1.0f, Min(x0, x1, x2), maxError);
    EXPECT_NEAR(3.0f, Max(x0, x1, x2), maxError);
}

TEST(MathEquationSolver, CubicEq_1_real_solution)
{
    float x0, x1, x2;
    float x1im, x2im;

    // solve x^3 - 7x^2 + 16x - 10 = 0
    ASSERT_EQ(1, EquationSolver::SolveCubicEquation(1.0f, -7.0f, 16.0f, -10.0f, x0, x1, x2, &x1im, &x2im));

    // x0 = 1
    // x1 = 3 + i
    // x2 = 3 - i
    EXPECT_NEAR(1.0f, x0, maxError);
    EXPECT_NEAR(3.0f, x1, maxError);
    EXPECT_NEAR(3.0f, x2, maxError);
    EXPECT_NEAR(1.0f, Max(x1im, x2im), maxError);
    EXPECT_NEAR(-1.0f, Min(x1im, x2im), maxError);
}

//////////////////////////////////////////////////////////////////////////

TEST(MathEquationSolver, LinearEquationsSystem3)
{
    // Solve this:
    //  x +  y +  z = 25
    // 5x + 2y + 2z = 1
    // 2x +  y -  z = 6

    const Matrix3 m(
        Float3(1.0f, 1.0f, 1.0f),
        Float3(5.0f, 2.0f, 2.0f),
        Float3(2.0f, 1.0f, -1.0f)
    );
    const Float3 b(25.0f, 1.0f, 6.0f);

    Float3 x;
    ASSERT_TRUE(EquationSolver::SolveLinearEquationsSystem3(m, b, x));

    EXPECT_FLOAT_EQ(-49.0f / 3.0f, x.x);
    EXPECT_FLOAT_EQ(40.0f, x.y);
    EXPECT_FLOAT_EQ(4.0f / 3.0f, x.z);
}

TEST(MathEquationSolver, LinearEquationsSystem3_Invalid)
{
    const Matrix3 m(
        Float3(1.0f, 1.0f, 2.0f),
        Float3(5.0f, 2.0f, 2.0f),
        Float3(12.0f, 6.0f, 8.0f)
    );
    const Float3 b(25.0f, 1.0f, 6.0f);

    Float3 x;
    ASSERT_FALSE(EquationSolver::SolveLinearEquationsSystem3(m, b, x));
}

//////////////////////////////////////////////////////////////////////////