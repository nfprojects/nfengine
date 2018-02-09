/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh resource definition.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Renderer/HighLevelRenderer.hpp"
#include "ResourcesManager.hpp"
#include "Engine.hpp"

#include "nfCommon/Utils/InputStream.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Memory/DefaultAllocator.hpp"


namespace NFE {
namespace Resource {

using namespace Math;
using namespace Renderer;

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    Release();
}

void Mesh::Release()
{
    mSubMeshes.clear();
}

bool Mesh::OnLoad()
{
    NFE_LOG_INFO("Loading mesh '%s'...", mName);
    Common::Timer timer;
    timer.Start();

    // relative file path
    std::string path = g_CookedDataPath + "Meshes/" + mName;

    Common::FileInputStream stream(path.c_str());
    MeshFile meshFile;
    if (!meshFile.Load(stream))
    {
        NFE_LOG_ERROR("Failed to load mesh '%s'", mName);
        return false;
    }

    const auto& vertices = meshFile.GetVertices();
    const auto& indices = meshFile.GetIndices();

    ResManager* rm = Engine::GetInstance()->GetResManager();
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    /// create renderer's vertex buffer
    BufferDesc bufferDesc;
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.size = meshFile.GetVertices().size() * sizeof(MeshVertex);
    bufferDesc.initialData = meshFile.GetVertices().data();
    bufferDesc.debugName = mName;
    mVB = renderer->GetDevice()->CreateBuffer(bufferDesc);
    if (!mVB)
    {
        NFE_LOG_ERROR("Failed to create vertex buffer for mesh '%s'.", mName);
        return false;
    }

    /// create renderer's index buffer
    bufferDesc.type = BufferType::Index;
    bufferDesc.size = meshFile.GetIndices().size() * sizeof(uint32);
    bufferDesc.initialData = meshFile.GetIndices().data();
    bufferDesc.debugName = mName;
    mIB = renderer->GetDevice()->CreateBuffer(bufferDesc);
    if (!mIB)
    {
        NFE_LOG_ERROR("Failed to create index buffer for mesh '%s'.", mName);
        return false;
    }


    const auto& subMeshes = meshFile.GetSubMeshes();
    mSubMeshes.resize(subMeshes.size());
    for (uint32 i = 0; i < subMeshes.size(); i++)
    {
        int startIndex = subMeshes[i].indexOffset;
        int lastIndex = subMeshes[i].indexOffset + 3 * subMeshes[i].triangleCount;

        mSubMeshes[i].indexOffset = subMeshes[i].indexOffset;
        mSubMeshes[i].trianglesCount = subMeshes[i].triangleCount;
        mSubMeshes[i].material = static_cast<Material*>(
            rm->GetResource(subMeshes[i].materialName, ResourceType::Material));
        mSubMeshes[i].material->AddRef();

        Vector4 vertex;
        Vector4 vMin = Vector4(vertices[indices[startIndex]].position);
        Vector4 vMax = vMin;

        for (int j = startIndex + 1; j < lastIndex; j++)
        {
            vertex = Vector4(vertices[indices[j]].position);
            vMin = Vector4::Min(vMin, vertex);
            vMax = Vector4::Max(vMax, vertex);
        }

        mSubMeshes[i].localBox.min = vMin;
        mSubMeshes[i].localBox.max = vMax;
    }

    // find AABB of entire mesh
    if (mSubMeshes.size() > 0)
    {
        mLocalBox = mSubMeshes[0].localBox;
        for (uint32 i = 1; i < mSubMeshes.size(); i++)
        {
            mLocalBox.min = Vector4::Min(mLocalBox.min, mSubMeshes[i].localBox.min);
            mLocalBox.max = Vector4::Max(mLocalBox.max, mSubMeshes[i].localBox.max);
        }
    }

    NFE_LOG_SUCCESS("Mesh '%s' loaded in %.3f sec. Verticies: %zu, Indices: %zu, Submeshes: %zu.", mName,
                timer.Stop(), vertices.size(), indices.size(), subMeshes.size());
    return true;
}

void Mesh::OnUnload()
{
    NFE_LOG_INFO("Unloading mesh '%s'...", mName);

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::lock_guard<std::recursive_mutex> lock(renderingMutex);

    mVB.Reset();
    mIB.Reset();

    // delete materials references
    for (auto& submesh : mSubMeshes)
    {
        if (submesh.material)
        {
            submesh.material->DelRef();
            submesh.material = nullptr;
        }
    }
}

} // namespace Resource
} // namespace NFE
