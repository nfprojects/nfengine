/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh resource file declarations
 */

#pragma once

#include "nfCommon/Math/Math.hpp"

#include <vector>


#define NFE_MESH_FILE_MAGIC         '\0mfn'     // "nfm\0"
#define NFE_MAT_NAME_MAX_LENGTH     120

namespace NFE {
namespace Resource {

struct MeshVertex
{
    Math::Float3 position;
    Math::Float2 texCoord;  // texture coordinate
    int8 normal[4];         // normal vector (encoded as normalized 8-bit value)
    int8 tangent[4];        // tangent vector (encoded as normalized 8-bit value)
};

struct MeshHeader
{
    uint32 magic;           // magic sequence "nfm\0"
    uint32 verticesCount;
    uint32 indicesCount;
    uint32 subMeshesCount;
};

// submesh is a set of triangles that use the same material
struct MeshSubMesh
{
    uint32 indexOffset;
    uint32 triangleCount;

    // TODO use resource ID instead when it's supported
    char materialName[NFE_MAT_NAME_MAX_LENGTH];
};

/**
 * Helper class for mesh resource binary file handling.
 */
class MeshFile
{
    // TODO - file format
    // * Level of Detail
    // * use of 16-bit indices when possible to save space
    // * bones information
    // * animations information
    // * internal materials

public:
    virtual ~MeshFile() {}

    /**
     * Load mesh form OBJ file.
     */
    bool LoadFromObj(const std::string& path);

    /**
     * Load mesh from a binary format.
     */
    // TODO use InputStream
    bool Load(const std::string& path);

    /**
     * Save mesh to binary (cooked) format.
     */
    // TODO use OutputStream
    bool Save(const std::string& path) const;

    NFE_INLINE const std::vector<Resource::MeshVertex>& GetVertices() const
    {
        return mVertices;
    }

    NFE_INLINE const std::vector<uint32>& GetIndices() const
    {
        return mIndices;
    }

    NFE_INLINE const std::vector<Resource::MeshSubMesh>& GetSubMeshes() const
    {
        return mSubMeshes;
    }

protected:
    std::vector<Resource::MeshVertex> mVertices;
    std::vector<uint32> mIndices;
    std::vector<Resource::MeshSubMesh> mSubMeshes;
};

} // namespace Resource
} // namespace NFE