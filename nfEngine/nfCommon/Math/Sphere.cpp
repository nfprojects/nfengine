/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class definition.
 */

#include "PCH.hpp"
#include "Sphere.hpp"
#include "EquationSolver.hpp"
#include "Matrix3.hpp"


namespace NFE {
namespace Math {


bool Sphere::ConstructFromPoints(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, Sphere& outSphere)
{
    Matrix3 a;
    Float3 b, x;

    // prepare linear equations system
    a.m[0][0] = p2.f[0] - p1.f[0];
    a.m[1][0] = p2.f[1] - p1.f[1];
    a.m[2][0] = p2.f[2] - p1.f[2];
    a.m[0][1] = p3.f[0] - p1.f[0];
    a.m[1][1] = p3.f[1] - p1.f[1];
    a.m[2][1] = p3.f[2] - p1.f[2];
    a.m[0][2] = p4.f[0] - p1.f[0];
    a.m[1][2] = p4.f[1] - p1.f[1];
    a.m[2][2] = p4.f[2] - p1.f[2];

    a *= 2.0f;

    const float tmp = Vector::Dot3(p1, p1);
    b.x = Vector::Dot3(p2, p2) - tmp;
    b.y = Vector::Dot3(p3, p3) - tmp;
    b.z = Vector::Dot3(p4, p4) - tmp;

    if (!EquationSolver::SolveLinearEquationsSystem3(a, b, x))
    {
        return false;
    }

    outSphere.origin = Vector(x);
    outSphere.r = (outSphere.origin - p1).Length3();
    return true;
}

} // namespace Math
} // namespace NFE
