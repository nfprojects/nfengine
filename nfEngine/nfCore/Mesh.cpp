/**
    NFEngine project

    \file   Mesh.cpp
    \brief  Mesh resource definition.
*/

#include "stdafx.hpp"
#include "Globals.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Renderer.hpp"
#include "ResourcesManager.hpp"
#include "Engine.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Resource {

using namespace Math;
using namespace Render;

Mesh::Mesh()
{
    mVerticies = 0;
    mIndicies = 0;

    mVeriticesCount = 0;
    mIndiciesCount = 0;
    mSubMeshesCount = 0;

    mVB = 0;
    mIB = 0;
    mSubMeshes = 0;
}

Mesh::~Mesh()
{
    Release();
}

Result Mesh::AllocateVerticies(uint32 count)
{
    mVerticies = (MeshVertex*)realloc(mVerticies, sizeof(MeshVertex) * count);

    if (mVerticies == 0) // TODO: fix memory leak on realloc failure
    {
        mVeriticesCount = 0;
        return Result::AllocationError;
    }

    mVeriticesCount = count;
    return Result::OK;
}

Result Mesh::AllocateIndicies(uint32 count)
{
    mIndicies = (uint32*)realloc(mIndicies, sizeof(uint32) * count);

    if (mIndicies == 0) // TODO: fix memory leak on realloc failure
    {
        mIndiciesCount = 0;
        return Result::AllocationError;
    }

    mIndiciesCount = count;
    return Result::OK;
}

Result Mesh::AllocateSubmeshes(uint32 count)
{
    mSubMeshes = (SubMesh*)realloc(mSubMeshes, sizeof(SubMesh) * count);

    if (mSubMeshes == 0) // TODO: fix memory leak on realloc failure
    {
        mSubMeshesCount = 0;
        return Result::AllocationError;
    }

    mSubMeshesCount = count;
    return Result::OK;
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

        mpIB = g_pRenderer->CreateIndexBuffer(mIndiciesCount, mpIndicies);
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
    fread(&mIndiciesCount, sizeof(int), 1, pFile);
    fread(&mSubMeshesCount, sizeof(int), 1, pFile);

    //read vertices
    MeshVertex* pVerticies = (MeshVertex*)malloc(mVeriticesCount * sizeof(MeshVertex));
    fread(pVerticies, sizeof(MeshVertex), mVeriticesCount, pFile);

    //TEMPORARY!!! Texture coordinates exported from cinema 4d are flipped in 't' axis!!!!
    for (uint32 i = 0; i < mVeriticesCount; i++)
        pVerticies[i].texCoord.y = 1.0f - pVerticies[i].texCoord.y;

    mVB = g_pRenderer->CreateBuffer();
    if (!mVB ||
            !mVB->Init(IRendererBuffer::Type::Vertex, pVerticies, mVeriticesCount * sizeof(MeshVertex)))
    {
        LOG_ERROR("Failed to create vertex buffer for mesh '%s'.", mName);
        fclose(pFile);
        return false;
    }

    uint32* pIndicies = (uint32*)malloc(mIndiciesCount * sizeof(uint32));
    fread(pIndicies, sizeof(uint32), mIndiciesCount, pFile);
    mIB = g_pRenderer->CreateBuffer();
    if (!mIB ||
            !mIB->Init(IRendererBuffer::Type::Index, pIndicies, mIndiciesCount * sizeof(uint32)))
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
        mSubMeshes[i].material = ENGINE_GET_MATERIAL(pSubMeshes[i].materialName);
        mSubMeshes[i].material->AddRef();

        Vector vertex;
        Vector vMin = pVerticies[pIndicies[startIndex]].position;
        Vector vMax = vMin;

        for (int j = startIndex + 1; j < lastIndex; j++)
        {
            vertex = pVerticies[pIndicies[j]].position;
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
    free(pIndicies);
    free(pVerticies);

    LOG_SUCCESS("Mesh '%s' loaded in %.3f sec. Verticies: %u, Indicies: %u, Submeshes: %u.", mName,
                timer.Stop(), mVeriticesCount, mIndiciesCount, mSubMeshesCount);
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

void Mesh::Create(const CustomMeshVertex* pVerticies, uint32 verticiesCount, uint32* pIndicies, uint32 indiciesCount, const CustomMeshSubMesh* pSubmeshes, uint32 submeshesCount)
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

    //copy indicies
    mIndiciesCount = indiciesCount;
    mpIndicies = (uint32*)malloc(sizeof(uint32) * indiciesCount);
    memcpy(mpIndicies, pIndicies, sizeof(uint32) * indiciesCount);


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
        Vector vMin = mpVerticies[mpIndicies[startIndex]].Position;
        Vector vMax = vMin;

        for (int j = startIndex+1; j<lastIndex; j++)
        {
            vertex = mpVerticies[mpIndicies[j]].Position;
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

    if (mIndicies)
    {
        free(mIndicies);
        mIndicies = 0;
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

    if (mSubMeshes)
    {
        //delete all references
        for (uint32 i = 0; i < mSubMeshesCount; i++)
            if (mSubMeshes[i].material)
                mSubMeshes[i].material->DelRef();
    }

    if (mVB)
    {
        delete mVB;
        mVB = 0;
    }

    if (mIB)
    {
        delete mIB;
        mIB = 0;
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
