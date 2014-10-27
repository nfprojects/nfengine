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
    FILEPtr pFile(fopen(filePath.c_str(), "r"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PackerResult::FileNotFound;

    PackerElement NewElement;

    NewElement.mFilePath = filePath;

    fseek(pFile.get(), 0, SEEK_END);
    NewElement.mFileSize = ftell(pFile.get());
    NewElement.mHash.Calculate(vfsFilePath);

    mFileList.push_back(NewElement);

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
    PackerResult pr;
    HANDLE file;
    std::string WorkDir;

    if (*(filePath.rbegin()) == '/')
        WorkDir = filePath + "*";
    else
        WorkDir = filePath + "/*";

    std::string ResultDir;
    uint64 counter = 0;

    WIN32_FIND_DATAA fd;
    file = FindFirstFileA(WorkDir.c_str(), &fd);

    if (file == INVALID_HANDLE_VALUE)
        return PackerResult::OK;

    do
    {
        if ((strcmp(fd.cFileName, ".") == 0) || (strcmp(fd.cFileName, "..") == 0)) continue;

        ResultDir = WorkDir;
        ResultDir.pop_back();
        ResultDir += fd.cFileName;

        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            AddFilesRecursively(ResultDir);
        else
        {
            pr = AddFile(ResultDir, ResultDir);
            ++counter;

            if (pr != PackerResult::OK)
                return pr;
        }
    }
    while (FindNextFileA(file, &fd) != 0);

    FindClose(file);

    return PackerResult::OK;
}

PackerResult PackerWriter::WritePAK() const
{
    FILEPtr pFile(fopen(mFilePath.c_str(), "wb"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PackerResult::FileNotCreated;

    //save file version
    if (!fwrite(reinterpret_cast<const void*>(&gPackFileVersion), sizeof(uint32), 1, pFile.get()))
        return PackerResult::WriteFailed;

    //save number of files
    size_t fileSize = mFileList.size();
    if (!fwrite(reinterpret_cast<void*>(&fileSize), sizeof(size_t), 1, pFile.get()))
        return PackerResult::WriteFailed;

    // Position of first file in archive, calculated as follows
    //   header_size             = version<uint32> + file_table_element_count<size_t>
    //   file_table_element_size = hash<MD5Hash> + current_pos<size_t> + file_size<size_t>
    //   curFilePos              = header_size + file_table_element_size * element_count
    size_t curFilePos = sizeof(uint32) + sizeof(size_t) +
                        (sizeof(MD5Hash) + sizeof(size_t) + sizeof(size_t)) * mFileList.size();

    //save file list
    for (size_t i = 0; i < mFileList.size(); i++)
    {
        if (!fwrite(reinterpret_cast<const void*>(&mFileList[i].mHash), sizeof(uint32), 4, pFile.get()))
            return PackerResult::WriteFailed;

        if (!fwrite(reinterpret_cast<const void*>(&curFilePos), sizeof(size_t), 1, pFile.get()))
            return PackerResult::WriteFailed;

        if (!fwrite(reinterpret_cast<const void*>(&mFileList[i].mFileSize), sizeof(size_t), 1, pFile.get()))
            return PackerResult::WriteFailed;

        curFilePos += mFileList[i].mFileSize;
    }

    FILEPtr appendedFile(nullptr, FILEPtrDestroy);
    unsigned char buffer[PACKER_DEF_BUFFER_SIZE];
    size_t readCount = 0;

    //copy files to PAK archive
    for (size_t i = 0; i < mFileList.size(); i++)
    {
        appendedFile.reset(fopen(mFileList[i].mFilePath.c_str(), "rb"));
        if (appendedFile == NULL)
            return PackerResult::FileNotFound;

        do
        {
            readCount = fread(buffer, sizeof(unsigned char), PACKER_DEF_BUFFER_SIZE, appendedFile.get());
            if (fwrite(buffer, sizeof(unsigned char), readCount, pFile.get()) != readCount)
                return PackerResult::WriteFailed;
        }
        while (readCount == PACKER_DEF_BUFFER_SIZE);
    }

    return PackerResult::OK;
}

void PackerWriter::PrintFilesToStdout() const
{
    for (const auto& it : mFileList)
        std::cout << "Element " << it.mHash << ", size " << it.mFileSize << std::endl;
}

const FileListType& PackerWriter::GetFiles() const
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
