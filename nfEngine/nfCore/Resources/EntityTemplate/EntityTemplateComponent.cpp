/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponent.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::IEntityTemplateComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {


IEntityTemplateComponent::IEntityTemplateComponent()
    : mName("component")
{
}


} // namespace Resource
} // namespace NFE
