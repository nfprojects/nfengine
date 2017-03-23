/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Main source file of ResourceManager tool
 */

#include "PCH.hpp"

#include "MeshImporter.hpp"

#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"


using namespace NFE;
using namespace Common;

// TODO temporary
const String DEFAULT_SOURCE_DIR = "nfEngineDemo/Data";
const String DEFAULT_TARGET_DIR = "nfEngineDemo/CookedData";

const String MESHES_DIR = "Meshes";
const String MATERIALS_DIR = "Materials";

bool IterateCallback(const String& sourcePath, bool isDirectory)
{
    if (isDirectory)
        return true;

    const String relPath = sourcePath.substr(DEFAULT_SOURCE_DIR.length() + 1);
    const String subDir = relPath.substr(0, relPath.find_first_of('/'));
    const String extension = FileSystem::ExtractExtension(relPath);

    if (subDir == MESHES_DIR)
        if (extension == "obj")
        {
            LOG_INFO("Found mesh file: %s", relPath.Str());

            String targetPath = DEFAULT_TARGET_DIR + '/' + relPath;
            targetPath = targetPath.substr(0, targetPath.length() - 4) + ".nfm";

            Resource::MeshImporter importer;
            importer.ImportOBJ(sourcePath, targetPath);
        }

    // TODO handle other resource types

    return true;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // change current dir
    String execPath = FileSystem::GetExecutablePath();
    String execDir = FileSystem::GetParentDir(execPath);
    FileSystem::ChangeDirectory(execDir + "/../../..");

    if (FileSystem::GetPathType(DEFAULT_SOURCE_DIR) != PathType::Directory)
    {
        LOG_ERROR("Path '%s' is not a directory!", DEFAULT_SOURCE_DIR.Str());
        return 1;
    }

    // create target directory if does not exist
    if (!FileSystem::CreateDirIfNotExist(DEFAULT_TARGET_DIR))
    {
        LOG_ERROR("Failed to create target directory: %s", DEFAULT_TARGET_DIR.Str());
        return 1;
    }

    FileSystem::CreateDirIfNotExist(DEFAULT_TARGET_DIR + '/' + MESHES_DIR);
    FileSystem::CreateDirIfNotExist(DEFAULT_TARGET_DIR + '/' + MATERIALS_DIR);

    // scan source directory
    if (!FileSystem::Iterate(DEFAULT_SOURCE_DIR, IterateCallback))
    {
        LOG_ERROR("Failed to scan directory '%s'!", DEFAULT_SOURCE_DIR.Str());
        return 1;
    }

    return 0;
}
