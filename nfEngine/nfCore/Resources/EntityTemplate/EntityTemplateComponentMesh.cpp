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
    NFE_CLASS_MEMBER(mMeshResourceName)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {

Scene::ComponentPtr EntityTemplateComponentMesh::CreateComponent() const
{
    Scene::MeshComponent* meshComponent = new Scene::MeshComponent;
    if (!meshComponent)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    meshComponent->SetMeshResource(mMeshResourceName.Str());
    return meshComponent;
}

} // namespace Resource
} // namespace NFE
