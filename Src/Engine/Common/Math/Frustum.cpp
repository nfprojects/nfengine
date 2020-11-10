/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frustum class definitions.
 */

#include "PCH.hpp"
#include "Frustum.hpp"

namespace NFE {
namespace Math {

Frustum Frustum::ConstructForPerspective(const Matrix4& matrix, float nearDist, float farDist, float cutoff, float aspect)
{
    const Vec4f pos = matrix[3] & Vec4f::MakeMask<1,1,1,0>();
    const float scale = tanf(cutoff / 2.0f);
    const Vec4f xAxis = aspect * scale * matrix[0];
    const Vec4f yAxis = scale * matrix[1];
    const Vec4f zAxis = scale * matrix[2];

    return ConstructForPerspective(pos, xAxis, yAxis, zAxis, nearDist, farDist);
}

Frustum Frustum::ConstructForPerspective(const Vec4f& origin,
                                         const Vec4f& xAxis, const Vec4f& yAxis, const Vec4f& zAxis,
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

float Frustum::SupportVertex(const Vec4f& dir) const
{
    float d = Vec4f::Dot3(dir, verticies[0]);
    for (int i = 1; i < 8; i++)
    {
        float tmp_d = Vec4f::Dot3(dir, verticies[i]);
        if (tmp_d > d)
        {
            d = tmp_d;
        }
    }
    return d;
}

} // namespace Math
} // namespace NFE
