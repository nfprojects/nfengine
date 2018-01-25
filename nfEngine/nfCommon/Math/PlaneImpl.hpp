/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Plane class definition.
 */

#pragma once

#include "Plane.hpp"

namespace NFE {
namespace Math {


Plane::Plane(const Vector& normal, const float distance)
    : v{ normal.x, normal.y, normal.z, distance }
{
}

Plane::Plane(const Vector& p0, const Vector& p1, const Vector& p2)
{
    const Vector vA = p0 - p1;
    const Vector vB = p0 - p2;
    const Vector n = Vector::Cross3(vA, vB).Normalized3();
    const float d = Vector::Dot3(n, p1);
    v = Vector(n.x, n.y, n.z, -d);
}

Plane::Plane(const Vector& normal, const Vector& point)
{
    const float d = Vector::Dot3(normal, point);
    v = Vector(normal.x, normal.y, normal.z, -d);
}

float Plane::PointDistance(const Vector& p) const
{
    return Vector::Dot3(v, p) + v.w;
}

Vector Plane::PointDistanceV(const Vector& p) const
{
    return Vector::Dot3V(v, p) + v.SplatW();
}

bool Plane::Side(const Vector& p) const
{
    return PointDistance(p) > 0.0f;
}

bool Plane::operator == (const Plane& rhs) const
{
    return v == rhs.v;
}

bool Plane::operator != (const Plane& rhs) const
{
    return v != rhs.v;
}

} // namespace Math
} // namespace NFE
