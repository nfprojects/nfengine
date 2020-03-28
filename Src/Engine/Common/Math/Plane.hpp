/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Plane class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"

namespace NFE {
namespace Math {

/**
 * 3D plane
 */
class NFE_ALIGN(16) Plane
{
public:
    Plane() = default;

    NFE_INLINE explicit Plane(const Vec4f& coeffs) : v(coeffs) { }

    // create plane with given normal and distance from origin
    NFE_INLINE Plane(const Vec4f& normal, const float distance);

    // create plane crossing three points
    NFE_INLINE Plane(const Vec4f& p0, const Vec4f& p1, const Vec4f& p2);

    // create plane with given normal and crossing point
    NFE_INLINE Plane(const Vec4f& normal, const Vec4f& point);

    NFE_FORCE_INLINE static const Plane PositiveX()
    {
        return Plane(Vec4f(1.0f, 0.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane PositiveY()
    {
        return Plane(Vec4f(0.0f, 1.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane PositiveZ()
    {
        return Plane(Vec4f(0.0f, 0.0f, 1.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane NegativeX()
    {
        return Plane(Vec4f(-1.0f, 0.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane NegativeY()
    {
        return Plane(Vec4f(0.0f, -1.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane NegativeZ()
    {
        return Plane(Vec4f(0.0f, 0.0f, -1.0f, 0.0f));
    }

    /*
     * Calculate signed distance to a given point.
     */
    NFE_INLINE float PointDistance(const Vec4f& p) const;
    NFE_INLINE Vec4f PointDistanceV(const Vec4f& p) const;

    /**
     * Determine plane side a point belongs to.
     * @return "true" - positive side, "false" - negative side
     */
    NFE_INLINE bool Side(const Vec4f& p) const;

    NFE_INLINE bool operator == (const Plane& rhs) const;
    NFE_INLINE bool operator != (const Plane& rhs) const;

    NFE_FORCE_INLINE bool IsValid() const
    {
        return v.IsValid();
    }

    // internal vector
    Vec4f v;
};

} // namespace Math
} // namespace NFE


#include "PlaneImpl.hpp"
