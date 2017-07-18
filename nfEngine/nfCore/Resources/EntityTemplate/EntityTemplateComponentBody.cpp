/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentBody.hpp"
#include "../../Scene/Components/ComponentBody.hpp"

#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateComponentBody)
    NFE_CLASS_PARENT(NFE::Resource::IEntityTemplateComponent)
    NFE_CLASS_MEMBER(mCollisionShapeResource)
    NFE_CLASS_MEMBER(mMass)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {


Scene::ComponentPtr EntityTemplateComponentBody::CreateComponent() const
{
    // TODO
    return nullptr;
}


} // namespace Resource
} // namespace NFE
