/**
 * @file   PackerDefines.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common definitions for Packer
 */

#include "../stdafx.hpp"
#include "PackerDefines.hpp"

namespace NFE {
namespace Common {

// Current version of PAK File
// Saved at the beginning of every PAK file, allows for file versioning
const uint32 gPackFileVersion = 1;

void FILEPtrDestroy(FILE* f)
{
    fclose(f);
}

NFCOMMON_API std::string Packer_GetErrorStr(PackResult pr)
{
    switch (pr)
    {
        case PackResult::OK:
            return "OK";
        case PackResult::Uninitialized:
            return "Uninitialized";
        case PackResult::FileNotFound:
            return "FileNotFound";
        case PackResult::FileNotCreated:
            return "FileNotCreated";
        case PackResult::NullPointer:
            return "NullPointer";
        case PackResult::FileExists:
            return "FileExists";
        case PackResult::InvalidInputParam:
            return "InvalidInputParam";
        case PackResult::AlreadyInitialized:
            return "AlreadyInitialized";
        case PackResult::WriteFailed:
            return "WriteFailed";
        case PackResult::ReadFailed:
            return "ReadFailed";
        default:
            return "Unknown";
    }
}

} // namespace Common
} // namespace NFE
