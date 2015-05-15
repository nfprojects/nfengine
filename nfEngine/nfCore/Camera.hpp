/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Camera component declaration.
 */

#pragma once

#include "Core.hpp"
#include "Component.hpp"

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
 * Camera entity descriptor used for serialization
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


class CORE_API Camera : public Component
{
    friend class SceneManager;
    friend class LightComponent;

private:
    // additional matrix that modifies camera matrix relative to parent entity
    Math::Matrix mParentOffset;

    /// matricies
    Math::Matrix mViewMatrix;
    Math::Matrix mProjMatrix;
    Math::Matrix mViewMatrixInv;
    Math::Matrix mProjMatrixInv;

    Math::Vector mScreenScale;

    Math::Matrix mSecondaryProjViewMatrix;

    Math::Frustum mFrustum;

public:
    //Projection
    ProjectionMode projMode;
    Perspective perspective;
    Ortho ortho;

    Camera(Entity* pParent);

    /**
     * Set perspective projection
     * @param pDesc Valid pointer to perspective projection settings
     */
    void SetPerspective(const Perspective* pDesc);
    void GetPerspective(Perspective* pDesc) const;

    /**
     * Set orthographic projection
     * @param pDesc Valid pointer to orthographic projection settings
     */
    void SetOrtho(const Ortho* pDesc);

    void OnUpdate(float dt);

    /**
     * Get a section of the camera frustum
     * @param zn Near distance
     * @param zf Far distance
     * @param[out] pFrustum Result
     */
    void SplitFrustum(float zn, float zf, Math::Frustum* pFrustum);

    Result Deserialize(Common::InputStream* pStream);
    Result Serialize(Common::OutputStream* pStream) const;
};

} // namespace Scene
} // namespace NFE
