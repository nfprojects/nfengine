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

#include "nfCommon/Logger.hpp"
#include "nfCommon/Memory/DefaultAllocator.hpp"


namespace NFE {
namespace Resource {

using namespace Math;
using namespace Renderer;

Mesh::Mesh()
    : mVeriticesCount(0)
    , mIndicesCount(0)
    , mSubMeshesCount(0)
    , mSubMeshes(nullptr)
{}

Mesh::~Mesh()
{
    Release();
}

void Mesh::Release()
{
    if (mSubMeshes)
    {
        NFE_FREE(mSubMeshes);
        mSubMeshes = nullptr;
    }
}

bool Mesh::OnLoad()
{
    LOG_INFO("Loading mesh '%s'...", mName);
    Common::Timer timer;
    timer.Start();


    //realtive file path
    std::string path = g_CookedDataPath + "Meshes/" + mName;

    // TODO support for loading from buffer
    Common::File file;
    if (!file.Open(path, Common::AccessMode::Read))
    {
        LOG_ERROR("Failed to load '%s'.", mName);
        return false;
    }

    MeshHeader header;
    if (file.Read(&header, sizeof(header)) != sizeof(header))
    {
        LOG_ERROR("Failed to load '%s' - can't read mesh signature", mName);
        return false;
    }

    if (header.magic != NFE_MESH_FILE_MAGIC)
    {
        LOG_ERROR("Failed to load '%s' - invalid signature", mName);
        return false;
    }

    // sanity checks
    if (header.indicesCount == 0 || header.subMeshesCount == 0 || header.verticesCount == 0)
    {
        LOG_ERROR("Failed to load '%s' - corrupted file?", mName);
        return false;
    }

    mVeriticesCount = header.verticesCount;
    mIndicesCount = header.indicesCount;
    mSubMeshesCount = header.subMeshesCount;


    const size_t verticesSize = mVeriticesCount * sizeof(MeshVertex);
    const size_t indicesSize = mIndicesCount * sizeof(uint32);
    const size_t subMeshesSize = mSubMeshesCount * sizeof(MeshSubMesh);
    const size_t bufferSize = verticesSize + indicesSize + subMeshesSize;
    char* buffer = reinterpret_cast<char*>(NFE_MALLOC(bufferSize, 1));
    if (!buffer)
    {
        LOG_ERROR("Failed to load '%s' - memory allocation error", mName);
        return false;
    }

    // read mesh data
    if (file.Read(buffer, bufferSize) != bufferSize)
    {
        NFE_FREE(buffer);
        LOG_ERROR("Failed to load '%s' - can't read mesh data", mName);
        return false;
    }
    file.Close();

    MeshVertex* vertices = reinterpret_cast<MeshVertex*>(buffer);
    uint32* indices = reinterpret_cast<uint32*>(buffer + verticesSize);
    MeshSubMesh* subMeshes = reinterpret_cast<MeshSubMesh*>(buffer + verticesSize + indicesSize);


    ResManager* rm = Engine::GetInstance()->GetResManager();
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    /// create renderer's vertex buffer
    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.size = verticesSize;
    bufferDesc.initialData = vertices;
    bufferDesc.debugName = mName;
    mVB.reset(renderer->GetDevice()->CreateBuffer(bufferDesc));
    if (!mVB)
    {
        NFE_FREE(buffer);
        LOG_ERROR("Failed to create vertex buffer for mesh '%s'.", mName);
        return false;
    }

    /// create renderer's index buffer
    bufferDesc.type = BufferType::Index;
    bufferDesc.size = indicesSize;
    bufferDesc.initialData = indices;
    bufferDesc.debugName = mName;
    mIB.reset(renderer->GetDevice()->CreateBuffer(bufferDesc));
    if (!mIB)
    {
        NFE_FREE(buffer);
        LOG_ERROR("Failed to create index buffer for mesh '%s'.", mName);
        return false;
    }

    mSubMeshes = reinterpret_cast<SubMesh*>(NFE_MALLOC(sizeof(SubMesh) * header.subMeshesCount, 16));
    if (!mSubMeshes)
    {
        NFE_FREE(buffer);
        LOG_ERROR("Failed to load '%s' - memory allocation error", mName);
        return false;
    }

    for (uint32 i = 0; i < mSubMeshesCount; i++)
    {
        int startIndex = subMeshes[i].indexOffset;
        int lastIndex = subMeshes[i].indexOffset + 3 * subMeshes[i].triangleCount;

        mSubMeshes[i].indexOffset = subMeshes[i].indexOffset;
        mSubMeshes[i].trianglesCount = subMeshes[i].triangleCount;
        mSubMeshes[i].material = static_cast<Material*>(
            rm->GetResource(subMeshes[i].materialName, ResourceType::Material));
        mSubMeshes[i].material->AddRef();

        Vector vertex;
        Vector vMin = vertices[indices[startIndex]].position;
        Vector vMax = vMin;

        for (int j = startIndex + 1; j < lastIndex; j++)
        {
            vertex = vertices[indices[j]].position;
            vMin = VectorMin(vMin, vertex);
            vMax = VectorMax(vMax, vertex);
        }

        mSubMeshes[i].localBox.min = vMin;
        mSubMeshes[i].localBox.max = vMax;
    }

    // find AABB of entire mesh
    if (mSubMeshesCount > 0)
    {
        mLocalBox = mSubMeshes[0].localBox;
        for (uint32 i = 1; i < mSubMeshesCount; i++)
        {
            mLocalBox.min = VectorMin(mLocalBox.min, mSubMeshes[i].localBox.min);
            mLocalBox.max = VectorMax(mLocalBox.max, mSubMeshes[i].localBox.max);
        }
    }

    NFE_FREE(buffer);
    LOG_SUCCESS("Mesh '%s' loaded in %.3f sec. Verticies: %u, Indices: %u, Submeshes: %u.", mName,
                timer.Stop(), mVeriticesCount, mIndicesCount, mSubMeshesCount);

    return true;
}

void Mesh::OnUnload()
{
    LOG_INFO("Unloading mesh '%s'...", mName);

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::lock_guard<std::recursive_mutex> lock(renderingMutex);

    mVB.reset();
    mIB.reset();

    if (mSubMeshes)
    {
        // delete materials references
        for (uint32 i = 0; i < mSubMeshesCount; i++)
            if (mSubMeshes[i].material)
            {
                mSubMeshes[i].material->DelRef();
                mSubMeshes[i].material = nullptr;
            }
    }
}

Box Mesh::GetGlobalAABB(const Matrix& matrix)
{
    Box result;
    result.min = result.max = LinearCombination3(mLocalBox.GetVertex(0), matrix);

    for (int i = 1; i < 8; i++)
    {
        Vector vert = LinearCombination3(mLocalBox.GetVertex(i), matrix);
        result.min = VectorMin(result.min, vert);
        result.max = VectorMax(result.max, vert);
    }

    return result;
}

} // namespace Resource
} // namespace NFE
