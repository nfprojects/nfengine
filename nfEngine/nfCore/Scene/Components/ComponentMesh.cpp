/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of mesh component class.
 */

#include "PCH.hpp"
#include "ComponentMesh.hpp"
#include "Engine.hpp"
#include "Resources/ResourcesManager.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"
#include "../Systems/RendererSystem.hpp"
#include "Renderer/RenderScene.hpp"

#include "nfCommon/System/Assertion.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::MeshComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Renderer;
using namespace Resource;
using namespace Math;

MeshComponent::MeshComponent()
    : mMesh(nullptr)
    , mRenderingProxy(INVALID_RENDER_PROXY)
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

Box MeshComponent::GetBoundingBox() const
{
    // TODO
    return Box();
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

    return true;
}

bool MeshComponent::SetMeshResource(const char* name)
{
    ResManager* rm = Engine::GetInstance()->GetResManager();
    Mesh* newMesh = static_cast<Mesh*>(rm->GetResource(name, ResourceType::Mesh));
    return SetMeshResource(newMesh);
}

void MeshComponent::OnAttach()
{
    if (mMesh)
    {
        // notify when the resource is loaded
        mMesh->AddPostLoadCallback(std::bind(&MeshComponent::OnMeshResourceLoaded, this));
    }
}

void MeshComponent::OnDetach()
{
    // TODO abort Resource Post Load Callback
    // TODO this is not thread-safe

    DeleteRenderingProxy();
}

RendererSystem* MeshComponent::GetRendererSystem() const
{
    SceneManager& scene = GetScene();
    RendererSystem* system = scene.GetSystem<RendererSystem>();
    NFE_ASSERT(system, "Invalid renderer system pointer");

    return system;
}

void MeshComponent::OnUpdate()
{
    // update proxy placement
    if (mRenderingProxy != INVALID_RENDER_PROXY)
    {
        MeshProxyDesc desc;
        desc.transform = GetEntity()->GetGlobalTransform().ToMatrix();
        desc.mesh = mMesh; // TODO this should be removed

        RendererSystem* system = GetRendererSystem();
        system->GetRenderScene()->UpdateMeshProxy(mRenderingProxy, desc);
    }
}

void MeshComponent::CreateRenderingProxy()
{
    // cleanup
    DeleteRenderingProxy();

    if (!GetEntity())
        return;

    // create the new proxy
    MeshProxyDesc desc;
    desc.transform = GetEntity()->GetGlobalTransform().ToMatrix();
    desc.mesh = mMesh;

    RendererSystem* system = GetRendererSystem();
    mRenderingProxy = system->GetRenderScene()->CreateMeshProxy(desc);
}

void MeshComponent::DeleteRenderingProxy()
{
    if (mRenderingProxy != INVALID_RENDER_PROXY)
    {
        RendererSystem* system = GetRendererSystem();
        system->GetRenderScene()->DeleteMeshProxy(mRenderingProxy);
        mRenderingProxy = INVALID_RENDER_PROXY;
    }
}

void MeshComponent::OnMeshResourceLoaded()
{
    CreateRenderingProxy();
}

} // namespace Scene
} // namespace NFE
