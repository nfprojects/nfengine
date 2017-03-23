/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh importer declarations
 */

#pragma once

#include "nfResources/MeshFile.hpp"


namespace NFE {
namespace Resource {

class MeshImporter : public MeshFile
{
    struct MaterialDesc
    {
        String name;
        String diffuseTextureFileName;
        String normalTextureFileName;
    };

    std::vector<MaterialDesc> mMaterials;

public:
    bool ImportOBJ(const String& sourceFilePath, const String& targetFilePath);
};

} // namespace Resource
} // namespace NFE