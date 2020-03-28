/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frustum class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"
#include "Matrix4.hpp"
#include "Box.hpp"
#include "Plane.hpp"


namespace NFE {
namespace Math {

/**
 * Frustum
 */
class NFCOMMON_API NFE_ALIGN(16) Frustum
{
public:
    /// planes IDs
    enum Side : uint8
    {
        Front = 0,
        Back,
        Left,
        Right,
        Bottom,
        Top
    };

    Plane planes[6];
    Vec4f verticies[8];
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
    static Frustum ConstructForPerspective(const Vec4f& origin,
                                           const Vec4f& xAxis, const Vec4f& yAxis, const Vec4f& zAxis,
                                           float nearDist, float farDist);

    /**
     * Construct frustum of perspective view (using camera matrix, clipping planes distances, FoV and aspect ratio).
     *
     * @param      matrix           Camera orientation.
     * @param      nearDist,farDist Clipping planes distances.
     * @param      cutoff           Cutoff angle in Y axis (in radians).
     * @param      aspect           Aspect ratio.
     */
    static Frustum ConstructForPerspective(const Matrix4& matrix, float nearDist, float farDist, float cutoff, float aspect);

    /**
     * Recalculate frustum planes' equations.
     */
    void CalculatePlanes();

    /**
     * Calculate maximum scalar projection of the shape along @dir direction.
     */
    float SupportVertex(const Vec4f& dir) const;
};


} // namespace Math
} // namespace NFE
