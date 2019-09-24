/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Plane class definition.
 */

#pragma once

#include "Plane.hpp"

namespace NFE {
namespace Math {


Plane::Plane(const Vector4& normal, const float distance)
    : v{ normal.x, normal.y, normal.z, distance }
{
}

Plane::Plane(const Vector4& p0, const Vector4& p1, const Vector4& p2)
{
    const Vector4 vA = p0 - p1;
    const Vector4 vB = p0 - p2;
    const Vector4 n = Vector4::Cross3(vA, vB).Normalized3();
    const float d = Vector4::Dot3(n, p1);
    v = Vector4(n.x, n.y, n.z, -d);
}

Plane::Plane(const Vector4& normal, const Vector4& point)
{
    const float d = Vector4::Dot3(normal, point);
    v = Vector4(normal.x, normal.y, normal.z, -d);
}

float Plane::PointDistance(const Vector4& p) const
{
    return Vector4::Dot3(v, p) + v.w;
}

Vector4 Plane::PointDistanceV(const Vector4& p) const
{
    return Vector4::Dot3V(v, p) + v.SplatW();
}

bool Plane::Side(const Vector4& p) const
{
    return PointDistance(p) > 0.0f;
}

bool Plane::operator == (const Plane& rhs) const
{
    return (v == rhs.v).All();
}

bool Plane::operator != (const Plane& rhs) const
{
    return (v != rhs.v).All();
}

} // namespace Math
} // namespace NFE
