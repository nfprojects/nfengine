/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh importer declarations
 */

#pragma once

#include "nfResources/MeshFile.hpp"
#include "../../nfCommon/Containers/String.hpp"


namespace NFE {
namespace Resource {

class MeshImporter : public MeshFile
{
    struct MaterialDesc
    {
        std::string name;
        std::string diffuseTextureFileName;
        std::string normalTextureFileName;
    };

    std::vector<MaterialDesc> mMaterials;

public:
    bool ImportOBJ(const Common::String& sourceFilePath, const Common::String& targetFilePath);
};

} // namespace Resource
} // namespace NFE