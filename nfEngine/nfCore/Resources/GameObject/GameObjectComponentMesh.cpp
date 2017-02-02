/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectComponentMesh.hpp"
#include "../../Scene/Components/ComponentMesh.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Resource {

Scene::IComponent* GameObjectComponentMesh::CreateComponent()
{
    Scene::MeshComponent* meshComponent = new Scene::MeshComponent;
    if (!meshComponent)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    meshComponent->SetMeshResource(mMeshResourceName.c_str());
    return meshComponent;
}

} // namespace Resource
} // namespace NFE
