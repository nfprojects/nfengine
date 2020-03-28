/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Plane class definition.
 */

#pragma once

#include "Plane.hpp"

namespace NFE {
namespace Math {


Plane::Plane(const Vec4f& normal, const float distance)
    : v{ normal.x, normal.y, normal.z, distance }
{
}

Plane::Plane(const Vec4f& p0, const Vec4f& p1, const Vec4f& p2)
{
    const Vec4f vA = p0 - p1;
    const Vec4f vB = p0 - p2;
    const Vec4f n = Vec4f::Cross3(vA, vB).Normalized3();
    const float d = Vec4f::Dot3(n, p1);
    v = Vec4f(n.x, n.y, n.z, -d);
}

Plane::Plane(const Vec4f& normal, const Vec4f& point)
{
    const float d = Vec4f::Dot3(normal, point);
    v = Vec4f(normal.x, normal.y, normal.z, -d);
}

float Plane::PointDistance(const Vec4f& p) const
{
    return Vec4f::Dot3(v, p) + v.w;
}

Vec4f Plane::PointDistanceV(const Vec4f& p) const
{
    return Vec4f::Dot3V(v, p) + v.SplatW();
}

bool Plane::Side(const Vec4f& p) const
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
