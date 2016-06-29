/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh converter implementation
 */

#include "PCH.hpp"
#include "MeshConverter.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/File.hpp"

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
    x *= static_cast<float>(SCHAR_MAX);
    x += 0.5f;

    if (x <= static_cast<float>(SCHAR_MIN)) return SCHAR_MIN;
    if (x >= static_cast<float>(SCHAR_MAX)) return SCHAR_MAX;

    return static_cast<char>(x);
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

    printf("Vertices:  %i\n", verticesCount);
    printf("Indices:   %i\n", indicesCount);
    printf("Submeshes: %i\n", subMeshesCount);
    printf("Materials: %i\n", matCount);

    mVertices.resize(verticesCount);
    mIndices.resize(indicesCount);
    mSubMeshes.resize(subMeshesCount);
    mMaterials.resize(matCount);

    // load vertices
    const ModelOBJ::Vertex* sourceVertices = model.getVertexBuffer();
    for (int i = 0; i < verticesCount; i++)
    {
        // vertex position
        mVertices[i].pos = Float3(sourceVertices[i].position[0],
                                  sourceVertices[i].position[1],
                                  -sourceVertices[i].position[2]);

        // vertex texture coordinates
        mVertices[i].texCoord = Float2(sourceVertices[i].texCoord[0],
                                       sourceVertices[i].texCoord[1]);

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
            mVertices[i].normal[0] = FloatToChar(sourceVertices[i].normal[0]);
            mVertices[i].normal[1] = FloatToChar(sourceVertices[i].normal[1]);
            mVertices[i].normal[2] = FloatToChar(-sourceVertices[i].normal[2]);
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
    Common::File outputFile;
    if (!outputFile.Open(filePath, Common::AccessMode::Write, true))
    {
        LOG_ERROR("Could not open output file '%s'!", filePath.c_str());
        return false;
    }

    struct Header
    {
        int magic;
        int verticesCount;
        int indicesCount;
        int subMeshesCount;
    };

    Header header;
    header.magic = '\0mfn'; // equals to "nfm" string
    header.verticesCount = static_cast<int>(mVertices.size());
    header.indicesCount = static_cast<int>(mIndices.size());
    header.subMeshesCount = static_cast<int>(mSubMeshes.size());

    if (!outputFile.Write(&header, sizeof(header)))
        return false;
    if (!outputFile.Write(mVertices.data(), sizeof(Vertex) * header.verticesCount))
        return false;
    if (!outputFile.Write(mIndices.data(), sizeof(int) * header.indicesCount))
        return false;
    if (!outputFile.Write(mSubMeshes.data(), sizeof(SubMesh) * header.subMeshesCount))
        return false;

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
        Common::File materialFile;
        if (!materialFile.Open(matFileName, Common::AccessMode::Write, true))
        {
            LOG_ERROR("Could not open output file '%s'!", matFileName.c_str());
            return false;
        }
        materialFile.Write(materialString.data(), materialString.length());
    }

    return true;
}

} // namespace NFE
