/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Equation solver class definitions.
 */

#pragma once

#include "PCH.hpp"
#include "EquationSolver.hpp"
#include "Matrix2.hpp"
#include "Matrix3.hpp"
#include "Matrix.hpp"


namespace NFE {
namespace Math {


uint32 EquationSolver::SolveQuadraticEquation(float a, float b, float c,
                                              float& x0, float& x1)
{
    const float delta = b * b - 4.0f * a * c;
    if (delta < 0.0f) // zero real solutions
    {
        // TODO handle complex roots
        return 0;
    }

    if (delta < NFE_MATH_EPSILON) // one solution (double root)
    {
        x0 = -0.5f * b / a;
        return 1;
    }

    const float invA = 1.0f / a;
    const float sqrtDelta = sqrtf(delta);
    x0 = 0.5f * a * (-b - sqrtDelta);
    x1 = 0.5f * a * (-b + sqrtDelta);
    return 2;
}

uint32 EquationSolver::SolveCubicEquation(float a, float b, float c, float d,
                                          float& x0, float& x1, float& x2, float* x1im, float* x2im)
{
    const double aD = static_cast<double>(a);
    const double bD = static_cast<double>(b);
    const double cD = static_cast<double>(c);
    const double dD = static_cast<double>(d);

    double x0D, x1D, x2D, x1imD = 0.0, x2imD = 0.0;
    uint32 ret = SolveCubicEquation(aD, bD, cD, dD, x0D, x1D, x2D, &x1imD, &x2imD);

    if (x1im)
        *x1im = static_cast<float>(x1imD);
    if (x2im)
        *x2im = static_cast<float>(x2imD);

    x0 = static_cast<float>(x0D);
    x1 = static_cast<float>(x1D);
    x2 = static_cast<float>(x2D);

    return ret;
}

uint32 EquationSolver::SolveCubicEquation(double a, double b, double c, double d,
                                          double& x0, double& x1, double& x2, double* x1im, double* x2im)
{
    // code based on:
    // https://www.easycalculation.com/algebra/cubic-equation.php

    const double epsilon = 1.0e-10;

    if (Abs(a) < epsilon)
    {
        // degenerate case: cubic equation can be simplified to quadratic equation
        return 0;
    }

    if (Abs(d) < epsilon)
    {
        // degenerate case: one root is equal to zero - equation can be simplified to quadratic by dividing by 'x'
        return 0;
    }

    const double invA = 1.0 / a;
    b *= invA;
    c *= invA;
    d *= invA;

    double q = (3.0 * c - (b * b)) / 9.0;
    const double r = (-27.0 / 54.0) * d + b * ((9.0 / 54.0) * c - (2.0 / 54.0) * (b * b));
    const double disc = q * q * q + r * r; // discriminant
    double term1 = b / 3.0;

    // one real root + two complex roots
    if (disc > 0.0)
    {
        double s = r + sqrt(disc);
        double t = r - sqrt(disc);
        s = (s < 0.0) ? -pow(-s, (1.0 / 3.0)) : pow(s, (1.0 / 3.0));
        t = (t < 0.0) ? -pow(-t, (1.0 / 3.0)) : pow(t, (1.0 / 3.0));

        x0 = -term1 + s + t;
        term1 += (s + t) / 2.0;
        x1 = -term1;
        x2 = -term1;

        // calculate imaginary part of second and third root
        const double sqrt3_div_2 = 0.86602540378443864676; // sqrt(3) / 2
        term1 = sqrt3_div_2 * (-t + s);

        if (x1im)
            *x1im = term1;

        if (x2im)
            *x2im = -term1;

        return 1;
    }

    // one unique real root + two double real roots
    if (Abs(disc) < epsilon)
    {
        double r13;
        if (r < 0.0)
            r13 = -pow(-r, 1.0 / 3.0);
        else
            r13 = pow(r, 1.0 / 3.0);

        x0 = -term1 + 2.0 * r13;
        x1 = -(r13 + term1);
        x2 = x1;

        return 2;
    }

    // all three roots are real and not equal to self
    q = -q;
    double dum1 = q * q * q;
    dum1 = acos(r / sqrt(dum1));
    double r13 = 2.0 * sqrt(q);

    x0 = -term1 + r13 * cos(dum1 / 3.0);
    x1 = -term1 + r13 * cos((dum1 + 2.0 * Constants::pi<double>) / 3.0);
    x2 = -term1 + r13 * cos((dum1 + 4.0 * Constants::pi<double>) / 3.0);

    return 3;
}

//////////////////////////////////////////////////////////////////////////

bool EquationSolver::SolveLinearEquationsSystem2(const Matrix2& a, const Float2& b, Float2& outX)
{
    Matrix2 aInverted;
    if (!a.Invert(aInverted))
        return false;

    outX = b * aInverted.Transposed();
    return true;
}

bool EquationSolver::SolveLinearEquationsSystem3(const Matrix3& a, const Float3& b, Float3& outX)
{
    Matrix3 aInverted;
    if (!a.Invert(aInverted))
        return false;

    outX = b * aInverted.Transposed();
    return true;
}

bool EquationSolver::SolveLinearEquationsSystem4(const Matrix& a, const Vector& b, Vector& outX)
{
    // TODO

    UNUSED(a);
    UNUSED(b);
    UNUSED(outX);
    return true;
}

} // namespace Math
} // namespace NFE
