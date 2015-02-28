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
#if defined(WIN32)
    HANDLE mFile;
#elif defined(__LINUX__) | defined(__linux__)
    // TODO
#else
    #error "Target system not supported!"
#endif

public:
    File();
    File(const std::string& path, AccessMode mode, bool overwrite = false);
    File(File&& other);
    ~File();

    bool IsOpened() const;

    /**
     * Opens a file. If the file does not exist it is created automatically.
     * @param path   File path.
     * @param access File access mode.
     * @param overwrite Overwrite a file if it already exists.
     */
    bool Open(const std::string& path, AccessMode access, bool overwrite = false);
    void Close();

    size_t Read(void* data, size_t size);
    size_t Write(const void* data, size_t size);
    uint64 GetSize() const;
    bool Seek(int64 pos, SeekMode mode);
    uint64 GetPos() const;
};

} // namespace Common
} // namespace NFE
