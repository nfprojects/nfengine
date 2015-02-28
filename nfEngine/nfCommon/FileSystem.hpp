/**
 * @file   FileSystem.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Filesystem utilities declarations.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

enum class PathType
{
    Invalid,   // non-existing path
    File,
    Directory,
};

class NFCOMMON_API FileSystem
{
public:
    typedef std::function<bool(const std::string& path, bool isDirectory)> DirIterateCallback;

    /**
     * Set current working directory.
     */
    static bool ChangeDirectory(const std::string& path);

    /**
     * Create an empty file if not exists.
     */
    static bool TouchFile(const std::string& path);

    /**
     * Check if a path is a file, directory or does not exist.
     */
    static PathType GetPathType(const std::string& path);

    /**
     * Create a directory.
     */
    static bool CreateDir(const std::string& path);

    /**
     * Remove a file or a directory.
     * @param recursive Removes all elements in a directory recursively
     */
    static bool Remove(const std::string& path, bool recursive = false);

    /**
     * Copy a file or a directory.
     * @param srcPath   Source path
     * @param destPath  Destination path
     * @param overwrite Overwrite destination path
     */
    static bool Copy(const std::string& srcPath, const std::string& destPath,
                     bool overwrite = false);

    /**
     * Move a file or a directory to another location.
     * @param srcPath   Source path
     * @param destPath  Destination path
     */
    static bool Move(const std::string& srcPath, const std::string& destPath);

    /**
     * Iterate through files and directories in a specified path.
     * @param callback Callback function called for each found object
     */
    static bool Iterate(const std::string& path, DirIterateCallback callback);
};

} // namespace Common
} // namespace NFE
