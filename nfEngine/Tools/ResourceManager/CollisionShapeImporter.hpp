/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Collision shape importer declarations
*/

#pragma once

#include "nfResources/CollisionShapeFile.hpp"


namespace NFE {
namespace Resource {

class CollisionShapeImporter : public CollisionShapeFile
{
public:
    /**
     * Import from configuration file.
     */
    bool ImportCFG(const std::string& sourceFilePath, const std::string& targetFilePath);

    /**
     * Import from OBJ model (triangle mesh only).
     */
    bool ImportOBJ(const std::string& sourceFilePath, const std::string& targetFilePath);
};

} // namespace Resource
} // namespace NFE