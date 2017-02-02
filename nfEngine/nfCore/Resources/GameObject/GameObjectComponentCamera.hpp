/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "GameObjectComponent.hpp"

#include "nfCommon/Math/Math.hpp"


namespace NFE {
namespace Resource {

/**
 * Camera projection mode.
 */
enum class ProjectionMode
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

    NFE_INLINE PerspectiveProjectionDesc()
    {
        farDist = 1000.0f;
        nearDist = 0.05f;
        aspectRatio = 1.0f;
        FoV = 60.0f / 180.0f * NFE_MATH_PI;
    }
};

/**
 * Orthogonal projection descriptor
 */
struct OrthoProjectionDesc
{
    float nearDist, farDist;
    float bottom, top;
    float left, right;

    NFE_INLINE OrthoProjectionDesc()
    {
        nearDist = -100.0f;
        farDist = 100.0f;
        bottom = -1.0f;
        top = 1.0f;
        left = -1.0f;
        right = 1.0f;
    }
};

/**
 * Serializable game object component that represents a Camera Component.
 */
class CORE_API GameObjectComponentCamera : public IGameObjectComponent
{
public:
    ProjectionMode projMode;

    union
    {
        PerspectiveProjectionDesc perspective;
        OrthoProjectionDesc ortho;
    };

    virtual Scene::IComponent* CreateComponent() override;
};

} // namespace Resource
} // namespace NFE
