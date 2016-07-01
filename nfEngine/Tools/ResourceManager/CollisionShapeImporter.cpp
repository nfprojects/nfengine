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

// parse common shape properties (transform matrix, mass, etc.)
bool ParseCommonProperties(const Config& config, const ConfigValue& value, CollisionShapeFile::Shape* shape)
{
    const char* TRANSLATION_PROPERTY = "translation";

    bool errorOccurred = false;
    auto callback = [&](const char* key, const ConfigValue& value) -> bool
    {
        if (strcmp(key, TRANSLATION_PROPERTY) == 0)
        {
            if (!value.IsArray())
            {
                LOG_ERROR("Float array expected");
                errorOccurred = true;
                return false;
            }

            if (!config.IterateArray(shape->translation.f, 3, value.GetArray()))
            {
                LOG_ERROR("Failed to parse '%s' array", TRANSLATION_PROPERTY);
                errorOccurred = true;
                return false;
            }
        }

        return true;
    };

    config.Iterate(callback, value.GetObj());
    return !errorOccurred;
}

std::unique_ptr<CollisionShapeFile::Shape> ParseBox(const Config& config, const ConfigValue& value)
{
    if (!value.IsObject())
    {
        LOG_ERROR("Object expected");
        return nullptr;
    }

    std::unique_ptr<CollisionShapeFile::BoxShape> result(new CollisionShapeFile::BoxShape);

    bool errorOccurred = false;
    auto callback = [&](const char* key, const ConfigValue& value) -> bool
    {
        if (strcmp(key, "halfSize") == 0)
        {
            if (!value.IsArray())
            {
                LOG_ERROR("Float array expected");
                errorOccurred = true;
                return false;
            }

            if (!config.IterateArray(result->halfSize.f, 3, value.GetArray()))
            {
                LOG_ERROR("Failed to parse 'halfSize' array");
                errorOccurred = true;
                return false;
            }
        }

        return true;
    };

    config.Iterate(callback, value.GetObj());
    if (errorOccurred)
    {
        return nullptr;
    }

    if (result->halfSize[0] <= 0.0f && result->halfSize[1] <= 0.0f && result->halfSize[2] <= 0.0f)
    {
        LOG_ERROR("Box size must be specified in the config");
        return nullptr;
    }

    return result;
}

std::unique_ptr<CollisionShapeFile::Shape> ParseCylinder(const Config& config, const ConfigValue& value)
{
    if (!value.IsObject())
    {
        LOG_ERROR("Object expected");
        return nullptr;
    }

    std::unique_ptr<CollisionShapeFile::CylinderShape> result(new CollisionShapeFile::CylinderShape);
    bool errorOccurred = false;
    result->radius = -1.0f;
    result->height = -1.0f;

    auto callback = [&](const char* key, const ConfigValue& value) -> bool
    {
        if (strcmp(key, "radius") == 0)
        {
            if (value.IsFloat())
            {
                result->radius = value.GetFloat();
                if (result->radius <= 0.0f)
                {
                    LOG_ERROR("Radius must be positive");
                    errorOccurred = true;
                    return false;
                }
            }
            else
            {
                LOG_ERROR("Float expected");
                errorOccurred = true;
                return false;
            }
        }
        else if (strcmp(key, "height") == 0)
        {
            if (value.IsFloat())
            {
                result->height = value.GetFloat();
                if (result->height <= 0.0f)
                {
                    LOG_ERROR("Height must be positive");
                    errorOccurred = true;
                    return false;
                }
            }
            else
            {
                LOG_ERROR("Float expected");
                errorOccurred = true;
                return false;
            }
        }

        return true;
    };

    config.Iterate(callback, value.GetObj());
    if (errorOccurred)
    {
        return nullptr;
    }

    if (result->radius < 0.0f && result->height < 0.0f)
    {
        LOG_ERROR("Cylinder radius and height must be specified in the config");
        return nullptr;
    }

    return result;
}

} // namespace


bool CollisionShapeImporter::ImportCFG(const std::string& sourceFilePath, const std::string& targetFilePath)
{
    File configFile;
    if (!configFile.Open(sourceFilePath, AccessMode::Read))
    {
        LOG_ERROR("Failed to open config file: %s", sourceFilePath.c_str());
        return false;
    }

    std::vector<char> str;
    size_t fileSize = static_cast<size_t>(configFile.GetSize());
    str.resize(fileSize + 1);
    if (configFile.Read(str.data(), fileSize) != fileSize)
    {
        LOG_ERROR("Unable to config file.");
        return false;
    }
    str[fileSize] = '\0';

    Config config;
    if (!config.ParseInPlace(str.data()))
    {
        LOG_ERROR("Failed to parse config file: %s", sourceFilePath.c_str());
        return false;
    }

    bool errorOccurred = false;
    config.Iterate([&](const char* key, const ConfigValue& value) -> bool
    {
        if (!value.IsObject())
        {
            LOG_ERROR("Object expected");
            return false;
        }

        if (strcmp(key, "box") == 0)
        {
            auto shape = ParseBox(config, value);
            if (!shape)
            {
                errorOccurred = true;
                return false;
            }

            if (!ParseCommonProperties(config, value, shape.get()))
            {
                errorOccurred = false;
                return false;
            }

            mShapes.push_back(std::move(shape));
        }
        else if (strcmp(key, "cylinder") == 0)
        {
            auto shape = ParseCylinder(config, value);
            if (!shape)
            {
                errorOccurred = true;
                return false;
            }

            if (!ParseCommonProperties(config, value, shape.get()))
            {
                errorOccurred = false;
                return false;
            }

            mShapes.push_back(std::move(shape));
        }
        else
        {
            LOG_ERROR("Invalid shape type: '%s'", key);
            return false;
        }

        return true;
    });

    if (errorOccurred)
        return false;

    Common::FileOutputStream stream(targetFilePath.c_str());
    return Save(stream);
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
