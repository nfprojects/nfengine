/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of mesh component class.
 */

#include "../PCH.hpp"
#include "MeshComponent.hpp"
#include "../Globals.hpp"
#include "../ResourcesManager.hpp"

namespace NFE {
namespace Scene {

using namespace Renderer;
using namespace Resource;

MeshComponent::MeshComponent()
{
    mMesh = nullptr;
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

    //add reference to the new mesh
    mMesh->AddRef();
    return true;
}

bool MeshComponent::SetMeshResource(const char* name)
{
    Mesh* newMesh = static_cast<Mesh*>(g_pResManager->GetResource(name, ResourceType::Mesh));
    return SetMeshResource(newMesh);
}

// TODO: move to renderer system?
/*
void MeshComponent::OnRenderDebug(RenderContext* pCtx)
{
    if (mMesh)
    {
        Math::Box box = mMesh->GetGlobalAABB(mOwner->mMatrix);
        // DebugRenderer::Get()->DrawBox(pCtx, box, 0xFF0000FF);
        DebugRenderer::Get()->DrawMesh(pCtx, mMesh, mOwner->mMatrix);
    }
}
*/

} // namespace Scene
} // namespace NFE
