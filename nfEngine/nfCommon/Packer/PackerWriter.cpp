/**
 * @file   PackerWriter.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to write files to PAK archive.
 */

#include "../stdafx.hpp"
#include "PackerWriter.hpp"

namespace NFE {
namespace Common {

PackerWriter::PackerWriter() {}
PackerWriter::PackerWriter(const std::string& archiveName)
{
    Init(archiveName);
}

PackerResult PackerWriter::Init(const std::string& archiveName)
{
    if (archiveName.empty())
        return PackerResult::InvalidInputParam;

    if (!mFilePath.empty())
        return PackerResult::AlreadyInitialized;

    mFilePath = archiveName;
    return PackerResult::OK;
}

PackerResult PackerWriter::AddFile(const std::string& filePath, const std::string& vfsFilePath)
{
    PackerResourceFile* resource = new PackerResourceFile();

    PackerResult pr = resource->Init(filePath, vfsFilePath);
    if (pr != PackerResult::OK)
    {
        delete resource;
        return pr;
    }

    mFileList.push_back(std::move(std::shared_ptr<PackerResource>(resource)));

    return PackerResult::OK;
}

PackerResult PackerWriter::AddFile(const Buffer& /*buffer*/, const std::string& /*vfsFilePath*/)
{
    // TODO implement after editing PackerElement (see PackerElement.hpp@15).
    //      To prevent usage of this function, Uninitialized error is returned

    return PackerResult::Uninitialized;
}

// TODO this function is platform-specific. Implement under Linux when nfCommons will be ported.
PackerResult PackerWriter::AddFilesRecursively(const std::string& filePath)
{
    FileSystem fileSystem;
    PackerResult pr;
    uint64 counter = 0;

    auto recursiveAddLambda = [&](const std::string& path, bool isDirectory) -> bool
    {
        if (!isDirectory)
        {
            pr = AddFile(path, path);
            ++counter;

            if (pr != PackerResult::OK)
                return false;
            return true;
        }
    };

    if(!fileSystem.Iterate(filePath, recursiveAddLambda))
        return pr;

    return PackerResult::OK;
}

PackerResult PackerWriter::WritePAK() const
{
    // it was "wb" here. Also...I added overwrite
    FilePtr pFile(new File(mFilePath, AccessMode::Write, true));
    if (!(pFile.get()->IsOpened()))
        return PackerResult::FileNotCreated;

    //save file version
    if (!pFile.get()->Write(reinterpret_cast<const void*>(&gPackFileVersion), sizeof(uint32)))
        return PackerResult::WriteFailed;

    //save number of files
    size_t fileSize = mFileList.size();
    if (!pFile.get()->Write(reinterpret_cast<void*>(&fileSize), sizeof(size_t)))
        return PackerResult::WriteFailed;

    // Position of first file in archive, calculated as follows
    //   header_size             = version<uint32> + file_table_element_count<size_t>
    //   file_table_element_size = current_pos<size_t> + hash<MD5Hash> + file_size<size_t>
    //   curFilePos              = header_size + file_table_element_size * element_count
    size_t curFilePos = sizeof(uint32) + sizeof(size_t) +
                        (sizeof(size_t) + sizeof(MD5Hash) + sizeof(size_t)) * mFileList.size();

    PackerResult pr;

    //save file list
    for (const auto& it : mFileList)
    {
        if (!pFile.get()->Write(reinterpret_cast<const void*>(&curFilePos), sizeof(size_t)))
            return PackerResult::WriteFailed;

        pr = it->SaveHeader(pFile.get());
        if (pr != PackerResult::OK)
            return pr;

        curFilePos += it->GetFileSize();
    }

    //copy files to PAK archive
    for (const auto& it : mFileList)
    {
        pr = it->Save(pFile.get());
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

size_t PackerWriter::GetFileCount() const
{
    return mFileList.size();
}

const std::string& PackerWriter::GetPAKName() const
{
    return mFilePath;
}

} // namespace Common
} // namespace NFE
