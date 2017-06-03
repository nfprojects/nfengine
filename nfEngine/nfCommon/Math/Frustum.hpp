/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frustum class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Box.hpp"

namespace NFE {
namespace Math {

/**
 * Frustum
 */
class NFCOMMON_API NFE_ALIGN(16) Frustum
{
public:
    /// planes IDs
    enum Plane : uint8
    {
        Front = 0,
        Back,
        Left,
        Right,
        Bottom,
        Top
    };

    Vector planes[6];
    Vector verticies[8];
    Box boundingBox;

    /**
     * Construct frustum of perspective view (using axes and clipping planes distances).
     *
     * @param origin   Camera position
     * @param xAxis    Horizontal extent at distance equal to 1.
     * @param yAxis    Vertical extent at distance equal to 1.
     * @param zAxis    Normalized frustum direction.
     * @param nearDist Near plane distance.
     * @param farDist  Far plane distance.
     */
    static Frustum ConstructForPerspective(const Vector& origin,
                                           const Vector& xAxis, const Vector& yAxis, const Vector& zAxis,
                                           float nearDist, float farDist);

    /**
     * Construct frustum of perspective view (using camera matrix, clipping planes distances, FoV and aspect ratio).
     *
     * @param      matrix           Camera orientation.
     * @param      nearDist,farDist Clipping planes distances.
     * @param      cutoff           Cutoff angle in Y axis (in radians).
     * @param      aspect           Aspect ratio.
     */
    static Frustum ConstructForPerspective(const Matrix& matrix, float nearDist, float farDist, float cutoff, float aspect);

    /**
     * Recalculate frustum planes' equations.
     */
    void CalculatePlanes();

    /**
     * Calculate maximum scalar projection of the shape along @dir direction.
     */
    float SupportVertex(const Vector& dir) const;
};


} // namespace Math
} // namespace NFE
