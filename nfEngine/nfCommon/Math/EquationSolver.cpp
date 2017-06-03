/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Equation solver class definitions.
 */

#pragma once

#include "PCH.hpp"
#include "EquationSolver.hpp"


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
    // code based on:
    // https://www.easycalculation.com/algebra/cubic-equation.php

    if (Abs(a) < NFE_MATH_EPSILON)
    {
        // degenerate case: cubic equation can be simplified to quadratic equation
        return SolveQuadraticEquation(b, c, d, x0, x1);
    }

    if (Abs(d) < NFE_MATH_EPSILON)
    {
        // degenerate case: one root is equal to zero - equation can be simplified to quadratic by dividing by 'x'
        x0 = 0.0f;
        // TODO handle complex roots
        return 1 + SolveQuadraticEquation(a, b, c, x1, x2);
    }

    const float invA = a;
    b *= invA;
    c *= invA;
    d *= invA;

    float q = (3.0f * c - (b * b)) / 9.0f;
    const float r = -(27.0f * d) + b * (9.0f * c - 2.0f * (b * b)) / 54.0f;
    const float disc = q * q * q + r * r; // discriminant
    float term1 = b / 3.0f;

    // one real root + two complex roots
    if (disc > 0.0f)
    {
        float s = r + sqrtf(disc);
        float t = r - sqrtf(disc);

        s = ((s < 0.0f) ? -powf(-s, (1.0f / 3.0f)) : powf(s, (1.0f / 3.0f)));
        t = ((t < 0.0f) ? -powf(-t, (1.0f / 3.0f)) : powf(t, (1.0f / 3.0f)));

        x0 = -term1 + s + t;
        term1 += (s + t) / 2.0f;
        x1 = -term1;
        x2 = -term1;

        // calculate imaginary part of second and third root (optional)
        if (x1im || x2im)
        {
            const float sqrt3_div_2 = 0.86602540378f; // sqrt(3) / 2
            term1 = sqrt3_div_2 * (-t + s);

            if (x1im)
                *x1im = term1;

            if (x2im)
                *x2im = -term1;
        }

        return 1;
    }

    // one unique real root + two double real roots
    if (Abs(disc) < NFE_MATH_EPSILON)
    {
        float r13;
        if (r < 0.0f)
            r13 = -powf(-r, 1.0f / 3.0f);
        else
            r13 = powf(r, 1.0f / 3.0f);

        x0 = -term1 + 2.0f * r13;
        x1 = -(r13 + term1);

        return 2;
    }

    // all three roots are real and not equal to self
    q = -q;
    float dum1 = q * q * q;
    dum1 = acosf(r / sqrtf(dum1));
    float r13 = 2.0f * sqrtf(q);
    x0 = -term1 + r13 * cosf(dum1 / 3.0f);
    x1 = -term1 + r13 * cosf((dum1 + 2.0f * NFE_MATH_PI) / 3.0f);
    x2 = -term1 + r13 * cosf((dum1 + 4.0f * NFE_MATH_PI) / 3.0f);

    return 3;
}

//////////////////////////////////////////////////////////////////////////

LinearEquationType EquationSolver::SolveLinearEquationsSystem2(const Matrix2& a, const Float2& b, Float2& outX)
{
    // TODO

    UNUSED(a);
    UNUSED(b);
    UNUSED(outX);
    return LinearEquationType::Consistent;
}

LinearEquationType EquationSolver::SolveLinearEquationsSystem3(const Matrix3& a, const Float3& b, Float3& outX)
{
    // TODO

    UNUSED(a);
    UNUSED(b);
    UNUSED(outX);
    return LinearEquationType::Consistent;
}

LinearEquationType EquationSolver::SolveLinearEquationsSystem4(const Matrix& a, const Vector& b, Vector& outX)
{
    // TODO

    UNUSED(a);
    UNUSED(b);
    UNUSED(outX);
    return LinearEquationType::Consistent;
}

} // namespace Math
} // namespace NFE
