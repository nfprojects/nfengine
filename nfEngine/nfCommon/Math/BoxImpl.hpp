/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class inline definitions.
 */

#pragma once

#include "Box.hpp"


namespace NFE {
namespace Math {

Box::Box(const Vector& center, float radius)
{
    min = center - Vector::Splat(radius);
    max = center + Vector::Splat(radius);
}

Box::Box(const Box& a, const Box& b)
{
    min = Vector::Min(a.min, b.min);
    max = Vector::Max(a.max, b.max);
}

Vector Box::GetCenter() const
{
    return (min + max) * 0.5f;
}

Vector Box::GetVertex(int id) const
{
    return Vector(
        (id & (1 << 0)) ? max.x : min.x,
        (id & (1 << 1)) ? max.y : min.y,
        (id & (1 << 2)) ? max.z : min.z
    );
}

Vector Box::SupportVertex(const Vector& dir) const
{
    return Vector::SelectBySign(max, min, dir);
}

bool Box::IsEmpty() const
{
    // box is empty if 'min' coordinate is greater than 'max' coordinate in at leas on axis
    return (Vector::GreaterMask(min, max) & 0x7) != 0;
}

Box Box::Empty()
{
    return Box(VECTOR_MAX, VECTOR_NEG_MAX);
}

Box& Box::AddBox(const Box& otherBox)
{
    min = Vector::Min(min, otherBox.min);
    max = Vector::Max(max, otherBox.max);
    return *this;
}

Box& Box::AddPoint(const Vector& point)
{
    min = Vector::Min(min, point);
    max = Vector::Max(max, point);
    return *this;
}

Box& Box::operator += (const Vector& offset)
{
    min += offset;
    max += offset;
    return *this;
}

Box& Box::operator -= (const Vector& offset)
{
    min -= offset;
    max -= offset;
    return *this;
}

Box& Box::operator *= (const Vector& scale)
{
    min *= scale;
    max *= scale;
    return *this;
}

Box& Box::operator *= (const float scale)
{
    min *= scale;
    max *= scale;
    return *this;
}

Box Box::operator + (const Vector& offset) const
{
    return Box(min + offset, max + offset);
}

Box Box::operator - (const Vector& offset) const
{
    return Box(min - offset, max - offset);
}

Box Box::operator * (const Vector& scale) const
{
    return Box(min * scale, max * scale);
}

Box Box::operator * (const float scale) const
{
    return Box(min * scale, max * scale);
}

bool Box::operator == (const Box& rhs) const
{
    return Vector::Equal3(min, rhs.min) && Vector::Equal3(max, rhs.max);
}

bool Box::operator != (const Box& rhs) const
{
    return !Vector::Equal3(min, rhs.min) || !Vector::Equal3(max, rhs.max);
}

} // namespace Math
} // namespace NFE
