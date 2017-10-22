/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common definitions for Packer
 */

#include "PCH.hpp"
#include "PackerDefines.hpp"

namespace NFE {
namespace Common {

// Current version of PAK File
// Saved at the beginning of every PAK file, allows for file versioning
const uint32 gPackFileVersion = 1;

NFCOMMON_API const char* Packer_GetErrorStr(PackerResult pr)
{
    switch (pr)
    {
        case PackerResult::OK:
            return "OK";
        case PackerResult::Uninitialized:
            return "Uninitialized";
        case PackerResult::FileNotFound:
            return "FileNotFound";
        case PackerResult::FileNotCreated:
            return "FileNotCreated";
        case PackerResult::NullPointer:
            return "NullPointer";
        case PackerResult::FileExists:
            return "FileExists";
        case PackerResult::InvalidInputParam:
            return "InvalidInputParam";
        case PackerResult::AlreadyInitialized:
            return "AlreadyInitialized";
        case PackerResult::WriteFailed:
            return "WriteFailed";
        case PackerResult::ReadFailed:
            return "ReadFailed";
        default:
            return "Unknown";
    }
}

} // namespace Common
} // namespace NFE
