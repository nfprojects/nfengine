/**
    NFEngine project

    \file   packerDefines.h
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Common definitions for Packer
*/

#pragma once

#include "../nfCommon.h"
#include <type_traits>
#include <cstdio>
#include <iostream>

#ifdef NFCOMMON_EXPORTS
#include <zlib/zlib.h>
#endif

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

namespace NFE {
namespace Common {

// return codes used by Packer
enum class PACK_RESULT : uint32
{
    OK = 0,
    UNINITIALIZED,
    FILE_NOT_FOUND,
    FILE_NOT_CREATED,
    NULLPOINTER,
    FILE_EXISTS,
    INVALID_INPUT_PARAM,
    ALREADY_INITALIZED,
    WRITE_FAILED,
    READ_FAILED
};

typedef std::underlying_type<PACK_RESULT>::type PACK_RESULT_TYPE;

// unique_ptr wrapper for FILE
typedef std::function<void(FILE*)> FILEPtrDeleter;
typedef std::unique_ptr<FILE, FILEPtrDeleter> FILEPtr;

NFCOMMON_API std::string Packer_GetErrorStr(PACK_RESULT pr);
void FILEPtrDestroy(FILE* f);

// useful macros
#define PACK_CHECK_ERROR(x) if(x) { cout << "Error " << x << " (" << Packer_GetErrorStr(x) << ")\n"; return x; }
#define PACK_RESULT_TO_STRING(x) std::to_string(static_cast<PACK_RESULT_TYPE>(x))

#define PACKER_DEF_BUFFER_SIZE 4096

// information about current file version
extern const uint32 NFCOMMON_API gPackFileVersion;

} // namespace Common
} // namespace NFE
