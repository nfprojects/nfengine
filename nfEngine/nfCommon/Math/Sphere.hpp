/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

/**
 * Sphere
 */
class NFE_ALIGN16 Sphere
{
public:
    Vector origin; //< Sphere center
    float r; //< Sphere radius

    NFE_INLINE Sphere() : origin(), r() {}
    NFE_INLINE Sphere(const Vector& origin, float r) : origin(origin), r(r) {}

    /**
     * Construct a sphere intersecting four points.
     */
    NFE_INLINE Sphere(const Vector& P1, const Vector& P2, const Vector& P3, const Vector& P4)
    {
        float matrix[4][3];

        matrix[0][0] = 2.0f * (P2.f[0] - P1.f[0]);
        matrix[1][0] = 2.0f * (P2.f[1] - P1.f[1]);
        matrix[2][0] = 2.0f * (P2.f[2] - P1.f[2]);

        matrix[0][1] = 2.0f * (P3.f[0] - P1.f[0]);
        matrix[1][1] = 2.0f * (P3.f[1] - P1.f[1]);
        matrix[2][1] = 2.0f * (P3.f[2] - P1.f[2]);

        matrix[0][2] = 2.0f * (P4.f[0] - P1.f[0]);
        matrix[1][2] = 2.0f * (P4.f[1] - P1.f[1]);
        matrix[2][2] = 2.0f * (P4.f[2] - P1.f[2]);

        float tmp = VectorDot3(P1, P1)[0];
        matrix[3][0] = VectorDot3(P2, P2)[0] - tmp;
        matrix[3][1] = VectorDot3(P3, P3)[0] - tmp;
        matrix[3][2] = VectorDot3(P4, P4)[0] - tmp;

        SolveEquationsSystem3(matrix, origin.f[0], origin.f[1], origin.f[2]);
        r = VectorLength3(origin - P1).f[0];
    }

    NFE_INLINE float SupportVertex(const Vector& dir) const
    {
        Vector pos = origin + r * dir;
        return VectorDot3(dir, pos).f[0];
    }
};

} // namespace Math
} // namespace NFE
