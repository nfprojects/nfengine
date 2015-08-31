/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frustum class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Box.hpp"

namespace NFE {
namespace Math {

/**
 * Frustum
 */
class NFE_ALIGN16 Frustum
{
public:
    /// planes IDs
    enum Plane
    {
        Front = 0,
        Back,
        Left,
        Right,
        Bottom,
        Top
    };

    Vector planes[6];
    Vector verticies[8];
    Box boundingBox;

    /**
     * Construct frustum of perspective view.
     * @param origin   Camera position
     * @param xAxis    Horizontal extent at distance equal to 1.
     * @param yAxis    Vertical extent at distance equal to 1.
     * @param zAxis    Normalized frustum direction.
     * @param nearDist Near plane distance.
     * @param farDist  Far plane distance.
     */
    NFE_INLINE void Construct(const Vector& origin, const Vector& xAxis, const Vector& yAxis,
                              const Vector& zAxis, float nearDist, float farDist);

    /**
      * Recalculate frustum planes' equations.
      */
    NFE_INLINE void CalculatePlanes();
    NFE_INLINE float SupportVertex(const Vector& dir) const;
};


void Frustum::CalculatePlanes()
{
    planes[Front]  = PlaneFromPoints(verticies[0], verticies[1], verticies[3]);
    planes[Back]   = PlaneFromPoints(verticies[7], verticies[5], verticies[4]);
    planes[Left]   = PlaneFromPoints(verticies[4], verticies[0], verticies[6]);
    planes[Right]  = PlaneFromPoints(verticies[7], verticies[3], verticies[5]);
    planes[Bottom] = PlaneFromPoints(verticies[5], verticies[1], verticies[4]);
    planes[Top]    = PlaneFromPoints(verticies[6], verticies[3], verticies[7]);

    boundingBox.MakeFromPoints(verticies, 8);
}

float Frustum::SupportVertex(const Vector& dir) const
{
    Vector d = VectorDot3(dir, verticies[0]);

    for (int i = 1; i < 8; i++)
    {
        Vector tmp_d = VectorDot3(dir, verticies[i]);
        if (_mm_comigt_ss(tmp_d, d))
        {
            d = tmp_d;
        }
    }

    return d.f[0];
}

void Frustum::Construct(const Vector& origin, const Vector& xAxis, const Vector& yAxis,
                        const Vector& zAxis, float nearDist, float farDist)
{
    verticies[0] = origin + nearDist * (zAxis - xAxis - yAxis);
    verticies[1] = origin + nearDist * (zAxis + xAxis - yAxis);
    verticies[2] = origin + nearDist * (zAxis - xAxis + yAxis);
    verticies[3] = origin + nearDist * (zAxis + xAxis + yAxis);
    verticies[4] = origin + farDist * (zAxis - xAxis - yAxis);
    verticies[5] = origin + farDist * (zAxis + xAxis - yAxis);
    verticies[6] = origin + farDist * (zAxis - xAxis + yAxis);
    verticies[7] = origin + farDist * (zAxis + xAxis + yAxis);

    CalculatePlanes();
}

} // namespace Math
} // namespace NFE
