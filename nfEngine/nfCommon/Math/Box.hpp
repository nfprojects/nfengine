#pragma once

#include "Vector4.hpp"

namespace NFE {
namespace Math {

/**
 * Axis Aligned Box
 */
class NFE_ALIGN(16) Box
{
    NFE_DECLARE_CLASS(Box);

public:

    Vector4 min;
    Vector4 max;

    NFE_FORCE_INLINE Box() = default;

    NFE_FORCE_INLINE Box(const Box&) = default;

    NFE_FORCE_INLINE explicit Box(const Vector4& point)
        : min(point)
        , max(point)
    {}

    NFE_FORCE_INLINE Box(const Vector4& min, const Vector4& max)
        : min(min)
        , max(max)
    {}

    NFE_FORCE_INLINE Box(const Vector4& a, const Vector4& b, const Vector4& c)
        : min(Vector4::Min(a, Vector4::Min(b, c)))
        , max(Vector4::Max(a, Vector4::Max(b, c)))
    {}

    NFE_FORCE_INLINE Box(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
        : min(Vector4::Min(Vector4::Min(a, b), Vector4::Min(c, d)))
        , max(Vector4::Max(Vector4::Max(a, b), Vector4::Max(c, d)))
    {}

    NFE_FORCE_INLINE static const Box Empty()
    {
        return { VECTOR_MAX, -VECTOR_MAX };
    }

    NFE_FORCE_INLINE static const Box Full()
    {
        return { -VECTOR_MAX, VECTOR_MAX };
    }

    // create box from center point and radius (e.g. bounding box of a sphere)
    NFE_FORCE_INLINE Box(const Vector4& center, float radius)
        : min(center - Vector4(radius))
        , max(center + Vector4(radius))
    {}

    // merge boxes
    NFE_FORCE_INLINE Box(const Box& a, const Box& b)
        : min(Vector4::Min(a.min, b.min))
        , max(Vector4::Max(a.max, b.max))
    {}

    NFE_FORCE_INLINE const Box operator + (const Vector4& offset) const
    {
        return Box{ min + offset, max + offset };
    }

    NFE_FORCE_INLINE const Vector4 GetCenter() const
    {
        return (min + max) * 0.5f;
    }

    NFE_FORCE_INLINE float SurfaceArea() const
    {
        Vector4 size = max - min;
        const float halfArea = size.x * (size.y + size.z) + size.y * size.z;
        return halfArea + halfArea;
    }

    NFE_FORCE_INLINE float Volume() const
    {
        Vector4 size = max - min;
        return size.x * size.y * size.z;
    }

    NFE_FORCE_INLINE Box& AddPoint(const Vector4& point)
    {
        min = Vector4::Min(min, point);
        max = Vector4::Max(max, point);
        return *this;
    }

    // point-box intersection test
    NFE_FORCE_INLINE bool Intersects(const Vector4& point) const
    {
        const VectorBool4 mask = (point >= min) & (point <= max);
        return (mask.GetMask() & 7) == 7;
    }

    NFE_FORCE_INLINE const Vector4 GetVertex(uint32 index) const
    {
        NFE_ASSERT(index < 8);
        return Vector4
        {
            (index & (1 << 0)) ? max.x : min.x,
            (index & (1 << 1)) ? max.y : min.y,
            (index & (1 << 2)) ? max.z : min.z
        };
    }

    NFE_FORCE_INLINE bool operator == (const Box& rhs) const
    {
        return (((min == rhs.min) & (max == rhs.max)).GetMask() & 7) == 7;
    }

    NFE_FORCE_INLINE bool Box::operator != (const Box& rhs) const
    {
        return !operator==(rhs);
    }
};


} // namespace Math
} // namespace NFE
