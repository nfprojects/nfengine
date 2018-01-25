/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"


namespace NFE {
namespace Math {

/**
 * Axis aligned 3D box shape.
 */
class NFE_ALIGN(16) Box
{
public:
    Vector min;
    Vector max;

    /**
     * Create default box - (0,0,0) point.
     */
    Box() : min(), max() {}

    explicit Box(const Vector& min, const Vector& max) : min(min), max(max) { }
    explicit Box(const Vector& minMax) : min(minMax), max(minMax) { }

    /**
     * Create a box from center point and radius (e.g. bounding box of a sphere).
     */
    NFE_INLINE Box(const Vector& center, float radius);

    /**
     * Create a box that includes two other boxes.
     */
    NFE_INLINE Box(const Box& a, const Box& b);

    /**
     * Offset box using a vector.
     */
    NFE_INLINE Box& operator += (const Vector& offset);
    NFE_INLINE Box& operator -= (const Vector& offset);
    NFE_INLINE Box operator + (const Vector& offset) const;
    NFE_INLINE Box operator - (const Vector& offset) const;

    /**
     * Scale the box.
     */
    NFE_INLINE Box& operator *= (const Vector& scale);
    NFE_INLINE Box& operator *= (const float scale);
    NFE_INLINE Box operator * (const Vector& scale) const;
    NFE_INLINE Box operator * (const float scale) const;

    /**
     * Calculate box center.
     */
    NFE_INLINE Vector GetCenter() const;

    /**
     * Calculate box vertex position.
     */
    NFE_INLINE Vector GetVertex(int id) const;

    /**
     * Find a point with maximum scalar projection value along @dir direction.
     */
    NFE_INLINE Vector SupportVertex(const Vector& dir) const;

    /**
     * Calculate box surface area.
     */
    NFCOMMON_API float SurfaceArea() const;

    /**
     * Calculate box volume.
     */
    NFCOMMON_API float Volume() const;

    /**
     * Create empty box.
     * @notes Empty box means no extents defined (not even a single point).
     */
    NFE_INLINE static Box Empty();

    NFE_INLINE bool operator == (const Box& rhs) const;
    NFE_INLINE bool operator != (const Box& rhs) const;

    /**
     * Extend this box so that it includes another box.
     */
    NFE_INLINE Box& AddBox(const Box& otherBox);

    /**
     * Extend this box so that it includes a point.
     */
    NFE_INLINE Box& AddPoint(const Vector& point);

    /**
     * Check if is empty (has no extents defined, even a single point).
     */
    NFE_INLINE bool IsEmpty() const;
};

} // namespace Math
} // namespace NFE


#include "BoxImpl.hpp"
