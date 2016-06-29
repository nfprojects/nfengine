/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh importer implementation
 */

#include "PCH.hpp"
#include "MeshImporter.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/File.hpp"

#include "model_obj/model_obj.h"


namespace NFE {

using namespace Math;
using namespace Resource;
using namespace Common;

namespace {

// replace backslashes with forward slashes
NFE_INLINE std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}

int8 FloatToInt8(float x)
{
    x *= static_cast<float>(SCHAR_MAX);
    x += 0.5f;

    if (x <= static_cast<float>(SCHAR_MIN)) return SCHAR_MIN;
    if (x >= static_cast<float>(SCHAR_MAX)) return SCHAR_MAX;

    return static_cast<int8>(x);
}

} // namespace

bool MeshImporter::ImportOBJ(const std::string& sourceFilePath, const std::string& targetFilePath)
{
    ModelOBJ model;

    // open mesh
    if (!model.import(sourceFilePath.c_str()))
    {
        LOG_ERROR("Failed to open file '%s'!", sourceFilePath.c_str());
        return false;
    }

    // write basic info
    int verticesCount = model.getNumberOfVertices();
    int indicesCount = model.getNumberOfIndices();
    int subMeshesCount = model.getNumberOfMeshes();
    int matCount = model.getNumberOfMaterials();

    LOG_INFO("Mesh '%s' loaded. Vertices: %i, Indices: %i, Submeshes: %i, Materials: %i",
             sourceFilePath.c_str(), verticesCount, indicesCount, subMeshesCount, matCount);

    mVertices.resize(verticesCount);
    mIndices.resize(indicesCount);
    mSubMeshes.resize(subMeshesCount);
    mMaterials.resize(matCount);

    // load vertices
    const ModelOBJ::Vertex* sourceVertices = model.getVertexBuffer();
    for (int i = 0; i < verticesCount; i++)
    {
        // vertex position
        mVertices[i].position = Float3( sourceVertices[i].position[0],
                                        sourceVertices[i].position[1],
                                       -sourceVertices[i].position[2]);

        // vertex texture coordinates
        mVertices[i].texCoord = Float2(sourceVertices[i].texCoord[0],
                                       1.0f - sourceVertices[i].texCoord[1]);

        // vertex normal
        if (IsNaN(sourceVertices[i].normal[0]) || IsNaN(sourceVertices[i].normal[1]) ||
            IsNaN(sourceVertices[i].normal[2]))
        {
            mVertices[i].normal[0] = 0;
            mVertices[i].normal[1] = SCHAR_MAX;
            mVertices[i].normal[2] = 0;
        }
        else
        {
            mVertices[i].normal[0] = FloatToInt8(sourceVertices[i].normal[0]);
            mVertices[i].normal[1] = FloatToInt8(sourceVertices[i].normal[1]);
            mVertices[i].normal[2] = FloatToInt8(-sourceVertices[i].normal[2]);
        }

        // vertex tangent
        if (IsNaN(sourceVertices[i].tangent[0]) || IsNaN(sourceVertices[i].tangent[1]) ||
            IsNaN(sourceVertices[i].tangent[2]))
        {
            mVertices[i].tangent[0] = SCHAR_MAX;
            mVertices[i].tangent[1] = 0;
            mVertices[i].tangent[2] = 0;
        }
        else
        {
            mVertices[i].tangent[0] = FloatToInt8(sourceVertices[i].tangent[0]);
            mVertices[i].tangent[1] = FloatToInt8(sourceVertices[i].tangent[1]);
            mVertices[i].tangent[2] = FloatToInt8(-sourceVertices[i].tangent[2]);
        }

        mVertices[i].normal[3] = 0;
        mVertices[i].tangent[3] = 0;
    }

    // read indices
    memcpy(mIndices.data(), model.getIndexBuffer(), indicesCount * sizeof(uint32));

    // read submeshes
    for (int i = 0; i < model.getNumberOfMeshes(); i++)
    {
        ModelOBJ::Mesh srcMesh = model.getMesh(i);
        MeshSubMesh& subMesh = mSubMeshes[i];

        subMesh.indexOffset = srcMesh.startIndex;
        subMesh.triangleCount = srcMesh.triangleCount;
        strcpy(subMesh.materialName, srcMesh.pMaterial->name.c_str());
    }

    // read materials
    for (int i = 0; i < matCount; i++)
    {
        const ModelOBJ::Material& sourceMaterial = model.getMaterial(i);
        MaterialDesc& material = mMaterials[i];

        material.name = sourceMaterial.name;
        material.diffuseTextureFileName = sourceMaterial.colorMapFilename;
        material.normalTextureFileName = sourceMaterial.bumpMapFilename;
    }

    // generate material files
    for (const MaterialDesc& mat : mMaterials)
    {
        std::string matFileName = FileSystem::GetParentDir(targetFilePath);
        matFileName += "/../Materials/";
        matFileName += mat.name;
        matFileName += ".json";

        // check if material already exists
        if (FileSystem::GetPathType(matFileName) == PathType::File)
        {
            LOG_INFO("Material %s already exists. Skipping material file generation...",
                     mat.name.c_str());
            continue;
        }


        std::string materialString;
        materialString = "{\n\t\"Layers\" :\n\t[\n\t\t{\n";

        if (mat.diffuseTextureFileName.length())
            materialString += "\t\t\t\"DiffuseTexture\" : \"" + FixTexturePath(mat.diffuseTextureFileName) + "\"";

        if (mat.diffuseTextureFileName.length() > 0 && mat.normalTextureFileName.length() > 0)
            materialString += ",\n";
        else
            materialString += "\n";

        if (mat.normalTextureFileName.length())
            materialString += "\t\t\t\"NormalTexture\" : \"" + FixTexturePath(mat.normalTextureFileName) + "\"\n";

        materialString += "\t\t}\n\t]\n}\n";


        // TODO This is temporary:
        // * material file generation must be redesigned
        // * material resource files must be binary
        File materialFile;
        if (!materialFile.Open(matFileName, AccessMode::Write, true))
        {
            LOG_ERROR("Could not open output file '%s'!", matFileName.c_str());
            return false;
        }
        materialFile.Write(materialString.data(), materialString.length());
    }

    return Save(FileOutputStream(targetFilePath.c_str()));
}

} // namespace NFE
