/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class inline definitions.
 */

#pragma once

#include "Box.hpp"


namespace NFE {
namespace Math {

Box::Box(const Vector4& center, float radius)
{
    min = center - Vector4::Splat(radius);
    max = center + Vector4::Splat(radius);
}

Box::Box(const Box& a, const Box& b)
{
    min = Vector4::Min(a.min, b.min);
    max = Vector4::Max(a.max, b.max);
}

Vector4 Box::GetCenter() const
{
    return (min + max) * 0.5f;
}

Vector4 Box::GetVertex(int id) const
{
    return Vector4(
        (id & (1 << 0)) ? max.x : min.x,
        (id & (1 << 1)) ? max.y : min.y,
        (id & (1 << 2)) ? max.z : min.z
    );
}

Vector4 Box::SupportVertex(const Vector4& dir) const
{
    return Vector4::SelectBySign(max, min, dir);
}

bool Box::IsEmpty() const
{
    // box is empty if 'min' coordinate is greater than 'max' coordinate in at leas on axis
    return (Vector4::GreaterMask(min, max) & 0x7) != 0;
}

Box Box::Empty()
{
    return Box(VECTOR_MAX, VECTOR_NEG_MAX);
}

Box& Box::AddBox(const Box& otherBox)
{
    min = Vector4::Min(min, otherBox.min);
    max = Vector4::Max(max, otherBox.max);
    return *this;
}

Box& Box::AddPoint(const Vector4& point)
{
    min = Vector4::Min(min, point);
    max = Vector4::Max(max, point);
    return *this;
}

Box& Box::operator += (const Vector4& offset)
{
    min += offset;
    max += offset;
    return *this;
}

Box& Box::operator -= (const Vector4& offset)
{
    min -= offset;
    max -= offset;
    return *this;
}

Box& Box::operator *= (const Vector4& scale)
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

Box Box::operator + (const Vector4& offset) const
{
    return Box(min + offset, max + offset);
}

Box Box::operator - (const Vector4& offset) const
{
    return Box(min - offset, max - offset);
}

Box Box::operator * (const Vector4& scale) const
{
    return Box(min * scale, max * scale);
}

Box Box::operator * (const float scale) const
{
    return Box(min * scale, max * scale);
}

bool Box::operator == (const Box& rhs) const
{
    return Vector4::Equal3(min, rhs.min) && Vector4::Equal3(max, rhs.max);
}

bool Box::operator != (const Box& rhs) const
{
    return !Vector4::Equal3(min, rhs.min) || !Vector4::Equal3(max, rhs.max);
}

} // namespace Math
} // namespace NFE
