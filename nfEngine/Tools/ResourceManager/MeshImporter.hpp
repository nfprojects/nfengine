/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh importer declarations
 */

#pragma once

#include "nfCommon/Math/Math.hpp"
#include "nfResources/MeshFile.hpp"


namespace NFE {
namespace Resource {

class MeshImporter : public MeshFile
{
public:
    bool ImportOBJ(const std::string& sourceFilePath, const std::string& targetFilePath);
};

} // namespace Resource
} // namespace NFE