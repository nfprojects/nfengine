/**
    NFEngine project

    \file   packerWriter.cpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Module with class used to write files to PAK archive.
*/

#include "stdafx.h"
#include "packerWriter.hpp"

namespace NFE {
namespace Common {

PackWriter::PackWriter() {}
PackWriter::PackWriter(const std::string& ArchiveName)
{
    Init(ArchiveName);
}

PACK_RESULT PackWriter::Init(const std::string& ArchiveName)
{
    if (ArchiveName.empty())
        return PACK_RESULT::INVALID_INPUT_PARAM;

    if (!mFilePath.empty())
        return PACK_RESULT::ALREADY_INITALIZED;

    mFilePath = ArchiveName;
    return PACK_RESULT::OK;
}

PACK_RESULT PackWriter::AddFile(const std::string& FilePath, const std::string& VFSFilePath)
{
    FILEPtr pFile(fopen(FilePath.c_str(), "r"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PACK_RESULT::FILE_NOT_FOUND;

    packElement NewElement;

    NewElement.FilePath = FilePath;

    fseek(pFile.get(), 0, SEEK_END);
    NewElement.FileSize = ftell(pFile.get());
    NewElement.mHash.Calculate(VFSFilePath);

    mFileList.push_back(NewElement);

    return PACK_RESULT::OK;
}

// TODO this function is platform-specific. Implement under Linux when nfCommons will be ported.
PACK_RESULT PackWriter::AddFilesRecursively(const std::string& FilePath)
{
    PACK_RESULT pr;
    HANDLE file;
    std::string WorkDir;

    if (*(FilePath.rbegin()) == '/')
        WorkDir = FilePath + "*";
    else
        WorkDir = FilePath + "/*";

    std::string ResultDir;
    uint64 counter = 0;

    WIN32_FIND_DATAA fd;
    file = FindFirstFileA(WorkDir.c_str(), &fd);

    if (file == INVALID_HANDLE_VALUE)
        return PACK_RESULT::OK;

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

            if (pr != PACK_RESULT::OK)
                return pr;
        }
    }
    while (FindNextFileA(file, &fd) != 0);

    FindClose(file);

    return PACK_RESULT::OK;
}

PACK_RESULT PackWriter::WritePAK() const
{
    FILEPtr pFile(fopen(mFilePath.c_str(), "wb"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PACK_RESULT::FILE_NOT_CREATED;

    //save file version
    if (!fwrite(reinterpret_cast<const void*>(&gPackFileVersion), sizeof(uint32), 1, pFile.get()))
        return PACK_RESULT::WRITE_FAILED;

    //save number of files
    size_t fileSize = mFileList.size();
    if (!fwrite(reinterpret_cast<void*>(&fileSize), sizeof(size_t), 1, pFile.get()))
        return PACK_RESULT::WRITE_FAILED;

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
            return PACK_RESULT::WRITE_FAILED;

        if (!fwrite(reinterpret_cast<const void*>(&ui_CurFilePos), sizeof(size_t), 1, pFile.get()))
            return PACK_RESULT::WRITE_FAILED;

        if (!fwrite(reinterpret_cast<const void*>(&mFileList[i].FileSize), sizeof(size_t), 1, pFile.get()))
            return PACK_RESULT::WRITE_FAILED;

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
            return PACK_RESULT::FILE_NOT_FOUND;

        do
        {
            readCount = fread(buffer, sizeof(unsigned char), PACKER_DEF_BUFFER_SIZE, p_AppendedFile.get());
            fseek(pFile.get(), 0, SEEK_END);
            if (fwrite(buffer, sizeof(unsigned char), readCount, pFile.get()) != readCount)
                return PACK_RESULT::WRITE_FAILED;
        }
        while (readCount == PACKER_DEF_BUFFER_SIZE);
    }

    return PACK_RESULT::OK;
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
