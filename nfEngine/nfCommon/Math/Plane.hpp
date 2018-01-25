/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Plane class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

/**
 * 3D plane
 */
class NFE_ALIGN(16) Plane
{
public:
    Plane() = default;
    NFE_INLINE explicit Plane(const Vector& coeffs);

    // create plane with given normal and distance from origin
    NFE_INLINE Plane(const Vector& normal, const float distance);

    // create plane crossing three points
    NFE_INLINE Plane(const Vector& p0, const Vector& p1, const Vector& p2);

    // create plane with given normal and crossing point
    NFE_INLINE Plane(const Vector& normal, const Vector& point);

    /*
     * Calculate signed distance to a given point.
     */
    NFE_INLINE float PointDistance(const Vector& p) const;
    NFE_INLINE Vector PointDistanceV(const Vector& p) const;

    /**
     * Determine plane side a point belongs to.
     * @return "true" - positive side, "false" - negative side
     */
    NFE_INLINE bool Side(const Vector& p) const;

    NFE_INLINE bool operator == (const Plane& rhs) const;
    NFE_INLINE bool operator != (const Plane& rhs) const;

    // internal vector
    Vector v;
};

} // namespace Math
} // namespace NFE


#include "PlaneImpl.hpp"
