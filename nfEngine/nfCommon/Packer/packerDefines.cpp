/**
    NFEngine project

    \file   packerDefines.cpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Common definitions for Packer
*/

#include "stdafx.hpp"
#include "packerDefines.hpp"

namespace NFE {
namespace Common {

// Current version of PAK File
// Saved at the beginning of every PAK file, allows for file versioning
const uint32 gPackFileVersion = 1;

void FILEPtrDestroy(FILE* f)
{
    fclose(f);
}

NFCOMMON_API std::string Packer_GetErrorStr(PACK_RESULT pr)
{
    switch (pr)
    {
        case PACK_RESULT::OK:
            return "OK";
        case PACK_RESULT::UNINITIALIZED:
            return "UNINITIALIZED";
        case PACK_RESULT::FILE_NOT_FOUND:
            return "FILE_NOT_FOUND";
        case PACK_RESULT::FILE_NOT_CREATED:
            return "FILE_NOT_CREATED";
        case PACK_RESULT::NULLPOINTER:
            return "NULLPOINTER";
        case PACK_RESULT::FILE_EXISTS:
            return "FILE_EXISTS";
        case PACK_RESULT::INVALID_INPUT_PARAM:
            return "INVALID_INPUT_PARAM";
        case PACK_RESULT::ALREADY_INITALIZED:
            return "ALREADY_INITIALIZED";
        case PACK_RESULT::WRITE_FAILED:
            return "WRITE_FAILED";
        case PACK_RESULT::READ_FAILED:
            return "READ_FAILED";
        default:
            return "UNKNOWN";
    }
}

} // namespace Common
} // namespace NFE
