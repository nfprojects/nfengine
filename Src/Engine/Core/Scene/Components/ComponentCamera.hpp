/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent declaration.
 */

#pragma once

#include "Component.hpp"

#include "../../../Common/Memory/Aligned.hpp"
#include "../../../Common/Math/Frustum.hpp"


namespace NFE {
namespace Scene {

/**
 * Camera projection mode.
 */
enum class ProjectionMode : uint8
{
    Perspective,
    Ortho,
};

/**
 * Perspective projection descriptor
 */
struct PerspectiveProjectionDesc
{
    float farDist;
    float nearDist;
    float aspectRatio;
    float FoV;          //< vertical Field of View angle (in up-down axis) in radians

    PerspectiveProjectionDesc()
        : farDist(1000.0f)
        , nearDist(0.05f)
        , aspectRatio(1.0f)
        , FoV(Math::DegToRad(60.0f))
    { }
};

/**
 * Orthogonal projection descriptor
 */
struct OrthoProjectionDesc
{
    float nearDist, farDist;
    float bottom, top;
    float left, right;

    OrthoProjectionDesc()
        : nearDist(-100.0f)
        , farDist(100.0f)
        , bottom(-1.0f)
        , top(1.0f)
        , left(-1.0f)
        , right(1.0f)
    { }
};

/**
 * Camera component - allows an entity to be a camera.
 */
class CORE_API NFE_ALIGN(32) CameraComponent
    : public IComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(CameraComponent)

public:
    NFE_ALIGNED_CLASS(32)

    CameraComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;

    /**
     * Set perspective projection
     * @param desc Valid pointer to perspective projection settings
     */
    void SetPerspective(const PerspectiveProjectionDesc* desc);

    /**
     * Get perspective projection
     * @param desc Valid pointer, where perspective projection settings will be saved to
     */
    void GetPerspective(PerspectiveProjectionDesc* desc) const;

    /**
     * Set orthographic projection
     * @param desc Valid pointer to orthographic projection settings
     */
    void SetOrtho(const OrthoProjectionDesc* desc);

    /**
     * Get orthographic projection
     * @param desc Valid pointer, where orthographic projection settings will be saved to
     */
    void GetOrtho(OrthoProjectionDesc* desc) const;

    /**
     * Get a section of the camera frustum.
     *
     * NOTE:
     * Used for Cascaded Shadow Mapping to calculate shadow cascades.
     * This function doesn't calculate frustum's planes
     *
     * @param zn Near distance
     * @param zf Far distance
     * @param[out] pFrustum Result
     */
    void SplitFrustum(const Math::Matrix4& matrix, float zn, float zf, Math::Frustum* frustum) const;

    const Math::Matrix4& GetViewMatrix() const { return mViewMatrix; }
    const Math::Matrix4& GetProjectionMatrix() const { return mProjMatrix; }
    const Math::Matrix4& GetSecondaryProjMatrix() const { return mSecondaryProjViewMatrix; }
    const Math::Vec4f& GetScreenScale() const { return mScreenScale; }
    const Math::Frustum& GetFrustum() const { return mFrustum; }

private:
    ProjectionMode mProjMode;

    union
    {
        PerspectiveProjectionDesc mPerspective;
        OrthoProjectionDesc mOrtho;
    };

    Math::Matrix4 mViewMatrix;
    Math::Matrix4 mProjMatrix;
    Math::Matrix4 mViewMatrixInv;
    Math::Matrix4 mProjMatrixInv;
    Math::Matrix4 mSecondaryProjViewMatrix;
    Math::Vec4f mScreenScale;
    Math::Frustum mFrustum;

    void OnUpdate() override;

    /**
     * Update camera's matrices and frustum.
     *
     * @param matrix          Camera view matrix
     * @param velocity        Camera velocity
     * @param angularVelocity Camera angular velocity
     * @param dt              Delta time (used to calculate secondary matrices for motion blur)
     */
    void Update(const Math::Matrix4& matrix, const Math::Vec4f& velocity, const Math::Vec4f& angularVelocity);
};

} // namespace Scene
} // namespace NFE
