/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentCamera.hpp"


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
    NFE_CLASS_MEMBER(projMode)
    NFE_CLASS_MEMBER(perspective)
    NFE_CLASS_MEMBER(ortho)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {


Scene::ComponentPtr EntityTemplateComponentCamera::CreateComponent() const
{
    // TODO
    return nullptr;
}


} // namespace Resource
} // namespace NFE
