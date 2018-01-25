/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frustum class definitions.
 */

#include "PCH.hpp"
#include "Frustum.hpp"

namespace NFE {
namespace Math {

Frustum Frustum::ConstructForPerspective(const Matrix& matrix, float nearDist, float farDist, float cutoff, float aspect)
{
    const Vector pos = matrix.GetRow(3) & VECTOR_MASK_XYZ;
    const float scale = tanf(cutoff / 2.0f);
    const Vector xAxis = aspect * scale * matrix.GetRow(0);
    const Vector yAxis = scale * matrix.GetRow(1);
    const Vector zAxis = scale * matrix.GetRow(2);

    return ConstructForPerspective(pos, xAxis, yAxis, zAxis, nearDist, farDist);
}

Frustum Frustum::ConstructForPerspective(const Vector& origin,
                                         const Vector& xAxis, const Vector& yAxis, const Vector& zAxis,
                                         float nearDist, float farDist)
{
    Frustum frustum;

    frustum.verticies[0] = origin + nearDist * (zAxis - xAxis - yAxis);
    frustum.verticies[1] = origin + nearDist * (zAxis + xAxis - yAxis);
    frustum.verticies[2] = origin + nearDist * (zAxis - xAxis + yAxis);
    frustum.verticies[3] = origin + nearDist * (zAxis + xAxis + yAxis);
    frustum.verticies[4] = origin + farDist * (zAxis - xAxis - yAxis);
    frustum.verticies[5] = origin + farDist * (zAxis + xAxis - yAxis);
    frustum.verticies[6] = origin + farDist * (zAxis - xAxis + yAxis);
    frustum.verticies[7] = origin + farDist * (zAxis + xAxis + yAxis);

    frustum.CalculatePlanes();
    return frustum;
}

void Frustum::CalculatePlanes()
{
    planes[Front]   = Plane(verticies[0], verticies[1], verticies[3]);
    planes[Back]    = Plane(verticies[7], verticies[5], verticies[4]);
    planes[Left]    = Plane(verticies[4], verticies[0], verticies[6]);
    planes[Right]   = Plane(verticies[7], verticies[3], verticies[5]);
    planes[Bottom]  = Plane(verticies[5], verticies[1], verticies[4]);
    planes[Top]     = Plane(verticies[6], verticies[3], verticies[7]);

    boundingBox = Box(verticies[0]);
    for (uint32 i = 1; i < 8; ++i)
    {
        boundingBox.AddPoint(verticies[i]);
    }
}

float Frustum::SupportVertex(const Vector& dir) const
{
    float d = Vector::Dot3(dir, verticies[0]);
    for (int i = 1; i < 8; i++)
    {
        float tmp_d = Vector::Dot3(dir, verticies[i]);
        if (tmp_d > d)
            d = tmp_d;
    }
    return d;
}

} // namespace Math
} // namespace NFE
