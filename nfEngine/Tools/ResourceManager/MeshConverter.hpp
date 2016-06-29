/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh converter declarations
 */

#pragma once

#include "nfCommon/Math/Math.hpp"
#include "Common/Resource/Mesh.hpp"

#include <vector>


// NFM file structures

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
    // internal material (TEMPORARY)
    struct MaterialDesc
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
    std::vector<Resource::MeshVertex> mVertices;
    std::vector<uint32> mIndices;
    std::vector<Resource::MeshSubMesh> mSubMeshes;
    std::vector<MaterialDesc> mMaterials;
};

} // namespace NFE