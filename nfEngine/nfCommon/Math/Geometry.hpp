/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Geometry objects declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "Ray.hpp"

namespace NFE {
namespace Math {

enum class IntersectionResult
{
    Outside   = 0, //< no intersection
    Inside    = 1, //< shapeA is inside shapeB
    Intersect = 2, //< shapeA and shapeB intersects
};

/**
 * Finds the nearest point on a line segment to a @a point.
 * @param[in]  p1,p2 Line segment ends.
 * @param[out] out Nearest point on the segment.
 * @return     Distance to the segment.
 */
NFCOMMON_API float ClosestPointOnSegment(const Vector& p, const Vector& p1, const Vector& p2, Vector& out);

/**
 * Template function for intersection tests.
 */
template<typename ShapeTypeA, typename ShapeTypeB>
bool Intersect(const ShapeTypeA& shapeA, const ShapeTypeB& shapeB);

/**
 * Template function for extended intersection tests.
 * @see IntersectionResult
 */
template<typename ShapeTypeA, typename ShapeTypeB>
IntersectionResult IntersectEx(const ShapeTypeA& shapeA, const ShapeTypeB& shapeB);

/**
 * Template function for ray intersection tests.
 * @param dist Distance to the intersection.
 */
template<typename ShapeType>
bool Intersect(const Ray& ray, const ShapeType& shape, Vector& dist);


/**
 * Transform Box by a 4x4 matrix.
 */
NFCOMMON_API Box TransformBox(const Matrix& matrix, const Box& localBox);

/**
 * Transform Box using a quaternion.
 */
NFCOMMON_API Box TransformBox(const Quaternion& quat, const Box& localBox);

/**
 * Transform Box using a transform object.
 */
NFCOMMON_API Box TransformBox(const Transform& transform, const Box& localBox);

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_SSE
#include "SSE/RayGeometry.hpp"
#else
#include "FPU/RayGeometry.hpp"
#endif
