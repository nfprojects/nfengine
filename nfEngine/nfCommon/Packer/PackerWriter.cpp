/**
 * @file
 * @author Mkkulagowski (mk.kulagowski(at)gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to write files to PAK archive.
 */

#include "PCH.hpp"
#include "PackerWriter.hpp"

namespace NFE {
namespace Common {

PackerWriter::PackerWriter() {}
PackerWriter::PackerWriter(StringView archiveName)
{
    Init(archiveName);
}

PackerResult PackerWriter::Init(StringView archiveName)
{
    if (archiveName.Empty())
        return PackerResult::InvalidInputParam;

    if (!mFilePath.Empty())
        return PackerResult::AlreadyInitialized;

    mFilePath = archiveName;
    return PackerResult::OK;
}

PackerResult PackerWriter::AddFile(StringView filePath, StringView vfsFilePath)
{
    PackerResourceFile* resource = new PackerResourceFile();

    PackerResult pr = resource->Init(filePath, vfsFilePath);
    if (pr != PackerResult::OK)
    {
        delete resource;
        return pr;
    }

    mFileList.PushBack(SharedPtr<PackerResource>(resource));

    return PackerResult::OK;
}

PackerResult PackerWriter::AddFile(const Buffer& /*buffer*/, StringView /*vfsFilePath*/)
{
    // TODO implement after editing PackerElement (see PackerElement.hpp@15).
    //      To prevent usage of this function, Uninitialized error is returned

    return PackerResult::Uninitialized;
}

PackerResult PackerWriter::AddFilesRecursively(StringView filePath)
{
    PackerResult pr;
    uint64 counter = 0;

    auto recursiveAddLambda = [&](const String& path, PathType type) -> bool
    {
        if (type == PathType::File)
        {
            pr = AddFile(path, path);
            ++counter;
            return pr == PackerResult::OK;
        }
        return true;
    };

    if (!FileSystem::Iterate(filePath, recursiveAddLambda))
        return PackerResult::FileNotFound;

    return pr;
}

PackerResult PackerWriter::WritePAK() const
{
    File file(mFilePath, AccessMode::Write, true);
    if (!(file.IsOpened()))
        return PackerResult::FileNotCreated;

    //save file version
    if (!file.Write(reinterpret_cast<const void*>(&gPackFileVersion), sizeof(uint32)))
        return PackerResult::WriteFailed;

    //save number of files
    uint32 fileSize = mFileList.Size();
    if (!file.Write(reinterpret_cast<void*>(&fileSize), sizeof(uint32)))
        return PackerResult::WriteFailed;

    // Position of first file in archive, calculated as follows
    //   header_size             = version<uint32> + file_table_element_count<size_t>
    //   file_table_element_size = current_pos<size_t> + hash<MD5Hash> + file_size<size_t>
    //   curFilePos              = header_size + file_table_element_size * element_count
    size_t curFilePos = sizeof(uint32) + sizeof(uint32) +
                        (sizeof(uint64) + sizeof(MD5Hash) + sizeof(uint32)) * mFileList.Size();

    PackerResult pr;

    //save file list
    for (const auto& it : mFileList)
    {
        if (!file.Write(reinterpret_cast<const void*>(&curFilePos), sizeof(uint64)))
            return PackerResult::WriteFailed;

        pr = it->SaveHeader(file);
        if (pr != PackerResult::OK)
            return pr;

        curFilePos += it->GetFileSize();
    }

    //copy files to PAK archive
    for (const auto& it : mFileList)
    {
        pr = it->Save(file);
        if (pr != PackerResult::OK)
            return pr;
    }

    return PackerResult::OK;
}

void PackerWriter::PrintFilesToStdout() const
{
    for (const auto& it : mFileList)
        it->PrintToStdout();
}

const ResourceListType& PackerWriter::GetFiles() const
{
    return mFileList;
}

uint32 PackerWriter::GetFileCount() const
{
    return mFileList.Size();
}

const String& PackerWriter::GetPAKName() const
{
    return mFilePath;
}

} // namespace Common
} // namespace NFE
