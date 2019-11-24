/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of mesh component class.
 */

#include "PCH.hpp"
#include "ComponentMesh.hpp"
#include "../Entity.hpp"
#include "../Scene.hpp"
#include "../Systems/RendererSystem.hpp"
#include "nfCommon/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::MeshComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Resource;
using namespace Math;

MeshComponent::MeshComponent()
{
}

MeshComponent::~MeshComponent()
{
    DeleteRenderingProxy();
}

Box MeshComponent::GetBoundingBox() const
{
    // TODO
    return Box();
}

void MeshComponent::OnAttach()
{
}

void MeshComponent::OnDetach()
{
    // TODO abort Resource Post Load Callback
    // TODO this is not thread-safe

    DeleteRenderingProxy();
}

RendererSystem* MeshComponent::GetRendererSystem() const
{
    Scene& scene = GetScene();
    RendererSystem* system = scene.GetSystem<RendererSystem>();
    NFE_ASSERT(system, "Invalid renderer system pointer");

    return system;
}

void MeshComponent::OnUpdate()
{
    // update proxy placement
    //if (mRenderingProxy != INVALID_RENDER_PROXY)
    //{
    //    MeshProxyDesc desc;
    //    desc.transform = GetEntity()->GetGlobalTransform().ToMatrix();
    //    desc.mesh = mMesh; // TODO this should be removed

    //    RendererSystem* system = GetRendererSystem();
    //    system->GetRenderScene()->UpdateMeshProxy(mRenderingProxy, desc);
    //}
}

void MeshComponent::CreateRenderingProxy()
{
    // cleanup
    DeleteRenderingProxy();

    if (!GetEntity())
        return;

    //// create the new proxy
    //MeshProxyDesc desc;
    //desc.transform = GetEntity()->GetGlobalTransform().ToMatrix();
    //desc.mesh = mMesh;

    //RendererSystem* system = GetRendererSystem();
    //mRenderingProxy = system->GetRenderScene()->CreateMeshProxy(desc);
}

void MeshComponent::DeleteRenderingProxy()
{
    //if (mRenderingProxy != INVALID_RENDER_PROXY)
    //{
    //    RendererSystem* system = GetRendererSystem();
    //    system->GetRenderScene()->DeleteMeshProxy(mRenderingProxy);
    //    mRenderingProxy = INVALID_RENDER_PROXY;
    //}
}

void MeshComponent::OnMeshResourceLoaded()
{
    CreateRenderingProxy();
}

} // namespace Scene
} // namespace NFE
