/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent component declaration.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "../Aligned.hpp"

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

    // vertical Field of View angle (in up-down axis)
    float FoV;

    /* //comented due to union in CameraDesc problems...
    Perspective()
    {
        farDist = 1000.0f;
        nearDist = 0.05f;
        aspectRatio = 1.0f;
        FoV = 60.0f / 180.0f * NFE_MATH_PI; // [rad]
    }
    */
};

/**
 * Orthogonal projection descriptor
 */
struct Ortho
{
    float nearDist, farDist;
    float bottom, top;
    float left, right;
};


/**
 * CameraComponent entity descriptor used for serialization
 */
#pragma pack(push, 1)
struct CameraDesc
{
    char projMode;    //cast to ProjectionMode
    union
    {
        Perspective perspective;
        Ortho ortho;
    };
};
#pragma pack(pop)

NFE_ALIGN16
class CORE_API CameraComponent : public ComponentBase<CameraComponent>, public Util::Aligned
{
    friend class SceneManager;
    friend class RendererSystem;

private:
    // additional matrix that modifies camera matrix relative to parent entity
    Math::Matrix mParentOffset;

    /// projection mode
    ProjectionMode projMode; // TODO: fix style
    Perspective perspective;
    Ortho ortho;

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
     * @param pDesc Valid pointer to perspective projection settings
     */
    void SetPerspective(const Perspective* desc);
    void GetPerspective(Perspective* desc) const;

    /**
     * Set orthographic projection
     * @param pDesc Valid pointer to orthographic projection settings
     */
    void SetOrtho(const Ortho* desc);

    /**
     * Update camera's matrices and frustum.
     */
    void Update(const Math::Matrix& matrix, const Math::Vector& velocity,
                const Math::Vector& angularVelocity, float dt);

    /**
     * Get a section of the camera frustum
     * @param zn Near distance
     * @param zf Far distance
     * @param[out] pFrustum Result
     */
    void SplitFrustum(const Math::Matrix& matrix, float zn, float zf, Math::Frustum* pFrustum);
};

} // namespace Scene
} // namespace NFE
