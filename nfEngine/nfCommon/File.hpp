/**
 * @file   File.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  File class declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

enum class SeekMode
{
    Begin,
    Current,
    End,
};

/**
 * File access mode.
 */
enum class AccessMode
{
    None,
    Read,
    Write,
    ReadWrite,
};

/**
 * Class allowing access to a binary file.
 */
class NFCOMMON_API File
{
private:
    HANDLE mFile;

public:
    File();
    File(const std::string& path, AccessMode mode);
    File(File&& other);
    ~File();

    bool Open(const std::string& path, AccessMode access,
              AccessMode share = AccessMode::Read);
    void Close();
    size_t Read(void* data, size_t size);
    size_t Write(const void* data, size_t size);
    uint64 GetSize() const;
    bool Seek(int64 pos, SeekMode mode);
    uint64 GetPos() const;
};

} // namespace Common
} // namespace NFE
