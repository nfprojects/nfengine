/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class definition.
 */

#include "PCH.hpp"
#include "Sphere.hpp"
#include "Matrix3.hpp"
#include "EquationSolver.hpp"


namespace NFE {
namespace Math {


bool Sphere::ConstructFromPoints(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, Sphere& outSphere)
{
    // prepare linear equations system
    Matrix3 a;

    a[0][0] = 2.0f * (p2.f[0] - p1.f[0]);
    a[1][0] = 2.0f * (p2.f[1] - p1.f[1]);
    a[2][0] = 2.0f * (p2.f[2] - p1.f[2]);

    a[0][1] = 2.0f * (p3.f[0] - p1.f[0]);
    a[1][1] = 2.0f * (p3.f[1] - p1.f[1]);
    a[2][1] = 2.0f * (p3.f[2] - p1.f[2]);

    a[0][2] = 2.0f * (p4.f[0] - p1.f[0]);
    a[1][2] = 2.0f * (p4.f[1] - p1.f[1]);
    a[2][2] = 2.0f * (p4.f[2] - p1.f[2]);

    const float tmp = Vector::Dot3(p1, p1);
    Float3 b;
    b.x = Vector::Dot3(p2, p2) - tmp;
    b.y = Vector::Dot3(p3, p3) - tmp;
    b.z = Vector::Dot3(p4, p4) - tmp;

    Float3 sphereOrigin;
    if (!EquationSolver::SolveLinearEquationsSystem3(a, b, sphereOrigin))
    {
        return false;
    }

    const float sphereRadius = (Vector(outSphere.origin) - p1).Length3();
    outSphere = Sphere(sphereOrigin, sphereRadius);
    return true;
}

} // namespace Math
} // namespace NFE
