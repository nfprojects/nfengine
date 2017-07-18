/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentMesh.hpp"
#include "../../Scene/Components/ComponentMesh.hpp"

#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateComponentMesh)
    NFE_CLASS_PARENT(NFE::Resource::IEntityTemplateComponent)
    NFE_CLASS_MEMBER(mMeshResource)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {

Scene::ComponentPtr EntityTemplateComponentMesh::CreateComponent() const
{
    auto component = Common::MakeUniquePtr<Scene::MeshComponent>();
    if (!component)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    // TODO
    component->SetMeshResource(mMeshResource.Str());

    return component;
}

} // namespace Resource
} // namespace NFE
