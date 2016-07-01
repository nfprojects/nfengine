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
    bool Import(const std::string& sourceFilePath, const std::string& targetFilePath);
};

} // namespace Resource
} // namespace NFE