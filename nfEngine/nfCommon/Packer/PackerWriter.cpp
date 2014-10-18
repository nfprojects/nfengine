/**
 * @file   PackerWriter.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to write files to PAK archive.
 */

#include "../stdafx.hpp"
#include "PackerWriter.hpp"

namespace NFE {
namespace Common {

PackWriter::PackWriter() {}
PackWriter::PackWriter(const std::string& archiveName)
{
    Init(archiveName);
}

PackResult PackWriter::Init(const std::string& archiveName)
{
    if (archiveName.empty())
        return PackResult::InvalidInputParam;

    if (!mFilePath.empty())
        return PackResult::AlreadyInitialized;

    mFilePath = archiveName;
    return PackResult::OK;
}

PackResult PackWriter::AddFile(const std::string& filePath, const std::string& vfsFilePath)
{
    FILEPtr pFile(fopen(filePath.c_str(), "r"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PackResult::FileNotFound;

    PackerElement NewElement;

    NewElement.FilePath = filePath;

    fseek(pFile.get(), 0, SEEK_END);
    NewElement.FileSize = ftell(pFile.get());
    NewElement.mHash.Calculate(vfsFilePath);

    mFileList.push_back(NewElement);

    return PackResult::OK;
}

// TODO this function is platform-specific. Implement under Linux when nfCommons will be ported.
PackResult PackWriter::AddFilesRecursively(const std::string& filePath)
{
    PackResult pr;
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
        return PackResult::OK;

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

            if (pr != PackResult::OK)
                return pr;
        }
    }
    while (FindNextFileA(file, &fd) != 0);

    FindClose(file);

    return PackResult::OK;
}

PackResult PackWriter::WritePAK() const
{
    FILEPtr pFile(fopen(mFilePath.c_str(), "wb"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PackResult::FileNotCreated;

    //save file version
    if (!fwrite(reinterpret_cast<const void*>(&gPackFileVersion), sizeof(uint32), 1, pFile.get()))
        return PackResult::WriteFailed;

    //save number of files
    size_t fileSize = mFileList.size();
    if (!fwrite(reinterpret_cast<void*>(&fileSize), sizeof(size_t), 1, pFile.get()))
        return PackResult::WriteFailed;

    // Position of first file in archive, calculated as follows
    //   header_size             = version<uint32> + file_table_element_count<size_t>
    //   file_table_element_size = hash<MD5Hash> + current_pos<size_t> + file_size<size_t>
    //   ui_CurFilePos           = header_size + file_table_element_size * element_count
    size_t ui_CurFilePos = sizeof(uint32) + sizeof(size_t) +
                           (sizeof(MD5Hash) + sizeof(size_t) + sizeof(size_t)) * mFileList.size();

    //save file list
    for (size_t i = 0; i < mFileList.size(); i++)
    {
        if (!fwrite(reinterpret_cast<const void*>(&mFileList[i].mHash), sizeof(uint32), 4, pFile.get()))
            return PackResult::WriteFailed;

        if (!fwrite(reinterpret_cast<const void*>(&ui_CurFilePos), sizeof(size_t), 1, pFile.get()))
            return PackResult::WriteFailed;

        if (!fwrite(reinterpret_cast<const void*>(&mFileList[i].FileSize), sizeof(size_t), 1, pFile.get()))
            return PackResult::WriteFailed;

        ui_CurFilePos += mFileList[i].FileSize;
    }

    FILEPtr p_AppendedFile(nullptr, FILEPtrDestroy);
    unsigned char buffer[PACKER_DEF_BUFFER_SIZE];
    size_t readCount = 0;

    //copy files to PAK archive
    for (size_t i = 0; i < mFileList.size(); i++)
    {
        p_AppendedFile.reset(fopen(mFileList[i].FilePath.c_str(), "rb"));
        if (p_AppendedFile == NULL)
            return PackResult::FileNotFound;

        do
        {
            readCount = fread(buffer, sizeof(unsigned char), PACKER_DEF_BUFFER_SIZE, p_AppendedFile.get());
            if (fwrite(buffer, sizeof(unsigned char), readCount, pFile.get()) != readCount)
                return PackResult::WriteFailed;
        }
        while (readCount == PACKER_DEF_BUFFER_SIZE);
    }

    return PackResult::OK;
}

void PackWriter::PrintFilesToStdout() const
{
    for (const auto& it : mFileList)
        std::cout << "Element " << it.mHash << ", size " << it.FileSize << std::endl;
}

const FileListType& PackWriter::GetFiles() const
{
    return mFileList;
}

size_t PackWriter::GetFileCount() const
{
    return mFileList.size();
}

const std::string& PackWriter::GetPAKName() const
{
    return mFilePath;
}

} // namespace Common
} // namespace NFE
