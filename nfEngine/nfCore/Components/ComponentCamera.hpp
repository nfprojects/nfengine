/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent declaration.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "nfCommon/Aligned.hpp"
#include "nfCommon/Math/Frustum.hpp"

namespace NFE {
namespace Scene {

enum class ProjectionMode
{
    Perspective,
    Ortho
};

/**
 * Perspective projection descriptor
 */
struct Perspective
{
    float farDist;
    float nearDist;
    float aspectRatio;
    float FoV;          //< vertical Field of View angle (in up-down axis) in radians

    Perspective()
    {
        farDist = 1000.0f;
        nearDist = 0.05f;
        aspectRatio = 1.0f;
        FoV = 60.0f / 180.0f * NFE_MATH_PI; // [rad]
    }
};

/**
 * Orthogonal projection descriptor
 */
struct Ortho
{
    float nearDist, farDist;
    float bottom, top;
    float left, right;

    Ortho()
    {
        nearDist = -100.0f;
        farDist = 100.0f;
        bottom = -1.0f;
        top = 1.0f;
        left = -1.0f;
        right = 1.0f;
    }
};


NFE_ALIGN16
class CORE_API CameraComponent
    : public ComponentBase<CameraComponent>
    , public Common::Aligned<16>
{
    // additional matrix that modifies camera matrix relative to parent entity
    Math::Matrix mParentOffset;

    /// projection mode
    ProjectionMode mProjMode;
    Perspective mPerspective;
    Ortho mOrtho;

    /// matricies
    Math::Matrix mViewMatrix;
    Math::Matrix mProjMatrix;
    Math::Matrix mViewMatrixInv;
    Math::Matrix mProjMatrixInv;
    Math::Matrix mSecondaryProjViewMatrix;
    Math::Vector mScreenScale;
    Math::Frustum mFrustum;

public:
    CameraComponent();
    ~CameraComponent() {}

    /**
     * Set perspective projection
     * @param desc Valid pointer to perspective projection settings
     */
    void SetPerspective(const Perspective* desc);

    /**
     * Get perspective projection
     * @param desc Valid pointer, where perspective projection settings will be saved to
     */
    void GetPerspective(Perspective* desc) const;

    /**
     * Set orthographic projection
     * @param desc Valid pointer to orthographic projection settings
     */
    void SetOrtho(const Ortho* desc);

    /**
     * Get orthographic projection
     * @param desc Valid pointer, where orthographic projection settings will be saved to
     */
    void GetOrtho(Ortho* desc) const;

    /**
     * Update camera's matrices and frustum.
     * @param matrix          Camera view matrix
     * @param velocity        Camera velocity
     * @param angularVelocity Camera angular velocity
     * @param dt              Delta time (used to calculate secondary matrices for motion blur)
     */
    void Update(const Math::Matrix& matrix, const Math::Vector& velocity,
                const Math::Vector& angularVelocity, float dt);

    /**
     * Get a section of the camera frustum
     * @param zn Near distance
     * @param zf Far distance
     * @param[out] pFrustum Result
     */
    void SplitFrustum(const Math::Matrix& matrix, float zn, float zf,
                      Math::Frustum* pFrustum) const;
};

} // namespace Scene
} // namespace NFE
