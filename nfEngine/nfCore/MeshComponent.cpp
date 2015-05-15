/**
 * @file   MeshComponent.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of mesh component class.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "MeshComponent.hpp"
#include "Entity.hpp"
#include "ResourcesManager.hpp"
#include "..\nfCommon\InputStream.hpp"
#include "..\nfCommon\OutputStream.hpp"

namespace NFE {
namespace Scene {

using namespace Render;
using namespace Resource;

MeshComponent::MeshComponent(Entity* pParent) : Component(pParent)
{
    mType = ComponentType::Mesh;
    mMesh = 0;
}

MeshComponent::~MeshComponent()
{
    if (mMesh)
    {
        mMesh->DelRef();
        mMesh = 0;
    }
}

void MeshComponent::CalcAABB()
{
    if (mMesh != NULL)
    {
        mGlobalAABB = TransformBox(mOwner->mMatrix, mMesh->mLocalBox);
    }
}

Result MeshComponent::SetMeshResource(Mesh* pResource)
{
    if (pResource == mMesh)
        return Result::OK;

    //delete reference to the previous mesh
    if (mMesh)
        mMesh->DelRef();


    mMesh = pResource;
    if (mMesh == 0)
        return Result::Error; //failed to open resource

    //add reference to the new mesh
    mMesh->AddRef();
    return Result::OK;
}

Result MeshComponent::SetMeshResource(const char* pName)
{
    Mesh* pNewMesh = (Mesh*)g_pResManager->GetResource(pName, ResourceType::Mesh);
    return SetMeshResource(pNewMesh);
}

void MeshComponent::OnRenderDebug(IRenderContext* pCtx)
{
    if (mMesh)
    {
        Math::Box box = mMesh->GetGlobalAABB(mOwner->mMatrix);
        g_pDebugRenderer->DrawBox(pCtx, box, 0xFF0000FF);
    }
}


Result MeshComponent::Deserialize(Common::InputStream* pStream)
{
    MeshComponentDesc desc;
    if (pStream->Read(sizeof(desc), &desc) != sizeof(desc))
        return Result::Error;

    //verify name
    for (int i = 0; i < RES_NAME_MAX_LENGTH; i++)
    {
        if (desc.meshResourceName[i] == 0) //find null-termination
            goto l_ValidName;
    }
    return Result::Error; //null-termination not found

l_ValidName:
    return SetMeshResource(desc.meshResourceName);
}

Result MeshComponent::Serialize(Common::OutputStream* pStream) const
{
    MeshComponentDesc desc;
    ZeroMemory(&desc, sizeof(desc));
    strcpy(desc.meshResourceName, mMesh->mName);

    if (pStream->Write(&desc, sizeof(desc)) != sizeof(desc))
        return Result::Error;

    return Result::OK;
}

} // namespace Scene
} // namespace NFE
