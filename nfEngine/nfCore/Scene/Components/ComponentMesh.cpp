/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of mesh component class.
 */

#include "PCH.hpp"
#include "ComponentMesh.hpp"
#include "Engine.hpp"
#include "Resources/ResourcesManager.hpp"
#include "../Scene/Entity.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Systems/RendererSystem.hpp"

namespace NFE {
namespace Scene {

using namespace Renderer;
using namespace Resource;


MeshComponent::MeshComponent()
    : mMesh(nullptr)
{
}

MeshComponent::~MeshComponent()
{
    DeleteRenderingProxy();

    if (mMesh != nullptr)
    {
        mMesh->DelRef();
        mMesh = nullptr;
    }
}

bool MeshComponent::SetMeshResource(Mesh* resource)
{
    // no change
    if (resource == mMesh)
        return true;

    // clean up proxy
    DeleteRenderingProxy();

    // delete reference to the previous mesh
    if (mMesh != nullptr)
    {
        mMesh->DelRef();
    }

    mMesh = resource;
    if (mMesh == nullptr)
    {
        return false;
    }

    // add reference to the new mesh
    mMesh->AddRef();

    // notify when the resource is loaded
    mMesh->AddPostLoadCallback(std::bind(&MeshComponent::OnMeshResourceLoaded, this));

    return true;
}

bool MeshComponent::SetMeshResource(const char* name)
{
    ResManager* rm = Engine::GetInstance()->GetResManager();
    Mesh* newMesh = static_cast<Mesh*>(rm->GetResource(name, ResourceType::Mesh));
    return SetMeshResource(newMesh);
}

void MeshComponent::OnDetach()
{
    DeleteRenderingProxy();
}

void MeshComponent::DeleteRenderingProxy()
{
    if (mRenderingProxy != INVALID_RENDER_PROXY)
    {
        GetScene()->GetRendererSystem()->DeleteMeshProxy(mRenderingProxy);
        mRenderingProxy = INVALID_RENDER_PROXY;
    }
}

void MeshComponent::OnMeshResourceLoaded()
{
    Entity* entity = GetEntity();
    NFE_ASSERT(entity, "Mesh component not attached to an entity");

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    // cleanup
    DeleteRenderingProxy();

    // create the new proxy
    MeshProxy proxy;
    proxy.transform = GetEntity()->GetGlobalMatrix();
    proxy.mesh = mMesh;
    mRenderingProxy = scene->GetRendererSystem()->CreateMeshProxy(proxy);
}

} // namespace Scene
} // namespace NFE
