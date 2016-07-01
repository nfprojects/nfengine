/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape importer implementation
 */

#include "PCH.hpp"
#include "CollisionShapeImporter.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/OutputStream.hpp"
#include "nfCommon/Config.hpp"

#include "model_obj/model_obj.h"


namespace NFE {
namespace Resource {

using namespace Math;
using namespace Common;

namespace {

// replace backslashes with forward slashes
NFE_INLINE std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}


CollisionShapeFile::Shape* ParseCylinder(const ConfigValue& value)
{
    // TODO
    return nullptr;
}

} // namespace


bool CollisionShapeImporter::ImportCFG(const std::string& sourceFilePath, const std::string& targetFilePath)
{
    Config config;

    auto rootIterator = [&] (const char* key, const ConfigValue& value)
    {
        CollisionShapeType type = CollisionShapeType::Invalid;

        if (strcmp(key, "box") == 0)
        {
            type = CollisionShapeType::Box;
        }
        else if (strcmp(key, "cylinder") == 0)
        {
            type = CollisionShapeType::Cylinder;
        }
        else
        {
            LOG_ERROR("Invalid shape type: '%s'", key);
            return false;
        }

        if (!value.IsObject())
        {
            LOG_ERROR("Object expected");
            return false;
        }

        return true;
    };

    config.Iterate(rootIterator);
}

bool CollisionShapeImporter::ImportOBJ(const std::string& sourceFilePath, const std::string& targetFilePath)
{
    ModelOBJ model;
    if (!model.import(sourceFilePath.c_str(), true))
    {
        LOG_ERROR("Failed to open file '%s'!\n", sourceFilePath.c_str());
        return false;
    }

    // collision shapes imported from OBJ will contain single triangle mesh
    std::unique_ptr<TriangleMeshShape> mesh(new TriangleMeshShape);

    // write vertices
    const ModelOBJ::Vertex* modelVertices = model.getVertexBuffer();
    mesh->vertices.reserve(model.getNumberOfVertices());
    for (int i = 0; i < model.getNumberOfVertices(); i++)
    {
        Math::Float3 vertex;
        vertex.x = modelVertices[i].position[0];
        vertex.y = modelVertices[i].position[1];
        vertex.z = -modelVertices[i].position[2];
        mesh->vertices.push_back(vertex);
    }


    // write triangles
    const int* modelIndicies = model.getIndexBuffer();
    for (int i = 0; i < model.getNumberOfMeshes(); i++)
    {
        ModelOBJ::Mesh srcMesh = model.getMesh(i);
        for (int j = 0; j < srcMesh.triangleCount; j++)
        {
            mesh->indices.push_back(modelIndicies[srcMesh.startIndex + 3 * j]);
            mesh->indices.push_back(modelIndicies[srcMesh.startIndex + 3 * j + 1]);
            mesh->indices.push_back(modelIndicies[srcMesh.startIndex + 3 * j + 2]);
        }
    }

    mShapes.emplace_back(std::move(mesh));


    Common::FileOutputStream stream(targetFilePath.c_str());
    return Save(stream);
}

} // namespace Resource
} // namespace NFE
