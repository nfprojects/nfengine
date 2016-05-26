/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of mesh component class.
 */

#include "PCH.hpp"
#include "MeshComponent.hpp"
#include "Engine.hpp"
#include "Resources/ResourcesManager.hpp"

namespace NFE {
namespace Scene {

using namespace Renderer;
using namespace Resource;

NFE_DEFINE_CLASS(MeshComponent, 16, ClassAllocatorType::Pool);
NFE_REGISTER_COMPONENT(MeshComponent);

MeshComponent::MeshComponent()
    : mMesh(nullptr)
{
}

MeshComponent::~MeshComponent()
{
    if (mMesh != nullptr)
    {
        mMesh->DelRef();
        mMesh = nullptr;
    }
}

void MeshComponent::CalcAABB(const Math::Matrix& transform)
{
    if (mMesh != nullptr)
    {
        mGlobalAABB = TransformBox(transform, mMesh->mLocalBox);
    }
}

MeshComponent::MeshComponent(const MeshComponent& other)
{
    mMesh = other.mMesh;
    if (mMesh != nullptr)
        mMesh->AddRef();
}

bool MeshComponent::SetMeshResource(Mesh* resource)
{
    // no change
    if (resource == mMesh)
        return true;

    // delete reference to the previous mesh
    if (mMesh != nullptr)
        mMesh->DelRef();

    mMesh = resource;
    if (mMesh == nullptr)
        return false;

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

} // namespace Scene
} // namespace NFE
