/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Plane class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"

namespace NFE {
namespace Math {

/**
 * 3D plane
 */
class NFE_ALIGN(16) Plane
{
public:
    Plane() = default;

    NFE_INLINE explicit Plane(const Vector4& coeffs) : v(coeffs) { }

    // create plane with given normal and distance from origin
    NFE_INLINE Plane(const Vector4& normal, const float distance);

    // create plane crossing three points
    NFE_INLINE Plane(const Vector4& p0, const Vector4& p1, const Vector4& p2);

    // create plane with given normal and crossing point
    NFE_INLINE Plane(const Vector4& normal, const Vector4& point);

    NFE_FORCE_INLINE static const Plane PositiveX()
    {
        return Plane(Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane PositiveY()
    {
        return Plane(Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane PositiveZ()
    {
        return Plane(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane NegativeX()
    {
        return Plane(Vector4(-1.0f, 0.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane NegativeY()
    {
        return Plane(Vector4(0.0f, -1.0f, 0.0f, 0.0f));
    }

    NFE_FORCE_INLINE static const Plane NegativeZ()
    {
        return Plane(Vector4(0.0f, 0.0f, -1.0f, 0.0f));
    }

    /*
     * Calculate signed distance to a given point.
     */
    NFE_INLINE float PointDistance(const Vector4& p) const;
    NFE_INLINE Vector4 PointDistanceV(const Vector4& p) const;

    /**
     * Determine plane side a point belongs to.
     * @return "true" - positive side, "false" - negative side
     */
    NFE_INLINE bool Side(const Vector4& p) const;

    NFE_INLINE bool operator == (const Plane& rhs) const;
    NFE_INLINE bool operator != (const Plane& rhs) const;

    NFE_FORCE_INLINE bool IsValid() const
    {
        return v.IsValid();
    }

    // internal vector
    Vector4 v;
};

} // namespace Math
} // namespace NFE


#include "PlaneImpl.hpp"
