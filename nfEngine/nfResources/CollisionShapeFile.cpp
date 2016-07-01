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

bool CollisionShapeFile::Shape::Load(InputStream& stream)
{
    PackedShapeHeader header;
    if (stream.Read(&header, sizeof(header)) != sizeof(header))
    {
        LOG_ERROR("Failed to read collision shape header");
        return false;
    }

    // unpack header
    shapeType = header.shapeType;
    defaultMaterialID = header.defaultMaterialID;
    translation = header.translation;
    orientation = header.orientation;

    // TODO validation

    return OnLoad(stream);
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

    // TODO
    
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
        LOG_ERROR("Failed to write shape data");
        return false;
    }

    halfSize = packedHalfSize;
    return true;
}

bool CollisionShapeFile::BoxShape::OnSave(OutputStream& stream) const
{
    Math::Float3 packedHalfSize(halfSize);

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
    UNUSED(stream);

    // TODO
}

bool CollisionShapeFile::CylinderShape::OnSave(OutputStream& stream) const
{
    UNUSED(stream);

    // TODO
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CollisionShapeFile::Load(InputStream& stream)
{
    UNUSED(stream);

    // TODO
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
