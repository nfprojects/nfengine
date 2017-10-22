/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common definitions for Packer
 */

#pragma once

#include "../nfCommon.hpp"
#include "../FileSystem/File.hpp"
#include "../FileSystem/FileSystem.hpp"
#include "../Memory/Buffer.hpp"

#include <type_traits>

#ifdef NFCOMMON_EXPORTS
#include <zlib/zlib.h>
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

NFCOMMON_API const char* Packer_GetErrorStr(PackerResult pr);

#define PACKER_DEF_BUFFER_SIZE 4096

// current archive version
extern const uint32 NFCOMMON_API gPackFileVersion;

} // namespace Common
} // namespace NFE
