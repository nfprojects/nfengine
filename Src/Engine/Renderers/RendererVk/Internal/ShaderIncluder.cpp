#include "../PCH.hpp"
#include "ShaderIncluder.hpp"

#include <Engine/Common/FileSystem/File.hpp>
#include <Engine/Common/FileSystem/FileSystem.hpp>


namespace {

const NFE::Common::String INCLUDER_ERROR_FILE_NOT_FOUND("Included file was not found");
const NFE::Common::String INCLUDER_ERROR_OPEN_FAILED("Failed to open included file");
const NFE::Common::String INCLUDER_ERROR_READ_FAILED("Failed to read included file");

} // namespace


namespace NFE {
namespace Renderer {

ShaderIncluder::ShaderIncluder(const Common::StringView& processedFile)
    : mLocalDir(Common::FileSystem::GetParentDir(processedFile))
    , mSystemDir("Data/Shaders")
{
}

ShaderIncluder::IncludeResult* ShaderIncluder::StoreFailedInclusion(IncludeCollection& collection,
                                                                    const Common::String& path,
                                                                    const Common::String& error)
{
    IncludeEntry entry;
    entry.ptr = Common::MakeUniquePtr<IncludeResult>("", error.Str(), error.Length(), nullptr);

    IncludeResult* resultPtr = entry.ptr.Get();
    collection.Insert(path, std::move(entry));
    return resultPtr;
}

ShaderIncluder::IncludeResult* ShaderIncluder::IncludeGeneric(IncludeCollection& collection,
                                                              const Common::String& searchPath,
                                                              const char* headerName,
                                                              const char* includerName,
                                                              size_t inclusionDepth)
{
    NFE_UNUSED(includerName);
    NFE_UNUSED(inclusionDepth);

    Common::String headerStr(headerName);
    Common::String includedFilePath = searchPath + '/' + headerName;

    // TODO if file is found, check if it wasn't modified and needs updates
    // Check currently cached include search results
    IncludeCollection::Iterator it = collection.Find(headerStr);
    if (it != collection.end())
    {
        return it->second.ptr.Get();
    }

    // Include was not found in collection - try getting to it and add to collection what came out of that
    Common::String fullPath = Common::FileSystem::GetAbsolutePath(includedFilePath);
    if (fullPath.Empty() || (Common::FileSystem::GetPathType(includedFilePath) != Common::PathType::File))
    {
        // path is not valid (either doesn't exist or is a dir) - remember as error
        return StoreFailedInclusion(collection, includedFilePath, INCLUDER_ERROR_FILE_NOT_FOUND);
    }

    // file found - try to open it
    Common::File file(fullPath, Common::AccessMode::Read);
    if (!file.IsOpened())
    {
        // failed to open file
        return StoreFailedInclusion(collection, includedFilePath, INCLUDER_ERROR_OPEN_FAILED);
    }

    // read the file
    IncludeEntry entry;

    uint32 includedFileSize = static_cast<uint32>(file.GetSize());
    entry.data.Resize(includedFileSize);
    if (file.Read(entry.data.Data(), includedFileSize) != includedFileSize)
    {
        // failed to read file
        return StoreFailedInclusion(collection, includedFilePath, INCLUDER_ERROR_READ_FAILED);
    }

    NFE_LOG_DEBUG("Read file contents:\n%s", entry.data.Data());

    // save the entry
    entry.ptr = Common::MakeUniquePtr<IncludeResult>(includedFilePath.Str(), entry.data.Data(), entry.data.Size(), nullptr);
    IncludeResult* resultPtr = entry.ptr.Get();
    collection.Insert(includedFilePath, std::move(entry));

    return resultPtr;
}

ShaderIncluder::IncludeResult* ShaderIncluder::includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth)
{
    NFE_LOG_DEBUG("includeLocal called: %s %s %zd", headerName, includerName, inclusionDepth);
    return IncludeGeneric(mLocalSearchResults, mLocalDir, headerName, includerName, inclusionDepth);
}

ShaderIncluder::IncludeResult* ShaderIncluder::includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth)
{
    NFE_LOG_DEBUG("includeSystem called: %s %s %zd", headerName, includerName, inclusionDepth);
    return IncludeGeneric(mSystemSearchResults, mSystemDir, headerName, includerName, inclusionDepth);
}

void ShaderIncluder::releaseInclude(ShaderIncluder::IncludeResult* result)
{
    // Nothing to do - includes are collected with Includer and after it's done and freed, they will free too
    NFE_UNUSED(result);
}

} // namespace Renderer
} // namespace NFE
