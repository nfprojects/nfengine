/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Reflection/ReflectionEnumMacros.hpp"


namespace NFE {
namespace Resource {

/**
 * Camera projection mode.
 */
enum class ProjectionMode : uint8
{
    Perspective,    // for PerspectiveProjectionDesc
    Ortho,          // for OrthoProjectionDesc
};

/**
 * Perspective projection descriptor
 */
class PerspectiveProjectionDesc
{
    NFE_DECLARE_CLASS(PerspectiveProjectionDesc)

public:
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
class OrthoProjectionDesc
{
    NFE_DECLARE_CLASS(OrthoProjectionDesc)

public:
    float nearDist;
    float farDist;
    float bottom;
    float top;
    float left;
    float right;

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
 * Serializable EntityTemplate's component that represents a Camera Component.
 */
class CORE_API EntityTemplateComponentCamera : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentCamera)

public:
    EntityTemplateComponentCamera();

    NFE_INLINE ProjectionMode GetProjectionMode() const { return mProjMode; }
    NFE_INLINE const PerspectiveProjectionDesc& GetPerspectiveDesc() const { return mPerspective; }
    NFE_INLINE const OrthoProjectionDesc& GetOrthoDesc() const { return mOrtho; }

    /**
     * Create camera component object.
     */
    virtual Scene::ComponentPtr CreateComponent() const override;

private:
    ProjectionMode mProjMode;

    // TODO if not reflection, we could use union here...
    PerspectiveProjectionDesc mPerspective;
    OrthoProjectionDesc mOrtho;
};

} // namespace Resource
} // namespace NFE


NFE_DECLARE_ENUM_TYPE(NFE::Resource::ProjectionMode)
