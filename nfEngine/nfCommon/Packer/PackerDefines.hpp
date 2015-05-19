/**
 * @file   PackerDefines.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common definitions for Packer
 */

#pragma once

#include "../nfCommon.hpp"
#include "../File.hpp"
#include "../FileSystem.hpp"
#include "../Buffer.hpp"
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
enum class PackerResult : uint32
{
    OK = 0,
    Uninitialized,
    FileNotFound,
    FileNotCreated,
    NullPointer,
    FileExists,
    InvalidInputParam,
    AlreadyInitialized,
    WriteFailed,
    ReadFailed
};

typedef std::underlying_type<PackerResult>::type PackerResultType;

// unique_ptr wrapper for File
typedef std::unique_ptr<File> FilePtr;

NFCOMMON_API std::string Packer_GetErrorStr(PackerResult pr);

#define PACK_RESULT_TO_STRING(x) std::to_string(static_cast<PackerResultType>(x))
#define PACKER_DEF_BUFFER_SIZE 4096

// current archive version
extern const uint32 NFCOMMON_API gPackFileVersion;

} // namespace Common
} // namespace NFE
