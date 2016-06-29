/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh resource structures declarations
 */

#pragma once

#include "nfCommon/Math/Math.hpp"

#include <vector>

#define MAT_NAME_MAX_LENGTH (120)

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
    uint32 magic;
    uint32 verticesCount;
    uint32 indicesCount;
    uint32 subMeshesCount;
};

// submesh is a set of triangles that use the same material
struct MeshSubMesh
{
    int indexOffset;
    int triangleCount;

    // TODO use resource ID instead when it's supported
    char materialName[MAT_NAME_MAX_LENGTH];
};


} // namespace Resource
} // namespace NFE