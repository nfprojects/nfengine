/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Mesh converter declarations
*/

#pragma once

#include "../../nfCommon/Math/Math.hpp"

#include <vector>

// NFM file structures

#define MAT_NAME_MAX_LENGTH (120)

namespace NFE {

/**
 * Helper class allowing for mesh file conversion.
 */
class MeshFile
{
    // TODO
    // * support for other files
    // * support for loading meshes from cooked file

    // TODO - file format
    // * Level of Detail
    // * 32-bit or 16-bit indices (to save space)
    // * bones information
    // * animations information
    // * internal materials

public:
    struct Vertex
    {
        Math::Float3 pos;       // position
        Math::Float2 texCoord;  // texture coordinate
        char normal[4];         // normal vector (encoded as normalized 8-bit value)
        char tangent[4];        // tangent vector (encoded as normalized 8-bit value)
    };

    // submes is a set of triangles that use the same material
    struct SubMesh
    {
        int indexOffset;
        int triangleCount;

        // TODO use resource ID instead when it's supported
        char materialName[MAT_NAME_MAX_LENGTH];
    };

    // internal material (TEMPORARY)
    struct Material
    {
        std::string name;
        std::string diffuseTextureFileName;
        std::string normalTextureFileName;
    };

    /**
     * Load mesh form OBJ file.
     */
    bool LoadFromObj(const std::string& path);

    /**
     * Save mesh to binary (cooked) format.
     */
    bool Save(const std::string& path) const;

private:
    std::vector<Vertex> mVertices;
    std::vector<unsigned int> mIndices;
    std::vector<SubMesh> mSubMeshes;
    std::vector<Material> mMaterials;
};

} // namespace NFE