#pragma once

#include "Vec4f.hpp"

namespace NFE {
namespace Math {

/**
 * Axis Aligned Box
 */
class NFE_ALIGN(16) Box
{
    NFE_DECLARE_CLASS(Box)

public:

    NFE_ALIGNED_CLASS(16)

    Vec4f min;
    Vec4f max;

    NFE_FORCE_INLINE Box() = default;

    NFE_FORCE_INLINE Box(const Box&) = default;

    // create a box containing a single point
    NFE_FORCE_INLINE explicit Box(const Vec4f& point)
        : min(point)
        , max(point)
    {}

    NFE_FORCE_INLINE Box(const Vec4f& min, const Vec4f& max)
        : min(min)
        , max(max)
    {}

    NFE_FORCE_INLINE Box(const Vec4f& a, const Vec4f& b, const Vec4f& c)
        : min(Vec4f::Min(a, Vec4f::Min(b, c)))
        , max(Vec4f::Max(a, Vec4f::Max(b, c)))
    {}

    NFE_FORCE_INLINE Box(const Vec4f& a, const Vec4f& b, const Vec4f& c, const Vec4f& d)
        : min(Vec4f::Min(Vec4f::Min(a, b), Vec4f::Min(c, d)))
        , max(Vec4f::Max(Vec4f::Max(a, b), Vec4f::Max(c, d)))
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
    NFE_FORCE_INLINE Box(const Vec4f& center, float radius)
        : min(center - Vec4f(radius))
        , max(center + Vec4f(radius))
    {}

    // merge boxes
    NFE_FORCE_INLINE Box(const Box& a, const Box& b)
        : min(Vec4f::Min(a.min, b.min))
        , max(Vec4f::Max(a.max, b.max))
    {}

    NFE_FORCE_INLINE const Box operator + (const Vec4f& offset) const
    {
        return Box{ min + offset, max + offset };
    }

    NFE_FORCE_INLINE const Vec4f GetCenter() const
    {
        return (min + max) * 0.5f;
    }

    NFE_FORCE_INLINE float SurfaceArea() const
    {
        Vec4f size = max - min;
        const float halfArea = size.x * (size.y + size.z) + size.y * size.z;
        return halfArea + halfArea;
    }

    NFE_FORCE_INLINE float Volume() const
    {
        Vec4f size = max - min;
        return size.x * size.y * size.z;
    }

    NFE_FORCE_INLINE Box& AddPoint(const Vec4f& point)
    {
        min = Vec4f::Min(min, point);
        max = Vec4f::Max(max, point);
        return *this;
    }

    // point-box intersection test
    NFE_FORCE_INLINE bool Intersects(const Vec4f& point) const
    {
        const auto mask = (point >= min) & (point <= max);
        return (mask.GetMask() & 7) == 7;
    }

    NFE_FORCE_INLINE const Vec4f GetVertex(uint32 index) const
    {
        NFE_ASSERT(index < 8, "Invalid vertex index");
        return Vec4f
        {
            (index & (1 << 0)) ? max.x : min.x,
            (index & (1 << 1)) ? max.y : min.y,
            (index & (1 << 2)) ? max.z : min.z
        };
    }

    NFE_FORCE_INLINE bool operator == (const Box& rhs) const
    {
        return ((min == rhs.min) & (max == rhs.max)).All3();
    }

    NFE_FORCE_INLINE bool operator != (const Box& rhs) const
    {
        return !operator==(rhs);
    }
};


} // namespace Math
} // namespace NFE
