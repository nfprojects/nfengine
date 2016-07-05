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

namespace NFE {
namespace Resource {

using namespace Math;
using namespace Renderer;

Mesh::Mesh()
{
    mVerticies = 0;
    mIndices = 0;

    mVeriticesCount = 0;
    mIndicesCount = 0;
    mSubMeshesCount = 0;

    mVB = 0;
    mIB = 0;
    mSubMeshes = 0;
}

Mesh::~Mesh()
{
    Release();
}

bool Mesh::AllocateVerticies(uint32 count)
{
    MeshVertex* newVertices = (MeshVertex*)realloc(mVerticies, sizeof(MeshVertex) * count);
    if (newVertices == nullptr)
        return false;

    mVerticies = newVertices;
    mVeriticesCount = count;
    return true;
}

bool Mesh::AllocateIndices(uint32 count)
{
    uint32* newIndices = (uint32*)realloc(mIndices, sizeof(uint32) * count);
    if (newIndices == 0)
        return false;

    mIndices = newIndices;
    mIndicesCount = count;
    return true;
}

bool Mesh::AllocateSubmeshes(uint32 count)
{
    SubMesh* newSubMeshes = (SubMesh*)realloc(mSubMeshes, sizeof(SubMesh) * count);
    if (newSubMeshes == nullptr)
        return false;

    mSubMeshes = newSubMeshes;
    mSubMeshesCount = count;
    return true;
}

bool Mesh::OnLoad()
{
    LOG_INFO("Loading mesh '%s'...", mName);
    Common::Timer timer;
    timer.Start();

    /*
    if (mCustom)
    {
        for (uint32 i = 0; i<mSubMeshesCount; i++)
        {
            if (mpSubMeshes[i].pMaterial)
                mpSubMeshes[i].pMaterial->AddRef();
        }

        mpVB = g_pRenderer->CreateVertexBuffer(mVeriticesCount * sizeof(MeshVertex), mpVerticies);
        if (mpVB == 0)
        {
            LOG_ERROR("Failed to create vertex buffer for mesh '%s'.", mName);
            return false;
        }

        mpIB = g_pRenderer->CreateIndexBuffer(mIndicesCount, mpIndices);
        if (mpIB == 0)
        {
            LOG_ERROR("Failed to create index buffer for mesh '%s'.", mName);
            return false;
        }
    }
    else
    {
    */
    //realtive file path
    std::string path = g_DataPath + "Meshes/" + mName;

    //open file
    FILE* pFile = 0;
    if (fopen_s(&pFile, path.c_str(), "rb") != 0)
    {
        //error, could not open file
        LOG_ERROR("Failed to load '%s'.", mName);
        return false;
    }

    //check file signature (first 4 bytes) - it should be "nfm"
    char signature[4];
    fread(signature, 1, 4, pFile);
    if ((signature[0] != 'n') || (signature[1] != 'f') || (signature[2] != 'm') || (signature[3] != 0))
    {
        //error, could not open file
        LOG_ERROR("Failed to load '%s' - invalid signature.", mName);
        fclose(pFile);
        return false;
    }

    //read basic mesh info
    fread(&mVeriticesCount, sizeof(int), 1, pFile);
    fread(&mIndicesCount, sizeof(int), 1, pFile);
    fread(&mSubMeshesCount, sizeof(int), 1, pFile);

    //read vertices
    MeshVertex* pVerticies = (MeshVertex*)malloc(mVeriticesCount * sizeof(MeshVertex));
    fread(pVerticies, sizeof(MeshVertex), mVeriticesCount, pFile);

    //TEMPORARY!!! Texture coordinates exported from cinema 4d are flipped in 't' axis!!!!
    for (uint32 i = 0; i < mVeriticesCount; i++)
        pVerticies[i].texCoord.y = 1.0f - pVerticies[i].texCoord.y;

    ResManager* rm = Engine::GetInstance()->GetResManager();
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    /// create renderer's vertex buffer
    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.size = mVeriticesCount * sizeof(MeshVertex);
    bufferDesc.initialData = pVerticies;
    bufferDesc.debugName = mName;
    mVB.reset(renderer->GetDevice()->CreateBuffer(bufferDesc));
    if (!mVB)
    {
        LOG_ERROR("Failed to create vertex buffer for mesh '%s'.", mName);
        fclose(pFile);
        return false;
    }

    uint32* pIndices = (uint32*)malloc(mIndicesCount * sizeof(uint32));
    fread(pIndices, sizeof(uint32), mIndicesCount, pFile);

    /// create renderer's index buffer
    bufferDesc.type = BufferType::Index;
    bufferDesc.size = mIndicesCount * sizeof(uint32);
    bufferDesc.initialData = pIndices;
    bufferDesc.debugName = mName;
    mIB.reset(renderer->GetDevice()->CreateBuffer(bufferDesc));
    if (!mIB)
    {
        LOG_ERROR("Failed to create index buffer for mesh '%s'.", mName);
        fclose(pFile);
        return false;
    }


    SubMeshDesc* pSubMeshes = (SubMeshDesc*)malloc(mSubMeshesCount * sizeof(SubMeshDesc));
    fread(pSubMeshes, sizeof(SubMeshDesc), mSubMeshesCount, pFile);
    fclose(pFile);

    mSubMeshes = (SubMesh*)_aligned_malloc(mSubMeshesCount * sizeof(SubMesh), 16);
    for (uint32 i = 0; i < mSubMeshesCount; i++)
    {
        int startIndex = pSubMeshes[i].indexOffset;
        int lastIndex = pSubMeshes[i].indexOffset + 3 * pSubMeshes[i].triangleCount;

        mSubMeshes[i].indexOffset = pSubMeshes[i].indexOffset;
        mSubMeshes[i].trianglesCount = pSubMeshes[i].triangleCount;
        mSubMeshes[i].material = 0;
        mSubMeshes[i].material =
            static_cast<Material*>(rm->GetResource(pSubMeshes[i].materialName,
                                                   ResourceType::Material));
        mSubMeshes[i].material->AddRef();

        Vector vertex;
        Vector vMin = pVerticies[pIndices[startIndex]].position;
        Vector vMax = vMin;

        for (int j = startIndex + 1; j < lastIndex; j++)
        {
            vertex = pVerticies[pIndices[j]].position;
            vMin = VectorMin(vMin, vertex);
            vMax = VectorMax(vMax, vertex);
        }

        mSubMeshes[i].localBox.min = vMin;
        mSubMeshes[i].localBox.max = vMax;
    }

    //find AABB of entire mesh
    if (mSubMeshesCount > 0)
    {
        mLocalBox = mSubMeshes[0].localBox;
        for (uint32 i = 1; i < mSubMeshesCount; i++)
        {
            mLocalBox.min = VectorMin(mLocalBox.min, mSubMeshes[i].localBox.min);
            mLocalBox.max = VectorMax(mLocalBox.max, mSubMeshes[i].localBox.max);
        }
    }

    free(pSubMeshes);
    free(pIndices);
    free(pVerticies);

    LOG_SUCCESS("Mesh '%s' loaded in %.3f sec. Verticies: %u, Indices: %u, Submeshes: %u.", mName,
                timer.Stop(), mVeriticesCount, mIndicesCount, mSubMeshesCount);
    return true;
}

/*
char FloatToChar(float x)
{
    x *= 127.0f;
    x += 0.5f;

    if (x <= -128.0f) return -128;
    if (x >= 127.0f) return 127;

    return (char)x;
}

void Mesh::Create(const CustomMeshVertex* pVerticies, uint32 verticiesCount, uint32* pIndices, uint32 indicesCount, const CustomMeshSubMesh* pSubmeshes, uint32 submeshesCount)
{
    Release();
    mCustom = true;

    //copy verticies
    mVeriticesCount = verticiesCount;
    mpVerticies = (MeshVertex*)malloc(sizeof(MeshVertex) * verticiesCount);
    for (int i = 0; i < verticiesCount; i++)
    {
        mpVerticies[i].Position = pVerticies[i].Position;
        mpVerticies[i].texCoord = pVerticies[i].texCoord;

        mpVerticies[i].Normal[0] = FloatToChar(pVerticies[i].Normal.x);
        mpVerticies[i].Normal[1] = FloatToChar(pVerticies[i].Normal.y);
        mpVerticies[i].Normal[2] = FloatToChar(pVerticies[i].Normal.z);
        mpVerticies[i].Normal[3] = 0;

        mpVerticies[i].Tangent[0] = FloatToChar(pVerticies[i].Tangent.x);
        mpVerticies[i].Tangent[1] = FloatToChar(pVerticies[i].Tangent.y);
        mpVerticies[i].Tangent[2] = FloatToChar(pVerticies[i].Tangent.z);
        mpVerticies[i].Tangent[3] = 0;
    }

    //copy indices
    mIndicesCount = indicesCount;
    mpIndices = (uint32*)malloc(sizeof(uint32) * indicesCount);
    memcpy(mpIndices, pIndices, sizeof(uint32) * indicesCount);


    //copy submeshew & calculate AABB
    mSubMeshesCount = submeshesCount;
    mpSubMeshes = (SubMesh*)_aligned_malloc(submeshesCount * sizeof(SubMesh), 16);
    for (uint32 i = 0; i<mSubMeshesCount; i++)
    {
        mpSubMeshes[i].mindexOffset = pSubmeshes[i].mindexOffset;
        mpSubMeshes[i].mTrianglesCount = pSubmeshes[i].mTrianglesCount;
        mpSubMeshes[i].pMaterial = pSubmeshes[i].pMaterial;

        int startIndex = mpSubMeshes[i].mindexOffset;
        int lastIndex = mpSubMeshes[i].mindexOffset + 3*mpSubMeshes[i].mTrianglesCount;


        Vector vertex;
        Vector vMin = mpVerticies[mpIndices[startIndex]].Position;
        Vector vMax = vMin;

        for (int j = startIndex+1; j<lastIndex; j++)
        {
            vertex = mpVerticies[mpIndices[j]].Position;
            vMin = VectorMin(vMin, vertex);
            vMax = VectorMax(vMax, vertex);
        }

        mpSubMeshes[i].mLocalBox.min = vMin;
        mpSubMeshes[i].mLocalBox.max = vMax;
    }


    //find AABB of entire mesh
    if (mSubMeshesCount > 0)
    {
        mLocalBox = mpSubMeshes[0].mLocalBox;
        for (uint32 i = 1; i<mSubMeshesCount; i++)
        {
            mLocalBox.min = VectorMin(mLocalBox.min, mpSubMeshes[i].mLocalBox.min);
            mLocalBox.max = VectorMax(mLocalBox.max, mpSubMeshes[i].mLocalBox.max);
        }
    }
}
*/

void Mesh::Release()
{
    if (mVerticies)
    {
        free(mVerticies);
        mVerticies = 0;
    }

    if (mIndices)
    {
        free(mIndices);
        mIndices = 0;
    }

    if (mSubMeshes)
    {
        _aligned_free(mSubMeshes);
        mSubMeshes = 0;
    }
}

void Mesh::OnUnload()
{
    LOG_INFO("Unloading mesh '%s'...", mName);

    std::mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::mutex> lock(renderingMutex);

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

//Calculate transformed mesh AABB (fast, not accurate)
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
