/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Equation solver class declarations.
 */

#pragma once

#include "Math.hpp"


namespace NFE {
namespace Math {


/**
 * Type of linear equations system.
 */
enum class LinearEquationType
{
    Consistent,     // there is one solution
    Inconsistent,   // there is no solutions
    Dependent,      // there is infinity number of solutions, because the given equations are dependent on each other
};


class EquationSolver
{
    /**
     * Solve quadratic equation.
     * @param       a,b,c   Polynomial coefficients.
     * @param[out]  x0,x1   Result (roots).
     * @return      Number of real roots.
     */
    NFCOMMON_API static uint32 SolveQuadraticEquation(float a, float b, float c,
                                                      float& x0, float& x1);

    /**
     * Solve cubic equation.
     * @param       a,b,c,d     Polynomial coefficients.
     * @param[out]  x0,x1,x2    Result (roots).
     * @param[out]  x0im,x1im   Imaginary part of second and third root (optional).
     * @return      Number of real roots.
     * @remarks     This function is very slow.
     */
    NFCOMMON_API static uint32 SolveCubicEquation(float a, float b, float c, float d,
                                                  float& x0, float& x1, float& x2, float* x1im = nullptr, float* x2im = nullptr);

    /**
     * Solve system of linear equations with two variables.
     * @param   a       Coefficients matrix.
     * @param   b       Coefficients vector.
     * @param   outX    Result - vector 'x' that satisfies equation a * b == x
     * @return  Type of the equations system (number of solutions found).
     */
    NFCOMMON_API static LinearEquationType SolveLinearEquationsSystem2(const Matrix2& a, const Float2& b, Float2& outX);

    /**
     * Solve system of linear equations with three variables.
     * @param   a       Coefficients matrix.
     * @param   b       Coefficients vector.
     * @param   outX    Result - vector 'x' that satisfies equation a * b == x
     * @return  Type of the equations system (number of solutions found).
     */
    NFCOMMON_API static LinearEquationType SolveLinearEquationsSystem3(const Matrix3& a, const Float3& b, Float3& outX);

    /**
     * Solve system of linear equations with four variables.
     * @param   a       Coefficients matrix.
     * @param   b       Coefficients vector.
     * @param   outX    Result - vector 'x' that satisfies equation a * b == x
     * @return  Type of the equations system (number of solutions found).
     */
    NFCOMMON_API static LinearEquationType SolveLinearEquationsSystem4(const Matrix& a, const Vector& b, Vector& outX);
};


} // namespace Math
} // namespace NFE
