/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class definition.
 */

#pragma once

#include "PCH.hpp"
#include "Sphere.hpp"


namespace NFE {
namespace Math {


bool Sphere::ConstructFromPoints(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, Sphere& outSphere)
{
    // prepare linear equations system
    float matrix[4][3];

    matrix[0][0] = 2.0f * (p2.f[0] - p1.f[0]);
    matrix[1][0] = 2.0f * (p2.f[1] - p1.f[1]);
    matrix[2][0] = 2.0f * (p2.f[2] - p1.f[2]);

    matrix[0][1] = 2.0f * (p3.f[0] - p1.f[0]);
    matrix[1][1] = 2.0f * (p3.f[1] - p1.f[1]);
    matrix[2][1] = 2.0f * (p3.f[2] - p1.f[2]);

    matrix[0][2] = 2.0f * (p4.f[0] - p1.f[0]);
    matrix[1][2] = 2.0f * (p4.f[1] - p1.f[1]);
    matrix[2][2] = 2.0f * (p4.f[2] - p1.f[2]);

    const float tmp = Vector::Dot3(p1, p1);
    matrix[3][0] = Vector::Dot3(p2, p2) - tmp;
    matrix[3][1] = Vector::Dot3(p3, p3) - tmp;
    matrix[3][2] = Vector::Dot3(p4, p4) - tmp;

    if (!SolveEquationsSystem3(matrix, outSphere.origin.f[0], outSphere.origin.f[1], outSphere.origin.f[2]))
    {
        return false;
    }

    outSphere.r = (outSphere.origin - p1).Length3();
    return true;
}

} // namespace Math
} // namespace NFE
