/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh converter implementation
 */

#include "PCH.hpp"
#include "MeshConverter.hpp"

#include "../../nfCommon/Logger.hpp"
#include "../../nfCommon/FileSystem.hpp"

#include "model_obj/model_obj.h"

namespace NFE {

using namespace Math;

namespace {

// replace backslashes with forward slashes
inline std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}

char FloatToChar(float x)
{
    x *= 127.0f;
    x += 0.5f;

    if (x <= -128.0f) return -128;
    if (x >= 127.0f) return 127;

    return (char)x;
}

} // namespace

bool MeshFile::LoadFromObj(const std::string& filePath)
{
    ModelOBJ model;

    // open mesh
    if (!model.import(filePath.c_str()))
    {
        LOG_ERROR("Failed to open file '%s'!", filePath.c_str());
        return false;
    }

    // write basic info
    int verticesCount = model.getNumberOfVertices();
    int indicesCount = model.getNumberOfIndices();
    int subMeshesCount = model.getNumberOfMeshes();
    int matCount = model.getNumberOfMaterials();

    printf("Veritces:  %i\n", verticesCount);
    printf("Indices:   %i\n", indicesCount);
    printf("Submeshes: %i\n", subMeshesCount);
    printf("Materials: %i\n", matCount);

    mVertices.resize(verticesCount);
    mIndices.resize(indicesCount);
    mSubMeshes.resize(subMeshesCount);
    mMaterials.resize(matCount);

    // load verticies
    const ModelOBJ::Vertex* sourceVertices = model.getVertexBuffer();
    for (int i = 0; i < verticesCount; i++)
    {
        //vertex position
        mVertices[i].pos = Float3(sourceVertices[i].position[0],
                                  sourceVertices[i].position[1],
                                  -sourceVertices[i].position[2]);

        //vertex texture coordinates
        mVertices[i].texCoord = Float2(sourceVertices[i].texCoord[0],
                                       sourceVertices[i].texCoord[1]);

        // vertex normal
        if (IsNaN(sourceVertices[i].normal[0]) || IsNaN(sourceVertices[i].normal[1]) ||
            IsNaN(sourceVertices[i].normal[2]))
        {
            mVertices[i].normal[0] = 0;
            mVertices[i].normal[1] = 127;
            mVertices[i].normal[2] = 0;
        }
        else
        {
            mVertices[i].normal[0] = FloatToChar(sourceVertices[i].normal[0]);
            mVertices[i].normal[1] = FloatToChar(sourceVertices[i].normal[1]);
            mVertices[i].normal[2] = FloatToChar(-sourceVertices[i].normal[2]);
        }

        // vertex tangent
        if (IsNaN(sourceVertices[i].tangent[0]) || IsNaN(sourceVertices[i].tangent[1]) ||
            IsNaN(sourceVertices[i].tangent[2]))
        {
            mVertices[i].tangent[0] = 127;
            mVertices[i].tangent[1] = 0;
            mVertices[i].tangent[2] = 0;
        }
        else
        {
            mVertices[i].tangent[0] = FloatToChar(sourceVertices[i].tangent[0]);
            mVertices[i].tangent[1] = FloatToChar(sourceVertices[i].tangent[1]);
            mVertices[i].tangent[2] = FloatToChar(-sourceVertices[i].tangent[2]);
        }

        mVertices[i].normal[3] = 0;
        mVertices[i].tangent[3] = 0;
    }

    // read indices
    memcpy(mIndices.data(), model.getIndexBuffer(), indicesCount * sizeof(unsigned int));

    // read submeshes
    for (int i = 0; i < model.getNumberOfMeshes(); i++)
    {
        ModelOBJ::Mesh srcMesh = model.getMesh(i);
        SubMesh& subMesh = mSubMeshes[i];

        subMesh.indexOffset = srcMesh.startIndex;
        subMesh.triangleCount = srcMesh.triangleCount;
        strcpy(subMesh.materialName, srcMesh.pMaterial->name.c_str());
    }

    // read materials
    for (int i = 0; i < matCount; i++)
    {
        const ModelOBJ::Material& sourceMaterial = model.getMaterial(i);
        Material& material = mMaterials[i];

        material.name = sourceMaterial.name;
        material.diffuseTextureFileName = sourceMaterial.colorMapFilename;
        material.normalTextureFileName = sourceMaterial.bumpMapFilename;
    }

    return true;
}

bool MeshFile::Save(const std::string& filePath) const
{
    // TODO use async IO
    FILE* outputFile = fopen(filePath.c_str(), "wb");
    if (outputFile == nullptr)
    {
        LOG_ERROR("Could not open output file '%s'!", filePath.c_str());
        return false;
    }

    int verticesCount = static_cast<int>(mVertices.size());
    int indicesCount = static_cast<int>(mIndices.size());
    int subMeshesCount = static_cast<int>(mSubMeshes.size());

    fwrite("nfm", 4, 1, outputFile); //signature
    fwrite(&verticesCount, sizeof(int), 1, outputFile);
    fwrite(&indicesCount, sizeof(int), 1, outputFile);
    fwrite(&subMeshesCount, sizeof(int), 1, outputFile);
    fwrite(mVertices.data(), sizeof(Vertex), verticesCount, outputFile);
    fwrite(mIndices.data(), sizeof(int), indicesCount, outputFile);
    fwrite(mSubMeshes.data(), sizeof(SubMesh), subMeshesCount, outputFile);
    fclose(outputFile);

    // generate material files
    for (const Material& mat : mMaterials)
    {
        std::string matFileName = Common::FileSystem::GetParentDir(filePath);
        matFileName += "/../Materials/";
        matFileName += mat.name;
        matFileName += ".json";

        // check if material already exists
        if (Common::FileSystem::GetPathType(matFileName) == Common::PathType::File)
        {
            LOG_INFO("Material %s already exists. Skipping material file generation...",
                     mat.name.c_str());
            continue;
        }

        // TODO This is temporary. Material file generation must be redesigned.
        FILE* materialFile = fopen(matFileName.c_str(), "w");
        if (materialFile == nullptr)
        {
            LOG_ERROR("Could not open output file '%s'!", matFileName.c_str());
            return false;
        }

        fprintf(materialFile, "{\n\t\"Layers\" :\n\t[\n\t\t{\n");

        if (mat.diffuseTextureFileName.length())
            fprintf(materialFile, "\t\t\t\"DiffuseTexture\" : \"%s\"",
                    FixTexturePath(mat.diffuseTextureFileName).c_str());

        if (mat.diffuseTextureFileName.length() > 0 && mat.normalTextureFileName.length() > 0)
            fprintf(materialFile, ",\n");
        else
            fprintf(materialFile, "\n");

        if (mat.normalTextureFileName.length())
            fprintf(materialFile, "\t\t\t\"NormalTexture\" : \"%s\"\n",
                    FixTexturePath(mat.normalTextureFileName).c_str());

        fprintf(materialFile, "\t\t}\n\t]\n}\n");
        fclose(materialFile);
    }

    return true;
}

} // namespace NFE
