/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent declaration.
 */

#pragma once

#include "../../Core.hpp"
#include "Component.hpp"
#include "../../Resources/GameObject/GameObjectComponentCamera.hpp"
#include "nfCommon/Aligned.hpp"
#include "nfCommon/Math/Frustum.hpp"

namespace NFE {
namespace Scene {


NFE_ALIGN16
class CORE_API CameraComponent
    : public IComponent
    , public Common::Aligned<16>
{
public:
    CameraComponent();
    ~CameraComponent() {}

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;

    /**
     * Set perspective projection
     * @param desc Valid pointer to perspective projection settings
     */
    void SetPerspective(const Resource::PerspectiveProjectionDesc* desc);

    /**
     * Get perspective projection
     * @param desc Valid pointer, where perspective projection settings will be saved to
     */
    void GetPerspective(Resource::PerspectiveProjectionDesc* desc) const;

    /**
     * Set orthographic projection
     * @param desc Valid pointer to orthographic projection settings
     */
    void SetOrtho(const Resource::OrthoProjectionDesc* desc);

    /**
     * Get orthographic projection
     * @param desc Valid pointer, where orthographic projection settings will be saved to
     */
    void GetOrtho(Resource::OrthoProjectionDesc* desc) const;

    /**
     * Update camera's matrices and frustum.
     *
     * @param matrix          Camera view matrix
     * @param velocity        Camera velocity
     * @param angularVelocity Camera angular velocity
     * @param dt              Delta time (used to calculate secondary matrices for motion blur)
     */
    void Update(const Math::Matrix& matrix, const Math::Vector& velocity, const Math::Vector& angularVelocity, float dt);

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
    void SplitFrustum(const Math::Matrix& matrix, float zn, float zf, Math::Frustum* frustum) const;

    NFE_INLINE const Math::Matrix& GetViewMatrix() const { return mViewMatrix; }
    NFE_INLINE const Math::Matrix& GetProjectionMatrix() const { return mProjMatrix; }
    NFE_INLINE const Math::Matrix& GetSecondaryProjMatrix() const { return mSecondaryProjViewMatrix; }
    NFE_INLINE const Math::Vector& GetScreenScale() const { return mScreenScale; }
    NFE_INLINE const Math::Frustum& GetFrustum() const { return mFrustum; }

private:
    Resource::ProjectionMode mProjMode;

    union
    {
        Resource::PerspectiveProjectionDesc mPerspective;
        Resource::OrthoProjectionDesc mOrtho;
    };

    Math::Matrix mViewMatrix;
    Math::Matrix mProjMatrix;
    Math::Matrix mViewMatrixInv;
    Math::Matrix mProjMatrixInv;
    Math::Matrix mSecondaryProjViewMatrix;
    Math::Vector mScreenScale;
    Math::Frustum mFrustum;
};

} // namespace Scene
} // namespace NFE
