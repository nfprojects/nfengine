/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Equation solver class declarations.
 */

#pragma once

#include "Math.hpp"


namespace NFE {
namespace Math {


class EquationSolver
{
public:

    /**
     * Solve quadratic equation.
     *
     * @param       a,b,c   Polynomial coefficients.
     * @param[out]  x0,x1   Result (roots).
     *
     * @return      Number of real roots.
     */
    NFCOMMON_API static uint32 SolveQuadraticEquation(float a, float b, float c,
                                                      float& x0, float& x1);

    /**
     * Solve cubic equation.
     * @param       a,b,c,d     Polynomial coefficients.
     * @param[out]  x0,x1,x2    Result (roots).
     * @param[out]  x0im,x1im   Imaginary part of second and third root (optional).
     *
     * @return      0 if the cubic equation is degenerated (can be transformed to quadratic equation, e.g. 'a' or 'd' is zero).
     *              1 if the cubic equation has one real root and two complex roots (the only case when x0im,x1im are filled).
     *              2 if the cubic equation has at least one double root (only x0 and x1 are written then). This also covers triple root case.
     *              3 if the cubic equation has three unique real roots.
     *
     * @remarks     This function is very slow.
     */
    NFCOMMON_API static uint32 SolveCubicEquation(float a, float b, float c, float d,
                                                  float& x0, float& x1, float& x2, float* x1im = nullptr, float* x2im = nullptr);
    NFCOMMON_API static uint32 SolveCubicEquation(double a, double b, double c, double d,
                                                  double& x0, double& x1, double& x2, double* x1im = nullptr, double* x2im = nullptr);

    /**
     * Solve system of linear equations with two variables.
     *
     * @param   a       Coefficients matrix.
     * @param   b       Coefficients vector.
     * @param   outX    Result - vector 'x' that satisfies equation a * b == x
     *
     * @return  True if the equations system has exactly one solution.
     */
    NFCOMMON_API static bool SolveLinearEquationsSystem2(const Matrix2& a, const Float2& b, Float2& outX);

    /**
     * Solve system of linear equations with three variables.
     *
     * @param   a       Coefficients matrix.
     * @param   b       Coefficients vector.
     * @param   outX    Result - vector 'x' that satisfies equation a * b == x
     *
     * @return  True if the equations system has exactly one solution.
     */
    NFCOMMON_API static bool SolveLinearEquationsSystem3(const Matrix3& a, const Float3& b, Float3& outX);

    /**
     * Solve system of linear equations with four variables.
     *
     * @param   a       Coefficients matrix.
     * @param   b       Coefficients vector.
     * @param   outX    Result - vector 'x' that satisfies equation a * b == x
     *
     * @return  True if the equations system has exactly one solution.
     */
    NFCOMMON_API static bool SolveLinearEquationsSystem4(const Matrix4& a, const Vector4& b, Vector4& outX);
};


} // namespace Math
} // namespace NFE
