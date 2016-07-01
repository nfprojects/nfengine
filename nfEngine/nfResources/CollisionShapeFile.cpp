/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape converter implementation
 */

#include "PCH.hpp"
#include "CollisionShapeFile.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Resource {

using namespace Math;
using namespace Common;

namespace {

#pragma pack(push, 1)
struct PackedShapeHeader
{
    CollisionShapeType shapeType;
    uint16 defaultMaterialID;
    Math::Float4 translation;
    Math::Float4 orientation;
};
#pragma pack(pop)

// replace backslashes with forward slashes
NFE_INLINE std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}

} // namespace


///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<CollisionShapeFile::Shape> CollisionShapeFile::Shape::Load(InputStream& stream)
{
    PackedShapeHeader header;
    if (stream.Read(&header, sizeof(header)) != sizeof(header))
    {
        LOG_ERROR("Failed to read collision shape header");
        return false;
    }

    std::unique_ptr<CollisionShapeFile::Shape> shape;
    switch (header.shapeType)
    {
    case CollisionShapeType::TriangleMesh:
        shape.reset(new TriangleMeshShape);
        break;
    case CollisionShapeType::Box:
        shape.reset(new BoxShape);
        break;
    case CollisionShapeType::Cylinder:
        shape.reset(new CylinderShape);
        break;
    default:
        LOG_ERROR("Invalid shape type: %u", static_cast<uint32>(header.shapeType));
    }

    // unpack header
    shape->defaultMaterialID = header.defaultMaterialID;
    shape->translation = header.translation;
    shape->orientation = header.orientation;

    if (!shape->OnLoad(stream))
        return nullptr;

    return shape;
}

bool CollisionShapeFile::Shape::Save(OutputStream& stream) const
{
    PackedShapeHeader header;
    header.shapeType = shapeType;
    header.defaultMaterialID = defaultMaterialID;
    header.translation = translation;
    header.orientation = orientation;

    if (stream.Write(&header, sizeof(header)) != sizeof(header))
    {
        LOG_ERROR("Failed to write collision shape header");
        return false;
    }

    return OnSave(stream);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CollisionShapeFile::TriangleMeshShape::OnLoad(InputStream& stream)
{
    Header triMeshHeader;
    if (stream.Read(&triMeshHeader, sizeof(Header)) != sizeof(Header))
    {
        LOG_ERROR("Failed to read header");
        return false;
    }

    vertices.resize(triMeshHeader.verticesNum);
    indices.resize(triMeshHeader.trianglesNum * 3);

    const size_t verticesDataSize = vertices.size() * sizeof(Float3);
    if (stream.Read(vertices.data(), verticesDataSize) != verticesDataSize)
    {
        LOG_ERROR("Failed to read vertices data");
        return false;
    }

    const size_t indicesDataSize = indices.size() * sizeof(uint32);
    if (stream.Read(indices.data(), indicesDataSize) != indicesDataSize)
    {
        LOG_ERROR("Failed to read indices data");
        return false;
    }

    overrideMaterials = triMeshHeader.overrideMaterials;
    if (overrideMaterials)
    {
        materialIDs.resize(triMeshHeader.trianglesNum);

        const size_t materialDataSize = triMeshHeader.trianglesNum * sizeof(uint16);
        if (stream.Read(materialIDs.data(), materialDataSize) != materialDataSize)
        {
            LOG_ERROR("Failed to read material data");
            return false;
        }
    }

    return true;
}

bool CollisionShapeFile::TriangleMeshShape::OnSave(OutputStream& stream) const
{
    NFE_ASSERT(indices.size() % 3 == 0, "Indicies array is corrupted - size must be multiple of 3");

    Header triMeshHeader;
    triMeshHeader.verticesNum = static_cast<uint32>(vertices.size());
    triMeshHeader.trianglesNum = static_cast<uint32>(indices.size()) / 3;
    triMeshHeader.overrideMaterials = overrideMaterials;
    if (stream.Write(&triMeshHeader, sizeof(Header)) != sizeof(Header))
    {
        LOG_ERROR("Failed to write header");
        return false;
    }

    const size_t verticesDataSize = vertices.size() * sizeof(Float3);
    if (stream.Write(vertices.data(), verticesDataSize) != verticesDataSize)
    {
        LOG_ERROR("Failed to write vertices data");
        return false;
    }

    const size_t indicesDataSize = indices.size() * sizeof(uint32);
    if (stream.Write(indices.data(), indicesDataSize) != indicesDataSize)
    {
        LOG_ERROR("Failed to write indices data");
        return false;
    }

    if (overrideMaterials)
    {
        NFE_ASSERT(materialIDs.size() == triMeshHeader.trianglesNum, "Materials array is corrupted");

        const size_t materialDataSize = materialIDs.size() * sizeof(uint16);
        if (stream.Write(materialIDs.data(), materialDataSize) != materialDataSize)
        {
            LOG_ERROR("Failed to write material data");
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CollisionShapeFile::BoxShape::OnLoad(InputStream& stream)
{
    Math::Float3 packedHalfSize;
    if (stream.Read(&packedHalfSize, sizeof(Float3)) != sizeof(Float3))
    {
        LOG_ERROR("Failed to load shape data");
        return false;
    }

    if (packedHalfSize.x <= 0.0f || packedHalfSize.y <= 0.0f || packedHalfSize.z <= 0.0f)
    {
        LOG_ERROR("File corrupted");
        return false;
    }

    halfSize = packedHalfSize;
    return true;
}

bool CollisionShapeFile::BoxShape::OnSave(OutputStream& stream) const
{
    Math::Float3 packedHalfSize(halfSize);
    NFE_ASSERT(packedHalfSize.x > 0.0f && packedHalfSize.y > 0.0f && packedHalfSize.z > 0.0f,
               "Box half sizes must be greater than zero");

    if (stream.Write(&packedHalfSize, sizeof(Float3)) != sizeof(Float3))
    {
        LOG_ERROR("Failed to write shape data");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CollisionShapeFile::CylinderShape::OnLoad(InputStream& stream)
{
    if (stream.Read(&height, 2 * sizeof(float)) != 2 * sizeof(float))
    {
        LOG_ERROR("Failed to load shape data");
        return false;
    }

    if (height <= 0.0f || radius <= 0.0f)
    {
        LOG_ERROR("File corrupted");
        return false;
    }

    return true;
}

bool CollisionShapeFile::CylinderShape::OnSave(OutputStream& stream) const
{
    NFE_ASSERT(radius > 0.0f && height > 0.0f, "Radius and height must be greater than zero");

    if (stream.Write(&height, 2 * sizeof(float)) != 2 * sizeof(float))
    {
        LOG_ERROR("Failed to save shape data");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CollisionShapeFile::Load(InputStream& stream)
{
    FileHeader header;
    if (stream.Read(&header, sizeof(FileHeader)) != sizeof(FileHeader))
    {
        LOG_ERROR("Failed to read file header");
        return false;
    }

    if (header.magic != NFE_COLLISION_SHAPE_FILE_MAGIC)
    {
        LOG_ERROR("Invalid file magic");
        return false;
    }

    // read and create shapes
    for (uint32 i = 0; i < header.numShapes; ++i)
    {
        std::unique_ptr<Shape> shape(Shape::Load(stream));
        if (!shape)
        {
            mShapes.clear();
            return false;
        }

        mShapes.push_back(std::move(shape));
    }

    // TODO materials

    return true;
}

bool CollisionShapeFile::Save(OutputStream& stream) const
{
    FileHeader header;
    header.magic = NFE_COLLISION_SHAPE_FILE_MAGIC;
    header.numShapes = static_cast<uint32>(mShapes.size());
    header.numMaterials = 0;

    if (stream.Write(&header, sizeof(FileHeader)) != sizeof(FileHeader))
    {
        LOG_ERROR("Failed to write file header");
        return false;
    }

    for (const auto& shape : mShapes)
        if (!shape->Save(stream))
            return false;

    // TODO materials

    return true;
}

} // namespace Resource
} // namespace NFE
