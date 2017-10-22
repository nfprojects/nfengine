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

bool IterateCallback(const String& sourcePath, PathType pathType)
{
    if (pathType != PathType::File)
        return true;

    const String relPath = sourcePath.ToView().Range(DEFAULT_SOURCE_DIR.Length() + 1, sourcePath.Length() - DEFAULT_SOURCE_DIR.Length() - 1);
    const StringView subDir = relPath.ToView().Range(0, relPath.ToView().FindFirst('/'));
    const StringView extension = FileSystem::GetExtension(relPath);

    if (subDir == MESHES_DIR)
    {
        if (extension == "obj")
        {
            NFE_LOG_INFO("Found mesh file: %s", relPath.Str());

            String targetPath = DEFAULT_TARGET_DIR + '/' + relPath;
            targetPath = targetPath.ToView().Range(0, targetPath.Length() - 4);
            targetPath += ".nfm";

            Resource::MeshImporter importer;
            importer.ImportOBJ(sourcePath, targetPath);
        }
    }

    // TODO handle other resource types

    return true;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // change current dir
    const String execPath = FileSystem::GetExecutablePath();
    const String execDir = FileSystem::GetParentDir(execPath);
    FileSystem::ChangeDirectory(execDir + "/../../..");

    if (FileSystem::GetPathType(DEFAULT_SOURCE_DIR) != PathType::Directory)
    {
        NFE_LOG_ERROR("Path '%s' is not a directory!", DEFAULT_SOURCE_DIR.Str());
        return 1;
    }

    // create target directory if does not exist
    if (!FileSystem::CreateDirIfNotExist(DEFAULT_TARGET_DIR))
    {
        NFE_LOG_ERROR("Failed to create target directory: %s", DEFAULT_TARGET_DIR.Str());
        return 1;
    }

    FileSystem::CreateDirIfNotExist(DEFAULT_TARGET_DIR + '/' + MESHES_DIR);
    FileSystem::CreateDirIfNotExist(DEFAULT_TARGET_DIR + '/' + MATERIALS_DIR);

    // scan source directory
    if (!FileSystem::Iterate(DEFAULT_SOURCE_DIR, IterateCallback))
    {
        NFE_LOG_ERROR("Failed to scan directory '%s'!", DEFAULT_SOURCE_DIR.Str());
        return 1;
    }

    NFE::Common::ShutdownSubsystems();

    return 0;
}
