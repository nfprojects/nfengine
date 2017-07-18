/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentCamera.hpp"
#include "../../Scene/Components/ComponentCamera.hpp"


NFE_BEGIN_DEFINE_ENUM(NFE::Resource::ProjectionMode)
    NFE_ENUM_OPTION(Perspective)
    NFE_ENUM_OPTION(Ortho)
NFE_END_DEFINE_ENUM()

NFE_BEGIN_DEFINE_CLASS(NFE::Resource::PerspectiveProjectionDesc)
    NFE_CLASS_MEMBER(farDist)
    NFE_CLASS_MEMBER(nearDist)
    NFE_CLASS_MEMBER(aspectRatio)
    NFE_CLASS_MEMBER(FoV)
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_CLASS(NFE::Resource::OrthoProjectionDesc)
    NFE_CLASS_MEMBER(nearDist)
    NFE_CLASS_MEMBER(farDist)
    NFE_CLASS_MEMBER(bottom)
    NFE_CLASS_MEMBER(top)
    NFE_CLASS_MEMBER(left)
    NFE_CLASS_MEMBER(right)
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateComponentCamera)
    NFE_CLASS_PARENT(NFE::Resource::IEntityTemplateComponent)
    NFE_CLASS_MEMBER(mProjMode)
    NFE_CLASS_MEMBER(mPerspective)
    NFE_CLASS_MEMBER(mOrtho)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {


EntityTemplateComponentCamera::EntityTemplateComponentCamera()
    : mProjMode(ProjectionMode::Perspective)
{ }

Scene::ComponentPtr EntityTemplateComponentCamera::CreateComponent() const
{
    auto component = Common::MakeUniquePtr<Scene::CameraComponent>();
    if (!component)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    if (mProjMode == ProjectionMode::Perspective)
    {
        component->SetPerspective(&mPerspective);
    }
    else if (mProjMode == ProjectionMode::Ortho)
    {
        component->SetOrtho(&mOrtho);
    }
    else
    {
        LOG_ERROR("Invalid projection mode");
    }

    // TODO set light properties
    return component;
}


} // namespace Resource
} // namespace NFE
